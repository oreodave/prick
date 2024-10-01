/* Copyright (C) 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the Unlicense
 * for details.

 * You may distribute and modify this code under the terms of the
 * Unlicense, which you should have received a copy of along with this
 * program.  If not, please go to <https://unlicense.org/>.

 * Created: 2024-10-01
 * Author: Aryadev Chavali
 * Description: A dynamically sized container.
 */

#ifndef VEC_H
#define VEC_H

#include <stdint.h>

typedef struct
{
  uint32_t size, capacity;
  uint8_t bytes[];
} vec_t;

#define VEC_GET(P)  (((vec_t *)(P)) - 1)
#define VEC_SIZE(P) (VEC_GET(P)->size)
#define VEC_CAP(P)  (VEC_GET(P)->capacity)

void vec_make(void **ptr, uint32_t size);
void vec_free(void **ptr);
void vec_append_byte(void **ptr, uint8_t byte);
void vec_append(void **ptr, void *data, uint32_t size);
void vec_clone(void **dest, void **src);
void vec_ensure_remaining(void **ptr, uint32_t space);

#define VEC_IMPL
#ifdef VEC_IMPL

#include <malloc.h>
#include <string.h>

#ifndef MAX
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif

#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

void vec_make(void **ptr, uint32_t size)
{
  if (!ptr)
    return;
  vec_t *vector    = calloc(1, sizeof(*vector) + size);
  vector->size     = 0;
  vector->capacity = size;
  *ptr             = (vector + 1);
}

void vec_free(void **data)
{
  if (!data || !*data)
    return;
  free(VEC_GET(*data));
  *data = NULL;
}

void vec_ensure_remaining(void **ptr, uint32_t space)
{
  if (!ptr || !*ptr)
    return;
  vec_t *vec = VEC_GET(*ptr);
  if (vec->capacity - vec->size < space)
  {
    void *new_vec = NULL;
    vec_make(&new_vec, MAX(vec->capacity * 2, vec->size + space));
    VEC_SIZE(new_vec) = vec->size;
    memcpy(new_vec, *ptr, vec->size);
    vec_free(ptr);
    *ptr = new_vec;
  }
}

void vec_append_byte(void **ptr, uint8_t byte)
{
  vec_ensure_remaining(ptr, 1);
  vec_t *vec              = VEC_GET(*ptr);
  vec->bytes[vec->size++] = byte;
}

void vec_append(void **ptr, void *data, uint32_t size)
{
  vec_ensure_remaining(ptr, size);
  vec_t *vec = VEC_GET(*ptr);
  memcpy(vec->bytes + vec->size, data, size);
  vec->size += size;
}

void vec_clone(void **dest, void **src)
{
  if (!dest || !src || !*src)
    return;
  vec_make(dest, VEC_SIZE(*src));
  memcpy(*dest, *src, VEC_SIZE(*src));
  VEC_SIZE(*dest) = VEC_SIZE(*src);
}
#endif

#endif
