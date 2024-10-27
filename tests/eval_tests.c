#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "../src/eval.h"
#include "../src/parser.h"
#include "../src/lexer.h"
#include "test.h"

#define MAX_TEST_ENV_SIZE 5
#define NOERR 0

typedef struct {
    ExprTree *tree;
    Env_t *env;
} Input;

typedef struct {
    const char *expr_str;
    const char *env_ids[MAX_TEST_ENV_SIZE];
    const char *env_raw_vals[MAX_TEST_ENV_SIZE];
} Raw_Input;

typedef struct {
    const char *tree;
    const char *env;
    int err;
} Ans;

typedef struct {
    const char *name;
    Raw_Input raw_input;
    Ans ans;
} Test;

int verbose = 0;
static int run_all_tests(const Test *tests, int num_tests);
static int run_test(const Test *test);
static Input *create_input(const Raw_Input *raw_input);
static void free_input(Input *input);

int main(int argc, char **argv) {
    Test tests[] = {
        {"basic addition", {"1 + 2", {NULL}, {NULL}}, {"(Int 3)", "[]", NOERR}},
        {"int float add sub", {"44 - 21 + 2.2", {NULL}, {NULL}}, {"(Float 25.200000)", "[]", NOERR}},
        {"arithmetic mix", {"2. * 4 / 12 + 5", {NULL}, {NULL}}, {"(Float 5.666667)", "[]", NOERR}},
        {"long add/sub", {"1 + 2 - 3 + 4 - 5", {NULL}, {NULL}}, {"(Int -1)", "[]", NOERR}},
        {"long mult/div", {"1 * 2 / 3 * 4 / 5", {NULL}, {NULL}}, {"(Float 0.533333)", "[]", NOERR}},
        {"simple assign", {"R = 500", {NULL}, {NULL}}, {"(Assign(ID R)(Int 500))", "[(R : (Int 500))]", NOERR}},
        {
            "arithmetic assign",
            {"circumference = 3.14 * 2 * r", {"r", NULL}, {"15", NULL}},
            {
                "(Assign(ID circumference)(Float 94.200000))",
                "[(circumference : (Float 94.200000)), (r : (Int 15))]",
                NOERR
            }
        },
        {
            "function defn",
            {"fn f(x, y) = x * y - 0.123456", {NULL}, {NULL}},
            {
                "(Fun f (Param(ID x)(Param(ID y)()))(Sub(Mult(ID x)(ID y))(Float 0.123456)))",
                "[(f : (Fun f (Param(ID x)(Param(ID y)()))(Sub(Mult(ID x)(ID y))(Float 0.123456))))]",
                NOERR
            }
        },
        {
            "function application",
            {"f(42, 0.01)", {"f", NULL}, {"fn f(x, y) = x * y - 0.123456", NULL}},
            {
                "(Float 0.296544)",
                "[(f : (Fun f (Param(ID x)(Param(ID y)()))(Sub(Mult(ID x)(ID y))(Float 0.123456))))]",
                NOERR
            }
        },
        {
            "reassign",
            {"num = num + 1", {"num", NULL}, {"42", NULL}},
            {"(Assign(ID num)(Int 43))", "[(num : (Int 43))]", NOERR}
        },
        {
            "function defn w/shadowed variable",
            {"fn area(r) = 3.14 * r*r", {"r", NULL}, {"4", NULL}},
            {
                "(Fun area (Param(ID r)())(Mult(Mult(Float 3.140000)(ID r))(ID r)))",
                "[(area : (Fun area (Param(ID r)())(Mult(Mult(Float 3.140000)(ID r))(ID r)))), (r : (Int 4))]",
                NOERR
            }
        },
        {"parens arithmetic", {"4 + 3 * (2 - 3)", {NULL}, {NULL}}, {"(Int 1)", "[]", NOERR}},
        {
            "self reassign",
            {"var=2*(var+1)/12", {"var", NULL}, {"11", NULL}},
            {"(Assign(ID var)(Int 2))", "[(var : (Int 2))]", NOERR}
        },
        {"simple exponent", {"2^3", {NULL}, {NULL}}, {"(Int 8)", "[]", NOERR}},
        {
            "complicated exponent",
            {"(4*2)^(f(23 - 2)^3)", {"f", NULL}, {"fn f(x) = x - 21", NULL}},
            {
                "(Int 1)",
                "[(f : (Fun f (Param(ID x)())(Sub(ID x)(Int 21))))]",
                NOERR
            }
        },
        {"float exponent", {"64^0.5", {NULL}, {NULL}}, {"(Float 8.000000)", "[]", NOERR}},
        {"lone variable", {"x", {"x", NULL}, {"5", NULL}}, {"(Int 5)", "[(x : (Int 5))]", NOERR}},
        {"lone undefined variable", {"x", {NULL}, {NULL}}, {"()", "[]", EINVAL}},
        {
            "expression w/undefined variable",
            {"3 - 2*x - y", {"x", NULL}, {"3", NULL}},
            {"(Sub(Int -3)())", "[(x : (Int 3))]", EINVAL}
        },
        {
            "function defn w/undefined variable",
            {"fn f(x) = x - y", {NULL}, {NULL}},
            {"(Fun f (Param(ID x)())(Sub(ID x)(ID y)))", "[]", EINVAL}
        },
        {
            "function defn w/param same as function name",
            {"fn f(f) = 5*f", {NULL}, {NULL}},
            {"(Fun f (Param(ID f)())(Mult(Int 5)(ID f)))", "[]", EINVAL}
        }
    };
    int num_tests = sizeof(tests) / sizeof(Test), num_passed, suite_result;

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

    compile_regexs();
    num_passed = run_all_tests(tests, num_tests);
    free_regexs();
    suite_result = num_passed == num_tests ? SUCCESS : FAILURE;

    printf("|\n| Ran (%d/%d) tests successfully\n", num_passed, num_tests);
    printf("| Test suite %s\n", suite_result == SUCCESS ? PASSED : FAILED);

    if (verbose) {
        printf("|\n");
        printf(SEP);
    }

    return suite_result;
}

static int run_all_tests(const Test *tests, int num_tests) {
    int num_passed = 0, i;

    for (i = 0; i < num_tests; i++) {
        int t_result;
        pid_t fork_result;

        if (i == 0 && verbose) {
            printf(SEP);
        }

        /* flush stdout because IO buffer is copied by fork */
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
    ExprTree *tree;
    Input *input = create_input(&(test->raw_input));
    char *tree_str, *input_str, *before_env_str, *after_env_str;
    int errno_before, correct_tree, correct_env, correct_err, t_result;

    if (verbose) {
        printf("| " C_TEST_NAME("%s") " test:\n", test->name);
        input_str = expr_tree_to_str(input->tree);
    }
    
    errno_before = errno;
    before_env_str = env_to_str(input->env);
    tree = eval(&(input->tree), input->env);
    tree_str = expr_tree_to_str(tree);
    after_env_str = env_to_str(input->env);

    if (verbose) {
        printf("| input: %s\n", input_str);
        printf("|--------------------\n");
        printf("| env (before eval): %s\n", before_env_str);
        printf("| env (after eval):  %s\n", after_env_str);
        printf("| expected env:      %s\n", test->ans.env);
        printf("|--------------------\n");
        printf("| errno (before eval): %d\n", errno_before);
        printf("| errno (after eval):  %d\n", errno);
        printf("| expected errno:      %d\n", test->ans.err);
        printf("|--------------------\n");
        printf("| eval return val: %p\n", (void *) tree);
        printf("| evaluated tree:  %s\n", tree_str);
        printf("| expected tree:   %s\n", test->ans.tree);

        free(input_str);
    }

    correct_tree = strcmp(tree_str, test->ans.tree) == 0;
    correct_env = strcmp(after_env_str, test->ans.env) == 0;
    correct_err = errno == test->ans.err;
    t_result = (correct_tree && correct_env && correct_err) ? SUCCESS : FAILURE;

    free(tree_str);
    free(before_env_str);
    free(after_env_str);
    free_input(input);

    return t_result;
}

static void free_input(Input *input) {
    free_expr_tree(input->tree);
    free_env(input->env);
    free(input);
}

/* extend_env in eval.c is static and I don't want to expose it just for this test suite */
static void t_extend_env(Env_t *env, const char *id, ExprTree *data) {
    Env_t *new_data;

    new_data = malloc(sizeof(Env_t));
    new_data->id = malloc(strlen(id) + 1);
    strcpy(new_data->id, id);
    new_data->data = data;
    new_data->next = env->next;

    env->next = new_data;
}

static Input *create_input(const Raw_Input *raw_input) {
    const char * const *env_ids = raw_input->env_ids;
    const char * const *env_raw_vals = raw_input->env_raw_vals;
    Input *input = malloc(sizeof(Input));
    TokenList *tok_l = tokenize(raw_input->expr_str);
    ExprTree *tree = parse(tok_l);
    Env_t *env = init_env();
    int i = 0;

    free_token_list(tok_l);

    while (env_raw_vals[i] != NULL && env_ids[i] != NULL) {
        TokenList *e_tok_l = tokenize(env_raw_vals[i]);
        ExprTree *e_tree = parse(e_tok_l);

        t_extend_env(env, env_ids[i], e_tree);

        free_token_list(e_tok_l);
        i++;
    }

    input->tree = tree;
    input->env = env;

    return input;
}

