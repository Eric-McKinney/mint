#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/eval.h"
#include "../src/parser.h"
#include "../src/lexer.h"
#include "test.h"

typedef struct {
    const char *name;
    ExprTree *input;
    const char *ans;
} Test;

int verbose = 0;
static int run_test(Test *test);
static ExprTree **create_inputs(int num_tests);

int main(int argc, char **argv) {
    const char *t_names[] = {
        ""
    };
    const char *t_ans[] = {
        ""
    };
    int num_tests = sizeof(t_names) / sizeof(char *), i, num_passed = 0;
    ExprTree **inputs = create_inputs(num_tests);

    if (argc == 2 && (strcmp(argv[1], "-v") == 0
                   || strcmp(argv[1], "--verbose") == 0)) {
        verbose = 1;
        printf(SEP);
        printf("|\n|                        ");
    } else {
        printf("| ");
    }

    printf(C_SUITE_NAME("eval tests") "\n");
    printf("|\n");

    for (i = 0; i < num_tests; i++) {
        Test t;
        int t_result;

        t.name = t_names[i];
        t.input = inputs[i];
        t.ans = t_ans[i];

        if (i == 0 && verbose) {
            printf(SEP);
        }

        t_result = run_test(&t);
        num_passed += t_result == SUCCESS ? 1 : 0;

        if (verbose) {
            printf("|\n");
        }

        printf("| " C_TEST_NAME("%s") " %s\n", t.name,
               t_result == SUCCESS ? PASSED : FAILED);

        if (verbose) {
            printf(SEP);
        }
    }

    free(inputs);

    printf("|\n| Ran (%d/%d) tests successfully\n", num_passed, num_tests);
    printf("| Test suite %s\n", num_passed == num_tests ? PASSED : FAILED);

    if (verbose) {
        printf("|\n");
        printf(SEP);
    }

    return 0;
}

static int run_test(Test *test) {
    ExprTree *tree;
    Env_t *env = init_env();
    char *tree_str, *before_input_str;
    int t_result;

    if (verbose) {
        printf("| " C_TEST_NAME("%s") " test:\n", test->name);
        before_input_str = expr_tree_to_str(test->input);
    }
    
    tree = eval(test->input, env);
    tree_str = expr_tree_to_str(tree);

    if (verbose) {
        char *after_input_str = expr_tree_to_str(test->input);
        printf("| input (before eval): %s\n", before_input_str);
        printf("| input (after eval):  %s\n", after_input_str);
        printf("| eval return val: %p\n", (void *) tree);
        printf("| evaluated tree: %s\n", tree_str);
        printf("| expected:       %s\n", test->ans);

        free(before_input_str);
        free(after_input_str);
    }

    t_result = strcmp(tree_str, test->ans) == 0 ? SUCCESS : FAILURE;

    free(tree_str);
    free_expr_tree(tree);
    free_expr_tree(test->input);

    return t_result;
}

static ExprTree **create_inputs(int num_tests) {return NULL;}
