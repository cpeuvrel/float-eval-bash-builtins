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
static double compute_ast(bin_tree* ast);
static bin_tree* find_node_to_swap_modulo(bin_tree* t, bin_tree* save);
static int tokenify(char *str, bin_tree* ast, char* op, int pass, int start, char prev_op);
static int write_op(bin_tree* t, char* str);
static int hook_scientific_notation(char* str, char* curr, int* pos_curr);
static int write_op(bin_tree* t, char* str);
static int is_op_current_prio(char* op, int prio);
static int is_op_prio_above(char* op, int prio);
static int check_syntax(char* str);

int float_eval_builtin(WORD_LIST *list)
{
    char res[SIZE_SLOT] = "", output_format[17] = "%.3f";
    int flags = 0, i = 0;
    double precision = 3;

    if (!HAS_WORD(list)) {
        builtin_usage();
        return EX_USAGE;
    }

    SHELL_VAR *reply_init = make_new_array_variable("REPLY");
    if (array_p(reply_init) == 0) {
        builtin_error("Failed to bind array: REPLY");
        return EXECUTION_FAILURE;
    }
    ARRAY *reply = array_cell(reply_init);

    for (i = 0; HAS_WORD(list); list = list->next) {
        if (strncmp("-v", list->word->word, 3) == 0 ||
            strncmp("--verbose", list->word->word, 10) == 0) {
            flags |= FLOAT_OPT_VERBOSE;
            continue;
        }
        else if (strncmp("-p", list->word->word, 3) == 0 ||
                strncmp("--precision", list->word->word, 12) == 0) {
            char* end = NULL;
            precision = strtod(list->next->word->word, &end);

            if (!HAS_WORD(list->next->next) || end[0]) {
                builtin_error("You must give an integer next to '-p' option");
                return EXECUTION_FAILURE;
            }

            snprintf(output_format, 16, "%%.%df", (int) precision);

            list = list->next;
            continue;
        }

        strncpy(res, list->word->word , SIZE_SLOT);

        if (check_syntax(res))
            return EXECUTION_FAILURE;

        snprintf(res, SIZE_SLOT,output_format, float_eval(res, flags));

        if(array_insert(reply, i, res) < 0)
            printf("Insert failed\n");

        i++;
    }

    return EXECUTION_SUCCESS;
}

double float_eval(char* str, int flags)
{
    double res = 0;
    bin_tree *ast = malloc(sizeof(bin_tree));
    init_bin_tree(ast);

    tokenify(str, ast, "", 0, 1, 0);

    res = compute_ast(ast);
    if (flags & FLOAT_OPT_VERBOSE) {
        printf("==> %s\n", str);
        print_bin_tree(ast);
    }

    free_bin_tree(ast);

    return res;
}

static double compute_ast(bin_tree* ast)
{
    //TODO: Check endptr to see if we miss sth
    if (!ast)
        return 0;
    else if (! is_op(ast->val[0]) || (ast->val[0] == '-' && ast->val[1]))
        return strtod(ast->val, NULL);

    switch (ast->val[0]) {
        case '+':
            return compute_ast(ast->next1) + compute_ast(ast->next2);
        case '-':
            return compute_ast(ast->next1) - compute_ast(ast->next2);
        case '*':
            return compute_ast(ast->next1) * compute_ast(ast->next2);
        case '/':
            return compute_ast(ast->next1) / compute_ast(ast->next2);
        case '%':
            return (int)compute_ast(ast->next1) % (int)compute_ast(ast->next2);
        case '&':
            if (ast->val[1] == '&')
                return (int)compute_ast(ast->next1) && (int)compute_ast(ast->next2);
            return (int)compute_ast(ast->next1) & (int)compute_ast(ast->next2);
        case '^':
            return (int)compute_ast(ast->next1) ^ (int)compute_ast(ast->next2);
        case '|':
            if (ast->val[1] == '|')
                return (int)compute_ast(ast->next1) || (int)compute_ast(ast->next2);
            return (int)compute_ast(ast->next1) | (int)compute_ast(ast->next2);
        case '<':
            if (ast->val[1] == '=')
                return (int)compute_ast(ast->next1) <= (int)compute_ast(ast->next2);
            if (ast->val[1] == '<')
                return (int)compute_ast(ast->next1) << (int)compute_ast(ast->next2);
            return (int)compute_ast(ast->next1) < (int)compute_ast(ast->next2);
        case '>':
            if (ast->val[1] == '=')
                return (int)compute_ast(ast->next1) >= (int)compute_ast(ast->next2);
            else if (ast->val[1] == '>')
                return (int)compute_ast(ast->next1) >> (int)compute_ast(ast->next2);
            return (int)compute_ast(ast->next1) > (int)compute_ast(ast->next2);
        case '=':
            return (int)compute_ast(ast->next1) == (int)compute_ast(ast->next2);
        case '!':
            if (ast->val[1] == '=')
                return (int)compute_ast(ast->next1) != (int)compute_ast(ast->next2);
            return ! (int)compute_ast(ast->next2);
        case '~':
            return ~ (int)compute_ast(ast->next2);
    }
    return 0;
}

static int tokenify(char *str, bin_tree* ast, char* op, int pass, int start, char prev_op)
{
    int pos_curr = 0, parentheses = 0, found_next_op = 0, i = 0, offset = 0;
    char curr[SIZE_SLOT] = {0}, tmp[SIZE_SLOT] = {0};
    bin_tree *fst = ast;

    int begin_parentheses = str[0] == '(' ? 1 : 0;

    int begin_sub = str[0] == '-' ? 1 : 0;
    if (begin_sub) {
        if (start && str[1] == '(') {
            snprintf(tmp, SIZE_SLOT,"0%s", str);
            begin_parentheses = 0;
            begin_sub = 0;
            strncpy(str, tmp , SIZE_SLOT);
            str[SIZE_SLOT-1] = 0;
        }
        else if (pass == SUM_LVL && (start || op[0] != 0))
            str++;
    }

    if (!start) {
        fst = find_first_empty(ast);

        if (op[0] == 0 && !begin_parentheses) {
            snprintf(fst->val, SIZE_SLOT,"%s", str);
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

            strncpy(fst_tmp->val, t->val, SIZE_SLOT);
            fst_tmp->next1 = t->next1;
            fst_tmp->next2 = t->next2;

            new_root->next1 = fst_tmp;

            strncpy(t->val, new_root->val, SIZE_SLOT);
            t->next1 = new_root->next1;
            t->next2 = new_root->next2;

            free(new_root);
        }

        if (begin_sub && pass == SUM_LVL) {
            begin_sub = 0;
            snprintf(tmp, SIZE_SLOT,"-%s", curr);
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

    for (i = 0; op2[i][0] ; i++) {
        if (strncmp(op2[i], str , 2) == 0){
            strncpy(t->val, op2[i] , 2);
            t->val[2] = 0;
            return 1;
        }
    }

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
