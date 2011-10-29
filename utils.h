/*
 * Copyright (c) 2011 Alexander Strange <astrange@ithinksw.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef fastdownscale_utils_h
#define fastdownscale_utils_h

#include <stdlib.h>
#include <stdio.h>

static inline unsigned round_up(unsigned a, unsigned b)
{
	if (a % b == 0) return a;
	return ((a / b) + 1) * b;
}

static inline unsigned round_down(unsigned a, unsigned b)
{
	if (a % b == 0) return a;
	return (a / b) * b;
}

// error handling functions

static inline void check_fatal(bool cond, const char *reason = "")
{
    if (cond) return;
    fprintf(stderr, "error: %s\n", reason);
    exit(1);
}

static inline void check_nonfatal(bool cond, const char *reason = "")
{
    if (cond) return;
    fprintf(stderr, "warning: %s\n", reason);
}

static inline void check_assert(bool cond, const char *reason)
{
    if (cond) return;
    fprintf(stderr, "assertion failed: %s\n", reason);
#ifdef DEBUG
    abort();
#else
    __builtin_unreachable();
#endif
}

#define assert(a) check_assert(a, #a)

#endif
