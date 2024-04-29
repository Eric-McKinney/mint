#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "lexer.h"

regex_t l_paren_re, r_paren_re, dot_re, equal_re, add_re, sub_re, mult_re;
regex_t div_re, fun_re, endln_re, int_re, float_re, id_re, whitespace_re;
static void compile_regexs();
static void free_regexs();
static TokenList *tok(const char *input, unsigned int pos, unsigned int length);

TokenList *tokenize(const char *input) {
    TokenList *tok_l = NULL;
    int len;

    compile_regexs();

    if (!input) {
        fprintf(stderr, "lexer: input is NULL\n");
        exit(EXIT_FAILURE);
    }
    
    len = strlen(input);
    tok_l = tok(input, 0, len);
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
        t->value.id = calloc(1, match_length + 1);
        strncpy(t->value.id, str, match_length);
        t->next = tok(input, pos + match_length, length);
        return t;
    } else if (regexec(&float_re, str, 1, &re_match, 0) == 0) {
        int match_length = re_match.rm_eo - re_match.rm_so;
        double num;
        char *float_str = calloc(1, match_length + 1);

        strncpy(float_str, str, match_length);
        num = atof(float_str);
        free(float_str);
        t = malloc(sizeof(TokenList));

        t->token = TOK_FLOAT;
        t->value.d = num;
        t->next = tok(input, pos + match_length, length);
        return t;
    } else if (regexec(&int_re, str, 1, &re_match, 0) == 0) {
        int match_length = re_match.rm_eo - re_match.rm_so;
        int num;
        char *int_str = calloc(1, match_length + 1);

        strncpy(int_str, str, match_length);
        num = atoi(int_str);
        free(int_str);
        t = malloc(sizeof(TokenList));

        t->token = TOK_INT;
        t->value.i = num;
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
        exit(EXIT_FAILURE);
    }
    
    return t;
}

static void compile_regexs() {
    regcomp(&l_paren_re,    "^\\(",                     REG_EXTENDED);
    regcomp(&r_paren_re,    "^\\)",                     REG_EXTENDED);
    regcomp(&dot_re,        "^\\.",                     REG_EXTENDED);
    regcomp(&equal_re,      "^=",                       REG_EXTENDED);
    regcomp(&add_re,        "^\\+",                     REG_EXTENDED);
    regcomp(&sub_re,        "^-",                       REG_EXTENDED);
    regcomp(&mult_re,       "^\\*",                     REG_EXTENDED);
    regcomp(&div_re,        "^/",                       REG_EXTENDED);
    regcomp(&fun_re,        "^fn",                      REG_EXTENDED);
    regcomp(&endln_re,      "^\n",                      REG_EXTENDED);
    regcomp(&int_re,        "^(-?[0-9]+)",              REG_EXTENDED);
    regcomp(&float_re,      "^(-?[0-9]+\\.[0-9]*)",     REG_EXTENDED);
    regcomp(&id_re,         "^([a-zA-Z][a-zA-Z0-9_]*)", REG_EXTENDED);
    regcomp(&whitespace_re, "^([ \t])+",                REG_EXTENDED);
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

    free_token_list(tok_l->next);

    if (tok_l->token == TOK_ID) {
        free(tok_l->value.id);
    }

    free(tok_l);
}

void print_token_list(TokenList *tok_l) {
    char *tok_l_str = token_list_to_str(tok_l);
    printf("%s\n", tok_l_str);
    free(tok_l_str);
}

char *token_list_to_str(TokenList *tok_l) {
    TokenList *curr = tok_l;
    char *str;
    int num_tok = 0;
    const int MAX_TOK_STR_LEN = 50; /* arbitrary upper limit on tok_str size */

    while (curr != NULL) {
        num_tok++;
        curr = curr->next;
    }

    str = malloc(num_tok * MAX_TOK_STR_LEN + 3);

    if (str == NULL) {
        fprintf(stderr, "lexer:token_list_to_str: Could not allocate space for tok_l_str\n");
    }

    str[0] = '\0';

    curr = tok_l;
    strcat(str, "[");
    while (curr != NULL) {
        char s[50] = {0};
        switch (curr->token) {
            case TOK_LPAREN: 
                strcat(str, "TOK_LPAREN");
                break;
            case TOK_RPAREN:
                strcat(str, "TOK_RPAREN");
                break;
            case TOK_DOT:
                strcat(str, "TOK_DOT");
                break;
            case TOK_EQUAL:
                strcat(str, "TOK_EQUAL");
                break;
            case TOK_ADD:
                strcat(str, "TOK_ADD");
                break;
            case TOK_SUB:
                strcat(str, "TOK_SUB");
                break;
            case TOK_MULT:
                strcat(str, "TOK_MULT");
                break;
            case TOK_DIV:
                strcat(str, "TOK_DIV");
                break;
            case TOK_FUN:
                strcat(str, "TOK_FUN");
                break;
            case TOK_ENDLN:
                strcat(str, "TOK_ENDLN");
                break;
            case TOK_INT:
                sprintf(s, "TOK_INT %d", curr->value.i);
                strcat(str, s);
                break;
            case TOK_FLOAT:
                sprintf(s, "TOK_FLOAT %f", curr->value.d);
                strcat(str, s);
                break;
            case TOK_ID:
                sprintf(s, "TOK_ID %s", curr->value.id);
                strcat(str, s);
                break;
            default:
                strcat(str, "Unrecognized token");
        }

        if (curr->next != NULL) {
            strcat(str, ", ");
        }

        curr = curr->next;
    }

    strcat(str, "]");
    return str;
}
