CC				= gcc
CFLAGS		= -Wall -Wextra -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRC				= main.c parse.c parse.h
OBJ				=	main.o parse.o
rm				= rm -f
DIFF			= diff -w
SHELL			= /bin/bash

default: ush

ush:	$(OBJ)
	$(CC) -o $@ $(OBJ)

debug: CFLAGS += -DDEBUG
debug: ush

tar:
	tar czvf ush.tar.gz $(SRC) makefile README

clean:
	$(RM) $(OBJ) ush *.gch *.o

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
	./ush <<< "shasum < tests/shasum.input > test.output" && $(DIFF) ./tests/shasum.output test.output  && echo "OK PASS" && \
	rm test.output && \
	./ush <<< "shasum < tests/shasum.input >> test.output" && $(DIFF) ./tests/shasum.output test.output  && echo "OK PASS" && \
	./ush <<< "shasum < tests/shasum.input >> test.output" && $(DIFF) ./tests/shasum.append.output test.output  && echo "OK PASS" && \
	rm test.output

testerrredirect: ush
	./ush <<< "ls nowhere >& test.output" && $(DIFF) ./tests/lserr.output test.output  && echo "OK PASS" && \
	./ush <<< "ls nowhere >>& test.output" && $(DIFF) ./tests/lserr.output test.output  && echo "OK PASS" && \
	rm test.output

testseparator: ush
	./ush <<< "expr 1 + 1; expr 2 + 3; expr 5 + 8; end" | $(DIFF) ./tests/exprx3.output -  && echo "OK PASS";
