CC        = gcc
CFLAGS    = -Wall -Wextra -std=c99 -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRCS      = master.c player.c utils.c utils.h makefile REFERENCES README
rm        = rm -f
DIFF      = diff -w
SHELL     = /bin/bash
UNAME     = $(shell uname -s)
CERT_NAME = gcc-signature
SLEEP     = sleep .1
PORT      = 54321

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
	$(RM) listen speak master player *.o *.testoutput potato.tar.gz

tar:
	tar czvf potato.tar.gz $(SRCS) makefile README REFERENCES

test: all testusage 1p0h 1p100h 1p1000h 2p1h 2p1000h 3p100h 10p8000h

testusage: master player
	./master 2>&1 | $(DIFF) - tests/master_usage.output && echo "OK PASS" && \
	./player 2>&1 | $(DIFF) - tests/player_usage.output && echo "OK PASS";

1p0h: master player
	./master $(PORT) 1 0 > master.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player0.testoutput && \
	$(DIFF) master.testoutput tests/1p0h/master.output && \
	$(DIFF) player0.testoutput tests/1p0h/player0.output && \
	echo "OK PASS";

1p100h: master player
	./master $(PORT) 1 100 > master.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player0.testoutput && \
	$(DIFF) master.testoutput tests/1p100h/master.output && \
	$(DIFF) player0.testoutput tests/1p100h/player0.output && \
	echo "OK PASS";

1p1000h: master player
	./master $(PORT) 1 1000 > master.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player0.testoutput && \
	$(DIFF) master.testoutput tests/1p1000h/master.output && \
	$(DIFF) player0.testoutput tests/1p1000h/player0.output && \
	echo "OK PASS";

# sleep between connections to ensure that player 0 is registered first

2p1h: master player
	./master $(PORT) 2 1 > master.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player0.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player1.testoutput && \
	$(DIFF) master.testoutput tests/2p1h/master.output && \
	$(DIFF) player0.testoutput tests/2p1h/player0.output && \
	$(DIFF) player1.testoutput tests/2p1h/player1.output && \
	echo "OK PASS";

2p1000h: master player
	./master $(PORT) 2 1000 > master.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player0.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player1.testoutput && \
	$(DIFF) master.testoutput tests/2p1000h/master.output && \
	$(DIFF) player0.testoutput tests/2p1000h/player0.output && \
	$(DIFF) player1.testoutput tests/2p1000h/player1.output && \
	echo "OK PASS";
	echo "OK PASS";

3p100h: master player
	./master $(PORT) 3 100 > master.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player0.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player1.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player2.testoutput && \
	$(DIFF) master.testoutput tests/3p100h/master.output && \
	$(DIFF) player0.testoutput tests/3p100h/player0.output && \
	$(DIFF) player1.testoutput tests/3p100h/player1.output && \
	$(DIFF) player2.testoutput tests/3p100h/player2.output && \
	echo "OK PASS";

10p8000h: master player
	./master $(PORT) 10 8000 > master.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player0.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player1.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player2.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player3.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player4.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player5.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player6.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player7.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player8.testoutput & \
	$(SLEEP); \
	./player localhost $(PORT) > player9.testoutput && \
	$(DIFF) master.testoutput tests/10p8000h/master.output && \
	$(DIFF) player0.testoutput tests/10p8000h/player0.output && \
	$(DIFF) player1.testoutput tests/10p8000h/player1.output && \
	$(DIFF) player2.testoutput tests/10p8000h/player2.output && \
	$(DIFF) player3.testoutput tests/10p8000h/player3.output && \
	$(DIFF) player4.testoutput tests/10p8000h/player4.output && \
	$(DIFF) player5.testoutput tests/10p8000h/player5.output && \
	$(DIFF) player6.testoutput tests/10p8000h/player6.output && \
	$(DIFF) player7.testoutput tests/10p8000h/player7.output && \
	$(DIFF) player8.testoutput tests/10p8000h/player8.output && \
	$(DIFF) player9.testoutput tests/10p8000h/player9.output && \
	echo "OK PASS";


