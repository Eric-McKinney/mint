#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "lexer.h"
#include "parser.h"
#include "eval.h"

#define TEAL "\033[38;5;49m"
#define END_COLOR "\033[0m"
#define PROMPT TEAL "mint" END_COLOR "|> "
#define BUF_SIZE 1024

static void process_cmd(char *cmd, Env_t *env);

int main(int argc, char **argv) {
    FILE *input = stdin;
    char line[BUF_SIZE] = {0};
    Env_t *env = init_env();

    /* Determine input stream */
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, "File \"%s\" not found\n", argv[1]);
            return 1;
        }
    } else if (argc > 2) {
        fprintf(stderr, "%s: Too many args\n", argv[0]);
        return 1;
    }

    if (argc == 1 && isatty(0)) {
        printf(PROMPT);
        fflush(stdout);
    }

    while(fgets(line, BUF_SIZE, input)) {
        char cmd[BUF_SIZE] = "";

        sscanf(line, "%s", cmd);

        if (strcmp(cmd, "") == 0) {
            if (argc == 1 && isatty(0)) {
                printf(PROMPT);
                fflush(stdout);
            }
            continue;
        }

        if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
            break;
        }

        process_cmd(line, env);

        if (argc == 1 && isatty(0)) {
            printf(PROMPT);
            fflush(stdout);
        }
    }
    
    free_env(env);
    fclose(input);
    return 0;
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
