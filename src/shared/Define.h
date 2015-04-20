/*
 * Copyright (C) 2005-2008 Mangos <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef TRINITY_DEFINE_H
#define TRINITY_DEFINE_H

#include "CompilerDefs.h"

#include <cinttypes>
#include <cstddef>
#include <climits>


#define TRINITY_LITTLEENDIAN 0
#define TRINITY_BIGENDIAN    1

#if !defined(TRINITY_ENDIAN)
#  if defined (BOOST_BIG_ENDIAN)
#    define TRINITY_ENDIAN TRINITY_BIGENDIAN
#  else
#    define TRINITY_ENDIAN TRINITY_LITTLEENDIAN
#  endif
#endif

#if PLATFORM == PLATFORM_WINDOWS
#  define TRINITY_LIBRARY_HANDLE HMODULE
#  define TRINITY_PATH_MAX MAX_PATH
#  ifndef DECLSPEC_NORETURN
#    define DECLSPEC_NORETURN __declspec(noreturn)
#  endif //DECLSPEC_NORETURN
#  ifndef DECLSPEC_DEPRECATED
#    define DECLSPEC_DEPRECATED __declspec(deprecated)
#  endif //DECLSPEC_DEPRECATED
#else //PLATFORM != PLATFORM_WINDOWS
#  define TRINITY_LIBRARY_HANDLE void*
#  define TRINITY_PATH_MAX PATH_MAX
#  define DECLSPEC_NORETURN
#  define DECLSPEC_DEPRECATED
#endif //PLATFORM

#if COMPILER == COMPILER_GNU
#  define ATTR_NORETURN __attribute__((noreturn))
#  define ATTR_PRINTF(F,V) __attribute__ ((format (printf, F, V)))
#  define ATTR_DEPRECATED __attribute__((__deprecated__))
#else //COMPILER != COMPILER_GNU
#  define ATTR_NORETURN
#  define ATTR_PRINTF(F,V)
#  define ATTR_DEPRECATED
#endif //COMPILER == COMPILER_GNU

#define UI64FMTD "%" PRIu64
#define UI64LIT(N) UINT64_C(N)

#define SI64FMTD "%u"
#define SI64LIT(N) INT64_C(N)

#define SZFMTD "%" PRIuPTR

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#endif //TRINITY_DEFINE_H
