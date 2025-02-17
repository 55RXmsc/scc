CFLAGS=-std=c11 -g -static

jcc: scc.c

test: scc
        ./test.sh

clean:
        rm -f scc *.o *~ tmp*

.PHONY: test clean
