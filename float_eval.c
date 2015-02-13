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
static double computeAst(binTree* ast);
static void tokenify(char *str, binTree* ast, char op, int pass, int start);

int float_eval_builtin(WORD_LIST *list)
{
    char res[SIZE_SLOT] = "";

    if (!HAS_WORD(list)) {
        builtin_usage();
        return EX_USAGE;
    }

    strncpy(res, list->word->word , SIZE_SLOT);
    snprintf(res, SIZE_SLOT,"%f", float_eval(res));

    if (bind_variable("REPLY", res, 0) == NULL) {
        builtin_error("Failed to bind variable: REPLY");
        return EXECUTION_FAILURE;
    }

    return EXECUTION_SUCCESS;
}

double float_eval(char* str)
{
    double res = 0;
    binTree *ast = malloc(sizeof(binTree));
    initBinTree(ast);

    tokenify(str, ast, 0, 0, 1);

    res = computeAst(ast);
    freeBinTree(ast);

    return res;
}

static double computeAst(binTree* ast)
{
    //TODO: Check endptr to see if we miss sth
    if (!ast)
        return 0;
    else if (! isOp(ast->val[0]) || (ast->val[0] == '-' && ast->val[1]))
        return strtod(ast->val, NULL);

    switch (ast->val[0]) {
        case '+':
            return computeAst(ast->next1) + computeAst(ast->next2);
        case '-':
            return computeAst(ast->next1) - computeAst(ast->next2);
        case '*':
            return computeAst(ast->next1) * computeAst(ast->next2);
        case '/':
            return computeAst(ast->next1) / computeAst(ast->next2);
        case '&':
            return (int)computeAst(ast->next1) & (int)computeAst(ast->next2);
    }
    return 0;
}

static void tokenify(char *str, binTree* ast, char op, int pass, int start)
{
    int pos_curr = 0, parentheses = 0, found_next_op = 0;
    char curr[SIZE_SLOT] = {0}, tmp[SIZE_SLOT] = {0};
    binTree *fst = ast;

    int beginParentheses = str[0] == '(' ? 1 : 0;

    int beginSub = str[0] == '-' ? 1 : 0;
    if (beginSub) {
        if (start && str[1] == '(') {
            snprintf(tmp, SIZE_SLOT,"0%s", str);
            beginParentheses = 0;
            beginSub = 0;
            strncpy(str, tmp , SIZE_SLOT);
            str[SIZE_SLOT-1] = 0;
        }
        else if (start == 0 || op != 0)
            str++;
    }

    if (!start) {
        fst = findFirstEmpty(ast);

        if (op == 0 && !beginParentheses) {
            snprintf(fst->val, SIZE_SLOT,"%s", str);
            return;
        }
        fst->val[0] = op;
        fst->val[1] = 0;
    }

    for (;;str++) {
        switch (*str) {
            case '(':
                if ( parentheses == 0 )
                    beginParentheses++;
                parentheses++;
                break;
            case ')':
                parentheses--;
                break;
            case '\t':
            case ' ':
                continue;
        }

        if (start && parentheses == 0 && isOpCurrentPrio(*str,(pass+1)))
            found_next_op++;

        if ((*str >= '0' && *str <= '9') || parentheses != 0 ||
                *str == '.' ||
                (*str == ')' && (beginParentheses != 2 || *(str+1))) ||
                (start && isOpCurrentPrio(*str, (pass+1)))) {
            curr[pos_curr++] = *str;
            continue;
        }

        curr[pos_curr] = '\0';

        if (beginSub) {
            beginSub = 0;
            snprintf(tmp, SIZE_SLOT,"-%s", curr);
            tokenify(tmp, ast, *str, 0, 0);
        }
        else if (beginParentheses == 2 && *str == ')')
            tokenify(1+curr, fst, 0, 0, 1);
        else if (*str == '\0' && found_next_op)
            tokenify(curr, ast, 0, ++pass, 1);
        else if (start || (!start && curr[0]))
            tokenify(curr, fst, *str, 0, 0);

        pos_curr = 0 ;

        if (! *str)
            break;
    }
}

int isOpCurrentPrio(char op, int prio)
{
    char* opPrio[SIZE_SLOT] = {
        "&",
        "+-",
        "*/",
        ""
    };
    int i = 0;

    while (opPrio[prio][i]) {
        if (op == opPrio[prio][i++])
            return 1;
    }

    return 0;
}
