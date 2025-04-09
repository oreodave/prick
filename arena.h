/* Copyright (C) 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the Unlicense
 * for details.

 * You may distribute and modify this code under the terms of the
 * Unlicense, which you should have received a copy of along with this
 * program.  If not, please go to <https://unlicense.org/>.

 * Created: 2024-11-01
 * Author: Aryadev Chavali
 * Description: Arena allocator.
 */

#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>

/**
   @brief A single block of memory to be used by an arena.

   @details Blocks of memory arranged in a singly linked list.
   Each individual node is a bump allocator.
*/
typedef struct Region
{
  struct Region *next;
  uint32_t size, capacity;
  uint8_t bytes[];
} region_t;

/**
   @brief Allocate a new region on the heap with requested size and a pointer to
   the next region.
   @details If capacity is less than REGION_DEFAULT_SIZE, capacity is set to
   REGION_DEFAULT_SIZE.
 */
region_t *region_make(uint32_t capacity, region_t *next);

/**
   @brief Allocate memory of requested size on the region.
   @details If the region cannot fit the requested size, then return NULL.
   Otherwise return a pointer to the start of the allocated memory, incrementing
   the region size appropriately.
 */
uint8_t *region_alloc_flat(region_t *region, uint32_t size);

/**
   @brief Allocate memory of requested size on the region.
   @details Iterates through the linked list of regions to find an appropriately
   sized region for the requested size, allocating a new region if one cannot be
   found.  This new region will have capacity at least size *
   REGION_CAPACITY_MULT.

   Returns a pointer to the start of the allocated memory, incrementing the
   appropriate region's size.
 */
uint8_t *region_alloc_rec(region_t *region, uint32_t size);

/**
   @brief Delete a region, freeing its memory.
   @details Will free all regions following it in the linked list.
 */
void region_delete_rec(region_t *region);

typedef struct
{
  region_t *beg, *end;
} arena_t;

/**
   @brief Allocate memory of requested size in the arena, returning a pointer to
   the start of it.
   @details Uses region_alloc_rec internally to allocate the memory required.
   arena->beg and arena->end are set appropriately for this task.
 */
uint8_t *arena_alloc(arena_t *arena, uint32_t size);

/**
   @brief Reallocate buffer of old_size to a buffer of new_size in the
   arena, returning a pointer to the start of the new buffer.

   @details If the pointer is not allocated in the arena, return NULL.  If the
   pointer and old_size cover a complete region reallocate the region itself to
   fit the newly requested size, relinking it in the linked list.  Otherwise,
   allocate as per usual.

   The contents of the old memory are copied into the new buffer.  If old_size >
   new_size, only new_size bytes will be copied from the old buffer into the new
   one.
 */
uint8_t *arena_realloc(arena_t *arena, uint8_t *pointer, uint32_t old_size,
                       uint32_t new_size);

/**
   @brief Reset the arena for reuse.

   @details Sets all regions to default values, setting size to 0.  No memory is
   deleted in this operation.
 */
void arena_reset(arena_t *arena);

/**
   @brief Free the memory associated with the arena.

   @details Deletes all regions of memory associated in the arena.
 */
void arena_free(arena_t *arena);

#ifdef ARENA_IMPL

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
