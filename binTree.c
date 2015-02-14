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
static int isLeafOp(binTree* t);

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

void freeBinTree(binTree* t)
{
    if (t->next1)
        freeBinTree(t->next1);
    if (t->next2)
        freeBinTree(t->next2);
    free(t);
}

binTree* findFirstEmpty(binTree* t)
{
    if (!t->val[0])
        return t;

    binTree *res = t, *tmp = NULL;

    while (res->val[0] && res->next1 && ! (tmp = findFirstLeftEmpty(res->next1)) && res->next2)
        res = res->next2;

    if (! res->val[0])
        return res;

    binTree *new = malloc(sizeof(binTree));
    initBinTree(new);

    if (tmp){
        if (!tmp->val[0]) {
            free(new);
            new = tmp;
        }
        else if (tmp->next1)
            tmp->next2 = new;
        else
            tmp->next1 = new;
    }
    else if (!res->next1)
        res->next1 = new;
    else
        res->next2 = new;

    return new;
}

binTree* findFirstLeftEmpty(binTree* t)
{
    if (t && t->val[0] && !isLeafOp(t))
        return NULL;
    else if (!t ||
            !(t->val[0]) ||
            !(t->next1) ||
            !(t->next2))
        return t;

    while (t->next1 && isLeafOp(t->next1) &&
            ((t->next1->next2 && isLeafOp(t->next1->next2)) || ! t->next1->next2 ))
        t = t->next1;

    if (t->next2 && isLeafOp(t->next2))
        return findFirstLeftEmpty(t->next2);
    else if (isLeafOp(t) && (! t->next1 || ! t->next2))
        return t;

    return NULL;
}

int isOp(char str)
{
    switch (str) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '&':
        case '^':
        case '|':
            return 1;
    }
    return 0;
}

static int isLeafOp(binTree* t)
{
    int a = isOp(t->val[0]), b = (! t->val[1]);
    return a && b;
}
