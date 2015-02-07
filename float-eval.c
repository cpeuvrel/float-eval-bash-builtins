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
    double res = float_eval("1+3*4");
    printf("Hello World : %f !\n", res);
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

static void compute_curr(char* curr, int *pos, double *val, int *op)
{
    double temp = 0;
    curr[*pos] = '\0';
    *pos = 0;
    temp = atof(curr);

    switch (*op) {
        case 1:
            *val += temp;
            break;
        case 2:
            *val -= temp;
            break;
        case 3:
            *val *= temp;
            break;
        case 4:
            *val /= temp;
            break;
        default:
            *val = temp;
    }
}
