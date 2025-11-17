/* prick_darr.h: A dynamically sized array, all on the heap.
 * Created: 2024-10-01
 * Author: Aryadev Chavali
 * License: see end of file
 * Commentary:

 To utilise this library, please put:
    #define PRICK_DARR_IMPL
    #include "prick_darr.h"
 in one of your code units.

 This library defines a dynamic array purely on the heap.  We split the one
 dynamic array allocation into two parts: the metadata, and the actual data.
 Consumers of the library will only ever need to deal with the latter component
 i.e. they'll only ever have access to the data they require.

 Unfortuntely this does mean that the underlying data pointer is _not_ stable
 during capacity reallocation, as we're allocating a whole new pointer with the
 correct size.  Therefore, if you're expecting the array to grow during the
 runtime of your program, you should ensure any pointers to components of it are
 updated as they will otherwise be dangling.
 */

#ifndef PRICK_DARR_H
#define PRICK_DARR_H

#include <stdint.h>

typedef struct
{
  uint32_t size, capacity;
  uint8_t bytes[];
} darr_t;

#define DARR_GET(P)  (((darr_t *)(P)) - 1)
#define DARR_SIZE(P) (DARR_GET(P)->size)
#define DARR_CAP(P)  (DARR_GET(P)->capacity)

void darr_make(void **ptr, uint32_t size);
void darr_free(void **ptr);
void darr_append_byte(void **ptr, uint8_t byte);
void darr_append(void **ptr, void *data, uint32_t size);
void darr_clone(void **dest, void **src);
void darr_ensure_remaining(void **ptr, uint32_t space);

#ifdef PRICK_DARR_IMPL

#include <malloc.h>
#include <string.h>

#ifndef DARR_MULT
#define DARR_MULT 2
#endif

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

void darr_make(void **ptr, uint32_t size)
{
  if (!ptr)
    return;
  darr_t *darrtor   = calloc(1, sizeof(*darrtor) + size);
  darrtor->size     = 0;
  darrtor->capacity = size;
  *ptr              = (darrtor + 1);
}

void darr_free(void **data)
{
  if (!data || !*data)
    return;
  free(DARR_GET(*data));
  *data = NULL;
}

void darr_ensure_remaining(void **ptr, uint32_t space)
{
  if (!ptr || !*ptr)
    return;
  darr_t *darr = DARR_GET(*ptr);
  if (darr->capacity - darr->size < space)
  {
    void *new_darr = NULL;
    darr_make(&new_darr, MAX(darr->capacity * DARR_MULT, darr->size + space));
    DARR_SIZE(new_darr) = darr->size;
    memcpy(new_darr, *ptr, darr->size);
    darr_free(ptr);
    *ptr = new_darr;
  }
}

void darr_append_byte(void **ptr, uint8_t byte)
{
  darr_ensure_remaining(ptr, 1);
  darr_t *darr              = DARR_GET(*ptr);
  darr->bytes[darr->size++] = byte;
}

void darr_append(void **ptr, void *data, uint32_t size)
{
  darr_ensure_remaining(ptr, size);
  darr_t *darr = DARR_GET(*ptr);
  memcpy(*ptr + darr->size, data, size);
  darr->size += size;
}

void darr_clone(void **dest, void **src)
{
  if (!dest || !src || !*src)
    return;
  darr_make(dest, DARR_SIZE(*src));
  memcpy(*dest, *src, DARR_SIZE(*src));
  DARR_SIZE(*dest) = DARR_SIZE(*src);
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
