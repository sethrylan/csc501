CC				= gcc
CFLAGS		= -Wall -Wextra -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRC				= main.c parse.c parse.h ush.c ush.h list.c list.h
OBJ				=	main.o parse.o ush.o list.o
rm				= rm -f
DIFF			= diff -w
SHELL			= /bin/bash
ERROR     = awk " BEGIN { print "danger!" > \"/dev/fd/2\" }"

default: ush

ush:	$(OBJ)
	$(CC) -o $@ $(OBJ)

debug: CFLAGS += -DDEBUG
debug: ush

tar:
	tar czvf ush.tar.gz $(SRC) makefile README REFERENCES

clean:
	$(RM) $(OBJ) ush *.gch *.o test.output ush.tar.gz

test: export HOSTNAME=eastbridge
test: testwhere testecho testexecute testredirect testerrredirect testseparator testls testpipe testpipeerr testsetenv

testwhere: ush
	./ush <<< "where; logout" | $(DIFF) ./tests/where.output -  && echo "OK PASS" && \
	./ush <<< "where where; logout" | $(DIFF) ./tests/wherewhere.output -  && echo "OK PASS"

testecho: ush
	./ush <<< "echo; logout" | $(DIFF) ./tests/echo.output -  && echo "OK PASS" && \
	./ush <<< "echo echo; logout" | $(DIFF) ./tests/echoecho.output -  && echo "OK PASS" && \
	./ush <<< "echo echo > test.output" && $(DIFF) ./tests/echoredirect.output test.output && echo "OK PASS" && \
	rm test.output

testexecute: ush
	./ush <<< "expr 1 + 1; logout" | $(DIFF) ./tests/execexpr.output -  && echo "OK PASS"

testredirect: ush
	./ush <<< "cat < tests/cat.input > test.output" && $(DIFF) ./tests/cat.output test.output  && echo "OK PASS" && \
	rm test.output;
	./ush <<< "cat < tests/cat.input >> test.output" && $(DIFF) ./tests/cat.output test.output  && echo "OK PASS" && \
	./ush <<< "cat < tests/cat.input >> test.output" && $(DIFF) ./tests/cat.appendx1.output test.output  && echo "OK PASS" && \
	rm test.output

testls: ush
	./ush <<< "ls makefile > test.output" && $(DIFF) ./tests/lsredirect.output test.output && echo "OK PASS" && \
	rm test.output;

testfilterstdout: ush
	./ush <<< "./tests/stdout_err.sh > /dev/null" &> test.output && $(DIFF) tests/stdout_filter.output test.output && echo "OK PASS";

testerrredirect: ush
	./ush <<< "./tests/stderr.sh >& test.output" && $(DIFF) ./tests/stderr.output test.output  && echo "OK PASS" && \
	./ush <<< "./tests/stderr.sh >>& test.output" && $(DIFF) ./tests/stderr.appendx1.output test.output  && echo "OK PASS" && \
	./ush <<< "./tests/stderr.sh >>& test.output" && $(DIFF) ./tests/stderr.appendx2.output test.output  && echo "OK PASS" && \
	rm test.output

testseparator: ush
	./ush <<< "expr 1 + 1; expr 2 + 3; expr 5 + 8; logout" | $(DIFF) ./tests/exprx3.output -  && echo "OK PASS";

testpipe: ush
	./ush <<< "ls | grep makefile" | $(DIFF) ./tests/ls_grep.output - && echo "OK PASS" && \
	./ush <<< "ls | grep makefile > test.output" && $(DIFF) ./tests/ls_grep_redirect.output test.output && echo "OK PASS" && \
	rm test.output;
	./ush <<< "seq 50 | head -n10 | wc -l" | $(DIFF) ./tests/seq_head_wc.output - && echo "OK PASS" && \
	./ush <<< "seq 50 | head -n10 | wc -l > test.output" && $(DIFF) <(printf "      10") test.output && echo "OK PASS" && \
	rm test.output;

testpipeerr: ush
	./ush <<< "./tests/stderr.sh | grep 'danger' | wc -l > test.output" && $(DIFF) test.output <(echo "0") && echo "OK PASS" && \
	rm test.output;
	./ush <<< "./tests/stderr.sh |& grep 'danger' | wc -l > test.output" && $(DIFF) test.output <(echo "1") && echo "OK PASS" && \
	rm test.output;

testsetenv: ush
	./ush <<< "setenv PEANUTTY paradoxography & setenv | grep PEANUTTY > test.output" && $(DIFF) test.output <(printf "PEANUTTY=paradoxography") && echo "OK PASS" && \
	rm test.output;
	./ush <<< "setenv PEANUTTY paradoxography & unsetenv PEANUTTY & setenv | grep PEANUTTY > test.output" && $(DIFF) test.output <(printf "") && echo "OK PASS" && \
	rm test.output;
