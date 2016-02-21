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
	$(RM) $(OBJ) ush

test: export HOSTNAME=eastbridge
test: testwhere testecho

testwhere: ush
	./ush <<< "where; end" | $(DIFF) ./tests/where.output -  && echo "OK PASS" && \
	./ush <<< "where ls; end" | $(DIFF) ./tests/wherels.output -  && echo "OK PASS" && \
	./ush <<< "where where; end" | $(DIFF) ./tests/wherewhere.output -  && echo "OK PASS"

testecho: ush
	./ush <<< "echo; end" | $(DIFF) ./tests/echo.output -  && echo "OK PASS" && \
	./ush <<< "echo echo; end" | $(DIFF) ./tests/echoecho.output -  && echo "OK PASS"
