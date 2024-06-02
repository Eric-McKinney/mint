#ifndef Eval_h
#define Eval_h

#include "parser.h"

typedef struct env {
    char *id;
    ExprTree *data;
    struct env *next;
} Env_t;

Env_t *init_env();
char *env_to_str(Env_t *env);
void free_env(Env_t *env);
ExprTree *eval(ExprTree **tree, Env_t *env);

#endif
