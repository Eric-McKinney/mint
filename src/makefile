CC=gcc
CFLAGS= -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors \
        -fstack-protector-all

PROGS= mint

.PHONY: clean

mint: lexer.o parser.o eval.o
	$(CC) -o mint $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm a.out *.o $(PROGS)
