UNAME_S != uname -s
.if $(UNAME_S) == "OpenBSD"
SRC=sysctlbyname.c bsdfetch.c
.else
SRC=bsdfetch.c
.endif
OBJ=bsdfetch
CC=cc
CFLAGS=-Wall -Wunused -Wextra -Wshadow -pedantic -O2
DEBUG=-g -ggdb

.PHONY: all
all:
	$(CC) $(SRC) -o $(OBJ) $(CFLAGS)

.PHONY: debug
debug:
	$(CC) $(SRC) -o $(OBJ) $(CFLAGS) $(DEBUG)

.PHONY: clean
clean:
	rm -f $(OBJ)
