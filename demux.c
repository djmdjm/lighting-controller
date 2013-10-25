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
#include <util/delay_basic.h>

#include "demux.h"

#define DEMUX_D_MASK	((1 << DEMUX_D0) | (1 << DEMUX_D1) | \
			 (1 << DEMUX_D2) | (1 << DEMUX_D3))
#define DEMUX_MASK	(DEMUX_D_MASK | \
			 (1 << DEMUX_INHIBIT) | (1 << DEMUX_STROBE))

void
demux_set_line(int val)
{
	/* Set value */
	DEMUX_PORT = (DEMUX_PORT & ~DEMUX_MASK) | (1 << DEMUX_STROBE) |
		((val & 0x1) ? 1 << DEMUX_D0 : 0) |
		((val & 0x2) ? 1 << DEMUX_D1 : 0) |
		((val & 0x4) ? 1 << DEMUX_D2 : 0) |
		((val & 0x8) ? 1 << DEMUX_D3 : 0);
	/* Take strobe low to latch value */
	_delay_loop_1(2);
	DEMUX_PORT &= ~(1 << DEMUX_STROBE);
	_delay_loop_1(4);
	DEMUX_PORT |= 1 << DEMUX_STROBE;
}

void
demux_inhibit(void)
{
	DEMUX_PORT |= 1 << DEMUX_INHIBIT;
}

void
demux_setup(void)
{
	DEMUX_DDR |= DEMUX_MASK;
	DEMUX_PORT |= 1 << DEMUX_STROBE;
	demux_inhibit();
}

