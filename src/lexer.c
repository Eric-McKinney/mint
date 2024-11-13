#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <err.h>
#include "lexer.h"

#define MAX_TOK_LEN 11     /* longest tok is TOK_COMMENT = 11 chars                       */
#define MAX_TOK_VAL_LEN 50 /* arbitrary upper limit on token value size (e.g. an integer) */
#define MAX_TOK_STR_LEN MAX_TOK_LEN + MAX_TOK_VAL_LEN

regex_t add_re, comma_re, comment_re, dot_re, div_re, equal_re, exp_re, float_re;
regex_t fun_re, id_re, int_re, l_paren_re, mult_re, r_paren_re, sub_re; 
regex_t whitespace_re;
static TokenList *tok(const char *input, unsigned int pos, unsigned int length);

TokenList *tokenize(const char *input) {
    TokenList *tok_l = NULL;
    int len;

    if (!input) {
        errno = EINVAL;
        warnx("error: (E0005) input is NULL");
        return NULL;
    }
    
    len = strlen(input);
    tok_l = tok(input, 0, len);

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
        long int num;
        char *int_str = calloc(1, match_length + 1);

        strncpy(int_str, str, match_length);
        num = atol(int_str);

        if (errno == ERANGE) {
            warnx(
                "error: (E0001) int %s does not fall within storable range of -9223372036854775808 to "
                "9223372036854775807\ntry declaring it as a float instead (i.e. %s.0)",
                int_str,
                int_str
            );
            free(int_str);
            return NULL;
        }

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
    } else if (regexec(&exp_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_EXP;
        t->next = tok(input, pos + 1, length);
        return t;
    } else if (regexec(&comment_re, str, 1, &re_match, 0) == 0) {
        int match_length = re_match.rm_eo - re_match.rm_so;
        t = malloc(sizeof(TokenList));
        t->token = TOK_COMMENT;
        t->next = tok(input, pos + match_length, length);
        return t;
    } else if (regexec(&comma_re, str, 0, NULL, 0) == 0) {
        t = malloc(sizeof(TokenList));
        t->token = TOK_COMMA;
        t->next = tok(input, pos + 1, length);
        return t;
    } else {
        errno = EINVAL;
        warnx("error: (E0002) Invalid token starting with \"%c\" at index %u", str[0], pos);
        return NULL;
    }
}

void compile_regexs() {
    regcomp(&add_re,        "^\\+",                                 REG_EXTENDED);
    regcomp(&comma_re,      "^,",                                   REG_EXTENDED);
    regcomp(&comment_re,    "^(#.*)$",                              REG_EXTENDED);
    regcomp(&div_re,        "^/",                                   REG_EXTENDED);
    regcomp(&dot_re,        "^\\.",                                 REG_EXTENDED);
    regcomp(&equal_re,      "^=",                                   REG_EXTENDED);
    regcomp(&exp_re,        "^\\^",                                 REG_EXTENDED);
    regcomp(&float_re,      "^-?([0-9]+\\.[0-9]*|[0-9]*\\.[0-9]+)", REG_EXTENDED);
    regcomp(&fun_re,        "^fn",                                  REG_EXTENDED);
    regcomp(&id_re,         "^[a-zA-Z][a-zA-Z0-9_]*",               REG_EXTENDED);
    regcomp(&int_re,        "^-?[0-9]+",                            REG_EXTENDED);
    regcomp(&l_paren_re,    "^\\(",                                 REG_EXTENDED);
    regcomp(&mult_re,       "^\\*",                                 REG_EXTENDED);
    regcomp(&r_paren_re,    "^\\)",                                 REG_EXTENDED);
    regcomp(&sub_re,        "^-",                                   REG_EXTENDED);
    regcomp(&whitespace_re, "^([ \t])+",                            REG_EXTENDED);
}

void free_regexs() {
    regfree(&add_re);
    regfree(&comma_re);
    regfree(&comment_re);
    regfree(&div_re);
    regfree(&dot_re);
    regfree(&equal_re);
    regfree(&exp_re);
    regfree(&float_re);
    regfree(&fun_re);
    regfree(&id_re);
    regfree(&int_re);
    regfree(&l_paren_re);
    regfree(&mult_re);
    regfree(&r_paren_re);
    regfree(&sub_re);
    regfree(&whitespace_re);
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

static int count_tokens(const TokenList *tok_l) {
    int num_tok = 0;

    while (tok_l != NULL) {
        num_tok++;
        tok_l = tok_l->next;
    }

    return num_tok;
}

void print_token_list(const TokenList *tok_l) {
    char *tok_l_str = token_list_to_str(tok_l);
    printf("%s\n", tok_l_str);
    free(tok_l_str);
}

char *token_to_str(const TokenList *tok_l) {
    char *str = malloc(MAX_TOK_STR_LEN + 1);
    char s[MAX_TOK_VAL_LEN] = {0};

    if (tok_l == NULL) {
        str[0] = '\0';
        return str;
    }
    
    switch (tok_l->token) {
        case TOK_ADD:
            strcpy(str, "TOK_ADD");
            break;
        case TOK_COMMA:
            strcpy(str, "TOK_COMMA");
            break;
        case TOK_COMMENT:
            strcpy(str, "TOK_COMMENT");
            break;
        case TOK_DIV:
            strcpy(str, "TOK_DIV");
            break;
        case TOK_DOT:
            strcpy(str, "TOK_DOT");
            break;
        case TOK_EQUAL:
            strcpy(str, "TOK_EQUAL");
            break;
        case TOK_EXP:
            strcpy(str, "TOK_EXP");
            break;
        case TOK_FLOAT:
            sprintf(s, "TOK_FLOAT %f", tok_l->value.d);
            strcpy(str, s);
            break;
        case TOK_FUN:
            strcpy(str, "TOK_FUN");
            break;
        case TOK_ID:
            sprintf(s, "TOK_ID %s", tok_l->value.id);
            strcpy(str, s);
            break;
        case TOK_INT:
            sprintf(s, "TOK_INT %ld", tok_l->value.i);
            strcpy(str, s);
            break;
        case TOK_LPAREN:
            strcpy(str, "TOK_LPAREN");
            break;
        case TOK_MULT:
            strcpy(str, "TOK_MULT");
            break;
        case TOK_RPAREN:
            strcpy(str, "TOK_RPAREN");
            break;
        case TOK_SUB:
            strcpy(str, "TOK_SUB");
            break;
        default:
            strcpy(str, "Unrecognized token");
    }

    return str;
}

char *token_list_to_str(const TokenList *tok_l) {
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

char *token_value_to_str(const TokenList *tok_l) {
    char *str, s[MAX_TOK_VAL_LEN] = {0};

    if (tok_l == NULL) {
        return calloc(1,1);
    }

    str = malloc(MAX_TOK_STR_LEN + 1);

    switch (tok_l->token) {
        case TOK_ADD:
            strcpy(str, " + ");
            break;
        case TOK_COMMA:
            strcpy(str, ", ");
            break;
        case TOK_COMMENT:
            str[0] = '\0';
            break;
        case TOK_DIV:
            strcpy(str, " / ");
            break;
        case TOK_DOT:
            strcpy(str, ".");
            break;
        case TOK_EQUAL:
            strcpy(str, " = ");
            break;
        case TOK_EXP:
            strcpy(str, "^");
            break;
        case TOK_FLOAT:
            sprintf(s, "%f", tok_l->value.d);
            strcpy(str, s);
            break;
        case TOK_FUN:
            strcpy(str, "fn ");
            break;
        case TOK_ID:
            sprintf(s, "%s", tok_l->value.id);
            strcpy(str, s);
            break;
        case TOK_INT:
            sprintf(s, "%ld", tok_l->value.i);
            strcpy(str, s);
            break;
        case TOK_LPAREN:
            strcpy(str, "(");
            break;
        case TOK_MULT:
            strcpy(str, " * ");
            break;
        case TOK_RPAREN:
            strcpy(str, ")");
            break;
        case TOK_SUB:
            strcpy(str, " - ");
            break;
        default:
            strcpy(str, "Unrecognized token");
    }

    return str;
}

char *token_values_to_str(const TokenList *tok_l) {
    char *str;
    int num_tok = count_tokens(tok_l);

    str = malloc(num_tok * MAX_TOK_STR_LEN + 1); 
    str[0] = '\0';

    while (tok_l != NULL) {
        char *tok_str = token_value_to_str(tok_l);

        strcat(str, tok_str);
        free(tok_str);

        tok_l = tok_l->next;
    }

    return str;
}
