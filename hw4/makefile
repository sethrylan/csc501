CC        = gcc
CFLAGS    = -Wall -Wextra -std=c99 -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRCS      = ramdisk.c ramdisk.h makefile REFERENCES README
rm        = rm -f
DIFF      = diff -w
SHELL     = /bin/bash
UNAME     = $(shell uname -s)
CERT_NAME = gcc-signature
PUSHD     = pushd $(TEST_DIR) > /dev/null
POPD      = popd > /dev/null
WAIT      = sleep .1

all: ramdisk

# Sign executable on OSX to exempt from "accept incoming network connections" firewall prompt
# this compilation target requires a code-signing certifacte named $(CERT_NAME)
# Also, add osxfuse include
ifeq ($(UNAME), Darwin)
CFLAGS += `pkg-config fuse --cflags --libs`
CFLAGS += -DFUSE_USE_VERSION=26
UMOUNT = umount
TEST_DIR  = /private/tmp/rd
all: signcode
else
CFLAGS += `pkg-config fuse --cflags --libs`
CFLAGS += -DFUSE_USE_VERSION=26
UMOUNT = fusermount -u -q
TEST_DIR  = /tmp/rd
endif

debug: CFLAGS += -DDEBUG
debug: all

ramdisk: ramdisk.o utils.o
	$(CC) $(CFLAGS) -o $@ $^

ramdisk.o: ramdisk.c ramdisk.h
utils.o:   utils.c utils.h

signcode:
	codesign --force -s $(CERT_NAME) ./ramdisk

clean:
	$(RM) ramdisk *.o *.testoutput ramdisk.tar.gz

unmount:
	$(UMOUNT) $(TEST_DIR)

tar:
	tar czvf ramdisk.tar.gz $(SRCS) makefile README REFERENCES

test: all testusage testmount

maketestdir:
	[ -e $(TEST_DIR) ] || mkdir -p $(TEST_DIR)

testusage: maketestdir ramdisk
	./ramdisk 2>&1 | $(DIFF) - tests/usage.output && echo "OK PASS";

testmount: maketestdir ramdisk
	./ramdisk $(TEST_DIR) 100 && \
	$(PUSHD) && \
	ls -l && \
	$(POPD) && \
	$(WAIT) && \
	$(UMOUNT) $(TEST_DIR) && \
	echo "OK PASS";

# ./postmark benchmark.conf
