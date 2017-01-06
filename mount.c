#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include "ns.h"

int mount_rootfs(const char *rootfs) {
	const char *source ,*target;

	source = target = rootfs;
	if (mount(source, target, "none", MS_BIND, NULL))
		err_print("mount rootfs");

	return 0;
	/* Parent mount namespace info may leak if just bind mount
	for container rootfs, the device info that mounted on the 
	parent mount namespace's / directory could be available in
	container, it will be fixed later*/

	/*if (mount("none", target, "none", MS_REMOUNT, NULL))
		err_print("remount rootfs");
	if (mount(target, "/", NULL, MS_BIND | MS_MOVE, NULL)) 
		err_print("mount rootfs");*/
}


int mount_procfs(const char *rootfs) {
	char *target = NULL;
	target = malloc(strlen(rootfs) + strlen(PROCFS) + 1);
	sprintf(target, "%s/%s", rootfs, PROCFS);
	if (mount("proc", target, "proc", 0, NULL))
		err_print("mount procfs");
	
	return 0;
}

int chrootfs(const char *rootfs) {
	// change into new root fs
	if (chdir(rootfs))
		err_print("chdir rootfs");

	if (chroot("."))
		err_print("chroot");

	if (chdir("/"))
		err_print("chdir /");

	/* It is clear that put_old must be an absolute path with prefex '/', 
	and there is a wired bug about invalid argument when calls pivot_root(), 
	it will be fixed later*/

	/*if (mkdir(put_old, 0755)) 
		perror("mkdir pivot_old");
		
	if ((pivot_root(".", put_old)))
		err_print("pivot_root");

	if ((chdir("./")) == -1) {
		err_print("chroot");
	}*/
	return 0;
}

