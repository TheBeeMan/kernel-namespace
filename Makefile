CC = gcc
CFLAGS = -g -std=gnu99 -Os -Wall -Wextra

BINARIES = ns
all: ${BINARIES}

clean: 
	rm -f ${BINARIES} *.o

ns: ns.o mount.o 

