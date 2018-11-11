/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Base on Fake86
  Copyright (C)2010-2013 Mike Chambers

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "MemUtils.h"

using namespace Faux86;

void* MemUtils::memmove(void *dst, const void *src, size_t len)
{
    size_t i;

    /*
        * If the buffers don't overlap, it doesn't matter what direction
        * we copy in. If they do, it does, so just assume they always do.
        * We don't concern ourselves with the possibility that the region
        * to copy might roll over across the top of memory, because it's
        * not going to happen.
        *
        * If the destination is above the source, we have to copy
        * back to front to avoid overwriting the data we want to
        * copy.
        *
        *      dest:       dddddddd
        *      src:    ssssssss   ^
        *              |   ^  |___|
        *              |___|
        *
        * If the destination is below the source, we have to copy
        * front to back.
        *
        *      dest:   dddddddd
        *      src:    ^   ssssssss
        *              |___|  ^   |
        *                     |___|
        */

    if ((uintptr_t)dst < (uintptr_t)src) {
            /*
                * As author/maintainer of libc, take advantage of the
                * fact that we know memcpy copies forwards.
                */
            return memcpy(dst, src, len);
    }

    /*
        * Copy by words in the common case. Look in memcpy.c for more
        * information.
        */

    if ((uintptr_t)dst % sizeof(long) == 0 &&
        (uintptr_t)src % sizeof(long) == 0 &&
        len % sizeof(long) == 0) {

            long *d = (long*)dst;
            const long *s = (const long*) src;

            /*
                * The reason we copy index i-1 and test i>0 is that
                * i is unsigned -- so testing i>=0 doesn't work.
                */

            for (i=len/sizeof(long); i>0; i--) {
                    d[i-1] = s[i-1];
            }
    }
    else {
            char *d = (char*)dst;
            const char *s = (const char*)src;

            for (i=len; i>0; i--) {
                    d[i-1] = s[i-1];
            }
    }

    return dst;
}


void* MemUtils::memcpy(void *dst, const void *src, size_t len)
{
    size_t i;

    /*
        * memcpy does not support overlapping buffers, so always do it
        * forwards. (Don't change this without adjusting memmove.)
        *
        * For speedy copying, optimize the common case where both pointers
        * and the length are word-aligned, and copy word-at-a-time instead
        * of byte-at-a-time. Otherwise, copy by bytes.
        *
        * The alignment logic below should be portable. We rely on
        * the compiler to be reasonably intelligent about optimizing
        * the divides and modulos out. Fortunately, it is.
        */

    if ((uintptr_t)dst % sizeof(long) == 0 &&
        (uintptr_t)src % sizeof(long) == 0 &&
        len % sizeof(long) == 0) {

            long *d = (long*)dst;
            const long *s = (const long*)src;

            for (i=0; i<len/sizeof(long); i++) {
                    d[i] = s[i];
            }
    }
    else {
            char *d = (char*)dst;
            const char *s = (const char*)src;

            for (i=0; i<len; i++) {
                    d[i] = s[i];
            }
    }

    return dst;
}

void* MemUtils::memset(void *ptr, int ch, size_t len)
{
    char *p = (char*)ptr;
    size_t i;
        
    for (i=0; i<len; i++) {
        p[i] = ch;
    }

    return ptr;
}