# kernel usernamespace
linux kernel introduced namespace mechanism since 2.4.19, especially with 3.8, creating user namespace for an unprivileged user comes true, in which a full set of caps granted to you. However it is just available in some linux distribution(ubuntu, debian...).

## 1. what is user namespce? ##
user namespce establish a mapping from host to container, which allows full caps in container while a normal unprivileged process on host.
it is to keep being rooted from container escape, with this security option, a process in container runs as root is able to do whatever he wants, in the meanwhile, a process outside the contaienr that created the new user namespace just runs as a normal user.

in conclusion, you are the king in container, but nothing outside.

## 2. how to use user namespce? ##

## 3. which part should keep attention? ##
