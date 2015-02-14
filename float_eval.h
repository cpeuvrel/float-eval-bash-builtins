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

double float_eval(char* str);

#endif /* end of include guard: FLOAT-EVAL_H */
