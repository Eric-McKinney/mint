#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "lexer.h"

#define MAX_TOK_LEN 10     /* longest tok is TOK_LPAREN = 10 chars                        */
#define MAX_TOK_VAL_LEN 50 /* arbitrary upper limit on token value size (e.g. an integer) */
#define MAX_TOK_STR_LEN MAX_TOK_LEN + MAX_TOK_VAL_LEN

regex_t l_paren_re, r_paren_re, dot_re, equal_re, add_re, sub_re, mult_re;
regex_t div_re, fun_re, endln_re, int_re, float_re, id_re, comma_re, whitespace_re;
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
    } else if (regexec(&comma_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_COMMA;
        t->next = tok(input, pos + 1, length);
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
    regcomp(&comma_re,      "^,",                       REG_EXTENDED);
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
    regfree(&comma_re);
    regfree(&whitespace_re);
}

static TokenList *reverse_aux(TokenList *tok_l, TokenList *prev) {
    TokenList *rev = tok_l;

    if (tok_l == NULL) {
        return NULL;
    }

    if (tok_l->next != NULL) {
        rev = reverse_aux(tok_l->next, tok_l);
    }

    tok_l->next = prev;

    if (prev != NULL) {
        prev->next = NULL;
    }

    return rev;
}

void reverse(TokenList **tok_l) {
    *tok_l = reverse_aux(*tok_l, NULL);
}

static void free_token(TokenList *tok_l) {
    if (tok_l->token == TOK_ID) {
        free(tok_l->value.id);
    }

    free(tok_l);
}

void free_token_list(TokenList *tok_l) {
    if (tok_l == NULL) {
        return;
    }

    free_token_list(tok_l->next);
    free_token(tok_l);
}

static int count_tokens(TokenList *tok_l) {
    int num_tok = 0;

    while (tok_l != NULL) {
        num_tok++;
        tok_l = tok_l->next;
    }

    return num_tok;
}

void print_token_list(TokenList *tok_l) {
    char *tok_l_str = token_list_to_str(tok_l);
    printf("%s\n", tok_l_str);
    free(tok_l_str);
}

char *token_to_str(TokenList *tok_l) {
    char *str = malloc(MAX_TOK_STR_LEN + 1);
    char s[MAX_TOK_VAL_LEN] = {0};

    if (tok_l == NULL) {
        str[0] = '\0';
        return str;
    }
    
    switch (tok_l->token) {
        case TOK_LPAREN: 
            strcpy(str, "TOK_LPAREN");
            break;
        case TOK_RPAREN:
            strcpy(str, "TOK_RPAREN");
            break;
        case TOK_DOT:
            strcpy(str, "TOK_DOT");
            break;
        case TOK_EQUAL:
            strcpy(str, "TOK_EQUAL");
            break;
        case TOK_ADD:
            strcpy(str, "TOK_ADD");
            break;
        case TOK_SUB:
            strcpy(str, "TOK_SUB");
            break;
        case TOK_MULT:
            strcpy(str, "TOK_MULT");
            break;
        case TOK_DIV:
            strcpy(str, "TOK_DIV");
            break;
        case TOK_FUN:
            strcpy(str, "TOK_FUN");
            break;
        case TOK_ENDLN:
            strcpy(str, "TOK_ENDLN");
            break;
        case TOK_INT:
            sprintf(s, "TOK_INT %d", tok_l->value.i);
            strcpy(str, s);
            break;
        case TOK_FLOAT:
            sprintf(s, "TOK_FLOAT %f", tok_l->value.d);
            strcpy(str, s);
            break;
        case TOK_ID:
            sprintf(s, "TOK_ID %s", tok_l->value.id);
            strcpy(str, s);
            break;
        case TOK_COMMA:
            strcpy(str, "TOK_COMMA");
            break;
        default:
            strcpy(str, "Unrecognized token");
    }

    return str;
}

char *token_list_to_str(TokenList *tok_l) {
    char *str;
    int num_tok = count_tokens(tok_l);

    /* + 2 for brackets [] and then + 1 for null terminator */
    str = malloc(num_tok * MAX_TOK_STR_LEN + 3); 

    strcpy(str, "[");
    while (tok_l != NULL) {
        char *token_str = token_to_str(tok_l);

        strcat(str, token_str);
        free(token_str);

        if (tok_l->next != NULL) {
            strcat(str, ", ");
        }

        tok_l = tok_l->next;
    }

    strcat(str, "]");
    return str;
}
