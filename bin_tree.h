/*
 * =====================================================================================
 *
 *       Filename:  bin_tree.h
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

typedef struct _bin_tree {
    char val[SIZE_SLOT];
    struct _bin_tree *next1;
    struct _bin_tree *next2;
} bin_tree;

void init_bin_tree(bin_tree* t);
void print_bin_tree(bin_tree* t);
void free_bin_tree(bin_tree* t);
bin_tree* find_first_empty(bin_tree* t);
bin_tree* find_first_left_empty(bin_tree* t);
int is_op(char str);

#endif /* end of include guard: BINTREE_H */
