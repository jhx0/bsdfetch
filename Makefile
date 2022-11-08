SRC=sysctlbyname.c bsdfetch.c
OBJ=bsdfetch
CC=cc
CFLAGS=-Wall -Werror -Wunused -Wextra -Wshadow -pedantic
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
