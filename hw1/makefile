CC      = gcc
SRC     = mythread.c queue.h
CFLAGS  = -Wall -Wextra -pedantic -ggdb -D_XOPEN_SOURCE=600 -Wno-deprecated
RM      = rm -f
DIFF    = diff
LIB     = mythread.a
TESTS   = fib one passing tree ping

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

# create test data:
# for n in 0 1 2 5 9; do for m in 0 1 2 5 9; do for yield in 0 1; do for join in 0 1; do echo "n=$n, m=$m, yield=$yield, join=$join" && tests/tree $n $m $yield $join > tests/tree-$n-$m-$yield-$join.output; done; done; done; done;
test: clean tests
	./tests/one   | $(DIFF) ./tests/one.output -  && echo "OK PASS" && \
	./tests/fib 0 | $(DIFF) ./tests/fib-0.output -  && echo "OK PASS" && \
	./tests/fib 2 | $(DIFF) ./tests/fib-2.output -  && echo "OK PASS" && \
	./tests/fib 3 | $(DIFF) ./tests/fib-3.output -  && echo "OK PASS" && \
	./tests/fib 9 | $(DIFF) ./tests/fib-9.output -  && echo "OK PASS" && \
	./tests/ping 0 | $(DIFF) ./tests/ping-0.output -  && echo "OK PASS" && \
	./tests/ping 1 | $(DIFF) ./tests/ping-1.output -  && echo "OK PASS" && \
	./tests/ping 2 | $(DIFF) ./tests/ping-2.output -  && echo "OK PASS" && \
	./tests/ping 5 | $(DIFF) ./tests/ping-5.output -  && echo "OK PASS" && \
	./tests/ping 9 | $(DIFF) ./tests/ping-9.output -  && echo "OK PASS" && \
	./tests/passing 0 0 | $(DIFF) ./tests/passing-0-0.output -  && echo "OK PASS" && \
	./tests/passing 1 0 | $(DIFF) ./tests/passing-1-0.output -  && echo "OK PASS" && \
	./tests/passing 2 0 | $(DIFF) ./tests/passing-2-0.output -  && echo "OK PASS" && \
	./tests/passing 9 0 | $(DIFF) ./tests/passing-9-0.output -  && echo "OK PASS" && \
	./tests/passing 0 1 | $(DIFF) ./tests/passing-0-1.output -  && echo "OK PASS" && \
	./tests/passing 1 1 | $(DIFF) ./tests/passing-1-1.output -  && echo "OK PASS" && \
	./tests/passing 2 1 | $(DIFF) ./tests/passing-2-1.output -  && echo "OK PASS" && \
	./tests/passing 9 1 | $(DIFF) ./tests/passing-9-1.output -  && echo "OK PASS" && \
	./tests/passing 0 2 | $(DIFF) ./tests/passing-0-2.output -  && echo "OK PASS" && \
	./tests/passing 1 2 | $(DIFF) ./tests/passing-1-2.output -  && echo "OK PASS" && \
	./tests/passing 2 2 | $(DIFF) ./tests/passing-2-2.output -  && echo "OK PASS" && \
	./tests/passing 9 2 | $(DIFF) ./tests/passing-9-2.output -  && echo "OK PASS" && \
	./tests/tree 0 0 0 0 | $(DIFF) ./tests/tree-0-0-0-0.output -  && echo "OK PASS" && \
	./tests/tree 0 0 1 0 | $(DIFF) ./tests/tree-0-0-1-0.output -  && echo "OK PASS" && \
	./tests/tree 0 0 0 1 | $(DIFF) ./tests/tree-0-0-0-1.output -  && echo "OK PASS" && \
	./tests/tree 0 0 1 1 | $(DIFF) ./tests/tree-0-0-1-1.output -  && echo "OK PASS" && \
	./tests/tree 1 1 0 0 | $(DIFF) ./tests/tree-1-1-0-0.output -  && echo "OK PASS" && \
	./tests/tree 1 1 1 0 | $(DIFF) ./tests/tree-1-1-1-0.output -  && echo "OK PASS" && \
	./tests/tree 1 1 0 1 | $(DIFF) ./tests/tree-1-1-0-1.output -  && echo "OK PASS" && \
	./tests/tree 1 1 1 1 | $(DIFF) ./tests/tree-1-1-1-1.output -  && echo "OK PASS" && \
	./tests/tree 2 2 0 0 | $(DIFF) ./tests/tree-2-2-0-0.output -  && echo "OK PASS" && \
	./tests/tree 2 2 1 0 | $(DIFF) ./tests/tree-2-2-1-0.output -  && echo "OK PASS" && \
	./tests/tree 2 2 0 1 | $(DIFF) ./tests/tree-2-2-0-1.output -  && echo "OK PASS" && \
	./tests/tree 2 2 1 1 | $(DIFF) ./tests/tree-2-2-1-1.output -  && echo "OK PASS" && \
	./tests/tree 5 5 0 0 | $(DIFF) ./tests/tree-5-5-0-0.output -  && echo "OK PASS" && \
	./tests/tree 5 5 1 0 | $(DIFF) ./tests/tree-5-5-1-0.output -  && echo "OK PASS" && \
	./tests/tree 5 5 0 1 | $(DIFF) ./tests/tree-5-5-0-1.output -  && echo "OK PASS" && \
	./tests/tree 5 5 1 1 | $(DIFF) ./tests/tree-5-5-1-1.output -  && echo "OK PASS" && \
	./tests/tree 9 9 0 0 | $(DIFF) ./tests/tree-9-9-0-0.output -  && echo "OK PASS" && \
	./tests/tree 9 9 1 0 | $(DIFF) ./tests/tree-9-9-1-0.output -  && echo "OK PASS" && \
	./tests/tree 9 9 0 1 | $(DIFF) ./tests/tree-9-9-0-1.output -  && echo "OK PASS" && \
	./tests/tree 9 9 1 1 | $(DIFF) ./tests/tree-9-9-1-1.output -  && echo "OK PASS";


	# @$(foreach series, $(SERIES), $(foreach order, $(ORDERS), \
	# 	echo "\n$(series) @ $(order)" && \
	# 	./$(EXE) $(INDEX) $(order) < files/input-01$(series).txt > files/test-output-01$(series).txt && \
	# 	./$(EXE) $(INDEX) $(order) < files/input-02$(series).txt > files/test-output-02$(series).txt && \
	# 	$(DIFF) files/test-output-01$(series).txt files/output-01$(series).txt && echo "OK PASS 01" && \
	# 	$(DIFF) files/test-output-02$(series).txt files/output-02$(series).txt && echo "OK PASS 02"; \
	# 	rm $(INDEX);))
