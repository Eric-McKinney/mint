#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/parser.h"
#include "../src/lexer.h"

#define SUCCESS 1
#define FAILURE 0
#define PASSED "\033[1;32mPASSED\033[0m"
#define FAILED "\033[1;31mFAILED\033[0m"
#define SEP "|------------------------------------------------------------|\n"

typedef struct {
    char name[80];
    TokenList input;
    char ans[1024];
} Test;

int verbose = 0;
int run_test(Test *test);

int main(int argc, char **argv) {
    Test tests[] = {
        {"basic_addition", {TOK_INT, }, "(+(1)(2))"}
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

int run_test(Test *test) {
    ExprTree *tree;
    TokenList *input = &(test->input);
    char *tree_str;
    int t_result;

    if (verbose) {
        printf("| \033[0;36m%s\033[0m test:\n", test->name);
    }

    tree = parse(input);
    tree_str = expr_tree_to_str(tree);

    if (verbose) {
        char *input_str = token_list_to_str(input);
        printf("| input: \"%s\"\n", input_str);
        printf("| parse return val: %p\n", (void *) tree);
        printf("| parse tree: %s\n", tree_str);
        printf("| expected: %s\n", test->ans);

        free(input_str);
    }

    t_result = strcmp(tree_str, test->ans) == 0 ? SUCCESS : FAILURE;

    free(tree_str);
    free_expr_tree(tree);

    return t_result;
}
