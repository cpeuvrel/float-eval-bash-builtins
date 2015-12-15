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

static char* tokenify(char** str, int* type, int* parenthesis);

static void compute_op(char* op, stack_t* stack_o, stack_t* stack_v);
static int cmp_op(char* op1, char* op2);

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

/*
 * Take a string and return the next token from it
 * The sring is set to just after the token at the end of this function
 *
 * input:
 *   * str: a pointer to the string from which we want the next token
 *
 * output:
 *  * type: an integer set to the type of the token (FLOAT_EVAL_TYPES)
 *  * parenthesis: an integer set to the depth of parentheses
 *
 * return: the token itself
 */
static char* tokenify(char** str, int* type, int* parenthesis)
{
    char *beg;
    char *res;
    char op;
    int i, res_len;

    *type = FLOAT_EVAL_NULL;

    beg = *str;

    if (**str == '\0') {
        return NULL;
    }

    if ((**str >= '0' && **str <= '9') ||
            **str == '.') {
        *type = FLOAT_EVAL_NUM;
    }
    else {
        *type = FLOAT_EVAL_OP;
        op = **str;
    }

    (*str)++;

    // if it's a number, it might be on several chars
    if (*type == FLOAT_EVAL_NUM) {
        while ((**str >= '0' && **str <= '9') ||
             **str == '.')
            (*str)++;
    }

    // lenght of result is lenght of the part of the string we want + 1 for final \0
    res_len = *str - beg + 1;

    res = malloc(res_len * sizeof(char));

    i=0;
    while (i < res_len - 1) {
        res[i] = *(beg + aptr);
        i++;
    }
    res[i] = '\0';

    return res;
}

/*
 * Do what's needed with the new operator got
 */
static void compute_op(char* op, stack_t* stack_o, stack_t* stack_v)
{
    char *prev_op;
    mpfr_t *val1 = NULL, *val2 = NULL;
    mpfr_t *res;

    // If we have already an operator in the stack
    // and its priority is above or equal to current operator
    // pop last 2 values and do the previous operation
    while (stack_o->pos != 0 &&
           cmp_op(op, stack_o->stack[stack_o->pos - 1]) >= 0) {
        res = malloc(sizeof(mpfr_t));
        mpfr_init2(*res, PRECISION);

        prev_op = pop(stack_o);

        val1 = pop(stack_v);

        val2 = pop(stack_v);
        calulus(res, *val2, *val1, prev_op);
        mpfr_clear(*val2);
        free(val2);

        mpfr_clear(*val1);
        free(val1);
        free(prev_op);

        push(res, stack_v);
    }

    push(op, stack_o);
}

/*
 * Compare 2 operators
 *
 * The first chars can be '(' to indicate the level of parentheses
 *
 * Return:
 *  * 0: same priority
 *  * <0: op1 priority > op2
 *  * >0: op1 priority < op2
 */
static int cmp_op(char* op1_prio, char* op2_prio)
{
    int i, j, offset1 = 0, offset2 = 0, prio1 = -1, prio2 = -1;
    char *op1 = op1_prio, *op2 = op2_prio;

    while (*op1 == '(') {
        offset1++;
        op1++;
    }
    while (*op2 == '(') {
        offset2++;
        op2++;
    }

    for (i = 0; i < FLOAT_EVAL_MAX_PRIO; i++) {
        for (j = 0; op_prio[i][j]; j++) {
            if (prio1 == -1 && strncmp(op1, op_prio[i][j], FLOAT_EVAL_MAX_PRIO_LEN) == 0)
                prio1 = i;

            if (prio2 == -1 && strncmp(op2, op_prio[i][j], FLOAT_EVAL_MAX_PRIO_LEN) == 0)
                prio2 = i;
        }

        if (prio1 != -1 && prio2 != -1)
            break;
    }

    prio1 += offset1 * FLOAT_EVAL_MAX_PRIO;
    prio2 += offset2 * FLOAT_EVAL_MAX_PRIO;

    return (prio2 - prio1);
}
