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
#define VERSION_MSG "mint 0.1.0\n"
#define HELP_MSG "Usage: mint\n" \
    "   or: mint EXPRESSION\n" \
    "   or: mint < FILE\n" \
    "   or: mint OPTION\n" \
    "Start an interactive session, evaluate a math EXPRESSION when given as an\n" \
    "argument, evaluate EXPRESSIONs from a FILE, or print help or version info.\n" \
    "\n" \
    "Available OPTIONs are:\n" \
    "  --help     print this help message and exit\n" \
    "  --version  print version info and exit\n" \
    "\n" \
    "An EXPRESSION is a sequence of numbers separated by operators. Operators like:\n" \
    "  + for addition\n" \
    "  - for subtraction\n" \
    "  * for multiplication\n" \
    "  / for division\n" \
    "  ^ for exponentiation\n" \
    "\n" \
    "Normal math order of operations is obeyed, so in order of evaluation:\n" \
    "  Parentheses\n" \
    "  Exponents\n" \
    "  Multiplication and division (left to right)\n" \
    "  Addition and subtraction (left to right)\n" \
    "\n" \
    "In an interactive session, variables and functions are also available. To assign\n" \
    "a value to a variable, use the = operator between the variable name and its\n" \
    "value. To use a variable's value, simply use its name in an expression. Here \n" \
    "are some examples:\n" \
    "  x = 4\n" \
    "  y = 2^8\n" \
    "  longVariableName = 44 - 8\n" \
    "  with_underscores = x * y\n" \
    "  with_1_numbers2 = x / y\n" \
    "\n" \
    "To define a function use the fn keyword followed by the function name and any\n" \
    "parameters it takes in a comma separated list in parentheses, then a = followed\n" \
    "by an expression to be the function body. To apply a function, simply use its\n" \
    "name followed by any arguments in a comma separated list wrapped in parentheses.\n" \
    "Here are some examples:\n" \
    "  fn f(x) = x + 4\n" \
    "  fn g(y) = f(y / 2)\n" \
    "  fn my_function(a, b, c) = a * b - c\n" \
    "\n" \
    "To end an interactive session, type exit or quit and hit enter. The keybind\n" \
    "ctrl+d will work too.\n" \
    "\n" \
    "When input redirection is used to read from a FILE, the functionality is the\n" \
    "same as an interactive session, but only the result of the last expression is\n" \
    "printed.\n" \
    "\n" \
    "For more info see the online wiki: <https://github.com/Eric-McKinney/mint/wiki>\n"

static char *process_input(char *input, Env_t *env);
static char *aggregate_args(int argc, char **argv);
static void process_file(FILE *file, Env_t *env);
static void repl_loop(Env_t *env);

int main(int argc, char **argv) {
    Env_t *env = init_env();
    compile_regexs();

    /* determine invocation method */
    if (argc >= 2 && strcmp(argv[1], "--help") == 0) {
        printf(HELP_MSG);
    } else if (argc >= 2 && strcmp(argv[1], "--version") == 0) {
        printf(VERSION_MSG);
    } else if (argc == 1) {
        if (isatty(0)) {
            repl_loop(env);
        } else {
            /* input redirection */
            process_file(stdin, env);
        }
    } else if (argc > 1) {
        char *expr, *result;

        /* process args as expr */
        expr = aggregate_args(argc, argv);
        result = process_input(expr, env);

        if (result != NULL) {
            printf("%s\n", result);
        }

        free(expr);
        free(result);
    }

    free_env(env);
    free_regexs();
    return 0;
}

static int is_empty(const char *str) {
    int i;

    for (i = 0; i < strlen(str); i++) {
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
            return 0;
        }
    }

    return 1;
}

static void repl_loop(Env_t *env) {
    char *line, *result;

    while ((line = readline(PROMPT))) {
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
            break;
        }

        result = process_input(line, env);

        if (result != NULL && strcmp(result, "") != 0) {
            printf("%s\n", result);
        }

        if (!is_empty(line)) {
            add_history(line);
        }

        free(line);
        free(result);
    }
}

static char *aggregate_args(int argc, char **argv) {
    int i, expr_length = 0;
    char *expr;

    for (i = 1; i < argc; i++) {
        expr_length += strlen(argv[i]) + 1; /* appending a space */
    }

    expr = malloc(expr_length + 1);
    expr[0] = '\0';

    for (i = 1; i < argc; i++) {
        strcat(expr, argv[i]);
        strcat(expr, " ");
    }

    return expr;
}

static void process_file(FILE *file, Env_t *env) {
    char line[BUF_SIZE] = {0}, *result = NULL, *result_to_print = NULL;

    while(fgets(line, BUF_SIZE, file)) {
        line[strlen(line) - 1] = '\0'; /* strip newline */

        result = process_input(line, env);

        if (result != NULL && strcmp(result, "") != 0) {
            free(result_to_print);
            result_to_print = result;
        } else {
            free(result);
        }
    }
    
    if (result_to_print != NULL) {
        printf("%s\n", result_to_print);
    }

    free(result_to_print);
    fclose(file);
}

static char *process_input(char *input, Env_t *env) {
    TokenList *tok_l;
    ExprTree *tree;
    char *result;

    errno = 0;
    tok_l = tokenize(input);

    if (errno != 0) {
        free_token_list(tok_l);
        return NULL;
    }

    tree = parse(tok_l);

    if (errno != 0) {
        free_token_list(tok_l);
        free_expr_tree(tree);
        return NULL;
    }
    
    eval(&tree, env);

    if (errno != 0) {
        free_token_list(tok_l);
        free_expr_tree(tree);
        return NULL;
    }

    result = eval_result_to_str(tree);

    free_token_list(tok_l);
    free_expr_tree(tree);

    return result;
}
