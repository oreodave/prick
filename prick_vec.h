/* prick_vec.h: A dynamically sized array with SBO.
 * Created: 2026-01-22
 * Author: Aryadev Chavali
 * License: See end of file
 * Commentary:

 To utilise this library, please put:
    #define PRICK_VEC_IMPL
    #include "prick_vec.h"
 in one of your code units.

 To remove the `prick_` namespacing, please put:
    #define PRICK_SHORTHAND
 in any files before including prick_vec.h.  Standard preprocesser rules apply
 with regards to hierarchy.

 This library defines another form of dynamically sized array as opposed to
 prick_darr.h.  This one is closer to the one classically implemented by most; a
 structure with some metadata and a pointer to the raw buffer.  This way,
 pointers to the dynamic array are stable (as the structure itself is never
 reallocated) and the array can still grow as required.

 We use a trick, called Small Buffer Optimisation (SBO), to inline elements
 directly into the structure when there are a small number of them (see
 PRICK_VEC_INLINE_CAPACITY).  This makes lookup _even faster_ (no derefence and
 possibility of the entire vector existing in the CPU cache) and allows us to
 avoid allocation for smaller use cases.  If the number of elements exceeds
 PRICK_VEC_INLINE_CAPACITY, we utilise the generic memory allocator (malloc).

 Tasks:
 - TODO: Provide generic allocator support.
 */

#ifndef PRICK_VEC_H
#define PRICK_VEC_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define PRICK_VEC_INLINE_CAPACITY 32
#define PRICK_VEC_MULT            2

typedef struct
{
  uint64_t size, capacity;
  uint8_t not_inlined;
  union
  {
    void *ptr;
    alignas(max_align_t) uint8_t inlined[PRICK_VEC_INLINE_CAPACITY];
  };
} prick_vec_t;

static_assert(sizeof(prick_vec_t) == 64,
              "Expected sizeof(prick_vec_t) to be 64");

void prick_vec_append(prick_vec_t *vec, const void *const ptr, uint64_t size);
void prick_vec_append_byte(prick_vec_t *vec, uint8_t byte);
uint8_t *prick_vec_data(prick_vec_t *vec);
void prick_vec_ensure_capacity(prick_vec_t *vec, uint64_t capacity);
void prick_vec_ensure_free(prick_vec_t *vec, uint64_t size);
void prick_vec_free(prick_vec_t *vec);
void prick_vec_clone(prick_vec_t *v2, prick_vec_t *v1);
void *prick_vec_pop(prick_vec_t *vec, size_t member_size);
size_t prick_vec_find(prick_vec_t *vec, void *ptr, size_t ptrsize);

#define PRICK_VEC_GET(VEC, INDEX, TYPE) (((TYPE *)prick_vec_data(VEC))[INDEX])

#ifdef PRICK_VEC_IMPL
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#include <stdlib.h>
#include <string.h>

void prick_vec_append(prick_vec_t *vec, const void *const ptr, uint64_t size)
{
  if (!vec || !ptr || !size)
    return;
  prick_vec_ensure_free(vec, size);
  memcpy(&PRICK_VEC_GET(vec, vec->size, uint8_t), ptr, size);
  vec->size += size;
}

void prick_vec_append_byte(prick_vec_t *vec, uint8_t byte)
{
  if (!vec)
    return;
  prick_vec_ensure_free(vec, 1);
  PRICK_VEC_GET(vec, vec->size, uint8_t) = byte;
  ++vec->size;
}

uint8_t *prick_vec_data(prick_vec_t *vec)
{
  if (!vec)
    return NULL;

  if (vec->not_inlined)
  {
    return vec->ptr;
  }
  else
  {
    return vec->inlined;
  }
}

void prick_vec_ensure_capacity(prick_vec_t *vec, uint64_t capacity)
{
  if (!vec)
    return;
  if (vec->capacity == 0)
    vec->capacity = PRICK_VEC_INLINE_CAPACITY;
  if (vec->capacity < capacity)
  {
    vec->capacity = MAX(vec->capacity * PRICK_VEC_MULT, capacity);
    if (!vec->not_inlined)
    {
      // We were a small buffer, and now we cannot be i.e. we need to allocate
      // on the heap.
      vec->not_inlined = 1;
      void *buffer     = calloc(1, vec->capacity);
      memcpy(buffer, vec->inlined, vec->size);
      memset(vec->inlined, 0, sizeof(vec->inlined));
      vec->ptr = buffer;
    }
    else
    {
      // We're already on the heap, just reallocate.
      vec->ptr = realloc(vec->ptr, vec->capacity);
    }
  }
}

void prick_vec_ensure_free(prick_vec_t *vec, uint64_t size)
{
  if (!vec)
    return;
  prick_vec_ensure_capacity(vec, vec->size + size);
}

void prick_vec_free(prick_vec_t *vec)
{
  if (!vec)
    return;
  if (vec->not_inlined)
    free(vec->ptr);
  memset(vec, 1, sizeof(*vec));
}

void prick_vec_clone(prick_vec_t *v2, prick_vec_t *v1)
{
  if (!v1 || !v2)
    return;
  prick_vec_append(v2, prick_vec_data(v1), v1->size);
}

void *prick_vec_pop(prick_vec_t *vec, size_t member_size)
{
  if (vec->size < member_size)
  {
    return NULL;
  }
  vec->size -= member_size;
  return prick_vec_data(vec) + vec->size;
}

size_t prick_vec_find(prick_vec_t *vec, void *ptr, size_t ptrsize)
{
  if (vec->size < ptrsize)
  {
    return vec->size + 1;
  }

  uint8_t *base = prick_vec_data(vec);
  for (size_t i = 0; i < vec->size; i += ptrsize)
  {
    void *member = base + i;
    if (!memcmp(member, ptr, ptrsize))
    {
      return i;
    }
  }

  return vec->size + 1;
}

#undef MAX
#endif

#ifdef PRICK_SHORTHAND

typedef prick_vec_t vec_t;
#define vec_append          prick_vec_append
#define vec_append_byte     prick_vec_append_byte
#define vec_data            prick_vec_data
#define vec_ensure_capacity prick_vec_ensure_capacity
#define vec_ensure_free     prick_vec_ensure_free
#define vec_free            prick_vec_free
#define vec_clone           prick_vec_clone
#define vec_pop             prick_vec_pop
#define vec_find            prick_vec_find
#define VEC_GET             PRICK_VEC_GET

#endif

#endif

/* Copyright (C) 2026 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the Unlicense for details.

 * You may distribute and modify this code under the terms of the Unlicense,
 * which you should have received a copy of along with this program.  If not,
 * please go to <https://unlicense.org/>.

 */
