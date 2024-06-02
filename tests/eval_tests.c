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
        "basic addition"
    };
    const char *t_ans[] = {
        "(Int 3)"
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
    
    tree = eval(&(test->input), env);
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
    free_env(env);

    return t_result;
}

static ExprTree *add_node(ExprTree *tree, Expr_t expr, int i, double d, char *id, Operator_t binop, int add_left) {
    ExprTree *new_node = malloc(sizeof(ExprTree));

    new_node->expr = expr;
    switch (expr) {
        case Int:
            new_node->value.i = i;
            break;
        case Float:
            new_node->value.d = d;
            break;
        case ID:
            new_node->value.id = id;
            break;
        case Fun:
            new_node->value.id = id;
            break;
        case Binop:
            new_node->value.binop = binop;
            break;
        default:
            break;
    }

    new_node->left = NULL;
    new_node->right = NULL;

    if (tree == NULL) {
        return new_node;
    }

    if (add_left) {
        tree->left = new_node;
    } else {
        tree->right = new_node;
    }

    return new_node;
}

static ExprTree **create_inputs(int num_tests) {
    ExprTree **inputs = malloc(num_tests * sizeof(ExprTree *));
    ExprTree *t, *t0;

    t0 = add_node(NULL, Binop, 0, 0, NULL, Add, 0);
    add_node(t0, Int, 1, 0, NULL, Add, 1);
    add_node(t0, Int, 2, 0, NULL, Add, 0);
    inputs[0] = t0;

    return inputs;
}
