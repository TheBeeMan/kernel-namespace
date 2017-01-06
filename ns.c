/*************************************************************************
	> File Name: crt_container.c
	> Author: thebeeman 
	> Mail: thebeemangg@gmail.com
	> Created Time: 2016年12月27日 星期二 18时21分44秒
 ************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/capability.h>
#include "ns.h"

static int fds[2];
extern int errno; 
static char child_stack[STACK_SIZE];

static void usage(char *pname)
{
	fprintf(stderr, "Usage: %s [options] rootfs [arg...]\n", pname);
	fprintf(stderr, "Options can be:\n");
	fprintf(stderr, "	 -d	  rootfs directory\n");
	fprintf(stderr, "	 -u	  'uid-in-ns uid-out-ns range'\n");
	fprintf(stderr, "	 -g	  'gid-in-ns gid-out-ns range'\n");
	exit(EXIT_FAILURE);
}

void set_ns_mapping(int mod, char *ids_mapping, int pid) {
	FILE *fp;
	char fname[1024] = {0};

	if (mod == UID) {
		sprintf(fname, "/proc/%d/uid_map", pid);
	} else if (mod == GID) {
		sprintf(fname, "/proc/%d/gid_map", pid);
	} else {
		printf("err mod\n");
		exit(EXIT_FAILURE);
	}
	/* Note user "w" option when calls fopen, I have no idea for it.*/
	fp = fopen(fname, "w");
	if (fp == NULL) 
		err_exit("fopen");

	if (fprintf(fp, "%s", ids_mapping) < 0)
		err_exit("writing uid/gid mapping");

	fclose(fp);
}

int child_func(void *arg) {
	char buf[MINSIZE];	  
	const char *rootfs;
	char *const argv[] = {SHELL, "-l", NULL};

	close(fds[1]);
	read(fds[0], buf, strlen(buf));
	
	/* check rootfs folder and mount system image to it.*/
	rootfs = arg;
	if (mount_rootfs(rootfs) || mount_procfs(rootfs))  
		exit(EXIT_FAILURE);
	
	if (chrootfs(rootfs))
		exit(EXIT_FAILURE);
	
	execv(argv[0], argv);
	err_exit("exec");
}

int modify_setgroups(int pid) {
	FILE *fgroups;
	char *text = "deny";
	char fname[MINSIZE] = {0};
	
	sprintf(fname, "/proc/%d/setgroups", pid);
	/* Note use "w+" instead of "w" when calls fopen(), or file content 
	   could not be modified, I have no idea for it.*/
	fgroups = fopen(fname, "w+");

	if (fgroups == NULL) {
		err_exit("fopen");
	} else if (fprintf(fgroups, "%s", text) != strlen(text)) {
		err_exit("fprintf");
	}
	fclose(fgroups);

	return 0;
}

int main(int argc, char *argv[]) {
	pid_t ws;
	int pid, status, opt;
	char *rootfs, *uid_map, *gid_map;
   
	rootfs = NULL;
	uid_map = NULL;
	gid_map = NULL;

	while ((opt = getopt(argc, argv, "+d:u:g:")) != -1) {
		switch (opt) {
		case 'd': rootfs = optarg;				break;
		case 'u': uid_map = optarg;				break;
		case 'g': gid_map = optarg;				break;
		default:  usage(argv[0]);
		}
	}
   
	if (!rootfs) {
		fprintf(stderr, "%s", "[+] rootfs directory is required!\n\n");
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	
	if (!(uid_map && gid_map)) {
		fprintf(stderr, "%s", "[+] uid/gid mapping is insufficient!\n\n");
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (pipe(fds) == -1) 
		err_exit("pipe");

	pid = clone(child_func, child_stack + STACK_SIZE, CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUSER | SIGCHLD, rootfs);
   
	if (pid == -1) {
		err_exit("clone");
	} else {
	/* since kernel 3.18, user namespace is not available for unprivileged user unless 
	   "deny" written to file named setgroups*/
		if (!modify_setgroups(pid)) {
			set_ns_mapping(UID, uid_map, pid);
			set_ns_mapping(GID, gid_map, pid);
		}
		
		close(fds[0]);
		close(fds[1]);

		ws = waitpid(pid, &status, 0);
		if (ws == -1) 
		{
			err_exit("waitpid");
		}
	}
	exit(EXIT_SUCCESS);
}

