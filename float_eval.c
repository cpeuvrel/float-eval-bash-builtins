#include "float_eval.h"

#define PRECISION 256

char* op_prio[16][8] = {
    {"||", ""},
    {"&&", ""},
    {"|", ""},
    {"^", ""},
    {"&", ""},
    {"==", "!=", ""},
    {"<=", ">=", "<", ">", ""},
    {"<<", ">>", ""},
    {"+", "-", ""},
    {"**", "*", "/", "%", ""},
    {"E", "e", ""},
    {"!", "~", ""},
    {""}
};

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "You must give a string to eval !\n");
        return 1;
    }

    mpfr_t val;
    char *str = strdup(argv[1]);

    mpfr_init2(val, PRECISION);

    float_eval(&val, str);
    free(str);

    // Cleanup
    mpfr_clear(val);

    return 0;
}

/*
 * Eval a string writen in infix notation
 */
void float_eval(mpfr_t *res, char* str)
{
    stack_t stack_v;     // Stack with all values
    stack_t stack_o;     // Stack with operators

    char *val;
    mpfr_t *val_num, *ptr;
    int type, parenthesis = 0;

    while ((val = tokenify(&str, &type, &parenthesis))) {
        if (type == FLOAT_EVAL_NUM) {
            val_num = malloc(sizeof(mpfr_t));
            mpfr_init2(*val_num, PRECISION);
            mpfr_set_str(*val_num, val, 0, MPFR_RNDN);

            push(val_num, &stack_v);
            free(val);
        }
        else if (type == FLOAT_EVAL_OP) {
            compute_op(val, &stack_o, &stack_v);
        }
    }

    ptr = pop(&stack_v);
    mpfr_set(*res, *ptr, MPFR_RNDN);

    mpfr_clear(*ptr);
    free(ptr);
}
