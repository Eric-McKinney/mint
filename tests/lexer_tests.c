#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/lexer.h"

#define SUCCESS 1
#define FAILURE 0
#define PASSED "\033[1;32mPASSED\033[0m"
#define FAILED "\033[1;31mFAILED\033[0m"
#define SEP "|------------------------------|\n"

typedef struct {
    char name[80];
    char input[256];
    char ans[1024];
} Test;

int verbose = 0;
int run_test(Test *test);

int main(int argc, char **argv) {
    Test tests[] = {
        {"empty_input", "", "[]"},
        {"arithmetic_toks", "+ - /*", "[TOK_ADD, TOK_SUB, TOK_DIV, TOK_MULT]"},
        {"other_toks", "().=\n", "[TOK_LPAREN, TOK_RPAREN, TOK_DOT, TOK_EQUAL, TOK_ENDLN]"},
        {"fn_tok", "fn", "[TOK_FUN]"},
        {"basic_addition", "1 + 3", "[TOK_INT 1, TOK_ADD, TOK_INT 3]"}
    };
    int num_tests = sizeof(tests) / sizeof(Test), i;

    if (argc == 2 && (strcmp(argv[1], "-v") == 0 
                   || strcmp(argv[1], "--verbose") == 0)) {
        verbose = 1;
    }
    
    for (i = 0; i < num_tests; i++) {
        Test *t = &(tests[i]);
        int t_result;

        if (i == 0 && verbose) {
            printf(SEP);
        }

        t_result = run_test(t);
        
        if (verbose) {
            printf("|\n");
        }

        printf("| \033[0;36m%s\033[0m %s\n", t->name, 
               t_result == SUCCESS ? PASSED : FAILED);

        if (verbose) {
            printf(SEP);
        }
    }
    
    return 0;
}

int run_test(Test *test) {
    TokenList *tok_l;
    char *tok_l_str;
    int t_result;

    if (verbose) {
        printf("| \033[0;36m%s\033[0m test:\n", test->name);
    }

    tok_l = tokenize(test->input);
    tok_l_str = token_list_to_str(tok_l);

    if (verbose) {
        printf("| tokenize return val: %p\n", (void *) tok_l);
        printf("| token list: %s\n", tok_l_str);
    }
    
    t_result = strcmp(tok_l_str, test->ans) == 0 ? SUCCESS : FAILURE;
    
    free(tok_l_str);
    free_token_list(tok_l);
    return t_result;
}
