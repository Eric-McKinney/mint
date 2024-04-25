#ifndef Lexer_h
#define Lexer_h

typedef enum {
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_DOT,
    TOK_EQUAL,
    TOK_ADD,
    TOK_SUB,
    TOK_MULT,
    TOK_DIV,
    TOK_FUN,
    TOK_ENDLN,
    TOK_INT,
    TOK_FLOAT,
    TOK_ID
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

#endif
