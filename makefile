CC=gcc
CFLAGS= -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors \
        -fstack-protector-all

SRC=src
OBJ=obj
BIN=bin
TEST_SRC=tests
TEST_BIN=$(BIN)/tests
TEST_LOG=$(BIN)/log
LEXER_LOG=$(TEST_LOG)/lexer_tests.log
PARSER_LOG=$(TEST_LOG)/parser_tests.log
GREP=grep --color=always

_OBJS= main.o lexer.o parser.o eval.o
OBJS=$(patsubst %,$(OBJ)/%,$(_OBJS))

.PHONY: tests runtests vvtests clean
.PRECIOUS: $(OBJ)/%_tests.o

$(BIN)/mint: $(OBJ) $(OBJS)
	mkdir -p $(BIN)
	$(CC) -o $@ $^

tests: $(OBJ) $(TEST_BIN) $(TEST_BIN)/lexer_tests $(TEST_BIN)/parser_tests
runtests: tests
	@$(TEST_BIN)/lexer_tests
	@echo "|"
	@$(TEST_BIN)/parser_tests
vvtests: $(TEST_LOG) tests
	@valgrind --log-file=$(LEXER_LOG) --track-origins=yes --leak-check=full $(TEST_BIN)/lexer_tests -v | tee -a $(LEXER_LOG)
	@$(GREP) --after-context 3 "HEAP SUMMARY" $(LEXER_LOG)
	@# in grep commands w/trailing || true, the pattern may not be present (and I don't want any output in this case)
	@$(GREP) --after-context 6 "LEAK SUMMARY" $(LEXER_LOG) || true
	@$(GREP) --after-context 1 "no leaks" $(LEXER_LOG) || true
	@$(GREP) "ERROR SUMMARY" $(LEXER_LOG)
	@valgrind --log-file=$(PARSER_LOG) --track-origins=yes --leak-check=full $(TEST_BIN)/parser_tests -v | tee -a $(PARSER_LOG)
	@$(GREP) --after-context 3 "HEAP SUMMARY" $(PARSER_LOG)
	@$(GREP) --after-context 6 "LEAK SUMMARY" $(PARSER_LOG) || true
	@$(GREP) --after-context 1 "no leaks" $(PARSER_LOG) || true
	@$(GREP) "ERROR SUMMARY" $(PARSER_LOG)
	@echo "|------------------------------------------------------------|"
	@echo "| Full test logs written to \e[0;36m./$(TEST_LOG)\e[0m"
	@echo "|------------------------------------------------------------|"

$(TEST_BIN)/lexer_tests: $(OBJ)/lexer_tests.o $(OBJ)/lexer.o
	$(CC) -o $@ $^

$(TEST_BIN)/parser_tests: $(OBJ)/parser_tests.o $(OBJ)/parser.o $(OBJ)/lexer.o
	$(CC) -o $@ $^

$(OBJ)/lexer_tests.o: $(TEST_SRC)/lexer_tests.c $(SRC)/lexer.h $(TEST_SRC)/test.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/parser_tests.o: $(TEST_SRC)/parser_tests.c $(SRC)/parser.h $(SRC)/lexer.h $(TEST_SRC)/test.h
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

$(TEST_LOG):
	mkdir -p $(TEST_LOG)

clean:
	rm -rf $(BIN) $(OBJ)

