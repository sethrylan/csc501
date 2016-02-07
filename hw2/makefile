CC				= gcc
CFLAGS		= -Wall -Wextra -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRC				= main.c parse.c parse.h
OBJ				=	main.o parse.o
rm				= rm -f

default: ush

ush:	$(OBJ)
	$(CC) -o $@ $(OBJ)

debug: CFLAGS += -DDEBUG
debug: ush

tar:
	tar czvf ush.tar.gz $(SRC) makefile README

clean:
	$(RM) $(OBJ) ush
