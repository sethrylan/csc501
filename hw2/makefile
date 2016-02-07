CC				= gcc
CFLAGS		= -Wall -Wextra -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
SRC				= main.c parse.c parse.h
OBJ				=	main.o parse.o

ush:	$(OBJ)
	$(CC) -o $@ $(OBJ)

tar:
	tar czvf ush.tar.gz $(SRC) makefile README

clean:
	\rm $(OBJ) ush
