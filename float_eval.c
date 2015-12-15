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

static void init_stack(stack_t* s);
static void print_stack(stack_t s);
static void print_mpfr_stack(stack_t s);
static void clear(stack_t s);
static void clear_mpfr(stack_t s);
static void push(void* val, stack_t* s);
static void* pop(stack_t* s);

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

    init_stack(&stack_v);
    init_stack(&stack_o);

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

    clear(stack_o);
    clear_mpfr(stack_v);

    mpfr_clear(*ptr);
    free(ptr);
}

/*
 * Initialize a stack_t
 */
static void init_stack(stack_t* s)
{
    s->size = FLOAT_EVAL_STACK_BEG_SIZE;
    s->pos = 0;
    s->stack = malloc(s->size * sizeof(void*));
    s->stack[0] = NULL;
}

/*
 * Print the whole stack given
 */
static void print_stack(stack_t s)
{
    int i;

    for (i = 0; s.stack[i]; i++) {
        fprintf(stderr, "%s\n", (char*)s.stack[i]);
    }
}

/*
 * Print the whole stack given (mpfr_t* values)
 */
static void print_mpfr_stack(stack_t s)
{
    int i;

    for (i = 0; s.stack[i]; i++) {
        mpfr_out_str(stderr, 10, 3, s.stack[i], MPFR_RNDN);
        putc('\n', stderr);
    }
}

/*
 * Free the memory declared on the heap for the stack given
 */
static void clear(stack_t s)
{
    for (; s.pos >= 0; s.pos--) {
        free(s.stack[s.pos]);
    }
    free(s.stack);
}

/*
 * Free the memory (mpfr_t*) declared on the heap for the stack given
 */
static void clear_mpfr(stack_t s)
{
    for (; s.pos > 0; s.pos--) {
        mpfr_clear(s.stack[s.pos]);
    }
    free(s.stack);
}

/*
 * Push a value to the stack
 */
static void push(void* val, stack_t* s)
{
    if (s->pos == s->size) {
        s->size *= 2;
        s->stack = realloc(s->stack, s->size * sizeof(void*));
    }

    // Push the value
    s->stack[s->pos] = val;
    s->pos++;

    s->stack[s->pos] = NULL;
}

/*
 * Pop a value from the stack and return it
 */
static void* pop(stack_t* s)
{
    void *res;

    s->pos--;
    res = s->stack[s->pos];
    s->stack[s->pos] = NULL;

    return res;
}
