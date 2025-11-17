/* prick_arena.h: An arena implementation.
 * Created: 2024-11-01
 * Author: Aryadev Chavali
 * License: see end of file
 * Commentary:

 To utilise this library, please put:
    #define PRICK_ARENA_IMPL
    #include "prick_arena.h"
 in one of your code units.

 This library defines both:
 - A simple bump allocator for regions, with the ability to attach more regions
   via a linked list in case the current region when the current region is full.
 - A simple arena memory allocator, using the bump allocator for its regions.

 The regions of the arena are arranged in a linked list for simplicity.  As
 regions aren't reallocated (instead, a new region is generated), they are
 stable pointers, and may be used throughout a program as long as the underlying
 arena isn't deleted.
 */

#ifndef PRICK_ARENA_H
#define PRICK_ARENA_H

#include <stdint.h>

typedef struct Region
{
  struct Region *next;
  uint32_t size, capacity;
  uint8_t bytes[];
} region_t;

region_t *region_make(uint32_t capacity, region_t *next);
uint8_t *region_alloc_flat(region_t *region, uint32_t size);
uint8_t *region_alloc_rec(region_t *region, uint32_t size);
void region_delete_rec(region_t *region);

typedef struct
{
  region_t *beg, *end;
} arena_t;

uint8_t *arena_alloc(arena_t *arena, uint32_t size);
uint8_t *arena_realloc(arena_t *arena, uint8_t *pointer, uint32_t old_size,
                       uint32_t new_size);
void arena_reset(arena_t *arena);
void arena_free(arena_t *arena);

#ifdef PRICK_ARENA_IMPL

#include <malloc.h>
#include <string.h>

#ifndef REGION_DEFAULT_SIZE
#define REGION_DEFAULT_SIZE 512
#endif

#ifndef REGION_CAPACITY_MULT
#define REGION_CAPACITY_MULT 2
#endif

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

region_t *region_make(uint32_t capacity, region_t *next)
{
  capacity         = MAX(capacity, REGION_DEFAULT_SIZE);
  region_t *region = calloc(1, sizeof(*region) + capacity);
  region->next     = next;
  region->size     = 0;
  region->capacity = capacity;
  return region;
}

uint8_t *region_alloc_rec(region_t *region, uint32_t capacity)
{
  if (!region)
    return NULL;

  for (; region->next && region->capacity - region->size < capacity;
       region = region->next)
    continue;

  if (region->capacity - region->size < capacity)
  {
    // no region->next, so make a new region that can fit the capacity required.
    region_t *new = region_make(capacity * REGION_CAPACITY_MULT, NULL);
    region->next  = new;
    region        = new;
  }

  uint8_t *start = region->bytes + region->size;
  region->size += capacity;
  return start;
}

void region_delete_rec(region_t *region)
{
  for (region_t *next = NULL; region;
       next = region->next, free(region), region = next)
    continue;
}

uint8_t *arena_alloc(arena_t *arena, uint32_t size)
{
  if (!arena->beg)
  {
    arena->beg = region_make(size, NULL);
    arena->end = arena->beg;
  }

  uint8_t *start = region_alloc_rec(arena->beg, size);
  // if we've attached a new region, end needs to be at that region
  if (arena->end->next)
    arena->end = arena->end->next;
  return start;
}

uint8_t *arena_realloc(arena_t *arena, uint8_t *pointer, uint32_t old_size,
                       uint32_t new_size)
{
  if (!pointer)
    // Basically the same as allocating at this point
    return arena_alloc(arena, newsize);

  // Firstly find the region the pointer exists in
  region_t *prev, *reg;
  for (prev = NULL, reg = arena->beg;
       reg &&
       !(reg->bytes <= pointer && reg->bytes + reg->size >= pointer + old_size);
       prev = reg, reg = reg->next)
    continue;

  uint8_t *new_ptr = NULL;

  // pointer isn't allocated in the arena, just allocate a new pointer
  if (!reg)
    goto arena_realloc__allocate_new;

  /*
    If `ptr` is the latest allocation in `reg` and `reg` has enough capacity to
    handle newsize, then we can adjust `reg` and not have to do any further
    allocation work.

    This check is not UB because ptr is confirmed to be in reg.
   */
  if (ptr + oldsize == reg->bytes + reg->size &&
      (reg->capacity - reg->size) > (newsize - oldsize))
  {
    reg->size += newsize - oldsize;
    return ptr;
  }

arena_realloc__allocate_new:
  new_ptr = arena_alloc(arena, newsize);
  memcpy(new_ptr, ptr, oldsize);
  return new_ptr;
}

void arena_reset(arena_t *arena)
{
  for (region_t *region = arena->beg; region; region = region->next)
  {
    region->size = 0;
    memset(region->bytes, 0, region->capacity);
  }
}

void arena_free(arena_t *arena)
{
  region_delete_rec(arena->beg);
  memset(arena, 0, sizeof(*arena));
}

#endif

#endif
/* Copyright (C) 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the Unlicense
 * for details.

 * You may distribute and modify this code under the terms of the
 * Unlicense, which you should have received a copy of along with this
 * program.  If not, please go to <https://unlicense.org/>.

 */
