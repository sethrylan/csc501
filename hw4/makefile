CC        = gcc
CFLAGS    = -Wall -Wextra -std=c99 -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRCS      = ramdisk.c ramdisk.h makefile REFERENCES README
rm        = rm -f
DIFF      = diff -w
SHELL     = /bin/bash
UNAME     = $(shell uname -s)
CERT_NAME = gcc-signature

all: ramdisk

# Sign executable on OSX to exempt from "accept incoming network connections" firewall prompt
# this compilation target requires a code-signing certifacte named $(CERT_NAME)
# Also, add osxfuse include
ifeq ($(UNAME), Darwin)
CFLAGS += `pkg-config fuse --cflags --libs`
CFLAGS += -DFUSE_USE_VERSION=26
all: signcode
else
CFLAGS += `pkg-config fuse --cflags --libs`
CFLAGS += -DFUSE_USE_VERSION=26
endif

debug: CFLAGS += -DDEBUG
debug: all

ramdisk: ramdisk.o
	$(CC) $(CFLAGS) -o $@ $^

ramdisk.o:	ramdisk.c ramdisk.h
# utils.o: utils.c utils.h

signcode:
	codesign --force -s $(CERT_NAME) ./ramdisk

clean:
	$(RM) ramdisk *.o *.testoutput ramdisk.tar.gz

tar:
	tar czvf ramdisk.tar.gz $(SRCS) makefile README REFERENCES

test: all testusage

testusage: ramdisk
	./ramdisk | $(DIFF) - tests/usage.output && echo "OK PASS";

# ./postmark benchmark.conf
