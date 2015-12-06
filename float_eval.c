/*
 * =====================================================================================
 *
 *       Filename:  float-eval.c
 *
 *    Description:  Transform a string with arithmetic operations to a double
 *
 *        Version:  1.0
 *        Created:  02/07/2015 03:54:05 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Corentin Peuvrel (cpe), cpeuvrel@exosec.fr
 *
 * =====================================================================================
 */

#include "float_eval.h"

#define PRECISION 256

static void compute_ast(mpfr_t *res, bin_tree* ast);
static bin_tree* find_node_to_swap_modulo(bin_tree* t, bin_tree* save);
static int tokenify(char *str, bin_tree* ast, char* op, int pass, int start, char prev_op);
static int write_op(bin_tree* t, char* str);
static int hook_scientific_notation(char* str, char* curr, int* pos_curr);
static int write_op(bin_tree* t, char* str);
static int is_op_current_prio(char* op, int prio);
static int is_op_prio_above(char* op, int prio);
static int check_syntax(char* str);

/*
 * Base for float_eval builtin
 */
int float_eval_builtin(WORD_LIST *list)
{
    char *res = NULL, output_format[17] = "%.3Rf", *end;
    int flags = 0, i = 0;
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
        if (strncmp("-v", list->word->word, 3) == 0 ||
            strncmp("--verbose", list->word->word, 10) == 0) {
            flags |= FLOAT_OPT_VERBOSE;
            continue;
        }
        else if (strncmp("-p", list->word->word, 3) == 0 ||
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

        // If syntax is incorect, exit
        if (check_syntax(res))
            return EXECUTION_FAILURE;

        // Do the magic here
        float_eval(&res_mpfr, res, flags);

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

/*
 * Evaluate the string str to a number res
 *
 * Default flag is 0. Only other supported is FLOAT_OPT_VERBOSE to print the AST (Abstract Syntax Tree).
 *
 * input: str, flags
 * output: res
 */
void float_eval(mpfr_t *res, char* str, int flags)
{
    bin_tree *ast = malloc(sizeof(bin_tree));
    init_bin_tree(ast);

    // Fill the AST
    tokenify(str, ast, "", 0, 1, 0);

    // Compute the AST and put final result in res
    compute_ast(res, ast);

    if (flags & FLOAT_OPT_VERBOSE) {
        printf("==> %s\n", str);
        print_bin_tree(ast);
    }

    // Cleanup
    free_bin_tree(ast);
}

/*
 * Compute the AST to have the resulting number
 *
 * Use recursive algorithm.
 *
 * input: ast
 * output: res
 */
static void compute_ast(mpfr_t *res,bin_tree* ast)
{
    int exp;

    mpfr_t n1, n2;
    mpz_t i1, i2, res_int;

    //TODO: Check endptr to see if we miss sth
    if (!ast) {
        // Empty ast means 0
        mpfr_set_zero(*res, 0);
        return;
    }
    else if (! is_op(ast->val[0]) || (ast->val[0] == '-' && ast->val[1])) {
        // If current node is a leaf, return its value
        mpfr_set_str(*res, ast->val, 0, MPFR_RNDN);
        return;
    }

    mpfr_init2(n1, PRECISION);
    mpfr_init2(n2, PRECISION);

    // We compute the next left and right nodes
    compute_ast(&n1, ast->next1);
    compute_ast(&n2, ast->next2);

    // FIXME: We used the simple way of always initializing Integers and floats
    //        variables, we should at least initialize integers only when needed

    mpz_init2(i1, PRECISION);
    mpz_init2(i2, PRECISION);
    mpz_init2(res_int, PRECISION);

    mpz_set_si(i1, mpfr_get_si(n1, MPFR_RNDN));
    mpz_set_si(i2, mpfr_get_si(n2, MPFR_RNDN));

    /* Do the right operation according to the operator
     * Some operators exist in logical and bitwise forms, For example :
     *  * & is bitwise AND. Result can be anything
     *  * && is logical AND. Result is either 1 or 0
     */
    switch (ast->val[0]) {
        case '+':
            mpfr_add(*res, n1, n2, MPFR_RNDN);
            break;
        case '-':
            mpfr_sub(*res, n1, n2, MPFR_RNDN);
            break;
        case '*':
            mpfr_mul(*res, n1, n2, MPFR_RNDN);
            break;
        case '/':
            mpfr_div(*res, n1, n2, MPFR_RNDN);
            break;
        case '%':
            mpfr_fmod(*res, n1, n2, MPFR_RNDN);
            break;
        case '&':
            mpz_and(res_int, i1, i2);
            mpfr_set_z(*res, res_int, MPFR_RNDN);

            if (ast->val[1] == '&') {
                // operator is && (logical AND)
                if ((mpfr_cmp_si(*res, 0)) == 0)
                    mpfr_set_zero(*res, 0);
                else
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
            }
            break;
        case '^':
            mpz_xor(res_int, i1, i2);
            mpfr_set_z(*res, res_int, MPFR_RNDN);
            break;
        case '|':
            mpz_ior(res_int, i1, i2);
            mpfr_set_z(*res, res_int, MPFR_RNDN);

            if (ast->val[1] == '|') {
                // operator is || (logical OR)
                if ((mpfr_cmp_si(*res, 0)) == 0)
                    mpfr_set_zero(*res, 0);
                else
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
            }
            break;
        case '<':
            if (ast->val[1] == '=') {
                // operator is <=
                if (mpfr_lessequal_p(n1, n2))
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
                else
                    mpfr_set_zero(*res, 0);
            }
            else if (ast->val[1] == '<') {
                // operator is << (bitwise left shift)
                exp = (int)mpfr_get_si(n2, MPFR_RNDN);
                mpfr_set_ui_2exp(n2, 1, exp, MPFR_RNDN);
                mpfr_mul(*res, n1, n2, MPFR_RNDN);
            }
            else {
                // operator is <
                if (mpfr_less_p(n1, n2))
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
                else
                    mpfr_set_zero(*res, 0);
            }
            break;
        case '>':
            if (ast->val[1] == '=') {
                // operator is >=
                if (mpfr_greaterequal_p(n1, n2))
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
                else
                    mpfr_set_zero(*res, 0);
            }
            else if (ast->val[1] == '>') {
                // operator is >> (bitwise right shift)
                exp = (int)mpfr_get_ld(n2, MPFR_RNDN);
                mpfr_set_ui_2exp(n2, 1, exp, MPFR_RNDN);
                mpfr_div(*res, n1, n2, MPFR_RNDN);
            }
            else {
                // operator is >
                if (mpfr_greater_p(n1, n2))
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
                else
                    mpfr_set_zero(*res, 0);
            }
            break;
        case '=':
            if (mpfr_equal_p(n1, n2))
                mpfr_set_ui(*res, 1, MPFR_RNDN);
            else
                mpfr_set_zero(*res, 0);
            break;
        case '!':
            if (ast->val[1] == '=') {
                // operator is !=
                if (mpfr_lessgreater_p(n1, n2))
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
                else
                    mpfr_set_zero(*res, 0);
            }
            else {
                // operator is ! (logical NOT)
                if ((mpfr_cmp_si(n2, 0)) == 0)
                    mpfr_set_ui(*res, 1, MPFR_RNDN);
                else
                    mpfr_set_zero(*res, 0);
            }
            break;
        case '~':
            mpz_com(res_int, i1);
            mpfr_set_z(*res, res_int, MPFR_RNDN);
            break;
    }

    // Cleanup
    mpfr_clear(n1);
    mpfr_clear(n2);
    mpz_clear(i1);
    mpz_clear(i2);
    mpz_clear(res_int);
}

/*
 * Fill the AST with the content of the string
 *
 * Use recursive algorithm.
 *
 * input: str
 * output: ast
 */
static int tokenify(char *str, bin_tree* ast, char* op, int pass, int start, char prev_op)
{
    int pos_curr = 0, parentheses = 0, found_next_op = 0, i = 0, offset = 0;

    size_t slot_len = strlen(str)+2;

    char *curr = calloc(slot_len, sizeof(char));
    char *tmp = calloc(slot_len, sizeof(char));

    bin_tree *fst = ast;

    int begin_parentheses = (str[0] == '(' ? 1 : 0);

    int begin_sub = (str[0] == '-' ? 1 : 0);

    if (begin_sub) {
        if (start && str[1] == '(') {
            snprintf(tmp, slot_len,"0%s", str);
            begin_parentheses = 0;
            begin_sub = 0;
            strncpy(str, tmp , slot_len);
            str[slot_len-1] = 0;
        }
        else if (pass == SUM_LVL && (start || op[0] != 0))
            str++;
    }

    if (!start) {
        fst = find_first_empty(ast);

        if (op[0] == 0 && !begin_parentheses) {
            snprintf(fst->val, slot_len,"%s", str);
            return 0;
        }
        offset = write_op(fst, op);
    }

    for (;;str++) {
        switch (*str) {
            case '(':
                if ( parentheses == 0 && begin_parentheses)
                    begin_parentheses++;
                parentheses++;
                break;
            case ')':
                parentheses--;
                break;
            case 'e':
            case 'E':
                if (parentheses != 0)
                    break;
                str += hook_scientific_notation(str, curr, &pos_curr);
                continue;
            case '\t':
            case ' ':
                continue;
        }

        if (parentheses == 0 && is_op_prio_above(str,pass))
            found_next_op++;

        if (*str != '\0' && ((*str >= '0' && *str <= '9') || parentheses != 0 ||
                *str == '.' ||
                (*str == ')' && (begin_parentheses != 2 || *(str+1))) ||
                (*str != ')' && !is_op_current_prio(str, pass)))) {
            curr[pos_curr++] = *str;
            continue;
        }

        curr[pos_curr] = '\0';

        if (prev_op && *str && (*str == '%' || prev_op == '%') ) {
            bin_tree *new_root = malloc(sizeof(bin_tree));
            init_bin_tree(new_root);
            bin_tree *fst_tmp = malloc(sizeof(bin_tree));
            init_bin_tree(fst_tmp);
            bin_tree *t = fst;

            t = find_node_to_swap_modulo(fst, NULL);

            strncpy(fst_tmp->val, t->val, slot_len);
            fst_tmp->next1 = t->next1;
            fst_tmp->next2 = t->next2;

            new_root->next1 = fst_tmp;

            strncpy(t->val, new_root->val, slot_len);
            t->next1 = new_root->next1;
            t->next2 = new_root->next2;

            free(new_root);
        }

        if (begin_sub && pass == SUM_LVL) {
            begin_sub = 0;
            snprintf(tmp, slot_len,"-%s", curr);
            i = tokenify(tmp, ast, str, 0, 0, 0);
        }
        else if (begin_parentheses == 2 && *str == ')')
            i = tokenify(1+curr, fst, "", 0, 1, 0);
        else if (*str == '\0' && found_next_op)
            i = tokenify(curr, ast, "", ++pass, 1, 0);
        else if (start || (!start && curr[0]))
            i = tokenify(curr, fst, str, 0, 0, prev_op);

        pos_curr = 0 ;

        for (; i > 0; i--)
            str++;

        if (is_op(*str))
           prev_op = *str;

        if (! *str)
            break;
    }
    return offset;
}

static int is_op_prio_above(char* op, int prio)
{
    int res = 0;

    if ((op[0] >= '0' && op[0] <= '9') || op[0] == '.' || op[0] == '\0')
        return 0;

    while ((res = is_op_current_prio(op,++prio)) == 0) {}

    if (res == -1)
        return 0;
    return 1;

}

static int is_op_current_prio(char* op, int prio)
{
    char* op_prio[16][8] = {
        {"||"},
        {"&&"},
        {"|"},
        {"^"},
        {"&"},
        {"==", "!="},
        {"<=", ">=", "<", ">"},
        {"<<", ">>"},
        {"+", "-"},
        {"*", "/", "%"},
        {"!", "~"},
        {""}
    };
    int i = 0;

    if ((op[0] >= '0' && op[0] <= '9') || op[0] == '.' || op[0] == '\0')
        return 0;

    if (!op_prio[prio][0][0])
       return -1;

    while (op_prio[prio][i]) {
        if (strncmp(op, op_prio[prio][i], strlen(op_prio[prio][i])) == 0)
            return 1;
        i++;
    }

    return 0;
}

static bin_tree* find_node_to_swap_modulo(bin_tree* t, bin_tree* save)
{
    bin_tree* left = NULL;
    if (save == NULL)
        save = t;

    if (!(left = find_first_left_empty(t->next1)) && !is_op_current_prio(t->val, MULT_LVL))
        return find_node_to_swap_modulo(t->next2, NULL);

    if (left)
        return find_node_to_swap_modulo(t->next1, NULL);
    else if (t->next2)
        return find_node_to_swap_modulo(t->next2, save);

    return save;
}

static int write_op(bin_tree* t, char* str)
{
    int i = 0;
    char op2[32][3] = {
        "||",
        "&&",
        "<=",
        ">=",
        "==",
        "!=",
        "<<",
        ">>",
        ""
    };
    char op_un[32] = "!~" ;

    // Look for binary operators with 2 chars
    for (i = 0; op2[i][0] ; i++) {
        if (strncmp(op2[i], str , 2) == 0){
            strncpy(t->val, op2[i] , 2);
            t->val[2] = 0;
            return 1;
        }
    }

    // Look for unary operators
    for (i = 0; op_un[i] ; i++) {
         if (*str == op_un[i]) {
             bin_tree *empty = malloc(sizeof(bin_tree));
             init_bin_tree(empty);
             empty->val[0] = '0';
             empty->val[1] = 0;
             t->next1 = empty;
             break;
         }
    }

    t->val[0] = str[0];
    t->val[1] = 0;
    return 0;
}

static int hook_scientific_notation(char* str, char* curr, int* pos_curr)
{
    int offset = 1;
    curr += *pos_curr;

    // The 'E'
    *(curr++) = *(str++);
    (*pos_curr)++;
    // Either '+' '-' or the first number of the exponent
    *(curr++) = *(str++);
    (*pos_curr)++;

    while (*str && !is_op(*str)){
        *(curr++) = *(str++);
        (*pos_curr)++;
        offset++;
    }

    return offset;
}

static int check_syntax(char* str)
{
    int parentheses = 0, unbalenced = 0;
    char err[32] = "";

    for (;*str; str++) {
        switch (*str) {
            case '(':
                parentheses++;
                break;
            case ')':
                parentheses--;
                break;
            case 'e':
            case 'E':
                // TODO
                break;
            case ' ':
            case '\t':
                continue;
            default:
                if (!is_op(*str) && (*str < '0' || *str > '9') && *str != '.') {
                    snprintf(err, 32, "Unrecognized character : '%c'", *str);
                    builtin_error(err);
                    return 1;
                }
        }

        if (parentheses < 0)
            unbalenced = 1;
    }

    if (parentheses || unbalenced) {
        builtin_error("Unbalenced parentheses");
        return 1;
    }

    return 0;
}
