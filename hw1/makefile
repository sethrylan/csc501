CC      = gcc
SRC     = mythread.c
CFLAGS  = -Wall -Wextra -pedantic -c -m32
RM      = rm -f
DIFF    = diff
LIB     = mythread.a
INDEX   = index.bin
SERIES  = micro small medium a
ORDERS  = 4

default: all

all: $(SRC)
	$(CC) $(CFLAGS) $(SRC)
	ar -rcs $(LIB) *.o

fib: $(SRCS) fib.c
	$(CC) $(CFLAGS) fib.c -o fib -L. -lmythread

clean:
	rm -rf $(LIB) mythread.o

# test: clean all
# 	@$(foreach series, $(SERIES), $(foreach order, $(ORDERS), \
# 		echo "\n$(series) @ $(order)" && \
# 		./$(EXE) $(INDEX) $(order) < files/input-01$(series).txt > files/test-output-01$(series).txt && \
# 		./$(EXE) $(INDEX) $(order) < files/input-02$(series).txt > files/test-output-02$(series).txt && \
# 		$(DIFF) files/test-output-01$(series).txt files/output-01$(series).txt && echo "OK PASS 01" && \
# 		$(DIFF) files/test-output-02$(series).txt files/output-02$(series).txt && echo "OK PASS 02"; \
# 		rm $(INDEX);))
