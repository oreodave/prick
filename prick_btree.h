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

typedef int (*prick_bnode_comp_fn)(void *, void *);
typedef prick_bnode_t *(*prick_bnode_alloc_fn)();
typedef void (*prick_bnode_free_fn)(prick_bnode_t *);
typedef void (*prick_print_fn)(FILE *, void *);

typedef struct
{
  prick_bnode_t *root;
  prick_bnode_comp_fn comp;
  prick_bnode_alloc_fn alloc;
  prick_bnode_free_fn free;
  prick_print_fn print;
} prick_btree_t;

void prick_btree_init(prick_btree_t *tree, prick_bnode_comp_fn comparator,
                      prick_bnode_alloc_fn allocator, prick_bnode_free_fn free,
                      prick_print_fn print);
prick_bnode_t *prick_btree_insert(prick_btree_t *tree, void *value);
void prick_btree_print(FILE *fp, prick_btree_t *tree);
void prick_btree_free(prick_btree_t *tree);

void prick_bnode_right_rotate(prick_bnode_t **node);
void prick_bnode_left_rotate(prick_bnode_t **node);
void prick_bnode_print(FILE *fp, prick_bnode_t *root, prick_print_fn print);

#ifdef PRICK_BTREE_IMPL

#include <assert.h>
#include <stdlib.h>

void prick_btree_init(prick_btree_t *tree, prick_bnode_comp_fn comparator,
                      prick_bnode_alloc_fn allocator, prick_bnode_free_fn free,
                      prick_print_fn print)
{
  // NOTE: These NEED to be supplied.
  assert(comparator);
  assert(allocator);
  assert(free);
  assert(print);
  if (tree)
  {
    tree->root  = NULL;
    tree->comp  = comparator;
    tree->alloc = allocator;
    tree->free  = free;
    tree->print = print;
  }
}

prick_bnode_t *prick_bnode_insert(prick_bnode_t *node, prick_btree_t *tree,
                                  void *value)
{
  if (!node)
  {
    node        = tree->alloc();
    node->value = value;
    node->left  = NULL;
    node->right = NULL;
    return node;
  }

  int comp                    = tree->comp(value, node->value);
  prick_bnode_t **picked_node = NULL;
  if (comp < 0)
    picked_node = &node->left;
  else
    picked_node = &node->right;

  if (*picked_node)
    prick_bnode_insert(*picked_node, tree, value);
  else
  {
    *picked_node          = tree->alloc();
    picked_node[0]->value = value;
    picked_node[0]->left  = NULL;
    picked_node[0]->right = NULL;
  }

  return node;
}

prick_bnode_t *prick_btree_insert(prick_btree_t *tree, void *value)
{
  tree->root = prick_bnode_insert(tree->root, tree, value);
  return tree->root;
}

void prick_btree_print(FILE *fp, prick_btree_t *tree)
{
  if (!tree->root)
  {
    fprintf(fp, "()");
  }
  else
  {
    prick_bnode_print(fp, tree->root, tree->print);
  }
}

void prick_bnode_free(prick_bnode_t *bnode, prick_bnode_free_fn free_fn)
{
  if (!bnode)
    return;
  prick_bnode_t *left  = bnode->left;
  prick_bnode_t *right = bnode->right;

  free_fn(bnode);
  prick_bnode_free(left, free_fn);
  prick_bnode_free(right, free_fn);
}

void prick_btree_free(prick_btree_t *tree)
{
  if (!tree)
    return;
  prick_bnode_free(tree->root, tree->free);
  tree->root = NULL;
}

void prick_bnode_right_rotate(prick_bnode_t **node)
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

void prick_bnode_left_rotate(prick_bnode_t **node)
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

void prick_bnode_print(FILE *fp, prick_bnode_t *root, prick_print_fn print)
{
  if (!root)
    return;
  fprintf(fp, "(");
  print(fp, root->value);
  if (root->left)
  {
    fprintf(fp, " l");
    prick_bnode_print(fp, root->left, print);
  }

  if (root->right)
  {
    fprintf(fp, " r");
    prick_bnode_print(fp, root->right, print);
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
