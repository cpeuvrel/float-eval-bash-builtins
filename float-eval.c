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

#include "float-eval.h"
static double computeAst(binTree* ast);
static void tokenify(char *str, binTree* ast, char op, int lookMult, int start);

int main(int argc, const char *argv[])
{
    if (! argv[1]) {
        printf("Missing argument\n");
        return 1;
    }

    char str[SIZE_SLOT] = "";
    strncpy(str, argv[1], SIZE_SLOT);

    str[SIZE_SLOT-1] = 0;
    printf("%f\n", float_eval(str));
    return 0;
}

double float_eval(char* str)
{
    double res = 0;
    binTree *ast = malloc(sizeof(binTree));
    initBinTree(ast);

    tokenify(str, ast, 0, 0, 1);

    res = computeAst(ast);
    free(ast);

    return res;
}

static double computeAst(binTree* ast)
{
    if (!ast)
        return 0;
    else if (! isOp(ast->val[0]) || (ast->val[0] == '-' && ast->val[1]))
        return atof(ast->val);

    switch (ast->val[0]) {
        case '+':
            return computeAst(ast->next1) + computeAst(ast->next2);
        case '-':
            return computeAst(ast->next1) - computeAst(ast->next2);
        case '*':
            return computeAst(ast->next1) * computeAst(ast->next2);
        case '/':
            return computeAst(ast->next1) / computeAst(ast->next2);
    }
    return 0;
}

static void tokenify(char *str, binTree* ast, char op, int lookMult, int start)
{
    int pos_curr = 0, parentheses = 0, mult_found = 0;
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
        else if (start || op != 0)
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

        if (start &&!lookMult && parentheses == 0 && (*str == '*' || * str == '/'))
            mult_found++;

        if ((*str >= '0' && *str <= '9') || parentheses != 0 ||
                *str == '.' ||
                (*str == ')' && (beginParentheses != 2 || *(str+1))) ||
                (!lookMult && start && (*str == '*' ||
                *str == '/' ))) {
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
            tokenify(1+curr, fst, 0, !start && !mult_found, 1);
        else if (*str == '\0' && mult_found && start)
            tokenify(curr, ast, 0, mult_found, 1);
        else if (start || (!start && curr[0]))
            tokenify(curr, fst, *str, 0, 0);

        pos_curr = 0 ;

        if (! *str)
            break;
    }
}
