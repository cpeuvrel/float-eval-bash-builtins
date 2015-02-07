/*
 * =====================================================================================
 *
 *       Filename:  binTree.c
 *
 *    Description:  Utilities for binary trees
 *
 *        Version:  1.0
 *        Created:  02/07/2015 05:56:35 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Corentin Peuvrel (cpe), cpeuvrel@exosec.fr
 *
 * =====================================================================================
 */

#include "binTree.h"
static void printBinTreeDepth(binTree*, int lvl, int pos);

void initBinTree(binTree* t)
{
    t->val[0] = 0;
    t->next1 = NULL;
    t->next2 = NULL;
}

void printBinTree(binTree* t)
{
    printBinTreeDepth(t, 0, 2);
}

static void printBinTreeDepth(binTree* t, int lvl, int pos)
{
    int i;
    for (i = 0; i < lvl; i++) {
        printf(" ");
    }

    if (pos == 1)
        printf("|");
    else
        printf("\\");
            
    printf("- %s\n",t->val);

    lvl++;

    if (t->next1)
        printBinTreeDepth(t->next1, lvl, 1);
    if (t->next2)
        printBinTreeDepth(t->next2, lvl, 2);
}

binTree* findFirstEmpty(binTree* t)
{
    if (!t->next1 && !t->next2 && !t->val[0])
        return t;

    binTree *res = t, *new = malloc(sizeof(binTree));
    initBinTree(new);

    while (res->next1 && res->next2)
       res = res->next2; 

    if (res->next1)
        res->next2 = new;
    else
        res->next1 = new;

    return new;
}