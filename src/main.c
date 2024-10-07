#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "lexer.h"
#include "parser.h"
#include "eval.h"

#define TEAL "\033[38;5;49m"
#define END_COLOR "\033[0m"
#define PROMPT TEAL "mint" END_COLOR "|> "
#define BUF_SIZE 1024

static void process_cmd(char *cmd, Env_t *env);

int main(int argc, char **argv) {
    Env_t *env = init_env();

    /* determine invocation method */
    if (argc == 1) {
        if (isatty(0)) {
            repl_loop(env);
        } else {
            process_file(stdin, env);
        }
    } else if (argc > 1) {
        /* process args as expr */
    }

    free_env(env);
    return 0;
}

static void process_file(FILE *file, Env_t *env) {
    char line[BUF_SIZE] = {0};

    while(fgets(line, BUF_SIZE, file)) {
        char cmd[BUF_SIZE] = "";

        sscanf(line, "%s", cmd);

        if (strcmp(cmd, "") == 0) {
            continue;
        }

        process_cmd(line, env);
    }

    fclose(file);
}

static void repl_loop(Env_t *env) {
    char *line;

    while ((line = readline(PROMPT))) {
        if (strcmp(line, "") == 0) {
            continue;
        }

        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
            break;
        }

        process_cmd(line, env);
        add_history(line);
    }
}

static void process_cmd(char *cmd, Env_t *env) {
    char *str;
    TokenList *tok_l;
    ExprTree *tree;

    errno = 0;
    tok_l = tokenize(cmd);

    if (errno != 0) {
        free_token_list(tok_l);
        return;
    }

    tree = parse(tok_l);

    if (errno != 0) {
        free_token_list(tok_l);
        free_expr_tree(tree);
        return;
    }
    
    eval(&tree, env);

    if (errno != 0) {
        free_token_list(tok_l);
        free_expr_tree(tree);
        return;
    }

    str = eval_result_to_str(tree);

    if (strlen(str) > 0) {
        printf("%s\n", str);
    }

    free(str);
    free_token_list(tok_l);
    free_expr_tree(tree);
}
