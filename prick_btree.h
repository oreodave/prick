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

typedef struct BNode
{
  void *value;
  struct BNode *left, *right;
} bnode_t;

typedef int (*bnode_comp_fn)(void *, void *);
typedef bnode_t *(*bnode_allocator_fn)();
typedef void (*bnode_free_fn)(bnode_t *);

bnode_t *bnode_insert(bnode_t *node, void *value, bnode_comp_fn comparator,
                      bnode_allocator_fn allocator);
void bnode_free(bnode_t *btree, bnode_free_fn);
void bnode_right_rotate(bnode_t **node);
void bnode_left_rotate(bnode_t **node);
void bnode_print(FILE *fp, bnode_t *root);

#ifdef PRICK_BTREE_IMPL

#include <stdlib.h>

bnode_t *bnode_insert(bnode_t *node, void *value, bnode_comp_fn comparator,
                      bnode_allocator_fn allocator)
{
  if (!node)
  {
    node        = allocator();
    node->value = value;
    node->left  = NULL;
    node->right = NULL;
    return node;
  }

  int comp              = comparator(value, node->value);
  bnode_t **picked_node = NULL;
  if (comp < 0)
    picked_node = &node->left;
  else
    picked_node = &node->right;

  if (*picked_node)
    bnode_insert(*picked_node, value, comparator, allocator);
  else
  {
    *picked_node          = allocator();
    picked_node[0]->value = value;
    picked_node[0]->left  = NULL;
    picked_node[0]->right = NULL;
  }

  return node;
}

void bnode_free(bnode_t *bnode, bnode_free_fn free_fn)
{
  if (!bnode)
    return;
  bnode_t *left  = bnode->left;
  bnode_t *right = bnode->right;

  free_fn(bnode);
  bnode_free(left, free_fn);
  bnode_free(right, free_fn);
}

void bnode_right_rotate(bnode_t **node)
{
  if (!node || !*node)
    return;

  bnode_t *left = (*node)->left;
  if (left)
  {
    (*node)->left = left->right;
    left->right   = *node;
    *node         = left;
  }
}

void bnode_left_rotate(bnode_t **node)
{
  if (!node || !*node)
    return;

  bnode_t *right = (*node)->right;
  if (right)
  {
    (*node)->right = right->left;
    right->left    = *node;
    *node          = right;
  }
}

void bnode_print(FILE *fp, bnode_t *root)
{
  if (!root)
    return;
  fprintf(fp, "(%p", root->value);
  if (root->left)
  {
    fprintf(fp, " ");
    bnode_print(fp, root->left);
  }

  if (root->right)
  {
    fprintf(fp, " ");
    bnode_print(fp, root->right);
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
