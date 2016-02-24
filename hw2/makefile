CC				= gcc
CFLAGS		= -Wall -Wextra -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRC				= main.c parse.c ush.c parse.h
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
	tar czvf ush.tar.gz $(SRC) makefile README

clean:
	$(RM) $(OBJ) ush *.gch *.o test.output

test: export HOSTNAME=eastbridge
test: testwhere testecho testexecute testredirect testerrredirect testseparator

testwhere: ush
	./ush <<< "where; end" | $(DIFF) ./tests/where.output -  && echo "OK PASS" && \
	./ush <<< "where ls; end" | $(DIFF) ./tests/wherels.output -  && echo "OK PASS" && \
	./ush <<< "where where; end" | $(DIFF) ./tests/wherewhere.output -  && echo "OK PASS"

testecho: ush
	./ush <<< "echo; end" | $(DIFF) ./tests/echo.output -  && echo "OK PASS" && \
	./ush <<< "echo echo; end" | $(DIFF) ./tests/echoecho.output -  && echo "OK PASS"

testexecute: ush
	./ush <<< "expr 1 + 1; end" | $(DIFF) ./tests/execexpr.output -  && echo "OK PASS"

testredirect: ush
	./ush <<< "cat < tests/cat.input > test.output" && $(DIFF) ./tests/cat.output test.output  && echo "OK PASS" && \
	rm test.output && \
	./ush <<< "cat < tests/cat.input >> test.output" && $(DIFF) ./tests/cat.output test.output  && echo "OK PASS" && \
	./ush <<< "cat < tests/cat.input >> test.output" && $(DIFF) ./tests/cat.appendx1.output test.output  && echo "OK PASS" && \
	rm test.output

# testfilterstdout: ush
# 	./ush <<< "./tests/stdout_err.sh > /dev/null" &> test.output && $(DIFF) tests/stdout_filter.output test.output && echo "OK PASS";

testerrredirect: ush
	./ush <<< "./tests/stderr.sh >& test.output" && $(DIFF) ./tests/stderr.output test.output  && echo "OK PASS" && \
	rm test.output
	# ./ush <<< "./tests/stderr.sh >>& test.output" && $(DIFF) ./tests/stderr.appendx1.output test.output  && echo "OK PASS" && \
	# ./ush <<< "./tests/stderr.sh >>& test.output" && $(DIFF) ./tests/stderr.appendx2.output test.output  && echo "OK PASS" && \

testseparator: ush
	./ush <<< "expr 1 + 1; expr 2 + 3; expr 5 + 8; end" | $(DIFF) ./tests/exprx3.output -  && echo "OK PASS";
