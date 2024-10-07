#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/parser.h"
#include "../src/lexer.h"
#include "test.h"

typedef struct {
    const char *name;
    TokenList *input;
    const char *ans;
} Test;

int verbose = 0;
static int run_test(Test *test);
static TokenList **create_inputs(int num_tests);

int main(int argc, char **argv) {
    const char *t_names[] = {
        "basic addition",              /* 1 + 2 */
        "int float add sub",           /* 44 - 21 + 2.2 */
        "arithmetic mix",              /* 2. * 4 / 12 + 5 */
        "long add/sub",                /* 1 + 2 - 3 + 4 - 5 */
        "long mult/div",               /* 1 * 2 / 3 * 4 / 5 */
        "simple assign",               /* R = 500 */
        "arithmetic assign",           /* circumference = 3.14 * 2 * r */
        "function defn",               /* fn f(x, y) = x * y - 0.123456 */
        "function application",        /* f(42, 0.01) */
        "lexer->fn defn",              /* fn area(r) = 3.14 * r*r */
        "lexer->parens",               /* 4 + 3 * (2 - 3) */
        "lexer->arithmetic assign",    /* var=2*(var+1)/12 */
        "lexer->function application", /* my_fun(0,arg, a, b, 2.0)*3 */
        "lexer->basic exponent",       /* 2^3 */
        "lexer->complicated exponent", /* (4*2)^(f(23 - 2)^3) */
        "lexer->comment",              /* # a comment */
        "lexer->math + comment"        /* 1 + 1 # a comment */
    };
    const char *t_ans[] = {
        "(Add(Int 1)(Int 2))",
        "(Add(Sub(Int 44)(Int 21))(Float 2.200000))",
        "(Add(Div(Mult(Float 2.000000)(Int 4))(Int 12))(Int 5))",
        "(Sub(Add(Sub(Add(Int 1)(Int 2))(Int 3))(Int 4))(Int 5))",
        "(Div(Mult(Div(Mult(Int 1)(Int 2))(Int 3))(Int 4))(Int 5))",
        "(Assign(ID R)(Int 500))",
        "(Assign(ID circumference)(Mult(Mult(Float 3.140000)(Int 2))(ID r)))",
        "(Fun f (Param(ID x)(Param(ID y)()))(Sub(Mult(ID x)(ID y))(Float 0.123456)))",
        "(App(ID f)(Arg(Int 42)(Arg(Float 0.010000)())))",
        "(Fun area (Param(ID r)())(Mult(Mult(Float 3.140000)(ID r))(ID r)))",
        "(Add(Int 4)(Mult(Int 3)(Sub(Int 2)(Int 3))))",
        "(Assign(ID var)(Div(Mult(Int 2)(Add(ID var)(Int 1)))(Int 12)))",
        "(Mult(App(ID my_fun)(Arg(Int 0)(Arg(ID arg)(Arg(ID a)(Arg(ID b)(Arg(Float 2.000000)()))))))(Int 3))",
        "(Exp(Int 2)(Int 3))",
        "(Exp(Mult(Int 4)(Int 2))(Exp(App(ID f)(Arg(Sub(Int 23)(Int 2))()))(Int 3)))",
        "()",
        "(Add(Int 1)(Int 1))"
    };
    int num_tests = sizeof(t_names) / sizeof(char *), i, num_passed = 0;
    TokenList **inputs;

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

    compile_regexs();
    inputs = create_inputs(num_tests);
    free_regexs();

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
    char *tree_str;
    int t_result;

    if (verbose) {
        printf("| " C_TEST_NAME("%s") " test:\n", test->name);
    }

    tree = parse(test->input);
    tree_str = expr_tree_to_str(tree);

    if (verbose) {
        char *input_str = token_list_to_str(test->input);
        printf("| input: %s\n", input_str);
        printf("| parse return val: %p\n", (void *) tree);
        printf("| parse tree: %s\n", tree_str);
        printf("| expected:   %s\n", test->ans);

        free(input_str);
    }

    t_result = strcmp(tree_str, test->ans) == 0 ? SUCCESS : FAILURE;

    free(tree_str);
    free_expr_tree(tree);
    free_token_list(test->input);

    return t_result;
}

static TokenList *append_token(TokenList *tok_l, Tok_t tok, int i, double d, char *id) {
    TokenList *new_node = malloc(sizeof(TokenList));

    new_node->token = tok;
    switch (tok) {
        case TOK_INT:
            new_node->value.i = i;
            break;
        case TOK_FLOAT:
            new_node->value.d = d;
            break;
        case TOK_ID:
            new_node->value.id = id;
            break;
        default:
            break;
    }
    new_node->next = NULL;

    if (tok_l == NULL) {
        return new_node;
    }

    while (tok_l->next != NULL) {
        tok_l = tok_l->next;
    }

    tok_l->next = new_node;

    return new_node;
}

static TokenList **create_inputs(int num_tests) {
    TokenList **inputs = malloc(num_tests * sizeof(TokenList *));
    TokenList *t0, *t1, *t2, *t3, *t4, *t5, *t6, *t7, *t8;

    t0 = append_token(NULL, TOK_INT, 1, 0, NULL);
    append_token(t0, TOK_ADD, 0, 0, NULL);
    append_token(t0, TOK_INT, 2, 0, NULL);
    inputs[0] = t0;

    t1 = append_token(NULL, TOK_INT, 44, 0, NULL);
    append_token(t1, TOK_SUB, 0, 0, NULL);
    append_token(t1, TOK_INT, 21, 0, NULL);
    append_token(t1, TOK_ADD, 0, 0, NULL);
    append_token(t1, TOK_FLOAT, 0, 2.2, NULL);
    inputs[1] = t1;

    t2 = append_token(NULL, TOK_FLOAT, 0, 2., NULL);
    append_token(t2, TOK_MULT, 0, 0, NULL);
    append_token(t2, TOK_INT, 4, 0, NULL);
    append_token(t2, TOK_DIV, 0, 0, NULL);
    append_token(t2, TOK_INT, 12, 0, NULL);
    append_token(t2, TOK_ADD, 0, 0, NULL);
    append_token(t2, TOK_INT, 5, 0, NULL);
    inputs[2] = t2;

    t3 = append_token(NULL, TOK_INT, 1, 0, NULL);
    append_token(t3, TOK_ADD, 0, 0, NULL);
    append_token(t3, TOK_INT, 2, 0, NULL);
    append_token(t3, TOK_SUB, 0, 0, NULL);
    append_token(t3, TOK_INT, 3, 0, NULL);
    append_token(t3, TOK_ADD, 0, 0, NULL);
    append_token(t3, TOK_INT, 4, 0, NULL);
    append_token(t3, TOK_SUB, 0, 0, NULL);
    append_token(t3, TOK_INT, 5, 0, NULL);
    inputs[3] = t3;

    t4 = append_token(NULL, TOK_INT, 1, 0, NULL);
    append_token(t4, TOK_MULT, 0, 0, NULL);
    append_token(t4, TOK_INT, 2, 0, NULL);
    append_token(t4, TOK_DIV, 0, 0, NULL);
    append_token(t4, TOK_INT, 3, 0, NULL);
    append_token(t4, TOK_MULT, 0, 0, NULL);
    append_token(t4, TOK_INT, 4, 0, NULL);
    append_token(t4, TOK_DIV, 0, 0, NULL);
    append_token(t4, TOK_INT, 5, 0, NULL);
    inputs[4] = t4;

    {
    char *id = malloc(strlen("R") + 1);
    strcpy(id, "R");

    t5 = append_token(NULL, TOK_ID, 0, 0, id);
    append_token(t5, TOK_EQUAL, 0, 0, NULL);
    append_token(t5, TOK_INT, 500, 0, NULL);
    inputs[5] = t5;
    }

    {
    char *id = malloc(strlen("circumference") + 1);
    char *id2 = malloc(strlen("r") + 1);
    strcpy(id, "circumference");
    strcpy(id2, "r");
    
    t6 = append_token(NULL, TOK_ID, 0, 0, id);
    append_token(t6, TOK_EQUAL, 0, 0, NULL);
    append_token(t6, TOK_FLOAT, 0, 3.14, NULL);
    append_token(t6, TOK_MULT, 0, 0, NULL);
    append_token(t6, TOK_INT, 2, 0, NULL);
    append_token(t6, TOK_MULT, 0, 0, NULL);
    append_token(t6, TOK_ID, 0, 0, id2);
    inputs[6] = t6;
    }

    {
    char *id = malloc(strlen("f") + 1);
    char *id2 = malloc(strlen("x") + 1);
    char *id3 = malloc(strlen("y") + 1);
    char *id4 = malloc(strlen("x") + 1);
    char *id5 = malloc(strlen("y") + 1);
    strcpy(id, "f");
    strcpy(id2, "x");
    strcpy(id3, "y");
    strcpy(id4, "x");
    strcpy(id5, "y");

    t7 = append_token(NULL, TOK_FUN, 0, 0, NULL);
    append_token(t7, TOK_ID, 0, 0, id);
    append_token(t7, TOK_LPAREN, 0, 0, NULL);
    append_token(t7, TOK_ID, 0, 0, id2);
    append_token(t7, TOK_COMMA, 0, 0, NULL);
    append_token(t7, TOK_ID, 0, 0, id3);
    append_token(t7, TOK_RPAREN, 0, 0, NULL);
    append_token(t7, TOK_EQUAL, 0, 0, NULL);
    append_token(t7, TOK_ID, 0, 0, id4);
    append_token(t7, TOK_MULT, 0, 0, NULL);
    append_token(t7, TOK_ID, 0, 0, id5);
    append_token(t7, TOK_SUB, 0, 0, NULL);
    append_token(t7, TOK_FLOAT, 0, 0.123456, NULL);
    inputs[7] = t7;
    }

    {
    char *id = malloc(strlen("f") + 1);
    strcpy(id, "f");

    t8 = append_token(NULL, TOK_ID, 0, 0, id);
    append_token(t8, TOK_LPAREN, 0, 0, NULL);
    append_token(t8, TOK_INT, 42, 0, NULL);
    append_token(t8, TOK_COMMA, 0, 0, NULL);
    append_token(t8, TOK_FLOAT, 0, 0.01, NULL);
    append_token(t8, TOK_RPAREN, 0, 0, NULL);
    inputs[8] = t8;
    }

    inputs[9] = tokenize("fn area(r) = 3.14 * r*r");
    inputs[10] = tokenize("4 + 3 * (2 - 3)");
    inputs[11] = tokenize("var=2*(var+1)/12");
    inputs[12] = tokenize("my_fun(0,arg, a, b, 2.0)*3");
    inputs[13] = tokenize("2^3");
    inputs[14] = tokenize("(4*2)^(f(23 - 2)^3)");
    inputs[15] = tokenize("# a comment");
    inputs[16] = tokenize("1 + 1 # a comment");

    return inputs;
}
