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

int main(int argc, const char *argv[])
{
    float_eval("1+3*4+2+8");
    return 0;
}

double float_eval(char* str)
{
    int pos_curr = 0;
    char curr[SIZE_SLOT] = {0};

    binTree *ast = malloc(sizeof(binTree));
    initBinTree(ast);

    for (;;str++) {
        switch (*str) {
            case '\t':
            case ' ':
                continue;
        }

        if ((*str >= '0' && *str <= '9') ||
                *str == '.' ||
                *str == '*' ||
                *str == '/' ||
                *str == '(' ||
                *str == ')' ) {
            curr[pos_curr++] = *str;
        }
        else {
            curr[pos_curr] = '\0';
            tokenify(curr, ast, *str);
            pos_curr = 0 ;
        }

        if (! *str)
            break;
    }

    free(ast);

    return 0;
}

void tokenify(char* str, binTree* ast, char op)
{
    int pos_curr = 0;
    char curr[SIZE_SLOT] = {0};
    binTree *fst = findFirstEmpty(ast);


    if (op == '\0') {
        sprintf(fst->val, "%s", str);
    }
    else {
        sprintf(fst->val, "%c",op);

        for (;;str++) {
            if ((*str >= '0' && *str <= '9') ||
                    *str == '.' ||
                    *str == '(' ||
                    *str == ')' ) {
                curr[pos_curr++] = *str;
            }
            else {
                curr[pos_curr] = '\0';
                tokenify(curr, fst, *str);
                pos_curr = 0 ;
            }

            if (! *str)
                break;
        }

    }
}
