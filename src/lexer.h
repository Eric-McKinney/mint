#ifndef Lexer_h
#define Lexer_h

typedef enum {
    TOK_ADD,
    TOK_COMMA,
    TOK_COMMENT,
    TOK_DIV,
    TOK_DOT,
    TOK_ENDLN,
    TOK_EQUAL,
    TOK_EXP,
    TOK_FLOAT,
    TOK_FUN,
    TOK_ID,
    TOK_INT,
    TOK_LPAREN,
    TOK_MULT,
    TOK_RPAREN,
    TOK_SUB
} Tok_t;

typedef struct token {
    Tok_t token;
    union {
        int i;
        double d;
        char *id;
    } value;
    struct token *next;
} TokenList;

TokenList *tokenize(const char *input);
void free_token_list(TokenList *tok_l);
void print_token_list(TokenList *tok_l);
char *token_to_str(TokenList *tok_l);
char *token_list_to_str(TokenList *tok_l);

#endif
