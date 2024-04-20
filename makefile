CC=gcc
CFLAGS= -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors \
        -fstack-protector-all

SRC=src
OBJ=obj
BIN=bin

_OBJS= main.o lexer.o parser.o eval.o
OBJS=$(patsubst %,$(OBJ)/%,$(_OBJS))

.PHONY: test clean

test:
	@echo $(OBJS)

$(BIN)/mint: $(OBJS)
	mkdir -p $(BIN)
	$(CC) -o $@ $^

$(OBJ)/main.o: main.c lexer.h parser.h eval.h $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/lexer.o: lexer.c lexer.h $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/parser.o: parser.c parser.h lexer.h $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/eval.o: eval.c eval.h $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ):
	mkdir -p $(OBJ)

clean:
	rm -rf $(BIN) $(OBJ)
