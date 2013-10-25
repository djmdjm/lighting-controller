/*
 * Copyright (c) 2013 Damien Miller <djm@mindrot.org>
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

#include <avr/io.h>
#include "rgbled.h"

#define RGBLED_MASK	((1 << RGBLED_R) | (1 << RGBLED_G) | (1 << RGBLED_B))

void
rgbled(int r, int g, int b)
{
	RGBLED_PORT = (RGBLED_PORT & ~RGBLED_MASK) |
	    (r ? 1 << RGBLED_R : 0) |
	    (g ? 1 << RGBLED_G : 0) |
	    (b ? 1 << RGBLED_B : 0);
}

void
rgbled_n(int v)
{
	rgbled(v & 0x1, v & 0x2, v & 0x4);
}

void
rgbled_setup(void)
{
	RGBLED_DDR |= RGBLED_MASK;
	rgbled(0, 0, 0);
}

