# kernel user namespace
Linux kernel introduced namespace mechanism since 2.4.19, especially with 3.8, creating user namespace for an unprivileged user comes true, in which a full set of caps granted to you. However it is just available in some linux distribution(ubuntu, debian...).

## 1. what is user namespce? ##
User namespce establish a mapping from host to container, which allows full caps in container while a normal unprivileged process on host.
it is to keep being rooted from container escape, with this security option, a process in container runs as root is able to do whatever he wants, in the meanwhile, a process outside the contaienr that created the new user namespace just runs as a normal user.

In conclusion, you are the king in container, but nothing outside.

## 2. how to use user namespce? ##
### 2.1 clone

User namespaces are created by specifying the CLONE_NEWUSER flag when calling clone() or unshare(). 

### 2.2 rules for mapping
Defining the mappings used for the user and group IDs of the processes that will be created in that namespace. This is done by writing mapping information to the /proc/PID/uid_map and /proc/PID/gid_map files corresponding to one of the processes in the user namespace. (Initially, these two files are empty.) This information consists of one or more lines, each of which contains three values separated by white space:

    ID-inside-ns   ID-outside-ns   length

ID-inside-ns defines the start point in the container corresponding to ID-outside-ns, which means the beginning outside the container, and the length specify the mapping ranges for uids and gids.

Usually, mapping could be done in the user namespace or parent user namespce. it all depends on:

- in user namespace, ***then ID-outside-ns is interpreted as a user ID (group ID) in the parent user namespace of the process PID***. The common case here is that a process is writing to its own mapping file (/proc/self/uid_map or /proc/self/gid_map).

- in parent user namespaces, then ID-outside-ns is interpreted as a user ID (group ID) in the user namespace of the process opening /proc/PID/uid_map (/proc/PID/gid_map). The writing process is then defining the mapping relative to its own user namespace.

The /proc/PID/uid_map file is owned by the user ID that created the namespace, and is writeable only by that user (or root user). In addition, all of the following requirements must be met:

- The writing process must have the CAP_SETUID/CAP_SETGID capability in the user namespace of the process PID(initial process in user namespace, processcess runing as parent user or privilege user in parent user namespace).


- The data written to uid_map (gid_map) consists of a single line that maps (only) the writing process's effective user ID (group ID) in the parent user namespace to a user ID (group ID) in the user namespace. This rule allows the initial process in a user namespace (i.e., the child created by clone()) to write a mapping for its own user ID (group ID).

- ***The process has the CAP_SETUID/CAP_SETGID capability in the parent user namespace. Such a process can define mappings to arbitrary user IDs (group IDs) in the parent user namespace***. As we noted earlier, the initial process in a new user namespace has no capabilities in the parent namespace. Thus, only a process in the parent namespace can write a mapping that maps arbitrary IDs in the parent user namespace.

## 3. keep attention for setgroups? ##
Linux 3.19 and later do not allow unprivileged processes to write a GID map
unless the setgroups() call has been permanently disabled by writing "deny"
to /proc/PID/setgroups. This is a fix for CVE-2014-8989 which applied to
strangely-configured systems where group membership implies more restricted
permissions rather than supplementary permissions.
