/*
 * =====================================================================================
 *
 *       Filename:  binTree.h
 *
 *    Description:  Utilities for binary trees
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
#ifndef BINTREE_H

#define BINTREE_H

#include <stdio.h>
#include <stdlib.h>

#define SIZE_SLOT 64

typedef struct _binTree {
    char val[SIZE_SLOT];
    struct _binTree *next1;
    struct _binTree *next2;
} binTree;

void initBinTree(binTree* t);
void printBinTree(binTree* t);
binTree* findFirstEmpty(binTree* t);

#endif /* end of include guard: BINTREE_H */
