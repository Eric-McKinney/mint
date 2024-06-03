#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/eval.h"
#include "../src/parser.h"
#include "../src/lexer.h"
#include "test.h"

typedef struct {
    const char *name;
    struct {
        ExprTree *tree;
        Env_t *env;
    } input;
    struct {
        const char *tree;
        const char *env;
    } ans;
} Test;

int verbose = 0;
static int run_test(Test *test);
static ExprTree **create_trees(int num_tests);
static Env_t **create_envs(int num_tests);

int main(int argc, char **argv) {
    const char *t_names[] = {
        "basic addition",         /* 1 + 2 */
        "int float add sub",      /* 44 - 21 + 2.2 */
        "arithmetic mix",         /* 2. * 4 / 12 + 5 */
        "long add/sub",           /* 1 + 2 - 3 + 4 - 5 */
        "long mult/div",          /* 1 * 2 / 3 * 4 / 5 */
        "simple assign",          /* R = 500 */
        "arithmetic assign",      /* circumference = 3.14 * 2 * r (where r = 15 in env)*/
    };
    const char *tree_ans[] = {
        "(Int 3)",
        "(Float 25.200000)",
        "(Float 5.666667)",
        "(Int -1)",
        "(Float 0.533333)",
        "(Assign(ID R)(Int 500))",
        "(Assign(ID circumference)(Float 94.200000))"
    };
    const char *env_ans[] = {
        "[]",
        "[]",
        "[]",
        "[]",
        "[]",
        "[(R : (Int 500))]",
        "[(circumference : (Float 94.200000)), (r : (Int 15))]"
    };
    int num_tests = sizeof(t_names) / sizeof(char *), i, num_passed = 0;
    ExprTree **input_trees = create_trees(num_tests);
    Env_t **input_envs = create_envs(num_tests);

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
        t.input.tree = input_trees[i];
        t.input.env = input_envs[i];
        t.ans.tree = tree_ans[i];
        t.ans.env = env_ans[i];

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

    free(input_trees);
    free(input_envs);

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
    char *tree_str, *input_str, *before_env_str, *after_env_str;
    int correct_tree, correct_env, t_result;

    if (verbose) {
        printf("| " C_TEST_NAME("%s") " test:\n", test->name);
        input_str = expr_tree_to_str(test->input.tree);
    }
    
    before_env_str = env_to_str(test->input.env);
    tree = eval(&(test->input.tree), test->input.env);
    tree_str = expr_tree_to_str(tree);
    after_env_str = env_to_str(test->input.env);

    if (verbose) {
        printf("| input: %s\n", input_str);
        printf("| env (before eval): %s\n", before_env_str);
        printf("| eval return val: %p\n", (void *) tree);
        printf("| env (after eval): %s\n", after_env_str);
        printf("| expected env:     %s\n", test->ans.env);
        printf("| evaluated tree: %s\n", tree_str);
        printf("| expected tree:  %s\n", test->ans.tree);

        free(input_str);
    }

    correct_tree = strcmp(tree_str, test->ans.tree) == 0;
    correct_env = strcmp(after_env_str, test->ans.env) == 0;
    t_result = correct_tree && correct_env ? SUCCESS : FAILURE;

    free(tree_str);
    free(before_env_str);
    free(after_env_str);
    free_expr_tree(tree);
    free_env(test->input.env);

    return t_result;
}

static ExprTree *add_node(ExprTree *tree, Expr_t expr, int i, double d, char *id, Operator_t binop, int add_right) {
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

    if (add_right) {
        tree->right = new_node;
    } else {
        tree->left = new_node;
    }

    return new_node;
}

static void extend_env(Env_t *env, const char *id, ExprTree *data) {
    Env_t *new_data;

    new_data = malloc(sizeof(Env_t));
    new_data->id = malloc(strlen(id) + 1);
    strcpy(new_data->id, id);
    new_data->data = data;
    new_data->next = env->next;

    env->next = new_data;
}

static Env_t **create_envs(int num_tests) {
    Env_t **envs = malloc(num_tests * sizeof(Env_t *));
    Env_t *e6;
    
    envs[0] = init_env();
    envs[1] = init_env();
    envs[2] = init_env();
    envs[3] = init_env();
    envs[4] = init_env();
    envs[5] = init_env();

    e6 = init_env();
    extend_env(e6, "r", add_node(NULL, Int, 15, 0, NULL, Add, 0));
    envs[6] = e6;

    return envs;
}

static ExprTree **create_trees(int num_tests) {
    ExprTree **trees = malloc(num_tests * sizeof(ExprTree *));
    ExprTree *t, *tt, *t0, *t1, *t2, *t3, *t4, *t5, *t6;

    t0 = add_node(NULL, Binop, 0, 0, NULL, Add, 0);
    add_node(t0, Int, 1, 0, NULL, Add, 0);
    add_node(t0, Int, 2, 0, NULL, Add, 1);
    trees[0] = t0;

    t1 = add_node(NULL, Binop, 0, 0, NULL, Add, 0);
    t = add_node(t1, Binop, 0, 0, NULL, Sub, 0);
    add_node(t1, Float, 0, 2.2, NULL, Add, 1);
    add_node(t, Int, 44, 0, NULL, Add, 0);
    add_node(t, Int, 21, 0, NULL, Add, 1);
    trees[1] = t1;

    t2 = add_node(NULL, Binop, 0, 0, NULL, Add, 0);
    t = add_node(t2, Binop, 0, 0, NULL, Div, 0);
    add_node(t2, Int, 5, 0, NULL, Add, 1);
    tt = add_node(t, Binop, 0, 0, NULL, Mult, 0);
    add_node(t, Int, 12, 0, NULL, Add, 1);
    add_node(tt, Float, 0, 2., NULL, Add, 0);
    add_node(tt, Int, 4, 0, NULL, Add, 1);
    trees[2] = t2;

    t3 = add_node(NULL, Binop, 0, 0, NULL, Sub, 0);
    t = add_node(t3, Binop, 0, 0, NULL, Add, 0);
    add_node(t3, Int, 5, 0, NULL, Add, 1);
    tt = add_node(t, Binop, 0, 0, NULL, Sub, 0);
    add_node(t, Int, 4, 0, NULL, Add, 1);
    t = add_node(tt, Binop, 0, 0, NULL, Add, 0);
    add_node(tt, Int, 3, 0, NULL, Add, 1);
    add_node(t, Int, 1, 0, NULL, Add, 0);
    add_node(t, Int, 2, 0, NULL, Add, 1);
    trees[3] = t3;

    t4 = add_node(NULL, Binop, 0, 0, NULL, Div, 0);
    t = add_node(t4, Binop, 0, 0, NULL, Mult, 0);
    add_node(t4, Int, 5, 0, NULL, Add, 1);
    tt = add_node(t, Binop, 0, 0, NULL, Div, 0);
    add_node(t, Int, 4, 0, NULL, Add, 1);
    t = add_node(tt, Binop, 0, 0, NULL, Mult, 0);
    add_node(tt, Int, 3, 0, NULL, Add, 1);
    add_node(t, Int, 1, 0, NULL, Add, 0);
    add_node(t, Int, 2, 0, NULL, Add, 1);
    trees[4] = t4;

    {
    char *id = malloc(strlen("R") + 1);
    strcpy(id, "R");

    t5 = add_node(NULL, Assign, 0, 0, NULL, Add, 0);
    add_node(t5, ID, 0, 0, id, Add, 0);
    add_node(t5, Int, 500, 0, NULL, Add, 1);
    trees[5] = t5;
    }

    {
    char *id = malloc(strlen("circumference") + 1);
    char *id2 = malloc(strlen("r") + 1);
    strcpy(id, "circumference");
    strcpy(id2, "r");

    t6 = add_node(NULL, Assign, 0, 0, NULL, Add, 0);
    add_node(t6, ID, 0, 0, id, Add, 0);
    t = add_node(t6, Binop, 0, 0, NULL, Mult, 1);
    tt = add_node(t, Binop, 0, 0, NULL, Mult, 0);
    add_node(t, ID, 0, 0, id2, Add, 1);
    add_node(tt, Float, 0, 3.14, NULL, Add, 0);
    add_node(tt, Int, 2, 0, NULL, Add, 1);
    trees[6] = t6;
    }

    return trees;
}
