CC				= gcc
CFLAGS		= -Wall -Wextra -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRC				= master.c player.c
rm				= rm -f
DIFF			= diff -w
SHELL			= /bin/bash

# comment line below for Linux machines
#LIB= -lsocket -lnsl

all: master player

debug: CFLAGS += -DDEBUG
debug: all

listen:	listen.o
	$(CC) $(CFLAGS) -o $@ listen.o $(LIB)

speak:	speak.o
	$(CC) $(CFLAGS) -o $@ speak.o $(LIB)

master:	master.o utils.o
	$(CC) $(CFLAGS) -o $@ $^

player:	player.o utils.o
	$(CC) $(CFLAGS) -o $@ $^

listen.o:	listen.c
speak.o:	speak.c
master.o:	master.c
player.o:	player.c
utils.o: utils.c utils.h

clean:
	$(RM) listen speak master player *.o

tar:
	tar czvf socket.tar.gz $(SRC) makefile README REFERENCES
