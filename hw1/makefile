CC      = gcc
SRC     = mythread.c
CFLAGS  = -Wall -Wextra -pedantic -ggdb -D_XOPEN_SOURCE=600
RM      = rm -f
DIFF    = diff
LIB     = mythread.a
TESTS   = fib one

# see also https://lists.apple.com/archives/darwin-dev/2008/Feb/msg00107.html

default: lib

lib: $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)
	ar -rcs $(LIB) *.o

tests: lib tests/*.c
	@$(foreach test, $(TESTS), \
		$(CC) $(CFLAGS) tests/$(test).c ./mythread.a -o tests/$(test);\
	)

clean:
	rm -rf $(LIB) mythread.o
	find ./tests/ -type f  ! -name "*.*"  -delete

# test: clean all
# 	@$(foreach series, $(SERIES), $(foreach order, $(ORDERS), \
# 		echo "\n$(series) @ $(order)" && \
# 		./$(EXE) $(INDEX) $(order) < files/input-01$(series).txt > files/test-output-01$(series).txt && \
# 		./$(EXE) $(INDEX) $(order) < files/input-02$(series).txt > files/test-output-02$(series).txt && \
# 		$(DIFF) files/test-output-01$(series).txt files/output-01$(series).txt && echo "OK PASS 01" && \
# 		$(DIFF) files/test-output-02$(series).txt files/output-02$(series).txt && echo "OK PASS 02"; \
# 		rm $(INDEX);))
