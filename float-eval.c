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
static void tokenifyStart(char *str, binTree* ast, int lookMult);
static double computeAst(binTree* ast);

int main(int argc, const char *argv[])
{
    if (! argv[1]) {
        printf("Missing argument\n");
        return 1;
    }

    printf("%f !\n", float_eval(argv[1]));
    return 0;
}

double float_eval(char* str)
{
    double res = 0;
    binTree *ast = malloc(sizeof(binTree));
    initBinTree(ast);

    tokenifyStart(str, ast, 0);

    res = computeAst(ast);
    free(ast);

    return res;
}

static double computeAst(binTree* ast)
{
    if (! isOp(ast->val[0]))
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

static void tokenifyStart(char *str, binTree* ast, int lookMult)
{
    int pos_curr = 0, parenthesis = 0, op_found = 0;
    char curr[SIZE_SLOT] = {0};

    int beginParenthesis = str[0] == '(' ? 1 : 0;

    for (;;str++) {
        switch (*str) {
            case '(':
                parenthesis++;
                break;
            case ')':
                parenthesis--;
                break;
            case '\t':
            case ' ':
                continue;
        }

        if (!lookMult && parenthesis == 0 && (*str == '*' || * str == '/'))
            op_found++;

        if ((*str >= '0' && *str <= '9') || parenthesis != 0 ||
                *str == '.' ||
                (*str == ')' && (!beginParenthesis || *(str+1))) ||
                (!lookMult && (*str == '*' ||
                *str == '/' ))) {
            curr[pos_curr++] = *str;
        }
        else {
            curr[pos_curr] = '\0';

            if (*str == ')')
                tokenifyStart(1+curr, ast, 0);
            else if (*str == '\0' && op_found)
                tokenifyStart(curr, ast, op_found);
            else
                tokenify(curr, ast, *str);

            pos_curr = 0 ;
        }

        if (! *str)
            break;
    }
}

void tokenify(char* str, binTree* ast, char op)
{
    int pos_curr = 0, parenthesis = 0, op_found = 0;
    char curr[SIZE_SLOT] = {0};
    binTree *fst = findFirstEmpty(ast);

    int beginParenthesis = str[0] == '(' ? 1 : 0;

    if (op == '\0' && ! beginParenthesis) {
        sprintf(fst->val, "%s", str);
    }
    else {
        sprintf(fst->val, "%c",op);

        for (;;str++) {
            switch (*str) {
                case '(':
                    parenthesis++;
                    break;
                case ')':
                    parenthesis--;
            }

            if (parenthesis == 1 && (*str == '+' || * str == '-'))
                op_found++;

            if ((*str >= '0' && *str <= '9') || parenthesis != 0 ||
                    (!beginParenthesis && *str == ')') ||
                    *str == '.' ) {
                curr[pos_curr++] = *str;
            }
            else {
                curr[pos_curr] = '\0';

                if (*str == ')')
                    tokenifyStart(1+curr, fst, !op_found);
                else if (curr[0])
                    tokenify(curr, fst, *str);

                pos_curr = 0 ;
            }

            if (! *str)
                break;
        }

    }
}
