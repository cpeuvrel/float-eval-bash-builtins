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
    int curr_pos = 0, op = 0;
    char curr[64] = {0};
    double curr_val = 0;

    for (;;str++) {
        switch (*str) {
            case '\t':
            case ' ':
                continue;
        }

        if ((*str >= '0' && *str <= '9') || *str == '.') {
            curr[curr_pos++] = *str;
        }
        else {
            compute_curr(curr, &curr_pos, &curr_val, &op);

            switch (*str) {
                case '+':
                    op = 1;
                    continue;
                case '-':
                    op = 2;
                    continue;
                case '*':
                    op = 3;
                    continue;
                case '/':
                    op = 4;
                    continue;
            }
        }

        printf("%f\n", curr_val);

        if (! *str)
           break;
    }
    return curr_val;
}

void tokenify(char* curr, binTree* ast, char op)
{
    switch (op) {
        case '+':
            sprintf(findFirstEmpty(ast)->val, "%c",op);
            sprintf(findFirstEmpty(ast)->val, "%s", curr);
            break;
        case '\0':
            sprintf(findFirstEmpty(ast)->val, "%s", curr);
            break;
    }
}
