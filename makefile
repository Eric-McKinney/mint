CC=gcc
CFLAGS= -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors \
        -fstack-protector-all

SRC=src
OBJ=obj
BIN=bin
TEST_SRC=tests
TEST_BIN=$(BIN)/tests

_OBJS= main.o lexer.o parser.o eval.o
OBJS=$(patsubst %,$(OBJ)/%,$(_OBJS))

.PHONY: tests runtests clean
.PRECIOUS: $(OBJ)/%_tests.o

$(BIN)/mint: $(OBJ) $(OBJS)
	mkdir -p $(BIN)
	$(CC) -o $@ $^

tests: $(OBJ) $(TEST_BIN) $(TEST_BIN)/lexer_tests $(TEST_BIN)/parser_tests
runtests: tests
	@$(TEST_BIN)/lexer_tests
	@echo "|"
	@$(TEST_BIN)/parser_tests

$(TEST_BIN)/lexer_tests: $(OBJ)/lexer_tests.o $(OBJ)/lexer.o
	$(CC) -o $@ $^

$(TEST_BIN)/parser_tests: $(OBJ)/parser_tests.o $(OBJ)/parser.o $(OBJ)/lexer.o
	$(CC) -o $@ $^

$(OBJ)/%_tests.o: $(TEST_SRC)/%_tests.c $(SRC)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/main.o: $(SRC)/main.c $(SRC)/lexer.h $(SRC)/parser.h $(SRC)/eval.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/lexer.o: $(SRC)/lexer.c $(SRC)/lexer.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/parser.o: $(SRC)/parser.c $(SRC)/parser.h $(SRC)/lexer.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/eval.o: $(SRC)/eval.c $(SRC)/eval.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ):
	mkdir -p $(OBJ)

$(TEST_BIN):
	mkdir -p $(TEST_BIN)

clean:
	rm -rf $(BIN) $(OBJ)

