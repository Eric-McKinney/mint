CC=gcc
CFLAGS= -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors \
        -fstack-protector-all

SRC=src
OBJ=obj
BIN=bin
TEST=tests

_OBJS= main.o lexer.o parser.o eval.o
OBJS=$(patsubst %,$(OBJ)/%,$(_OBJS))

.PHONY: test tests clean

$(BIN)/mint: $(OBJS)
	mkdir -p $(BIN)
	$(CC) -o $@ $^

test: runtests
runtests: lexer_tests parser_tests eval_tests

lexer_tests: $(OBJ)/lexer_tests.o $(OBJ)/lexer.o
	$(CC) -o lexer_tests 

$(OBJ)/main.o: $(SRC)/main.c $(SRC)/lexer.h $(SRC)/parser.h $(SRC)/eval.h $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/lexer.o: $(SRC)/lexer.c $(SRC)/lexer.h $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/parser.o: $(SRC)/parser.c $(SRC)/parser.h $(SRC)/lexer.h $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/eval.o: $(SRC)/eval.c $(SRC)/eval.h $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ):
	mkdir -p $(OBJ)

clean:
	rm -rf $(BIN) $(OBJ)
