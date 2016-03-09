CC				= gcc
CFLAGS		= -Wall -Wextra -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRC				= master.c player.c
rm				= rm -f
DIFF			= diff -w
SHELL			= /bin/bash

# comment line below for Linux machines
#LIB= -lsocket -lnsl

all: listen speak

debug: CFLAGS += -DDEBUG
debug: all

listen:	listen.o
	$(CC) $(CFLAGS) -o $@ listen.o $(LIB)

speak:	speak.o
	$(CC) $(CFLAGS) -o $@ speak.o $(LIB)

master:	listen.o
	$(CC) $(CFLAGS) -o $@ listen.o $(LIB)

player:	speak.o
	$(CC) $(CFLAGS) -o $@ speak.o $(LIB)

listen.o:	listen.c
speak.o:	speak.c
master.o:	master.c
player.o:	player.c

clean:
	\rm -f listen speak master player *.o

tar:
	tar czvf socket.tar.gz $(SRC) makefile README REFERENCES
