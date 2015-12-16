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
    {"#", "_", "!", "~", ""},
    {""}
};

static void init_stack(stack_t* s);
static void clear(stack_t s);
static void clear_mpfr(stack_t s);
static void push(void* val, stack_t* s);
static void* pop(stack_t* s);
#ifdef DEBUG
static void print_stack(stack_t s);
static void print_mpfr_stack(stack_t s);
#endif

static char* tokenify(char** str, int* type, int* parenthesis);
static void end_calculus(stack_t* stack_o, stack_t* stack_v);

static void compute_op(char* op, stack_t* stack_o, stack_t* stack_v);
static int calulus(mpfr_t* res, mpfr_t v1, mpfr_t v2, char* op);
static int calulus_unary(mpfr_t* res, mpfr_t* v, char* op);
static int is_unary(const char* op);
static int cmp_op(char* op1, char* op2);

/*
 * Base for float_eval builtin
 */
#ifdef BASH_BUILTIN
int float_eval_builtin(WORD_LIST *list)
{
    char *res = NULL, output_format[17] = "%.3Rf", *end;
    int i = 0;
    size_t slot_len;
    double precision = 3;
    SHELL_VAR *reply_init;
    ARRAY *reply;
    mpfr_t res_mpfr;

    mpfr_init2(res_mpfr, PRECISION);

    // If there is no args, print usage and exit
    if (!HAS_WORD(list)) {
        builtin_usage();
        return EX_USAGE;
    }

    // Init Bash variables
    reply_init = make_new_array_variable("REPLY");
    if (array_p(reply_init) == 0) {
        builtin_error("Failed to bind array: REPLY");
        return EXECUTION_FAILURE;
    }
    reply = array_cell(reply_init);

    // Parse args
    for (i = 0; HAS_WORD(list); list = list->next) {
        if (strncmp("-p", list->word->word, 3) == 0 ||
                strncmp("--precision", list->word->word, 12) == 0) {
            // sets the number of digits to keep after the comma (default: 3)
            end = NULL;
            precision = strtod(list->next->word->word, &end);

            if (!HAS_WORD(list->next->next) || end[0]) {
                builtin_error("You must give an integer next to '-p' option");
                return EXECUTION_FAILURE;
            }

            snprintf(output_format, 16, "%%.%dRf", (int) precision);

            list = list->next;
            continue;
        }
        else if (strncmp("-h", list->word->word, 3) == 0 ||
            strncmp("--help", list->word->word, 7) == 0) {
            builtin_usage();
            return EX_USAGE;
        }

        // For each args that aren't an option, we try to parse it as a number

        slot_len = strlen(list->word->word)+2;
        res = calloc(slot_len, sizeof(char));
        strncpy(res, list->word->word , slot_len);

        // Be sure that we can put at least the string "nan"
        if (slot_len < 4)
            slot_len=4;

        // If syntax is incorect, exit
        /*if (check_syntax(res))*/
            /*return EXECUTION_FAILURE;*/

        // Do the magic here
        float_eval(&res_mpfr, res);

        // Append to Bash array
        mpfr_snprintf(res, slot_len, output_format, res_mpfr);
        if(array_insert(reply, i, res) < 0)
            printf("Insert failed\n");

        i++;
    }

    // Cleanup
    mpfr_clear(res_mpfr);

    return EXECUTION_SUCCESS;
}

#else

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

    mpfr_out_str(stderr, 10, 3, val, MPFR_RNDN);
    putc('\n', stderr);

    // Cleanup
    mpfr_clear(val);

    return 0;
}
#endif /* end of ifdef BASH_BUILTIN */

/*
 * Eval a string writen in infix notation
 */
void float_eval(mpfr_t *res, char* str)
{
    stack_t stack_v;     // Stack with all values
    stack_t stack_o;     // Stack with operators

    char *val;
    mpfr_t *val_num, *ptr;
    int type, last_type, parenthesis = 0;

    init_stack(&stack_v);
    init_stack(&stack_o);
    last_type = FLOAT_EVAL_NULL;

    while ((val = tokenify(&str, &type, &parenthesis))) {
#ifdef DEBUG
        fprintf(stderr, "\n--> %s\n", val);
#endif /*  end ifdef DEBUG */

        if (type == FLOAT_EVAL_NUM) {
            val_num = malloc(sizeof(mpfr_t));
            mpfr_init2(*val_num, PRECISION);
            mpfr_set_str(*val_num, val, 0, MPFR_RNDN);

            push(val_num, &stack_v);
            free(val);
        }
        else if (type == FLOAT_EVAL_OP) {
            if (last_type == FLOAT_EVAL_OP || last_type == FLOAT_EVAL_NULL) {
                if (strncmp(val, "-", 2) == 0)
                    val[0] = '_';
                else if (strncmp(val, "+", 2) == 0)
                    val[0] = '#';
            }

            compute_op(val, &stack_o, &stack_v);
        }
        last_type = type;

#ifdef DEBUG
        fprintf(stderr, "##### VALUES #####\n");
        print_mpfr_stack(stack_v);
        fprintf(stderr, "#### OPERATORS ###\n");
        print_stack(stack_o);
        fprintf(stderr, "##################\n\n");
#endif /*  end ifdef DEBUG */
    }

#ifdef DEBUG
    fprintf(stderr, ">>> Before end_calculus >>>\n");
        fprintf(stderr, "##### VALUES #####\n");
    print_mpfr_stack(stack_v);
        fprintf(stderr, "#### OPERATORS ###\n");
    print_stack(stack_o);
    fprintf(stderr, "<<<\n");
#endif /*  end ifdef DEBUG */

    end_calculus(&stack_o, &stack_v);

#ifdef DEBUG
    fprintf(stderr, ">>> After end_calculus >>>\n");
        fprintf(stderr, "##### VALUES #####\n");
    print_mpfr_stack(stack_v);
        fprintf(stderr, "#### OPERATORS ###\n");
    print_stack(stack_o);
    fprintf(stderr, "<<<\n");
#endif /*  end ifdef DEBUG */

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

#ifdef DEBUG
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
#endif /*  end ifdef DEBUG */

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

    if (s->pos < 1) {
        return NULL;
    }

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
    int i, j, offset_parent, res_len;

    *type = FLOAT_EVAL_NULL;

    while (**str == ' ' || **str == '(' || **str == ')') {
        switch (**str) {
            case '(':
                (*parenthesis)++;
                break;
            case ')':
                (*parenthesis)--;
                break;
        }
        (*str)++;
    }

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
    else if (*type == FLOAT_EVAL_OP) {
        switch (op) {
            case '<':   // <=  (or << with fallthrough)
            case '>':   // >=  (or >> with fallthrough)
                if (**str == '=') {
                    (*str)++;
                    break;
                }
                /* FALLTHROUGH */
            case '|':   // ||
            case '&':   // &&
            case '=':   // ==
            case '*':   // **
                if (**str == op)
                    (*str)++;
                break;
            case '!':   // !=
                if (**str == '=')
                    (*str)++;
                break;
        }
    }

    // lenght of result is lenght of the part of the string we want + 1 for final \0
    // + if it's an operator, the depth of parentheses (to remember the priority)
    offset_parent = (*type == FLOAT_EVAL_OP ? *parenthesis : 0);
    res_len = *str - beg + 1 + offset_parent;

    res = malloc(res_len * sizeof(char));

    i=0;
    for (i = 0; i < offset_parent; i++) {
        res[i] = '(';
    }

    j = i;

    while (i < res_len - 1) {
        int aptr = i - offset_parent;
        if (*(beg + aptr) == ' ') {
            i++;
            continue;
        }

        res[j] = *(beg + aptr);
        i++;
        j++;
    }
    res[j] = '\0';

    return res;
}

/*
 * Finish the stacks to have the final value
 */
static void end_calculus(stack_t* stack_o, stack_t* stack_v)
{
    char *op;
    mpfr_t *res;
    mpfr_t *val1, *val2;
    int unary;

    if (stack_o->pos == 0)
        return;

    while (stack_o->pos > 0) {
        res = malloc(sizeof(mpfr_t));
        mpfr_init2(*res, PRECISION);

        op = pop(stack_o);
        unary = is_unary(op);

        val1 = pop(stack_v);

        if (unary) {
            calulus_unary(res, val1, op);
        }
        else {
            val2 = pop(stack_v);
            calulus(res, *val2, *val1, op);
            mpfr_clear(*val2);
            free(val2);
        }

        mpfr_clear(*val1);
        free(val1);
        free(op);

        push(res, stack_v);
    }
}

/*
 * Do what's needed with the new operator got
 */
static void compute_op(char* op, stack_t* stack_o, stack_t* stack_v)
{
    char *prev_op;
    mpfr_t *val1 = NULL, *val2 = NULL;
    mpfr_t *res;
    int unary;

    // If we have already an operator in the stack
    // and its priority is above or equal to current operator
    // pop last 2 values and do the previous operation
    while (stack_o->pos != 0 &&
           cmp_op(op, stack_o->stack[stack_o->pos - 1]) >= 0) {
        res = malloc(sizeof(mpfr_t));
        mpfr_init2(*res, PRECISION);

        prev_op = pop(stack_o);
        unary = is_unary(prev_op);

        val1 = pop(stack_v);

        if (unary) {
            calulus_unary(res, val1, prev_op);
        }
        else {
            val2 = pop(stack_v);
            calulus(res, *val2, *val1, prev_op);
            mpfr_clear(*val2);
            free(val2);
        }

        mpfr_clear(*val1);
        free(val1);
        free(prev_op);

        push(res, stack_v);
    }

    push(op, stack_o);
}

/*
 * Compute the result for the binary operator on the values
 */
static int calulus(mpfr_t* res, mpfr_t val1, mpfr_t val2, char* op)
{
    int offset = 0, exp, ret = 1;
    mpfr_t pow;

    mpz_t int1, int2, res_int;

    mpz_init2(int1, PRECISION);
    mpz_init2(int2, PRECISION);
    mpz_init2(res_int, PRECISION);

    mpz_set_si(int1, mpfr_get_si(val1, MPFR_RNDN));
    mpz_set_si(int2, mpfr_get_si(val2, MPFR_RNDN));

    while (op[offset] == '(')
        offset++;

    /* 
     * Do the right operation according to the operator
     * Some operators exist in logical and bitwise forms, For example :
     *  * & is bitwise AND. Result can be anything
     *  * && is logical AND. Result is either 1 or 0
     */
    switch (op[offset]) {
        case '+':
            mpfr_add(*res, val1, val2, MPFR_RNDN);
            break;
        case '-':
            mpfr_sub(*res, val1, val2, MPFR_RNDN);
            break;
        case '*':
            if (op[offset+1] == '*') {
                mpfr_pow(*res, val1, val2, MPFR_RNDN);
            }
            else {
                mpfr_mul(*res, val1, val2, MPFR_RNDN);
            }
            break;
        case '/':
            mpfr_div(*res, val1, val2, MPFR_RNDN);
            break;
        case '%':
            mpfr_fmod(*res, val1, val2, MPFR_RNDN);
            break;
        case '&':
            mpz_and(res_int, int1, int2);
            mpfr_set_z(*res, res_int, MPFR_RNDN);

            if (op[offset+1] == '&') {
                // operator is && (logical AND)
                if ((mpfr_cmp_si(*res, 0)) == 0)
                    mpfr_set_zero(*res, 0);
                else
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
            }
            break;
        case '^':
            mpz_xor(res_int, int1, int2);
            mpfr_set_z(*res, res_int, MPFR_RNDN);
            break;
        case '|':
            mpz_ior(res_int, int1, int2);
            mpfr_set_z(*res, res_int, MPFR_RNDN);

            if (op[offset+1] == '|') {
                // operator is || (logical OR)
                if ((mpfr_cmp_si(*res, 0)) == 0)
                    mpfr_set_zero(*res, 0);
                else
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
            }
            break;
        case '<':
            if (op[offset+1] == '=') {
                // operator is <=
                if (mpfr_lessequal_p(val1, val2))
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
                else
                    mpfr_set_zero(*res, 0);
            }
            else if (op[offset+1] == '<') {
                // operator is << (bitwise left shift)
                exp = (int)mpfr_get_si(val2, MPFR_RNDN);
                mpfr_set_ui_2exp(val2, 1, exp, MPFR_RNDN);
                mpfr_mul(*res, val1, val2, MPFR_RNDN);
            }
            else {
                // operator is <
                if (mpfr_less_p(val1, val2))
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
                else
                    mpfr_set_zero(*res, 0);
            }
            break;
        case '>':
            if (op[offset+1] == '=') {
                // operator is >=
                if (mpfr_greaterequal_p(val1, val2))
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
                else
                    mpfr_set_zero(*res, 0);
            }
            else if (op[offset+1] == '>') {
                // operator is >> (bitwise right shift)
                exp = (int)mpfr_get_ld(val2, MPFR_RNDN);
                mpfr_set_ui_2exp(val2, 1, exp, MPFR_RNDN);
                mpfr_div(*res, val1, val2, MPFR_RNDN);
            }
            else {
                // operator is >
                if (mpfr_greater_p(val1, val2))
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
                else
                    mpfr_set_zero(*res, 0);
            }
            break;
        case '=':
            if (mpfr_equal_p(val1, val2))
                mpfr_set_ui(*res, 1, MPFR_RNDN);
            else
                mpfr_set_zero(*res, 0);
            break;
        case '!':
            // operator is !=
            if (mpfr_lessgreater_p(val1, val2))
                mpfr_set_ui(*res, 1, MPFR_RNDN);
            else
                mpfr_set_zero(*res, 0);
            break;
        case 'e':
        case 'E':
            mpfr_init2(pow, PRECISION);
            mpfr_set_ui(pow, 10, MPFR_RNDN);

            mpfr_pow(pow, pow, val2, MPFR_RNDN);
            mpfr_mul(*res, val1, pow, MPFR_RNDN);

            mpfr_clear(pow);
            break;
        default:
            mpfr_set_nan(*res);
            ret = 0;
            break;
    }

    mpz_clear(res_int);
    mpz_clear(int1);
    mpz_clear(int2);

    return ret;
}

/*
 * Compute the result for the unary operator on the value
 */
static int calulus_unary(mpfr_t* res, mpfr_t* val, char* op)
{
    int offset = 0, ret = 1;

    mpfr_t zero;
    mpz_t val_int, res_int;

    mpz_init2(val_int, PRECISION);
    mpz_init2(res_int, PRECISION);

    mpz_set_si(val_int, mpfr_get_si(*val, MPFR_RNDN));

    while (op[offset] == '(')
        offset++;

    switch (op[offset]) {
        case '#':
            mpfr_init2(zero, PRECISION);
            mpfr_set_zero(zero, 0);

            mpfr_add(*res, zero, *val, MPFR_RNDN);

            mpfr_clear(zero);
            break;
        case '_':
            mpfr_init2(zero, PRECISION);
            mpfr_set_zero(zero, 0);

            mpfr_sub(*res, zero, *val, MPFR_RNDN);

            mpfr_clear(zero);
            break;
        case '!':
            if ((mpfr_cmp_si(*val, 0)) == 0)
                mpfr_set_ui(*res, 1, MPFR_RNDN);
            else
                mpfr_set_zero(*res, 0);
            break;
        case '~':
            mpz_com(res_int, val_int);
            mpfr_set_z(*res, res_int, MPFR_RNDN);
            break;
        default:
            mpfr_set_nan(*res);
            ret = 0;
            break;
    }

    mpz_clear(res_int);
    mpz_clear(val_int);

    return ret;
}

/*
 * Is the operator given is unary (instead of binary) ?
 *
 * i.e. : is it one of # _ ! ~
 */
static int is_unary(const char* op)
{
    if (op[1] == '\0' &&
        (op[0] == '#' || op[0] == '_' || op[0] == '!' || op[0] == '~'))
        return 1;

    return 0;
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
