#include <regex.h>
#include "lexer.h"

TokenList *tokenize(const char *input) {
    regex_t l_paren_re;
    regex_t r_paren_re;
    regex_t dot_re;
    regex_t equal_re;
    regex_t add_re;
    regex_t sub_re;
    regex_t mult_re;
    regex_t div_re;
    regex_t fun_re;
    regex_t endln_re;
    regex_t int_re;
    regex_t float_re;
    regex_t id_re;

    regcomp(&l_paren_re, "^\\(",                   REG_EXTENDED);
    regcomp(&r_paren_re, "^\\)",                   REG_EXTENDED);
    regcomp(&dot_re,     "^\\.",                   REG_EXTENDED);
    regcomp(&equal_re,   "^=",                     REG_EXTENDED);
    regcomp(&add_re,     "^\\+",                   REG_EXTENDED);
    regcomp(&sub_re,     "^-",                     REG_EXTENDED);
    regcomp(&mult_re,    "^\\*",                   REG_EXTENDED);
    regcomp(&div_re,     "^/",                     REG_EXTENDED);
    regcomp(&fun_re,     "^fn",                    REG_EXTENDED);
    regcomp(&endln_re,   "^\\n",                   REG_EXTENDED);
    regcomp(&int_re,     "^-?[0-9]+",              REG_EXTENDED);
    regcomp(&float_re,   "^-?[0-9]+\\.[0-9]*",     REG_EXTENDED);
    regcomp(&id_re,      "^[a-zA-Z][a-zA-Z0-9_]*", REG_EXTENDED);

    return NULL;
}
