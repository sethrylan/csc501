CC      = gcc
SRC     = mythread.c queue.h
CFLAGS  = -Wall -Wextra -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
RM      = rm -f
DIFF    = diff
LIB     = mythread.a
TESTS   = fib one passing

# see also https://lists.apple.com/archives/darwin-dev/2008/Feb/msg00107.html

default: lib

lib: $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)
	ar -rcs $(LIB) *.o

debug: CFLAGS += -DDEBUG
debug: tests

tests: lib tests/*.c
	@$(foreach test, $(TESTS), \
		$(CC) $(CFLAGS) tests/$(test).c ./mythread.a -o tests/$(test);\
	)

clean:
	rm -rf $(LIB) mythread.o *.dSYM tests/*.dSYM *.gch
	find ./tests/ -type f  ! -name "*.*"  -delete

test: clean tests
	./tests/one   | $(DIFF) ./tests/one.output -  && echo "OK PASS" && \
	./tests/fib 0 | $(DIFF) ./tests/fib-0.output -  && echo "OK PASS" && \
	./tests/fib 2 | $(DIFF) ./tests/fib-2.output -  && echo "OK PASS" && \
	./tests/fib 3 | $(DIFF) ./tests/fib-3.output -  && echo "OK PASS" && \
	./tests/fib 9 | $(DIFF) ./tests/fib-9.output -  && echo "OK PASS";

	# @$(foreach series, $(SERIES), $(foreach order, $(ORDERS), \
	# 	echo "\n$(series) @ $(order)" && \
	# 	./$(EXE) $(INDEX) $(order) < files/input-01$(series).txt > files/test-output-01$(series).txt && \
	# 	./$(EXE) $(INDEX) $(order) < files/input-02$(series).txt > files/test-output-02$(series).txt && \
	# 	$(DIFF) files/test-output-01$(series).txt files/output-01$(series).txt && echo "OK PASS 01" && \
	# 	$(DIFF) files/test-output-02$(series).txt files/output-02$(series).txt && echo "OK PASS 02"; \
	# 	rm $(INDEX);))
