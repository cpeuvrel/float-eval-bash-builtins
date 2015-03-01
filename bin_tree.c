/*
 * =====================================================================================
 *
 *       Filename:  bin_tree.c
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

#include "bin_tree.h"
static void print_bin_tree_depth(bin_tree*, int lvl, int pos);
static int is_leaf_op(bin_tree* t);

void init_bin_tree(bin_tree* t)
{
    t->val[0] = 0;
    t->next1 = NULL;
    t->next2 = NULL;
}

void print_bin_tree(bin_tree* t)
{
    print_bin_tree_depth(t, 0, 2);
}

static void print_bin_tree_depth(bin_tree* t, int lvl, int pos)
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
        print_bin_tree_depth(t->next1, lvl, 1);
    if (t->next2)
        print_bin_tree_depth(t->next2, lvl, 2);
}

void free_bin_tree(bin_tree* t)
{
    if (t->next1)
        free_bin_tree(t->next1);
    if (t->next2)
        free_bin_tree(t->next2);
    free(t);
}

bin_tree* find_first_empty(bin_tree* t)
{
    if (!t->val[0])
        return t;

    bin_tree *res = t, *tmp = NULL;

    while (res->val[0] && res->next1 && ! (tmp = find_first_left_empty(res->next1)) && res->next2)
        res = res->next2;

    if (! res->val[0])
        return res;

    bin_tree *new = malloc(sizeof(bin_tree));
    init_bin_tree(new);

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

bin_tree* find_first_left_empty(bin_tree* t)
{
    if (t && t->val[0] && !is_leaf_op(t))
        return NULL;
    else if (!t ||
            !(t->val[0]) ||
            !(t->next1) ||
            !(t->next2))
        return t;

    while (t->next1 && is_leaf_op(t->next1) &&
            ((t->next1->next2 && is_leaf_op(t->next1->next2)) || ! t->next1->next2 ))
        t = t->next1;

    if (t->next2 && is_leaf_op(t->next2))
        return find_first_left_empty(t->next2);
    else if (is_leaf_op(t) && (! t->next1 || ! t->next2))
        return t;

    return NULL;
}

int is_op(char str)
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
        case '<':
        case '>':
        case '=':
        case '!':
        case '~':
            return 1;
    }
    return 0;
}

static int is_leaf_op(bin_tree* t)
{
    int a = is_op(t->val[0]), b = (! t->val[1]);
    return a && b;
}
