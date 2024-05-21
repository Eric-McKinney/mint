#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/parser.h"

#define SUCCESS 1
#define FAILURE 0
#define PASSED "\033[1;32mPASSED\033[0m"
#define FAILED "\033[1;31mFAILED\033[0m"
#define SEP "|------------------------------------------------------------|\n"

typedef struct {
    char name[80];
    TokenList input;
    char ans[1024];
} Test;

int verbose = 0;
int run_test(Test *test);

int main(int argc, char **argv) {
    Test tests[] = {};
    int num_tests = sizeof(tests) / sizeof(Test), i, num_passed = 0;

    if (argc == 2 && (strcmp(argv[1], "-v") == 0
                   || strcmp(argv[1], "--verbose") == 0)) {
        verbose = 1;
    }

    for (i = 0; i < num_tests; i++) {
        Test *t = &(tests[i]);
        int t_result;

        if (i == 0 && verbose) {
            printf(SEP);
        }

        t_result = run_test(t);
        num_passed += t_result == SUCCESS ? 1 : 0;

        if (verbose) {
            printf("|\n");
        }

        printf("| \033[0;36m%s\033[0m %s\n", t->name,
               t_result == SUCCESS ? PASSED : FAILED);

        if (verbose) {
            printf(SEP);
        }
    }

    printf("|\n| Ran (%d/%d) tests successfully\n", num_passed, num_tests);
    printf("| Test suite %s\n", num_passed == num_tests ? PASSED : FAILED);

    if (verbose) {
        printf("|\n");
        printf(SEP);
    }
}


