#ifndef FLOAT_EVAL_H

#define FLOAT_EVAL_H

#define FLOAT_EVAL_STACK_BEG_SIZE 32
#define FLOAT_EVAL_MAX_PRIO 12
#define FLOAT_EVAL_MAX_PRIO_LEN 3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Bash includes required for builtins */

#include "builtins.h"
#include "shell.h"
#include "common.h"

#include <gmp.h>
#include <mpfr.h>

#define HAS_WORD(wordlist) \
    ((wordlist) && (wordlist)->word && (wordlist)->word->word)

typedef struct _stack_t {
    void** stack;
    int    pos;
    int    size;
} stack_t;

enum FLOAT_EVAL_TYPES{
    FLOAT_EVAL_NULL,
    FLOAT_EVAL_NUM,
    FLOAT_EVAL_OP,
    FLOAT_EVAL_PARENT,
};

int float_eval(mpfr_t* res, char* str);

#ifdef BASH_BUILTIN
int float_eval_builtin(WORD_LIST *list);

/*  builtin short documentation */
char float_eval_short_doc[] = "float_eval [-v] [-p INTEGER] STRING...";

/*  builtin long documentation */
char *float_eval_long_doc[] = {
    "Give one or more arithmetic string to be parsed in a double",
    "The result(s) will be store in bash array $REPLY",
    "Use -v/--verbose to show the syntax tree constructed",
    "Use -p/--precision to set the number of decimal wanted",
    NULL
};

struct builtin float_eval_struct = {
    "float_eval",
    float_eval_builtin,
    BUILTIN_ENABLED,
    float_eval_long_doc,
    float_eval_short_doc,
    0
};
#endif

#endif /* end of include guard: FLOAT_EVAL_H */
