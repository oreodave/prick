/* prick_aliases.h:
 * Created: 2025-04-09
 * Author: Aryadev Chavali
 * License: See end of file
 * Commentary:

 This library defines some useful aliases for common types.  These are mostly
 handpicked, and aren't necessary.
 */

#ifndef PRICK_ALIASES_H
#define PRICK_ALIASES_H

#include <assert.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

static_assert(sizeof(float) == 4, "f32 requires 4 byte floats");
static_assert(sizeof(double) == 8, "f64 requires 8 byte doubles");
typedef float f32;
typedef double f64;

#endif

/* Copyright (C) 2025 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the Unlicense for details.

 * You may distribute and modify this code under the terms of the Unlicense,
 * which you should have received a copy of along with this program.  If not,
 * please go to <https://unlicense.org/>.

*/
