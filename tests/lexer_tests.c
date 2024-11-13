#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "../src/lexer.h"
#include "test.h"

typedef struct {
    const char *tok_l;
    int err;
} Ans;

typedef struct {
    const char *name;
    const char *input;
    Ans ans;
} Test;

int verbose = 0;
static int run_all_tests(const Test *tests, int num_tests);
static int run_test(const Test *test);

int main(int argc, char **argv) {
    Test tests[] = {
        {"empty_input", "", {"[]", NOERR}},
        {"comment", "# a comment", {"[TOK_COMMENT]", NOERR}},
        {"math + comment", "1 + 1 # a comment", {"[TOK_INT 1, TOK_ADD, TOK_INT 1, TOK_COMMENT]", NOERR}},
        {"arithmetic_toks", "+ - /*", {"[TOK_ADD, TOK_SUB, TOK_DIV, TOK_MULT]", NOERR}},
        {"other_toks", "().=^", {"[TOK_LPAREN, TOK_RPAREN, TOK_DOT, TOK_EQUAL, TOK_EXP]", NOERR}},
        {"fn_tok", "fn", {"[TOK_FUN]", NOERR}},
        {"basic_addition", "1 + 3", {"[TOK_INT 1, TOK_ADD, TOK_INT 3]", NOERR}},
        {
            "float + int + float",
            ".20 + 5+5.",
            {"[TOK_FLOAT 0.200000, TOK_ADD, TOK_INT 5, TOK_ADD, TOK_FLOAT 5.000000]", NOERR}
        },
        {"big ints", "232342 4444", {"[TOK_INT 232342, TOK_INT 4444]", NOERR}},
        {"negative nums", "-23 - -33.4563", {"[TOK_INT -23, TOK_SUB, TOK_FLOAT -33.456300]", NOERR}},
        {"ids", "id1 snake_case CamelCase", {"[TOK_ID id1, TOK_ID snake_case, TOK_ID CamelCase]", NOERR}},
        {"real use", "R = 500R*5", {"[TOK_ID R, TOK_EQUAL, TOK_INT 500, TOK_ID R, TOK_MULT, TOK_INT 5]", NOERR}},
        {"all whitespace", "      \t\t  \t\t\t  ", {"[]", NOERR}},
        {"comma", ",,,", {"[TOK_COMMA, TOK_COMMA, TOK_COMMA]", NOERR}},
        {
            "function",
            "fn f(a, b) = a + b", 
            {"[TOK_FUN, TOK_ID f, TOK_LPAREN, TOK_ID a, TOK_COMMA, TOK_ID b, TOK_RPAREN, TOK_EQUAL, TOK_ID a,"
             " TOK_ADD, TOK_ID b]", NOERR}
        },
        {"unrecognized token", "&", {"[]", EINVAL}},
        {"unrecognized tokens", "~&`", {"[]", EINVAL}},
        {"unrecognized recognized combo", "123 - &4", {"[TOK_INT 123, TOK_SUB]", EINVAL}}
    };
    int num_tests = sizeof(tests) / sizeof(Test), num_passed;

    if (argc == 2 && (strcmp(argv[1], "-v") == 0 
                   || strcmp(argv[1], "--verbose") == 0)) {
        verbose = 1;
        printf(SEP);
        printf("|\n|                        ");
    } else {
        printf("| ");
    }

    printf(C_SUITE_NAME("lexer tests") "\n");
    printf("|\n");

    num_passed = run_all_tests(tests, num_tests);

    printf("|\n| Ran (%d/%d) tests successfully\n", num_passed, num_tests);
    printf("| Test suite %s\n", num_passed == num_tests ? PASSED : FAILED);

    if (verbose) {
        printf("|\n");
        printf(SEP);
    }
    
    return 0;
}

static int run_all_tests(const Test *tests, int num_tests) {
    int i, num_passed = 0;

    for (i = 0; i < num_tests; i++) {
        int t_result;
        pid_t fork_result;

        if (i == 0 && verbose) {
            printf(SEP);
        }

        fflush(stdout);
        fork_result = fork();
        if (fork_result < 0) {
            fprintf(stderr, "Test %d: fork failed\n", i);
            exit(EXIT_FAILURE);
        } else if (fork_result != 0) {
            int status;

            wait(&status);

            if (WIFEXITED(status)) {
                t_result = WEXITSTATUS(status);
            } else {
                t_result = FAILURE;
            }
        } else {
            t_result = run_test(&(tests[i]));
            exit(t_result);
        }

        num_passed += t_result == SUCCESS ? 1 : 0;
        
        if (verbose) {
            printf("|\n");
        }

        printf("| " C_TEST_NAME("%s") " %s\n", tests[i].name,
               t_result == SUCCESS ? PASSED : FAILED);

        if (verbose) {
            printf(SEP);
        }
    }

    return num_passed;
}

static int run_test(const Test *test) {
    TokenList *tok_l;
    char *tok_l_str;
    int errno_before, correct_tok_l, correct_err, t_result;

    if (verbose) {
        printf("| " C_TEST_NAME("%s") " test:\n", test->name);
    }

    compile_regexs();

    errno_before = errno;
    tok_l = tokenize(test->input);
    tok_l_str = token_list_to_str(tok_l);

    if (verbose) {
        printf("| input: \"%s\"\n", test->input);
        printf(SMALL_SEP);
        printf("| errno (before tokenize): %d\n", errno_before);
        printf("| errno (after tokenize):  %d\n", errno);
        printf("| expected errno:          %d\n", test->ans.err);
        printf(SMALL_SEP);
        printf("| tokenize return val: %p\n", (void *) tok_l);
        printf("| token list: %s\n", tok_l_str);
        printf("| expected:   %s\n", test->ans.tok_l);
    }
    
    correct_tok_l = strcmp(tok_l_str, test->ans.tok_l) == 0;
    correct_err = errno == test->ans.err;
    t_result = (correct_tok_l && correct_err) ? SUCCESS : FAILURE;
    
    free(tok_l_str);
    free_token_list(tok_l);
    free_regexs();

    return t_result;
}
