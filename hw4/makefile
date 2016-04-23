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
LS 				= ls -w 10000 -C
# used for non-exact matches; does not show non-matching result like diff
GREP      = grep -q

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

test: all testusage testmount testmemory0 testtouch testdir1 testdirerror testnesteddirs testremove testwrite

maketestdir:
	[ -e $(TEST_DIR) ] || mkdir -p $(TEST_DIR)

testusage: maketestdir ramdisk
	./ramdisk 2>&1 | $(DIFF) - tests/usage.output && echo "OK PASS";

testmount: maketestdir ramdisk
	./ramdisk $(TEST_DIR) 100 && \
	$(PUSHD) && \
	ls -l | diff <(echo "total 0") - && \
	pwd | diff <(echo $(TEST_DIR)) - && \
	$(POPD) && \
	$(WAIT) && \
	$(UMOUNT) $(TEST_DIR) && \
	echo "OK PASS";

testmemory0: maketestdir ramdisk
	./ramdisk $(TEST_DIR) 0 && \
	$(PUSHD) && \
	mkdir testdir 2>&1 | $(GREP) "No space left on device" && \
	touch testfile1 && \
	$(LS) | $(GREP) "testfile1" - && \
	(echo "data" > testfile2) 2>&1 | $(GREP)  "No space left on device" && \
	$(LS) | $(GREP) "testfile1  testfile2" - && \
	$(POPD) && \
	$(WAIT) && \
	$(UMOUNT) $(TEST_DIR) && \
	echo "OK PASS";

testtouch: maketestdir ramdisk
	./ramdisk $(TEST_DIR) 100 && \
	$(PUSHD) && \
	touch testfile && \
	$(LS) | diff <(echo "testfile") - && \
	$(POPD) && \
	$(WAIT) && \
	$(UMOUNT) $(TEST_DIR) && \
	echo "OK PASS";

testdir1: maketestdir ramdisk
	./ramdisk $(TEST_DIR) 100 && \
	$(PUSHD) && \
	mkdir testdir && \
	$(LS) | diff <(echo "testdir") - && \
	cd testdir && \
	ls -l | diff <(echo "total 0") - && \
	touch testfile && \
	$(LS) | diff <(echo "testfile") - && \
	$(POPD) && \
	$(WAIT) && \
	$(UMOUNT) $(TEST_DIR) && \
	echo "OK PASS";

testdirerror: maketestdir ramdisk
	./ramdisk $(TEST_DIR) 100 && \
	$(PUSHD) && \
	mkdir testdir && \
	mkdir testdir 2>&1 | $(GREP) "File exists" && \
	rmdir testdir && mkdir testdir && \
	$(LS) | diff <(echo "testdir") - && \
	$(POPD) && \
	$(WAIT) && \
	$(UMOUNT) $(TEST_DIR) && \
	echo "OK PASS";

# create and destroy directory strucutre
# /
# |-- dir1
# |-- dir2
#   |-- dir3
#     |-- dir4
#       |-- file
#   |-- file
# |-- file
testnesteddirs: maketestdir ramdisk
	./ramdisk $(TEST_DIR) 100 && \
	$(PUSHD) && \
	mkdir dir1 && \
	$(LS)             | diff <(echo "dir1") - && \
	$(LS) $(TEST_DIR) | diff <(echo "dir1") - && \
	touch file && \
	$(LS)             | diff <(echo "dir1  file") - && \
	$(LS) $(TEST_DIR) | diff <(echo "dir1  file") - && \
	mkdir dir2 && \
	$(LS)             | diff <(echo "dir1  dir2  file") - && \
	$(LS) $(TEST_DIR) | diff <(echo "dir1  dir2  file") - && \
	cd dir2 && \
	ls -l                  | diff <(echo "total 0") - && \
	ls -l $(TEST_DIR)/dir2 | diff <(echo "total 0") - && \
	touch file &&	mkdir dir3 && \
	$(LS)                  | diff <(echo "dir3  file") - && \
	$(LS) $(TEST_DIR)/dir2 | diff <(echo "dir3  file") - && \
	cd dir3 && \
	ls -l                       | diff <(echo "total 0") - && \
	ls -l $(TEST_DIR)/dir2/dir3 | diff <(echo "total 0") - && \
	touch file &&	mkdir dir4 && \
	$(LS)                       | diff <(echo "dir4  file") - && \
	$(LS) $(TEST_DIR)/dir2/dir3 | diff <(echo "dir4  file") - && \
	cd dir4 && \
	ls -l                            | diff <(echo "total 0") - && \
	ls -l $(TEST_DIR)/dir2/dir3/dir4 | diff <(echo "total 0") - && \
	touch file && \
	$(LS)                            | diff <(echo "file") - && \
	$(LS) $(TEST_DIR)/dir2/dir3/dir4 | diff <(echo "file") - && \
	pwd | diff <(echo $(TEST_DIR)/dir2/dir3/dir4) - && \
	cd .. && rmdir dir4 2>&1 | diff <(echo "rmdir: failed to remove \`dir4': Directory not empty") - && \
	rm dir4/file && rmdir dir4 && \
	$(LS)                       | diff <(echo "file") - && \
	$(LS) $(TEST_DIR)/dir2/dir3 | diff <(echo "file") - && \
	rm $(TEST_DIR)/dir2/dir3/file && \
	ls -l                       | diff <(echo "total 0") - && \
	ls -l $(TEST_DIR)/dir2/dir3 | diff <(echo "total 0") - && \
	$(POPD) && \
	$(WAIT) && \
	$(UMOUNT) $(TEST_DIR) && \
	echo "OK PASS";

# create and remove file in root
# make directory, create file and remove
testremove: maketestdir ramdisk
	./ramdisk $(TEST_DIR) 100 && \
	$(PUSHD) && \
	touch testfile && \
	$(LS) | diff <(echo "testfile") - && \
	rm testfile && \
	ls -l | diff <(echo "total 0") - && \
	mkdir testdir && cd testdir && touch testfile && \
	$(LS) | diff <(echo "testfile") - && \
	rm testfile && \
	ls -l | diff <(echo "total 0") - && \
	$(POPD) && \
	$(WAIT) && \
	$(UMOUNT) $(TEST_DIR) && \
	echo "OK PASS";

# cat to /testfile and /testdir/testfile2
# delete both files using full-qualified paths
testwrite: maketestdir ramdisk
	./ramdisk $(TEST_DIR) 100 && \
	$(PUSHD) && \
	echo "test content" > testfile && \
	cat testfile | diff <(echo "test content") - && \
	stat -c "%s" testfile | diff <(echo "13") - && \
	mkdir testdir && cd testdir && \
	echo "test content two" > testfile2 && \
	cat testfile2 | diff <(echo "test content two") - && \
	stat -c "%s" testfile2 | diff <(echo "17") - && \
	rm $(TEST_DIR)/testfile && rm $(TEST_DIR)/testdir/testfile2 && \
	$(LS) $(TEST_DIR)  | diff <(echo "testdir") - && \
	$(LS) $(TEST_DIR)/ | diff <(echo "testdir") - && \
	ls -l $(TEST_DIR)/testdir  | diff <(echo "total 0") - && \
	ls -l $(TEST_DIR)/testdir/ | diff <(echo "total 0") - && \
	$(POPD) && \
	$(WAIT) && \
	$(UMOUNT) $(TEST_DIR) && \
	echo "OK PASS";


# ./postmark benchmark.conf
