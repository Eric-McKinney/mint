#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "lexer.h"

#define REG_COMP_FLAGS REG_EXTENDED | REG_NOSUB

regex_t l_paren_re, r_paren_re, dot_re, equal_re, add_re, sub_re, mult_re;
regex_t div_re, fun_re, endln_re, int_re, float_re, id_re, whitespace_re;
static void compile_regexs();
static void free_regexs();
static TokenList *tok(const char *input, unsigned int pos, unsigned int length);

TokenList *tokenize(const char *input) {
    TokenList *tok_l = NULL;

    compile_regexs();

    if (!input) {
        fprintf(stderr, "lexer: input is NULL\n");
        exit(1);
    }

    tok_l = tok(input, 0, (unsigned int) strlen(input));
    free_regexs();

    return tok_l;
}

static TokenList *tok(const char *input, unsigned int pos, unsigned int length) {
    const char *str = input + pos;
    regmatch_t re_match;
    TokenList *t;

    if (pos >= length) {
        return NULL;
    }

    if (regexec(&whitespace_re, str, 1, &re_match, 0) == 0) {
        int match_length = re_match.rm_eo - re_match.rm_so;
        return tok(input, pos + match_length, length);
    } else if (regexec(&id_re, str, 1, &re_match, 0) == 0) {
        int match_length = re_match.rm_eo - re_match.rm_so;
        t = malloc(sizeof(TokenList));

        if (strncmp(str, "fn", 2) == 0) {
            t->token = TOK_FUN;
            t->next = tok(input, pos + 2, length);
            return t;
        }

        t->token = TOK_ID;
        t->value.id = malloc(match_length + 1);
        strncpy(t->value.id, str, match_length);
        t->next = tok(input, pos + match_length, length);
        return t;
    } else if (regexec(&int_re, str, 1, &re_match, 0) == 0) {
        int match_length = re_match.rm_eo - re_match.rm_so;
        int num;
        char *int_str = malloc(match_length + 1);

        strncpy(int_str, str, match_length);
        num = atoi(int_str);
        free(int_str);
        t = malloc(sizeof(TokenList));

        t->token = TOK_INT;
        t->value.i = num;
        t->next = tok(input, pos + match_length, length);
        return t;
    } else if (regexec(&float_re, str, 1, &re_match, 0) == 0) {
        int match_length = re_match.rm_eo - re_match.rm_so;
        double num;
        char *float_str = malloc(match_length + 1);

        strncpy(float_str, str, match_length);
        num = atof(float_str);
        free(float_str);
        t = malloc(sizeof(TokenList));

        t->token = TOK_FLOAT;
        t->value.d = num;
        t->next = tok(input, pos + match_length, length);
        return t;
    } else if (regexec(&l_paren_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_LPAREN;
        t->next = tok(input, pos + 1, length);
        return t;
    } else if (regexec(&r_paren_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_RPAREN;
        t->next = tok(input, pos + 1, length);
        return t;
    } else if (regexec(&dot_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_DOT;
        t->next = tok(input, pos + 1, length);
        return t;
    } else if (regexec(&equal_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_EQUAL;
        t->next = tok(input, pos + 1, length);
        return t;
    } else if (regexec(&add_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_ADD;
        t->next = tok(input, pos + 1, length);
        return t;
    } else if (regexec(&sub_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_SUB;
        t->next = tok(input, pos + 1, length);
        return t;
    } else if (regexec(&mult_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_MULT;
        t->next = tok(input, pos + 1, length);
        return t;
    } else if (regexec(&div_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_DIV;
        t->next = tok(input, pos + 1, length);
        return t;
    } else if (regexec(&endln_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_ENDLN;
        t->next = tok(input, pos + 1, length);
        return t;
    } else {
        fprintf(stderr, "Invalid token starting with \"%c\" at position %u\n", str[pos], pos);
        exit(1);
    }
    
    return t;
}

static void compile_regexs() {
    regcomp(&l_paren_re,    "^\\(",                     REG_COMP_FLAGS);
    regcomp(&r_paren_re,    "^\\)",                     REG_COMP_FLAGS);
    regcomp(&dot_re,        "^\\.",                     REG_COMP_FLAGS);
    regcomp(&equal_re,      "^=",                       REG_COMP_FLAGS);
    regcomp(&add_re,        "^\\+",                     REG_COMP_FLAGS);
    regcomp(&sub_re,        "^-",                       REG_COMP_FLAGS);
    regcomp(&mult_re,       "^\\*",                     REG_COMP_FLAGS);
    regcomp(&div_re,        "^/",                       REG_COMP_FLAGS);
    regcomp(&fun_re,        "^fn",                      REG_COMP_FLAGS);
    regcomp(&endln_re,      "^\\n",                     REG_COMP_FLAGS);
    regcomp(&int_re,        "^(-?[0-9]+)",              REG_COMP_FLAGS);
    regcomp(&float_re,      "^(-?[0-9]+\\.[0-9]*)",     REG_COMP_FLAGS);
    regcomp(&id_re,         "^([a-zA-Z][a-zA-Z0-9_]*)", REG_COMP_FLAGS);
    regcomp(&whitespace_re, "^([ \\t])+",               REG_COMP_FLAGS);
}

static void free_regexs() {
    regfree(&l_paren_re);
    regfree(&r_paren_re);
    regfree(&dot_re);
    regfree(&equal_re);
    regfree(&add_re);
    regfree(&sub_re);
    regfree(&mult_re);
    regfree(&div_re);
    regfree(&fun_re);
    regfree(&endln_re);
    regfree(&int_re);
    regfree(&float_re);
    regfree(&id_re);
    regfree(&whitespace_re);
}

void free_token_list(TokenList *tok_l) {
    if (tok_l == NULL) {
        return;
    }

    if (tok_l->next == NULL) {
        if (tok_l->token == TOK_ID) {
            free(tok_l->value.id);
        }

        free(tok_l);
        return;
    }

    free_token_list(tok_l->next);
}

void print_token_list(TokenList *tok_l) {
    TokenList *curr = tok_l;

    printf("[");
    while (curr != NULL) {
        switch (curr->token) {
            case TOK_LPAREN: 
                printf("TOK_LPAREN");
                break;
            case TOK_RPAREN:
                printf("TOK_RPAREN");
                break;
            case TOK_DOT:
                printf("TOK_DOT");
                break;
            case TOK_EQUAL:
                printf("TOK_EQUAL");
                break;
            default:
                printf("Unrecognized token");
        }

        printf(", ");
    }
    printf("]\n");
}
