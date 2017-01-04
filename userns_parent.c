/*************************************************************************
	> File Name: crt_container.c
	> Author: thebeeman 
	> Mail: thebeemangg@gmail.com
	> Created Time: 2016年12月27日 星期二 18时21分44秒
 ************************************************************************/
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sched.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/capability.h>

#define UID 0
#define GID 1
#define STACK_SIZE 0x1000 * 0x10
#define err_exit(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int fds[2];

extern int errno; 

static child_stack[STACK_SIZE];
static int ns_in_ids[] = {0};
static int ns_out_ids[] = {1000};

char *string(const char *format, ...) {
  char *result;
  va_list args;

  va_start(args, format);
  if (vasprintf(&result, format, args) < 0)
    error(1, errno, "asprintf");
  va_end(args);
  return result;
}

void set_ns_mapping(int mod, int *ns_in_ids, int *ns_out_ids, int length, int range, int pid) {
    FILE *fp;
    size_t n;
    char *pn;
    char fname[1024] = {0};
    char ns_map[1024] = {0};

    if (mod == UID) {
    	sprintf(fname, "/proc/%d/uid_map", pid);
    } else if (mod == GID) {
    	sprintf(fname, "/proc/%d/gid_map", pid);
    } else {
	printf("err mod\n");
	exit(EXIT_FAILURE);
    }

    fp = fopen(fname, "w");
    if (fp == NULL) {
        err_exit("fopen");
    }

    pn = ns_map;
    for (int i = 0; i < length; i++) {
        //n = sprintf(pn, "%d %d %d\n", ns_in_ids[i], ns_out_ids[i], range);
        n = sprintf(pn, "%d %d %d", ns_in_ids[i], ns_out_ids[i], range);
        if (n < 0) {
    	    err_exit("fwrite");
	}
	pn += n;
    }
    printf("writing ns_map to file: %s\n", ns_map);
    n = fprintf(fp, "%s", ns_map);
    if (n <= 0) {
	fprintf(stderr, "fprintf err: %s", strerror(errno));
    }
    fclose(fp);
}

int child_func(void *arg) {
    cap_t caps;
    const char* path;
    char *const argv[] = {"/bin/bash", NULL};
    char rbuf[128];    

    caps = cap_get_proc();
    printf("I am child, my uid is [%d], gid is [%d]\n", getuid(), getgid());
    printf("I get args [%s] from my pid\n", argv);
    printf("capabilities: %s\n", cap_to_text(caps, NULL));

    close(fds[1]);
    read(fds[0], rbuf, 128);
    printf("in child: pid have seted up mapping\n");

    path = arg;
    if (execv(path, argv) == -1) 
        err_exit("exec");
}

unsigned int check_ns_ids() {
    int ilen, olen, ret;
    ilen = sizeof(ns_in_ids)/sizeof(ns_in_ids[0]);
    olen = sizeof(ns_out_ids)/sizeof(ns_out_ids[0]);
    ret = ilen == olen ? ilen : 0;
   
    return ret;
}

void modify_setgroups(int pid) {
    pid_t ws;
    int child, status;
    FILE *fgroups;
    char *text = "deny";
    char fname[32] = {0};
    
    /*pid_t ws;
    char *path, *text = "deny";
    int fd, child, status;*/

    sprintf(fname, "/proc/%d/setgroups", pid);
    fgroups = fopen(fname, "w+");

    if (fgroups == NULL) {
	err_exit("fopen");
    } else if (fprintf(fgroups, "%s", text) != (size_t)(strlen(text))) {
	err_exit("fprintf");
    }

    fclose(fgroups);

/*    path = string("/proc/%d/setgroups", pid);
    if ((fd = open(path, O_WRONLY)) < 0)
        error(1, 0, "Failed to disable setgroups() in container");
    else if (write(fd, text, strlen(text)) != (ssize_t) strlen(text))
        error(1, 0, "Failed to disable setgroups() in container");
    close(fd);
    free(path);*/
}

int main(int argc, char *argv[]) {
    int pid, status, len;

    pid_t ws;
    cap_t caps;
    
    if (argc < 2) 
    {
        fprintf(stderr, "%s", "[+] too few params provided\n"
                "[+] usage: ./crt_container args...\n");
        exit(EXIT_FAILURE);
    }

    if (pipe(fds) == -1) {
	err_exit("pipe");
    }

    pid = clone(child_func, child_stack + STACK_SIZE, CLONE_NEWUSER | SIGCHLD, argv[1]);
   
    if (pid == -1) {
    	err_exit("clone");
    } else {
        caps = cap_get_proc();
        printf("I am parant, my uid is [%d], gid is [%d]\n", getuid(), getgid());
        printf("I wait my son [%d] until state change\n", pid);
        printf("capabilities: %s\n", cap_to_text(caps, NULL));

	modify_setgroups(pid);
	
        len = check_ns_ids();
	if (len) {
	    set_ns_mapping(UID, ns_in_ids, ns_out_ids, len, 1, pid);
	    set_ns_mapping(GID, ns_in_ids, ns_out_ids, len, 1, pid);
	} else {
	    printf("check_ns_ids err\n");
	}	
	printf("in pid: pid have seted up mapping\n");
	close(fds[0]);
	close(fds[1]);

        ws == waitpid(pid, &status, 0);
	if (ws == -1) 
        {
	    err_exit("waitpid");
	}
    }
    
    exit(EXIT_SUCCESS);
}

