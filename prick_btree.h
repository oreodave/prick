/* prick_btree.h: A generic ordered binary tree.
 * Created: 2025-04-09
 * Author: Aryadev Chavali
 * License: See end of file
 * Commentary:

 To utilise this library, please put:
    #define PRICK_BTREE_IMPL
    #include "prick_btree.h"
 in one of your code units.

 An ordered binary tree implementation, allowing the use of custom comparators
 and allocators.
 */

#ifndef PRICK_BTREE_H
#define PRICK_BTREE_H

#include <stdio.h>

typedef struct Prick_Bnode
{
  void *value;
  struct Prick_Bnode *left, *right;
} prick_bnode_t;

typedef int (*prick_btree_comp_fn)(void *, void *);
typedef prick_bnode_t *(*prick_btree_alloc_fn)();
typedef void (*prick_btree_free_fn)(prick_bnode_t *);
typedef void (*prick_btree_print_fn)(FILE *, void *);

prick_bnode_t *prick_btree_insert(prick_bnode_t *node, void *value,
                                  prick_btree_comp_fn comp,
                                  prick_btree_alloc_fn alloc);
void prick_btree_right_rotate(prick_bnode_t **node);
void prick_btree_left_rotate(prick_bnode_t **node);
void prick_btree_print(prick_bnode_t *root, FILE *fp,
                       prick_btree_print_fn print);
void prick_btree_free(prick_bnode_t *node, prick_btree_free_fn free);

#ifdef PRICK_BTREE_IMPL

#include <assert.h>
#include <stdlib.h>

prick_bnode_t *prick_btree_insert(prick_bnode_t *node, void *value,
                                  prick_btree_comp_fn comp,
                                  prick_btree_alloc_fn alloc)
{
  if (!node)
  {
    node        = alloc();
    node->value = value;
    node->left  = NULL;
    node->right = NULL;
    return node;
  }

  int comparison              = comp(value, node->value);
  prick_bnode_t **picked_node = NULL;
  if (comparison < 0)
    picked_node = &node->left;
  else
    picked_node = &node->right;

  if (*picked_node)
    prick_btree_insert(*picked_node, value, comp, alloc);
  else
  {
    *picked_node          = alloc();
    picked_node[0]->value = value;
    picked_node[0]->left  = NULL;
    picked_node[0]->right = NULL;
  }

  return node;
}

void prick_btree_free(prick_bnode_t *bnode, prick_btree_free_fn free_fn)
{
  if (!bnode)
    return;
  prick_bnode_t *left  = bnode->left;
  prick_bnode_t *right = bnode->right;

  free_fn(bnode);
  prick_btree_free(left, free_fn);
  prick_btree_free(right, free_fn);
}

void prick_btree_right_rotate(prick_bnode_t **node)
{
  if (!node || !*node)
    return;

  prick_bnode_t *left = (*node)->left;
  if (left)
  {
    (*node)->left = left->right;
    left->right   = *node;
    *node         = left;
  }
}

void prick_btree_left_rotate(prick_bnode_t **node)
{
  if (!node || !*node)
    return;

  prick_bnode_t *right = (*node)->right;
  if (right)
  {
    (*node)->right = right->left;
    right->left    = *node;
    *node          = right;
  }
}

void prick_btree_print(prick_bnode_t *root, FILE *fp,
                       prick_btree_print_fn print)
{
  if (!root)
  {
    fprintf(fp, "()");
    return;
  }
  fprintf(fp, "(");
  print(fp, root->value);
  if (root->left)
  {
    fprintf(fp, " l");
    prick_btree_print(root->left, fp, print);
  }

  if (root->right)
  {
    fprintf(fp, " r");
    prick_btree_print(root->right, fp, print);
  }

  fprintf(fp, ")");
}

#endif

#endif

/* Copyright (C) 2025 Aryadev Chavali

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the Unlicense
 * for details.

 * You may distribute and modify this code under the terms of the
 * Unlicense, which you should have received a copy of along with this
 * program.  If not, please go to <https://unlicense.org/>.

 */
