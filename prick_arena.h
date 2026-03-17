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

// A region is a fixed sized allocation on the heap.  Allocations against a
// region will depend on the difference between `size` and `capacity` and may
// fail if there isn't enough free space.  Regions are arranged in a linked list
// to allow for recursion on failure to allocate.
typedef struct PrickArenaRegion
{
  struct PrickArenaRegion *next;
  uint32_t size, capacity;
  uint8_t bytes[];
} prick_arena_region_t;

// An arena is simply a linked list of regions.
typedef struct
{
  prick_arena_region_t *beg, *end;
} prick_arena_t;

uint8_t *prick_arena_alloc(prick_arena_t *arena, uint32_t size);
uint8_t *prick_arena_realloc(prick_arena_t *arena, uint8_t *pointer,
                             uint32_t old_size, uint32_t new_size);
// Reset the arena but do not delete any regions, allowing for fresh allocations
// to take place.
void prick_arena_reset(prick_arena_t *arena);
void prick_arena_free(prick_arena_t *arena);

#ifdef PRICK_ARENA_IMPL

#include <malloc.h>
#include <string.h>

#ifndef REGION_DEFAULT_SIZE
#define REGION_DEFAULT_SIZE 512
#endif

#ifndef REGION_CAPACITY_MULT
#define REGION_CAPACITY_MULT 2
#endif

#ifndef MAX
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif

#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

prick_arena_region_t *prick_region_make(uint32_t capacity,
                                        prick_arena_region_t *next)
{
  capacity                     = MAX(capacity, REGION_DEFAULT_SIZE);
  prick_arena_region_t *region = calloc(1, sizeof(*region) + capacity);
  region->next                 = next;
  region->size                 = 0;
  region->capacity             = capacity;
  return region;
}

uint8_t *prick_region_alloc_rec(prick_arena_region_t *region, uint32_t capacity)
{
  if (!region)
    return NULL;

  for (; region->next && region->capacity - region->size < capacity;
       region = region->next)
    continue;

  if (region->capacity - region->size < capacity)
  {
    // no region->next, so make a new region that can fit the capacity required.
    prick_arena_region_t *new =
        prick_region_make(capacity * REGION_CAPACITY_MULT, NULL);
    region->next = new;
    region       = new;
  }

  uint8_t *start = region->bytes + region->size;
  region->size += capacity;
  return start;
}

void prick_region_delete_rec(prick_arena_region_t *region)
{
  for (prick_arena_region_t *next = NULL; region;
       next = region->next, free(region), region = next)
    continue;
}

uint8_t *prick_arena_alloc(prick_arena_t *arena, uint32_t size)
{
  if (!arena->beg)
  {
    arena->beg = prick_region_make(size, NULL);
    arena->end = arena->beg;
  }

  uint8_t *start = prick_region_alloc_rec(arena->beg, size);
  // if we've attached a new region, end needs to be at that region
  if (arena->end->next)
    arena->end = arena->end->next;
  return start;
}

uint8_t *prick_arena_realloc(prick_arena_t *arena, uint8_t *ptr,
                             uint32_t old_size, uint32_t new_size)
{
  if (!ptr)
  {
    // Basically the same as allocating at this point
    return prick_arena_alloc(arena, new_size);
  }

  /*
    There are two options for reallocation strategy depending on where `ptr`
    lies in its parent region (NOTE: certainly there can only be one parent
    region):
    1) If `ptr` lies at the end of the allocation and there is enough space to
       fit the new_size requirement, just bump the parent region's size and
       return the pointer (nearly "for free")
    2) Otherwise, we must allocate a new block of memory and copy data over.
   */

  // Linear search through our regions until we find parent region for `ptr`.
  // FIXME: I'm pretty sure this is technically UB since we will almost
  // certainly be comparing different batch allocations.  Shame!
  prick_arena_region_t *parent = NULL;
  for (parent = arena->beg;
       parent && !(parent->bytes <= ptr &&
                   parent->bytes + parent->size >= ptr + old_size);
       parent = parent->next)
  {
    continue;
  }

  // If `parent` is not NULL, then it is the region hosting `ptr`.  Check if it
  // satisfies the "lies at the end" condition.
  if (parent && (ptr + old_size == parent->bytes + parent->size) &&
      ((parent->capacity - parent->size) > (new_size - old_size)))
  {
    parent->size += new_size - old_size;
    return ptr;
  }

  // Otherwise, we'll need to reallocate.
  uint8_t *new_ptr = prick_arena_alloc(arena, new_size);
  memcpy(new_ptr, ptr, old_size);
  return new_ptr;
}

void prick_arena_reset(prick_arena_t *arena)
{
  for (prick_arena_region_t *region = arena->beg; region; region = region->next)
  {
    region->size = 0;
    memset(region->bytes, 0, region->size);
  }
}

void prick_arena_free(prick_arena_t *arena)
{
  prick_region_delete_rec(arena->beg);
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
