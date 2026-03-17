/* prick_darr.h: A dynamically sized array, all on the heap.
 * Created: 2024-10-01
 * Author: Aryadev Chavali
 * License: see end of file
 * Commentary:

 To utilise this library, please put:
    #define PRICK_DARR_IMPL
    #include "prick_darr.h"
 in one of your code units.

 This library defines a dynamic array purely on the heap.  Both the raw data for
 the array as well as the metadata are in one allocation.  Consumers of the
 library will only ever need to deal with the former component i.e. they'll only
 deal with the raw data pointer.

 Unfortuntely this does mean that the overall pointer to the vector is _not_
 stable during capacity reallocation, as on reallocation we allocate a whole new
 area on the heap and copy over the data.  Therefore, if you're expecting the
 array to grow during the runtime of your program, you should ensure any
 pointers to components of it are updated as they will otherwise be dangling.

 You may want to consider prick_vec.h if you want to more explicit control of
 the dynamic array, and would like a stable pointer to the container itself.

 Tasks:
 - TODO: Implement ability to use a custom allocator.
 - TODO: QoL shorthand macro.
 */

#ifndef PRICK_DARR_H
#define PRICK_DARR_H

#include <stdint.h>

typedef struct
{
  uint32_t size, capacity;
  uint8_t bytes[];
} prick_darr_t;

#define PRICK_DARR_GET(P)  (((prick_darr_t *)(P)) - 1)
#define PRICK_DARR_SIZE(P) (PRICK_DARR_GET(P)->size)
#define PRICK_DARR_CAP(P)  (PRICK_DARR_GET(P)->capacity)

void prick_darr_make(void **ptr, uint32_t size);
void prick_darr_free(void **ptr);
void prick_darr_append_byte(void **ptr, uint8_t byte);
void prick_darr_append(void **ptr, void *data, uint32_t size);
void prick_darr_clone(void **dest, void **src);
void prick_darr_ensure_remaining(void **ptr, uint32_t space);

#ifdef PRICK_DARR_IMPL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PRICK_DARR_MULT
#define PRICK_DARR_MULT 2
#endif

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define FAIL(...)                          \
  do                                       \
  {                                        \
    fprintf(stderr, "FAIL: prick_darr: "); \
    fprintf(stderr, __VA_ARGS__);          \
    exit(1);                               \
  } while (0)

void prick_darr_make(void **ptr, uint32_t size)
{
  if (!ptr)
    return;
  prick_darr_t *darr = calloc(1, sizeof(*darr) + size);
  if (!darr)
    FAIL("Could not allocate memory (%lu bytes) for new dynamic array.\n",
         sizeof(*darr) + size);
  darr->size     = 0;
  darr->capacity = size;
  *ptr           = (darr + 1);
}

void prick_darr_free(void **data)
{
  if (!data || !*data)
    return;
  free(PRICK_DARR_GET(*data));
  *data = NULL;
}

void prick_darr_ensure_remaining(void **ptr, uint32_t space)
{
  if (!ptr || !*ptr)
    return;
  prick_darr_t *darr = PRICK_DARR_GET(*ptr);
  if (darr->capacity - darr->size < space)
  {
    void *new_darr = NULL;
    uint64_t new_capacity =
        MAX(darr->capacity * PRICK_DARR_MULT, darr->size + space);
    prick_darr_make(&new_darr, new_capacity);

    if (!new_darr)
      FAIL("Could not allocate new dynamic array with %lu bytes capacity.\n",
           new_capacity);

    PRICK_DARR_SIZE(new_darr) = darr->size;
    memcpy(new_darr, *ptr, darr->size);
    prick_darr_free(ptr);
    *ptr = new_darr;
  }
}

void prick_darr_append_byte(void **ptr, uint8_t byte)
{
  if (!ptr || !*ptr)
    return;
  prick_darr_ensure_remaining(ptr, 1);
  prick_darr_t *darr        = PRICK_DARR_GET(*ptr);
  darr->bytes[darr->size++] = byte;
}

void prick_darr_append(void **ptr, void *data, uint32_t size)
{
  if (!ptr || !*ptr || !data)
    return;
  prick_darr_ensure_remaining(ptr, size);
  prick_darr_t *darr = PRICK_DARR_GET(*ptr);
  memcpy(((uint8_t *)*ptr) + darr->size, data, size);
  darr->size += size;
}

void prick_darr_clone(void **dest, void **src)
{
  if (!dest || !src || !*src)
    return;
  prick_darr_make(dest, PRICK_DARR_SIZE(*src));
  memcpy(*dest, *src, PRICK_DARR_SIZE(*src));
  PRICK_DARR_SIZE(*dest) = PRICK_DARR_SIZE(*src);
}

#undef MAX
#undef MIN

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
