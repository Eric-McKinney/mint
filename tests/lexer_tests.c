#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/lexer.h"

#define SUCCESS 1
#define FAILURE 0
#define PASSED "\033[1;32mPASSED\033[0m"
#define FAILED "\033[1;31mFAILED\033[0m"
#define SEP "|------------------------------------------------------------|\n"

typedef struct {
    char name[80];
    char input[256];
    char ans[1024];
} Test;

int verbose = 0;
static int run_test(Test *test);

int main(int argc, char **argv) {
    Test tests[] = {
        {"empty_input", "", "[]"},
        {"arithmetic_toks", "+ - /*", "[TOK_ADD, TOK_SUB, TOK_DIV, TOK_MULT]"},
        {"other_toks", "().=\n", "[TOK_LPAREN, TOK_RPAREN, TOK_DOT, TOK_EQUAL, TOK_ENDLN]"},
        {"fn_tok", "fn", "[TOK_FUN]"},
        {"basic_addition", "1 + 3", "[TOK_INT 1, TOK_ADD, TOK_INT 3]"},
        {"float + int + float", "2.20 + 5+5.", "[TOK_FLOAT 2.200000, TOK_ADD, TOK_INT 5, TOK_ADD, TOK_FLOAT 5.000000]"},
        {"big ints", "232342 4444", "[TOK_INT 232342, TOK_INT 4444]"},
        {"negative nums", "-23 - -33.4563", "[TOK_INT -23, TOK_SUB, TOK_FLOAT -33.456300]"},
        {"ids", "id1 snake_case CamelCase", "[TOK_ID id1, TOK_ID snake_case, TOK_ID CamelCase]"},
        {"real use", "R = 500\nR*5", "[TOK_ID R, TOK_EQUAL, TOK_INT 500, TOK_ENDLN, TOK_ID R, TOK_MULT, TOK_INT 5]"},
        {"all whitespace", "      \t\t \n \t\t\t  ", "[TOK_ENDLN]"},
        {"comma", ",,,", "[TOK_COMMA, TOK_COMMA, TOK_COMMA]"},
        {"function", "fn f(a, b) = a + b", 
         "[TOK_FUN, TOK_ID f, TOK_LPAREN, TOK_ID a, TOK_COMMA, TOK_ID b, TOK_RPAREN, TOK_EQUAL, TOK_ID a, TOK_ADD, TOK_ID b]"}
    };
    int num_tests = sizeof(tests) / sizeof(Test), i, num_passed = 0;

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
        num_passed += t_result == SUCCESS ? 1 : 0;
        
        if (verbose) {
            printf("|\n");
        }

        printf("| \033[0;36m%s\033[0m %s\n", t->name, 
               t_result == SUCCESS ? PASSED : FAILED);

        if (verbose) {
            printf(SEP);
        }
    }

    printf("|\n| Ran (%d/%d) tests successfully\n", num_passed, num_tests);
    printf("| Test suite %s\n", num_passed == num_tests ? PASSED : FAILED);

    if (verbose) {
        printf("|\n");
        printf(SEP);
    }
    
    return 0;
}

static int run_test(Test *test) {
    TokenList *tok_l;
    char *tok_l_str;
    int t_result;

    if (verbose) {
        printf("| \033[0;36m%s\033[0m test:\n", test->name);
    }

    tok_l = tokenize(test->input);
    tok_l_str = token_list_to_str(tok_l);

    if (verbose) {
        printf("| input: \"%s\"\n", test->input);
        printf("| tokenize return val: %p\n", (void *) tok_l);
        printf("| token list: %s\n", tok_l_str);
        printf("| expected:   %s\n", test->ans);
    }
    
    t_result = strcmp(tok_l_str, test->ans) == 0 ? SUCCESS : FAILURE;
    
    free(tok_l_str);
    free_token_list(tok_l);
    return t_result;
}
