/*
 * =====================================================================================
 *
 *       Filename:  float-eval.h
 *
 *    Description:  Transform a string with arithmetic operations to a double
 *
 *        Version:  1.0
 *        Created:  02/07/2015 04:00:38 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Corentin Peuvrel (cpe), cpeuvrel@exosec.fr
 *
 * =====================================================================================
 */
#ifndef FLOAT_EVAL_H

#define FLOAT_EVAL_H

#define FLOAT_OPT_VERBOSE 1
#define SUM_LVL 8
#define MULT_LVL 9

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Bash includes required for builtins */

#include "builtins.h"
#include "shell.h"
#include "common.h"

#define HAS_WORD(wordlist) \
        ((wordlist) && (wordlist)->word && (wordlist)->word->word)

#include "binTree.h"

double float_eval(char* str, int flags);
int float_eval_builtin(WORD_LIST *list);

/*  builtin short documentation */
char float_eval_short_doc[] = "float_eval [-v] [-p INTEGER] STRING...";

/*  builtin long documentation */
char *float_eval_long_doc[] = {
    "Give one arithmetic string to be parsed in a double",
    "Use -v/--verbose to show the syntax tree constructed",
    "Use -p/--precision to set the number of decimal wanted",
    NULL
};

struct builtin float_eval_struct = {
    "float_eval",
    float_eval_builtin,
    BUILTIN_ENABLED,
    float_eval_long_doc,
    float_eval_short_doc,
    0
};

#endif /* end of include guard: FLOAT-EVAL_H */
