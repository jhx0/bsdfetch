UNAME_S != uname -s
.if $(UNAME_S) == "OpenBSD"
SRC=sysctlbyname.c bsdfetch.c
.else
SRC=bsdfetch.c
.endif
OBJ=bsdfetch
CC=cc
CFLAGS=-Wall -Wunused -Wextra -Wshadow -pedantic
DEBUG=-g -ggdb
OPTIMIZATION=-O2

.PHONY: all
all:
	$(CC) $(SRC) -o $(OBJ) $(CFLAGS) $(OPTIMIZATION)

.PHONY: debug
debug:
	$(CC) $(SRC) -o $(OBJ) $(CFLAGS) $(DEBUG)

.PHONY: clean
clean:
	rm -f $(OBJ)
