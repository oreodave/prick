/* prick_sv.h: String Views.
 * Created: 2026-03-01
 * Author: Aryadev Chavali
 * License: See end of file
 * Commentary:

 To utilise this library, please put:
    #define PRICK_SV_IMPL
    #include "prick_sv.h"
 in one of your code units.

 This is a simple read-only string view library.  It defines some extremely
 common functions you'd expect for a string view library, excluding any that
 require allocation.
 */

#ifndef PRICK_SV_H
#define PRICK_SV_H

#include <stdint.h>

typedef struct
{
  uint64_t size;
  const char *data;
} prick_sv_t;

#define PRICK_SV(DATA, SIZE) ((prick_sv_t){.data = (DATA), .size = (SIZE)})
#define PRICK_SV_AUTO(DATA) \
  ((prick_sv_t){.data = (void *)(DATA), .size = sizeof(DATA) - 1})

// Pretty printers
#define PRICK_SV_FMT(SV) (int)(SV).size, (SV).data
#define PR_PRICK_SV      "%.*s"

prick_sv_t prick_sv_chop_left(prick_sv_t, uint64_t size);
prick_sv_t prick_sv_chop_right(prick_sv_t, uint64_t size);
prick_sv_t prick_sv_truncate(prick_sv_t, uint64_t newsize);
prick_sv_t prick_sv_substr(prick_sv_t, uint64_t position, uint64_t size);

prick_sv_t prick_sv_till(prick_sv_t, const char *reject);
prick_sv_t prick_sv_while(prick_sv_t, const char *accept);

#ifdef PRICK_SV_IMPL
#include <stddef.h>
#include <string.h>

prick_sv_t prick_sv_chop_left(prick_sv_t sv, uint64_t size)
{
  if (sv.size <= size)
    return PRICK_SV(NULL, 0);
  return PRICK_SV(sv.data + size, sv.size - size);
}

prick_sv_t prick_sv_chop_right(prick_sv_t sv, uint64_t size)
{
  if (sv.size <= size)
    return PRICK_SV(NULL, 0);
  return PRICK_SV(sv.data, sv.size - size);
}

prick_sv_t prick_sv_truncate(prick_sv_t sv, uint64_t newsize)
{
  if (newsize > sv.size)
    return PRICK_SV(NULL, 0);
  return PRICK_SV(sv.data, newsize);
}

prick_sv_t prick_sv_substr(prick_sv_t sv, uint64_t position, uint64_t size)
{
  prick_sv_t result = prick_sv_truncate(prick_sv_chop_left(sv, position), size);
  return result;
}

prick_sv_t prick_sv_till(prick_sv_t sv, const char *reject)
{
  if (sv.size == 0 || !sv.data)
    return PRICK_SV(NULL, 0);

  uint64_t offset;
  for (offset = 0; offset < sv.size && strchr(reject, sv.data[offset]) == NULL;
       ++offset)
    continue;

  return prick_sv_truncate(sv, offset);
}

prick_sv_t prick_sv_while(prick_sv_t sv, const char *accept)
{
  if (sv.size == 0 || !sv.data)
    return PRICK_SV(NULL, 0);

  uint64_t offset;
  for (offset = 0; offset < sv.size && strchr(accept, sv.data[offset]) != NULL;
       ++offset)
    continue;

  return prick_sv_truncate(sv, offset);
}

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
