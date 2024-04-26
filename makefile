CC=gcc
CFLAGS= -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors \
        -fstack-protector-all

SRC=src
OBJ=.obj
BIN=bin
TEST_SRC=tests
TEST_BIN=$(BIN)/tests

_OBJS= main.o lexer.o parser.o eval.o
OBJS=$(patsubst %,$(OBJ)/%,$(_OBJS))

.PHONY: tests runtests clean

$(BIN)/mint: $(OBJS)
	mkdir -p $(BIN)
	$(CC) -o $@ $^

tests: $(TEST_BIN) $(TEST_BIN)/lexer_tests $(TEST_BIN)/parser_tests $(TEST_BIN)/eval_tests
runtests: tests
	@$(TEST)/lexer_tests
	@$(TEST)/parser_tests
	@$(TEST)/eval_tests

$(TEST_BIN)/%_tests: $(OBJ)/%_tests.o $(OBJ)/%.o
	$(CC) -o $@ $^

#$(TEST_BIN)/parser_tests: $(OBJ)/parser_tests.o $(OBJ)/parser.o
	#$(CC) -o $@ $^

#$(TEST_BIN)/eval_tests: $(OBJ)/eval_tests.o $(OBJ)/eval.o
	#$(CC) -o $@ $^

$(OBJ)/%_tests.o: $(TEST_SRC)/%_tests.c $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $<

#$(OBJ)/parser_tests.o: $(TEST_SRC)/parser_tests.c $(OBJ)
	#$(CC) $(CFLAGS) -c -o $@ $<

#$(OBJ)/eval_tests.o: $(TEST_SRC)/eval_tests.c $(OBJ)
	#$(CC) $(CFLAGS) -c -o $@ $<

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

$(TEST_BIN):
	mkdir -p $(TEST_BIN)

clean:
	rm -rf $(BIN) $(OBJ)

