#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../src/eval.h"
#include "../src/parser.h"
#include "../src/lexer.h"
#include "test.h"

typedef struct {
    ExprTree *tree;
    Env_t *env;
} Input;

typedef struct {
    const char *tree;
    const char *env;
} Ans;

typedef struct {
    const char *name;
    Input *(*create_input)();
    Ans ans;
} Test;

int verbose = 0;
static int run_all_tests(const Test *tests, int num_tests);
static int run_test(const Test *test);
static void free_input(Input *input);
static Input *create_input1();
static Input *create_input2();
static Input *create_input3();
static Input *create_input4();
static Input *create_input5();
static Input *create_input6();
static Input *create_input7();
static Input *create_input8();
static Input *create_input9();
static Input *create_input10();
static Input *create_input11();
static Input *create_input12();
static Input *create_input13();
static Input *create_input14();
static Input *create_input15();
static Input *create_input16();
static Input *create_input17();

int main(int argc, char **argv) {
    Test tests[] = {
        /* 1 + 2 */
        {"basic addition", create_input1, {"(Int 3)", "[]"}},
        /* 44 - 21 + 2.2 */
        {"int float add sub", create_input2, {"(Float 25.200000)", "[]"}},
        /* 2. * 4 / 12 + 5 */
        {"arithmetic mix", create_input3, {"(Float 5.666667)", "[]"}},
        /* 1 + 2 - 3 + 4 - 5 */
        {"long add/sub", create_input4, {"(Int -1)", "[]"}},
        /* 1 * 2 / 3 * 4 / 5 */
        {"long mult/div", create_input5, {"(Float 0.533333)", "[]"}},
        /* R = 500 */
        {"simple assign", create_input6, {"(Assign(ID R)(Int 500))", "[(R : (Int 500))]"}},
        /* circumference = 3.14 * 2 * r where r = 15 in env */
        {
            "arithmetic assign",
            create_input7,
            {"(Assign(ID circumference)(Float 94.200000))", "[(circumference : (Float 94.200000)), (r : (Int 15))]"}
        },
        /* fn f(x, y) = x * y - 0.123456 */
        {
            "function defn",
            create_input8,
            {
                "(Fun f (Param(ID x)(Param(ID y)()))(Sub(Mult(ID x)(ID y))(Float 0.123456)))",
                "[(f : (Fun f (Param(ID x)(Param(ID y)()))(Sub(Mult(ID x)(ID y))(Float 0.123456))))]"
            }
        },
        /* f(42, 0.01) */
        {
            "lexer->parser->function application",
            create_input9,
            {"(Float 0.296544)", "[(f : (Fun f (Param(ID x)(Param(ID y)()))(Sub(Mult(ID x)(ID y))(Float 0.123456))))]"}
        },
        /* num = num + 1 where num = 42 in env */
        {"lexer->parser->reassign", create_input10, {"(Assign(ID num)(Int 43))", "[(num : (Int 43))]"}},
        /* fn area(r) = 3.14 * r*r */
        {
            "lexer->parser->fn defn",
            create_input11,
            {
                "(Fun area (Param(ID r)())(Mult(Mult(Float 3.140000)(ID r))(ID r)))",
                "[(area : (Fun area (Param(ID r)())(Mult(Mult(Float 3.140000)(ID r))(ID r)))), (r : (Int 4))]"
            }
        },
        /* 4 + 3 * (2 - 3) */
        {"lexer->parser->parens arithmetic", create_input12, {"(Int 1)", "[]"}},
        /* var=2*(var+1)/12 where var = 11 in env */
        {"lexer->parser->self reassign", create_input13, {"(Assign(ID var)(Int 2))", "[(var : (Int 2))]"}},
        /* 2^3 */
        {"lexer->parser->simple exponent", create_input14, {"(Int 8)", "[]"}},
        /* (4*2)^(f(23 - 2)^3) where f(x) = x - 21 in env */
        {
            "lexer->parser->complicated exponent",
            create_input15,
            {
                "(Int 1)",
                "[(f : (Fun f (Param(ID x)())(Sub(ID x)(Int 21))))]"
            }
        },
        /* 64^0.5 */
        {"lexer->parser->float exponent", create_input16, {"(Float 8.000000)", "[]"}},
        /* x where x = 5 in env */
        {"lexer->parser->lone variable", create_input17, {"(Int 5)", "[(x : (Int 5))]"}}
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
    Input *input = test->create_input();
    char *tree_str, *input_str, *before_env_str, *after_env_str;
    int correct_tree, correct_env, t_result;

    if (verbose) {
        printf("| " C_TEST_NAME("%s") " test:\n", test->name);
        input_str = expr_tree_to_str(input->tree);
    }
    
    before_env_str = env_to_str(input->env);
    tree = eval(&(input->tree), input->env);
    tree_str = expr_tree_to_str(tree);
    after_env_str = env_to_str(input->env);

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
    free_input(input);

    return t_result;
}

static void free_input(Input *input) {
    free_expr_tree(input->tree);
    free_env(input->env);
    free(input);
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

static void t_extend_env(Env_t *env, const char *id, ExprTree *data) {
    Env_t *new_data;

    new_data = malloc(sizeof(Env_t));
    new_data->id = malloc(strlen(id) + 1);
    strcpy(new_data->id, id);
    new_data->data = data;
    new_data->next = env->next;

    env->next = new_data;
}

static Input *create_input1() {
    Input *input = malloc(sizeof(Input));
    ExprTree *tree = add_node(NULL, Binop, 0, 0, NULL, Add, 0);

    add_node(tree, Int, 1, 0, NULL, Add, 0);
    add_node(tree, Int, 2, 0, NULL, Add, 1);

    input->tree = tree;
    input->env = init_env();
    
    return input;
}

static Input *create_input2() {
    Input *input = malloc(sizeof(Input));
    ExprTree *tree = add_node(NULL, Binop, 0, 0, NULL, Add, 0);
    ExprTree *t;

    t = add_node(tree, Binop, 0, 0, NULL, Sub, 0);
    add_node(tree, Float, 0, 2.2, NULL, Add, 1);
    add_node(t, Int, 44, 0, NULL, Add, 0);
    add_node(t, Int, 21, 0, NULL, Add, 1);

    input->tree = tree;
    input->env = init_env();

    return input;
}

static Input *create_input3() {
    Input *input = malloc(sizeof(Input));
    ExprTree *tree = add_node(NULL, Binop, 0, 0, NULL, Add, 0);
    ExprTree *t, *tt;

    t = add_node(tree, Binop, 0, 0, NULL, Div, 0);
    add_node(tree, Int, 5, 0, NULL, Add, 1);
    tt = add_node(t, Binop, 0, 0, NULL, Mult, 0);
    add_node(t, Int, 12, 0, NULL, Add, 1);
    add_node(tt, Float, 0, 2., NULL, Add, 0);
    add_node(tt, Int, 4, 0, NULL, Add, 1);

    input->tree = tree;
    input->env = init_env();

    return input;
}

static Input *create_input4() {
    Input *input = malloc(sizeof(Input));
    ExprTree *tree = add_node(NULL, Binop, 0, 0, NULL, Sub, 0);
    ExprTree *t, *tt;
    
    t = add_node(tree, Binop, 0, 0, NULL, Add, 0);
    add_node(tree, Int, 5, 0, NULL, Add, 1);
    tt = add_node(t, Binop, 0, 0, NULL, Sub, 0);
    add_node(t, Int, 4, 0, NULL, Add, 1);
    t = add_node(tt, Binop, 0, 0, NULL, Add, 0);
    add_node(tt, Int, 3, 0, NULL, Add, 1);
    add_node(t, Int, 1, 0, NULL, Add, 0);
    add_node(t, Int, 2, 0, NULL, Add, 1);

    input->tree = tree;
    input->env = init_env();

    return input;
}

static Input *create_input5() {
    Input *input = malloc(sizeof(Input));
    ExprTree *tree = add_node(NULL, Binop, 0, 0, NULL, Div, 0);
    ExprTree *t, *tt;
    
    t = add_node(tree, Binop, 0, 0, NULL, Mult, 0);
    add_node(tree, Int, 5, 0, NULL, Add, 1);
    tt = add_node(t, Binop, 0, 0, NULL, Div, 0);
    add_node(t, Int, 4, 0, NULL, Add, 1);
    t = add_node(tt, Binop, 0, 0, NULL, Mult, 0);
    add_node(tt, Int, 3, 0, NULL, Add, 1);
    add_node(t, Int, 1, 0, NULL, Add, 0);
    add_node(t, Int, 2, 0, NULL, Add, 1);

    input->tree = tree;
    input->env = init_env();

    return input;
}

static Input *create_input6() {
    Input *input = malloc(sizeof(Input));
    ExprTree *tree = add_node(NULL, Assign, 0, 0, NULL, Add, 0);
    char *id = malloc(strlen("R") + 1);

    strcpy(id, "R");
    
    add_node(tree, ID, 0, 0, id, Add, 0);
    add_node(tree, Int, 500, 0, NULL, Add, 1);

    input->tree = tree;
    input->env = init_env();

    return input;
}

static Input *create_input7() {
    Input *input = malloc(sizeof(Input));
    ExprTree *tree = add_node(NULL, Assign, 0, 0, NULL, Add, 0);
    ExprTree *t, *tt;
    Env_t *env = init_env();
    char *id = malloc(strlen("circumference") + 1);
    char *id2 = malloc(strlen("r") + 1);
    
    strcpy(id, "circumference");
    strcpy(id2, "r");

    add_node(tree, ID, 0, 0, id, Add, 0);
    t = add_node(tree, Binop, 0, 0, NULL, Mult, 1);
    tt = add_node(t, Binop, 0, 0, NULL, Mult, 0);
    add_node(t, ID, 0, 0, id2, Add, 1);
    add_node(tt, Float, 0, 3.14, NULL, Add, 0);
    add_node(tt, Int, 2, 0, NULL, Add, 1);

    t_extend_env(env, "r", add_node(NULL, Int, 15, 0, NULL, Add, 0));

    input->tree = tree;
    input->env = env;

    return input;
}

static Input *create_input8() {
    Input *input = malloc(sizeof(Input));
    ExprTree *tree;
    ExprTree *t, *tt;
    char *id = malloc(strlen("f") + 1);
    char *id2a = malloc(strlen("x") + 1);
    char *id2b = malloc(strlen("x") + 1);
    char *id3a = malloc(strlen("y") + 1);
    char *id3b = malloc(strlen("y") + 1);

    strcpy(id, "f");
    strcpy(id2a, "x");
    strcpy(id2b, "x");
    strcpy(id3a, "y");
    strcpy(id3b, "y");
    
    tree = add_node(NULL, Fun, 0, 0, id, Add, 0);
    t = add_node(tree, Parameter, 0, 0, NULL, Add, 0);
    add_node(t, ID, 0, 0, id2a, Add, 0);
    tt = add_node(t, Parameter, 0, 0, NULL, Add, 1);
    add_node(tt, ID, 0, 0, id3a, Add, 0);
    t = add_node(tree, Binop, 0, 0, NULL, Sub, 1);
    tt = add_node(t, Binop, 0, 0, NULL, Mult, 0);
    add_node(t, Float, 0, 0.123456, NULL, Add, 1);
    add_node(tt, ID, 0, 0, id2b, Add, 0);
    add_node(tt, ID, 0, 0, id3b, Add, 1);

    input->tree = tree;
    input->env = init_env();

    return input;
}

static Input *create_input9() {
    Input *input = malloc(sizeof(Input));
    TokenList *tok_l = tokenize("f(42, 0.01)"), *tok_l_2;
    ExprTree *tree = parse(tok_l);
    Env_t *env = init_env();

    tok_l_2 = tokenize("fn f(x, y) = x * y - 0.123456");
    t_extend_env(env, "f", parse(tok_l_2));

    input->tree = tree;
    input->env = env;

    free_token_list(tok_l);
    free_token_list(tok_l_2);

    return input;
}

static Input *create_input10() {
    Input *input = malloc(sizeof(Input));
    TokenList *tok_l = tokenize("num = num + 1");
    ExprTree *tree = parse(tok_l);
    Env_t *env = init_env();

    t_extend_env(env, "num", add_node(NULL, Int, 42, 0, NULL, Add, 0));

    input->tree = tree;
    input->env = env;

    free_token_list(tok_l);

    return input;
}

static Input *create_input11() {
    Input *input = malloc(sizeof(Input));
    TokenList *tok_l = tokenize("fn area(r) = 3.14 * r*r");
    ExprTree *tree = parse(tok_l);
    Env_t *env = init_env();

    t_extend_env(env, "r", add_node(NULL, Int, 4, 0, NULL, Add, 0));

    input->tree = tree;
    input->env = env;

    free_token_list(tok_l);

    return input;
}

static Input *create_input12() {
    Input *input = malloc(sizeof(Input));
    TokenList *tok_l = tokenize("4 + 3 * (2 - 3)");
    ExprTree *tree = parse(tok_l);
    Env_t *env = init_env();

    input->tree = tree;
    input->env = env;

    free_token_list(tok_l);

    return input;
}

static Input *create_input13() {
    Input *input = malloc(sizeof(Input));
    TokenList *tok_l = tokenize("var = 2*(var+1)/12");
    ExprTree *tree = parse(tok_l);
    Env_t *env = init_env();

    t_extend_env(env, "var", add_node(NULL, Int, 11, 0, NULL, Add, 0));

    input->tree = tree;
    input->env = env;

    free_token_list(tok_l);

    return input;
}

static Input *create_input14() {
    Input *input = malloc(sizeof(Input));
    TokenList *tok_l = tokenize("2^3");
    ExprTree *tree = parse(tok_l);
    Env_t *env = init_env();

    input->tree = tree;
    input->env = env;

    free_token_list(tok_l);

    return input;
}

static Input *create_input15() {
    Input *input = malloc(sizeof(Input));
    TokenList *tok_l = tokenize("(4*2)^(f(23 - 2)^3)"), *tok_l_2;
    ExprTree *tree = parse(tok_l);
    Env_t *env = init_env();
    
    tok_l_2 = tokenize("fn f(x) = x - 21");
    t_extend_env(env, "f", parse(tok_l_2));

    input->tree = tree;
    input->env = env;

    free_token_list(tok_l);
    free_token_list(tok_l_2);

    return input;
}

static Input *create_input16() {
    Input *input = malloc(sizeof(Input));
    TokenList *tok_l = tokenize("64^0.5");
    ExprTree *tree = parse(tok_l);
    Env_t *env = init_env();

    input->tree = tree;
    input->env = env;

    free_token_list(tok_l);

    return input;
}

static Input *create_input17() {
    Input *input = malloc(sizeof(Input));
    TokenList *tok_l = tokenize("x");
    ExprTree *tree = parse(tok_l);
    Env_t *env = init_env();

    t_extend_env(env, "x", add_node(NULL, Int, 5, 0, NULL, Add, 0));

    input->tree = tree;
    input->env = env;

    free_token_list(tok_l);

    return input;
}
