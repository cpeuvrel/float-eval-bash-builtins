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

    char str[SIZE_SLOT] = "";
    strncpy(str, argv[1], SIZE_SLOT);

    printf("%f\n", float_eval(str));
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

static void tokenifyStart(char *str, binTree* ast, int lookMult)
{
    int pos_curr = 0, parentheses = 0, mult_found = 0;
    char curr[SIZE_SLOT] = {0};

    int beginParentheses = str[0] == '(' ? 1 : 0;

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

        if (!lookMult && parentheses == 0 && (*str == '*' || * str == '/'))
            mult_found++;

        if ((*str >= '0' && *str <= '9') || parentheses != 0 ||
                *str == '.' ||
                (*str == ')' && (beginParentheses != 2 || *(str+1))) ||
                (!lookMult && (*str == '*' ||
                *str == '/' ))) {
            curr[pos_curr++] = *str;
        }
        else {
            curr[pos_curr] = '\0';

            if (beginParentheses == 2 && *str == ')')
                tokenifyStart(1+curr, ast, 0);
            else if (*str == '\0' && mult_found)
                tokenifyStart(curr, ast, mult_found);
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
    int pos_curr = 0, parentheses = 0, mult_found = 0;
    char curr[SIZE_SLOT] = {0};
    binTree *fst = findFirstEmpty(ast);

    int beginParentheses = str[0] == '(' ? 1 : 0;

    if (op == '\0' && ! beginParentheses) {
        sprintf(fst->val, "%s", str);
    }
    else {
        sprintf(fst->val, "%c",op);

        for (;;str++) {
            switch (*str) {
                case '(':
                    if (! parentheses)
                        beginParentheses++;
                    parentheses++;
                    break;
                case ')':
                    parentheses--;
            }

            if (parentheses == 1 && (*str == '+' || * str == '-'))
                mult_found++;

            if ((*str >= '0' && *str <= '9') || parentheses != 0 ||
                    (*str == ')' && (beginParentheses != 2 || *(str+1))) ||
                    *str == '.' ) {
                curr[pos_curr++] = *str;
            }
            else {
                curr[pos_curr] = '\0';

                if (beginParentheses == 2 && *str == ')')
                    tokenifyStart(1+curr, fst, !mult_found);
                else if (curr[0])
                    tokenify(curr, fst, *str);

                pos_curr = 0 ;
            }

            if (! *str)
                break;
        }

    }
}
