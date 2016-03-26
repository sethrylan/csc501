CC        = gcc
CFLAGS    = -Wall -Wextra -std=c99 -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRC       = master.c player.c
rm        = rm -f
DIFF      = diff -w
SHELL     = /bin/bash
UNAME     = $(shell uname -s)
CERT_NAME = gcc-signature
SLEEP     = sleep 1

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
	$(RM) listen speak master player *.o *.testoutput

tar:
	tar czvf socket.tar.gz $(SRC) makefile README REFERENCES

test: all testusage 1p0h 1p100h 2p1h

testusage: master player
	./master 2>&1 | $(DIFF) - tests/master_usage.output && echo "OK PASS" && \
	./player 2>&1 | $(DIFF) - tests/player_usage.output && echo "OK PASS";

1p0h: master player
	./master 9999 1 0 > master.testoutput & ./player localhost 9999 > player0.testoutput && \
	$(DIFF) master.testoutput tests/1p0h/master.output && \
	$(DIFF) player0.testoutput tests/1p0h/player0.output && \
	echo "OK PASS";

1p100h: master player
	./master 9999 1 100 > master.testoutput & ./player localhost 9999 > player0.testoutput && \
	$(DIFF) master.testoutput tests/1p100h/master.output && \
	$(DIFF) player0.testoutput tests/1p100h/player0.output && \
	echo "OK PASS";

# sleep between connections to ensure that player 0 is registered first
2p1h: master player
	./master 9999 2 1 > master.testoutput & \
	./player localhost 9999 > player0.testoutput & \
	$(SLEEP) && \
	./player localhost 9999 > player1.testoutput && \
	$(DIFF) master.testoutput tests/2p1h/master.output && \
	$(DIFF) player0.testoutput tests/2p1h/player0.output && \
	$(DIFF) player1.testoutput tests/2p1h/player1.output && \
	echo "OK PASS";

3p100h: master player
	./master 9999 3 100 > master.testoutput & ./player localhost 9999 > player0.testoutput & $(SLEEP) && ./player localhost 9999 > player1.testoutput & $(SLEEP) && ./player localhost 9999 > player2.testoutput && \
	$(DIFF) master.testoutput tests/3p100h/master.output && \
	$(DIFF) player0.testoutput tests/3p100h/player0.output && \
	$(DIFF) player1.testoutput tests/3p100h/player1.output && \
	$(DIFF) player2.testoutput tests/3p100h/player2.output && \
	echo "OK PASS";

