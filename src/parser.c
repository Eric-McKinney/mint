#include <stdlib.h>
#include "parser.h"
#include "lexer.h"

typedef struct {
    ExprTree *ast;
    TokenList *tok_l;
} Parse_t;

static TokenList* match_token(TokenList *tok_l, TokenList *tok) {
    char *expected, *input, *arg;
    
    if (tok_l->token == tok->token) {
        int vals_equal;
        switch (tok_l->token) {
            TOK_INT:
                vals_equal = tok_l->value.i == tok->value.i;
                break;
            TOK_FLOAT:
                vals_equal = tok_l->value.d == tok->value.d;
                break;
            TOK_ID:
                vals_equal = strcmp(tok_l->value.id, tok->value.id) == 0;
                break;
            default:
                vals_equal = 1;
        }
        
        if (vals_equal) {
            return tok_l->next;
        }
    }

    expected = token_to_str(tok);
    input = token_list_to_str(tok_l);
    arg = token_to_str(tok_l);

    fprintf(stderr, "Expected %s from input %s, got %s\n", expected, input, arg);
    
    free(expected);
    free(input);
    free(arg);

    exit(EXIT_FAILURE);
}

static Tok_t lookahead(TokenList *tok_l) {
    if (tok_l == NULL) {
        return NULL;
    }

    return tok_l->token;
}

ExprTree *parse(TokenList *tok_l) {
    ExprTree *ast;
    return parse_expr(&ast, tok_l)->ast;
}

static Parse_t *parse_expr(ExprTree **ast, TokenList *tok_l) {
    return NULL;
}
