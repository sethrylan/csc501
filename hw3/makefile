CC        = gcc
CFLAGS    = -Wall -Wextra -std=c99 -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRC       = master.c player.c
rm        = rm -f
DIFF      = diff -w
SHELL     = /bin/bash
UNAME     = $(shell uname -s)
CERT_NAME = gcc-signature

# comment line below for Linux machines
#LIB= -lsocket -lnsl

all: master player

# sign executable on OSX to exempt from "accept incoming network connections" firewall prompt
# this compilation target requires a code-signing certifacte named $(CERT_NAME)
ifeq ($(UNAME), Darwin)
all: signcode
endif

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

signcode:
	codesign --force -s $(CERT_NAME) ./master
	codesign --force -s $(CERT_NAME) ./player

clean:
	$(RM) listen speak master player *.o

tar:
	tar czvf socket.tar.gz $(SRC) makefile README REFERENCES

test: all testusage test-player1-hops0

testusage: master player
	./master 2>&1 | $(DIFF) - tests/master_usage.output && echo "OK PASS" && \
	./player 2>&1 | $(DIFF) - tests/player_usage.output && echo "OK PASS";

test-player1-hops0: master player
	./master 9999 1 0 > master.testoutput & ./player localhost 9999 > player.testoutput && \
	$(DIFF) master.testoutput tests/master_1_0.output && \
	$(DIFF) player.testoutput tests/player0_1_0.output && \
	echo "OK PASS";
