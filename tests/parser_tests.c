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
    const char *name;
    TokenList *input;
    const char *ans;
} Test;

int verbose = 0;
static int run_test(Test *test);
static TokenList **create_inputs();

int main(int argc, char **argv) {
    const char *t_names[] = {
        "basic_addition",
        "int float add sub",
        "arithmetic mix",
        "long add/sub",
        "long mult/div"
    };
    const char *t_ans[] = {
        "(Add(Int 1)(Int 2))", /* 1 + 2 */
        "(Add(Sub(Int 44)(Int 21))(Float 2.200000))", /* 44 - 21 + 2.2 */
        "(Add(Div(Mult(Float 2.000000)(Int 4))(Int 12))(Int 5))", /* 2. * 4 / 12 + 5 */
        "(Sub(Add(Sub(Add(Int 1)(Int 2))(Int 3))(Int 4))(Int 5))", /* 1 + 2 - 3 + 4 - 5 */
        "(Div(Mult(Div(Mult(Int 1)(Int 2))(Int 3))(Int 4))(Int 5))" /* 1 * 2 / 3 * 4 / 5 */
    };
    int num_tests = sizeof(t_names) / sizeof(char *), i, num_passed = 0;
    TokenList **inputs = create_inputs(num_tests);

    if (argc == 2 && (strcmp(argv[1], "-v") == 0
                   || strcmp(argv[1], "--verbose") == 0)) {
        verbose = 1;
        printf(SEP);
        printf("|\n|                        ");
    } else {
        printf("| ");
    }
    
    printf("\033[44mparser tests\033[0m\n");
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

        printf("| \033[0;36m%s\033[0m %s\n", t.name,
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
        printf("| \033[0;36m%s\033[0m test:\n", test->name);
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
    switch(tok) {
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
    TokenList *t0, *t1, *t2, *t3, *t4;

    t0 = append_token(NULL, TOK_INT, 1, 0, NULL);
    append_token(t0, TOK_ADD, 0, 0, NULL);
    append_token(t0, TOK_INT, 2, 0, NULL);
    append_token(t0, TOK_ENDLN, 0, 0, NULL);
    inputs[0] = t0;

    t1 = append_token(NULL, TOK_INT, 44, 0, NULL);
    append_token(t1, TOK_SUB, 0, 0, NULL);
    append_token(t1, TOK_INT, 21, 0, NULL);
    append_token(t1, TOK_ADD, 0, 0, NULL);
    append_token(t1, TOK_FLOAT, 0, 2.2, NULL);
    append_token(t1, TOK_ENDLN, 0, 0, NULL);
    inputs[1] = t1;

    t2 = append_token(NULL, TOK_FLOAT, 0, 2., NULL);
    append_token(t2, TOK_MULT, 0, 0, NULL);
    append_token(t2, TOK_INT, 4, 0, NULL);
    append_token(t2, TOK_DIV, 0, 0, NULL);
    append_token(t2, TOK_INT, 12, 0, NULL);
    append_token(t2, TOK_ADD, 0, 0, NULL);
    append_token(t2, TOK_INT, 5, 0, NULL);
    append_token(t2, TOK_ENDLN, 0, 0, NULL);
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
    append_token(t3, TOK_ENDLN, 0, 0, NULL);
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
    append_token(t4, TOK_ENDLN, 0, 0, NULL);
    inputs[4] = t4;

    return inputs;
}
