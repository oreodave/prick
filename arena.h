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
 * Description: Arena allocator
 */

#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>

struct Region;
typedef struct
{
  struct Region *beg, *end;
} arena_t;

uint8_t *arena_alloc(arena_t *arena, uint32_t size);
uint8_t *arena_realloc(arena_t *arena, uint8_t *pointer, uint32_t old_size,
                       uint32_t new_size);
void arena_reset(arena_t *arena);
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

void region_delete(region_t *region)
{
  while (region)
  {
    region_t *next = region->next;
    free(region);
    region = next;
  }
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
  // Firstly find the region the pointer exists in
  region_t *prev, *reg;
  for (prev = NULL, reg = arena->beg;
       reg &&
       !(reg->bytes <= pointer && reg->bytes + reg->size >= pointer + old_size);
       prev = reg, reg = reg->next)
    continue;

  if (!reg)
    // pointer isn't allocated in the arena
    return NULL;

  uint8_t *new_ptr = NULL;
  if (old_size == reg->size && reg->capacity == reg->size)
  {
    // Completely filled region, may as well reallocate
    region_t *new_reg = region_make(new_size * REGION_CAPACITY_MULT, reg->next);
    // Chain this new region in place
    if (prev)
      prev->next = new_reg;
    if (reg == arena->end)
      arena->end = new_reg;
    free(reg);
    new_ptr = new_reg->bytes;
    new_reg->size += new_size;
  }
  else
  {
    // Allocate a new portion of memory on the arena
    new_ptr = arena_alloc(arena, new_size);
  }
  memcpy(new_ptr, pointer, old_size);
  return new_ptr;
}

void arena_free(arena_t *arena)
{
  region_delete(arena->beg);
  memset(arena, 0, sizeof(*arena));
}

#endif

#endif
