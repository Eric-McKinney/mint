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
    TokenList **inputs = create_inputs();
    const char *t_names[] = {
        "basic_addition"
    };
    const char *t_ans[] = {
        "(Add(Int 1)(Int 2))"
    };
    int num_tests = sizeof(t_names) / sizeof(char *), i, num_passed = 0;

    if (argc == 2 && (strcmp(argv[1], "-v") == 0
                   || strcmp(argv[1], "--verbose") == 0)) {
        verbose = 1;
    }

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
        printf("| input: \"%s\"\n", input_str);
        printf("| parse return val: %p\n", (void *) tree);
        printf("| parse tree: %s\n", tree_str);
        printf("| expected: %s\n", test->ans);

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

static TokenList **create_inputs() {
    TokenList **inputs = malloc(1 * sizeof(TokenList *));
    TokenList *t1;

    t1 = append_token(NULL, TOK_INT, 1, 0, NULL);
    append_token(t1, TOK_ADD, 0, 0, NULL);
    append_token(t1, TOK_INT, 2, 0, NULL);
    append_token(t1, TOK_ENDLN, 0, 0, NULL);
    inputs[0] = t1;

    return inputs;
}
