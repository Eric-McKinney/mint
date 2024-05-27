#ifndef Eval_h
#define Eval_h

typedef struct {
    enum {
        Int,
        Float
    } type;
    
    union {
        int i;
        double d;
    } value;
} Value_t;

typedef struct env {
    char *id;
    Value_t data;
    struct env *next;
} Env_t;

#endif
