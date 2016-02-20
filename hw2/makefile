CC				= gcc
CFLAGS		= -Wall -Wextra -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRC				= main.c parse.c parse.h
OBJ				=	main.o parse.o
rm				= rm -f
DIFF    = diff

default: ush

ush:	$(OBJ)
	$(CC) -o $@ $(OBJ)

debug: CFLAGS += -DDEBUG
debug: ush

tar:
	tar czvf ush.tar.gz $(SRC) makefile README

clean:
	$(RM) $(OBJ) ush

test: testwhere

testwhere: ush
	./ush <<< "where ls" | $(DIFF) ./tests/wherels.output -  && echo "OK PASS" && \
	./ush <<< "where where" | $(DIFF) ./tests/wherewhere.output -  && echo "OK PASS"