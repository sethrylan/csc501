#
#
CC=gcc
CFLAGS=-g

# comment line below for Linux machines
#LIB= -lsocket -lnsl

all: listen speak

listen:	listen.o
	$(CC) $(CFLAGS) -o $@ listen.o $(LIB)

speak:	speak.o
	$(CC) $(CFLAGS) -o $@ speak.o $(LIB)

listen.o:	listen.c

speak.o:	speak.c

clean:
	\rm -f listen speak *.o

tar:
	cd ..; tar czvf socket.tar.gz socket/Makefile socket/listen.c socket/speak.c socket/README; cd socket; mv ../socket.tar.gz .

