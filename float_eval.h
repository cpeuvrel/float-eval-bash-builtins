#ifndef FLOAT_EVAL_H

#define FLOAT_EVAL_H

#define FLOAT_EVAL_STACK_BEG_SIZE 32
#define FLOAT_EVAL_MAX_PRIO 12
#define FLOAT_EVAL_MAX_PRIO_LEN 3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmp.h>
#include <mpfr.h>

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

void float_eval(mpfr_t* res, char* str);

#endif /* end of include guard: FLOAT_EVAL_H */
