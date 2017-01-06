#ifndef _NS_H_
#define _NS_H_

#define UID 0
#define GID 1
#define MINSIZE 32
#define STACK_SIZE 0x400 * 0x400

#define PROCFS "proc"
#define SHELL "/bin/bash"

#define err_print(msg) do { perror(msg); } while (0)
#define err_exit(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

extern int mount_rootfs(const char *);

extern int mount_procfs(const char *);

extern int chrootfs(const char *);

#endif
