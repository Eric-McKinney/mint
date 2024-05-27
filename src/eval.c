#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eval.h"
#include "parser.h"

Env_t *init_env() {
    return NULL;
}

static void extend_env(Env_t **env, const char *id, Value_t *v) {
    Env_t *new_data;

    new_data = malloc(sizeof(Env_t));
    new_data->id = malloc(strlen(id) + 1);
    strcpy(new_data->id, id);
    new_data->data.type = v->type;
    new_data->data.value = v->value;
    new_data->next = *env;

    *env = new_data;
}

static Value_t *lookup(Env_t *env, const char *id) {
    while (env != NULL) {
        if (strcmp(env->id, id) == 0) {
            return &env->data;
        }
        env = env->next;
    }

    fprintf(stderr, "Unbound variable %s", id);
    exit(EXIT_FAILURE);
}

static void extend_env_tmp(Env_t **env, const char *id) {
    Value_t tmp;
    extend_env(env, id, &tmp);
}

static void update_env(Env_t *env, const char *id, Value_t *v) {
    Value_t *old_v = lookup(env, id);

    old_v->type = v->type;
    old_v->value = v->value;
}

void free_env(Env_t *env) {
    if (env == NULL) {
        return;
    }

    free_env(env->next);
    free(env->id);
    free(env);
}
