#ifndef Test_h
#define Test_h

/* colors */
#define GREEN "\033[1;32m"
#define RED "\033[1;31m"
#define BLUE_HL "\033[44m"
#define BLUE "\033[0;36m"
#define END_COLOR "\033[0m"

/* test outputs */
#define SUCCESS 1
#define FAILURE 0
#define PASSED GREEN "PASSED" END_COLOR
#define FAILED RED "FAILED" END_COLOR

/* separator */
#define SEP "|------------------------------------------------------------|\n"

/* color formatting macros */
#define C_SUITE_NAME(x) BLUE_HL x END_COLOR
#define C_TEST_NAME(x) BLUE x END_COLOR

#endif
