CC      = gcc
SRC     = mythread.c
CFLAGS  = -Wall -Wextra -pedantic
RM      = rm -f
DIFF    = diff
LIB     = mythread.a

default: lib

lib: $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)
	ar -rcs $(LIB) *.o

fib: lib tests/fib.c
	$(CC) $(CFLAGS) tests/fib.c ./mythread.a -o tests/fib

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
