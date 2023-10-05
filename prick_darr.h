/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the MIT
 * license.  You should have received a copy of the MIT license with
 * this file.  If not, please write to: aryadev@aryadevchavali.com.

 * Created: 2023-10-05
 * Author: Aryadev Chavali
 * Description: A type homogeneous dynamic array
 */

#ifndef PRICK_DARR_H
#define PRICK_DARR_H

#include <stdint.h>
#include <stdlib.h>

#define PRICK_DARR_ALLOC_MULT   2
#define PRICK_DARR_DEFAULT_SIZE 8

typedef struct
{
  size_t size;      // size of each "member"
  size_t used;      // number of elements currently used
  size_t available; // number of elements allocated
  uint8_t *data;
} prick_darr_t;

/**
 * UNSAFE!
 *
 * Gets NTH member of DARR, casting result to TYPE.  Does not bounds
 * check and relies on runtime to cast correctly.  Only use when sure
 * that the dynamic array is well formed for this.
 *
 * @param DARR: Dynamic array to get member from
 *
 * @param TYPE: Type to cast member to
 *
 * @param N: Index of member
 */
#define PRICK_DARR_AT(DARR, TYPE, N) (((TYPE *)(DARR).data)[N])

/**
 * Initialises the dynamic array given with PRICK_DARR_DEFAULT_SIZE
 * number of elements.
 *
 * @param prick_darr_t *: Dynamic array to initialise
 *
 * @param size_t: Size of member type in bytes
 */
void prick_darr_init(prick_darr_t *, size_t);

/**
 * Frees the memory associated with dynamic array, using the object
 * free function given to free each member of the dynamic array before
 * freeing associated container.  If the free function is NULL, then
 * the only container is freed.
 *
 * @param prick_darr_t *: Dynamic array to free
 *
 * @param void (*)(void *): Member freeing function (can be NULL)
 */
void prick_darr_free(prick_darr_t *, void (*)(void *));

/**
 * Ensures there's enough capacity available for the size requested in
 * the given dynamic array.
 *
 * @param prick_darr_t *: Dynamic array to check
 *
 * @param size_t: Number of members requested
 */
void prick_darr_ensure_capacity(prick_darr_t *, size_t);

/**
 * Attempts to shrink and tighten the container of the dynamic array
 * such that prick_darr_t.available == prick_darr_t.used.  NOTE:
 * Useful as a preparation step when later using or returning a
 * vanilla C pointer for other APIs.
 *
 * @param prick_darr_t *: Dynamic array to tighten
 */
void prick_darr_tighten(prick_darr_t *);

/**
 * Appends the element (at pointer) to the dynamic array.  Assumes
 * element is of the same type as the members of the dynamic array
 * (hence has the same size in bytes as prick_darr_t.size).
 *
 * @param prick_darr_t *: Dynamic array to append to
 *
 * @param void *: (Pointer to) element to append
 */
void prick_darr_append(prick_darr_t *, void *);

/**
 * Appends array of n elements (referred by pointer) to the dynamic
 * array.  Assumes each element is of the same type as the members of
 * the dynamic array (hence has the same size in bytes as
 * prick_darr_t.size).
 *
 * @param prick_darr_t *: Dynamic array to append to
 *
 * @param void *: (Pointer to) array of elements to append
 *
 * @param size_t: Number of elements in array to append
 */
void prick_darr_append_n(prick_darr_t *, void *, size_t);

/**
 * Writes the element (at pointer) at a specific position in the
 * dynamic array.  Assumes element is of the same type as the members
 * of the dynamic array (hence has the same size in bytes as
 * prick_darr_t.size).  Will fail (i.e. do nothing) if position is out
 * of bounds.
 *
 * @param prick_darr_t *: Dynamic array to insert in
 *
 * @param void *: (Pointer to) element to insert
 *
 * @param size_t: Index where to write element
 */
void prick_darr_write(prick_darr_t *, void *, size_t);

/**
 * Writes array of n elements (referred by pointer) at a specific
 * position in the dynamic array.  Assumes each element is of the same
 * type as the members of the dynamic array (hence has the same size
 * in bytes as prick_darr_t.size).  Will fail (i.e. do nothing) if
 * position is out of bounds, or if position + number of elements is
 * out of bounds.
 *
 * @param prick_darr_t *: Dynamic array to append to
 *
 * @param void *: (Pointer to) array of elements to append
 *
 * @param size_t: Number of elements in array to append
 *
 * @param size_t: Index where to start overwriting elements
 */
void prick_darr_write_n(prick_darr_t *, void *, size_t, size_t);

#endif
