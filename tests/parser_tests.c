#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "../src/parser.h"
#include "../src/lexer.h"
#include "test.h"

typedef struct {
    const char *tree;
    int err;
} Ans;

typedef struct {
    const char *name;
    const char *raw_input;
    Ans ans;
} Test;

int verbose = 0;
static int run_all_tests(const Test *tests, int num_tests);
static int run_test(const Test *test);

int main(int argc, char **argv) {
    Test tests[] = {
        {"basic addition", "1 + 2", {"(Add(Int 1)(Int 2))", NOERR}},
        {"int float add sub", "44 - 21 + 2.2", {"(Add(Sub(Int 44)(Int 21))(Float 2.200000))", NOERR}},
        {"arithmetic mix", "2. * 4 / 12 + 5", {"(Add(Div(Mult(Float 2.000000)(Int 4))(Int 12))(Int 5))", NOERR}},
        {"long add/sub", "1 + 2 - 3 + 4 - 5", {"(Sub(Add(Sub(Add(Int 1)(Int 2))(Int 3))(Int 4))(Int 5))", NOERR}},
        {"long mult/div", "1 * 2 / 3 * 4 / 5", {"(Div(Mult(Div(Mult(Int 1)(Int 2))(Int 3))(Int 4))(Int 5))", NOERR}},
        {"simple assign", "R = 500", {"(Assign(ID R)(Int 500))", NOERR}},
        {
            "arithmetic assign",
            "circumference = 3.14 * 2 * r",
            {"(Assign(ID circumference)(Mult(Mult(Float 3.140000)(Int 2))(ID r)))", NOERR}
        },
        {
            "function defn",
            "fn f(x, y) = x * y - 0.123456",
            {"(Fun f (Param(ID x)(Param(ID y)()))(Sub(Mult(ID x)(ID y))(Float 0.123456)))", NOERR}
        },
        {"function application", "f(42, 0.01)", {"(App(ID f)(Arg(Int 42)(Arg(Float 0.010000)())))", NOERR}},
        {
            "fn defn",
            "fn area(r) = 3.14 * r*r",
            {"(Fun area (Param(ID r)())(Mult(Mult(Float 3.140000)(ID r))(ID r)))", NOERR}
        },
        {"parens", "4 + 3 * (2 - 3)", {"(Add(Int 4)(Mult(Int 3)(Sub(Int 2)(Int 3))))", NOERR}},
        {
            "arithmetic assign",
            "var=2*(var+1)/12",
            {"(Assign(ID var)(Div(Mult(Int 2)(Add(ID var)(Int 1)))(Int 12)))", NOERR}
        },
        {
            "function application",
            "my_fun(0,arg, a, b, 2.0)*3",
            {
                "(Mult(App(ID my_fun)(Arg(Int 0)(Arg(ID arg)(Arg(ID a)(Arg(ID b)(Arg(Float 2.000000)()))))))(Int 3))",
                NOERR
            }
        },
        {"basic exponent", "2^3", {"(Exp(Int 2)(Int 3))", NOERR}},
        {
            "complicated exponent",
            "(4*2)^(f(23 - 2)^3)",
            {"(Exp(Mult(Int 4)(Int 2))(Exp(App(ID f)(Arg(Sub(Int 23)(Int 2))()))(Int 3)))", NOERR}
        },
        {"comment", "# a comment", {"()", NOERR}},
        {"math + comment", "1 + 1 # a comment", {"(Add(Int 1)(Int 1))", NOERR}},
        {"function definition w/no params", "fn f() = 4", {"()", EINVAL}},
        {"function application w/no args", "f()", {"(App(ID f)())", EINVAL}}
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
    
    printf(C_SUITE_NAME("parser tests") "\n");
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
    int num_passed = 0, i;

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
    TokenList *input;
    ExprTree *tree;
    char *tree_str, *input_str;
    int errno_before, correct_tree, correct_err, t_result;

    compile_regexs();
    input = tokenize(test->raw_input);
    free_regexs();

    if (verbose) {
        printf("| " C_TEST_NAME("%s") " test:\n", test->name);
        input_str = token_list_to_str(input);
    }

    errno_before = errno;
    tree = parse(input);
    tree_str = expr_tree_to_str(tree);

    if (verbose) {
        printf("| raw string input: %s\n", test->raw_input);
        printf("| token list input: %s\n", input_str);
        printf(SMALL_SEP);
        printf("| errno (before parse): %d\n", errno_before);
        printf("| errno (after parse):  %d\n", errno);
        printf("| expected errno:       %d\n", test->ans.err);
        printf(SMALL_SEP);
        printf("| parse return val: %p\n", (void *) tree);
        printf("| parse tree:    %s\n", tree_str);
        printf("| expected tree: %s\n", test->ans.tree);

        free(input_str);
    }

    correct_tree = strcmp(tree_str, test->ans.tree) == 0;
    correct_err = errno == test->ans.err;
    t_result = (correct_tree && correct_err) ? SUCCESS : FAILURE;

    free(tree_str);
    free_expr_tree(tree);
    free_token_list(input);

    return t_result;
}
