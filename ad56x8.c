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
#include <util/delay.h>
#include "spi.h"
#include "ad56x8.h"

static void
ad56x8_select(void)
{
	AD56X8_PORT |= (1 << AD56X8_SS);
	_delay_us(0.020);
	AD56X8_PORT &= ~(1 << AD56X8_SS);
}

static void
ad56x8_command(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	ad56x8_select();
	spi_write(a);
	spi_write(b);
	spi_write(c);
	spi_write(d);
	_delay_us(0.020);
	ad56x8_select();
}

void
ad56x8_setup(int vref_on)
{
	AD56X8_DDR |= (1 << AD56X8_SS);
	ad56x8_command(0x07, 0x00, 0x00, 0x00);
	if (vref_on)
		ad56x8_command(0x08, 0x00, 0x00, 0x01);
}

void
ad56x8_write_update(int channel, uint16_t val)
{
	if (channel == -1)
		channel = 15;
	ad56x8_command(0x03, (channel & 0xf) << 4,
	    (val >> 8) & 0xff, val & 0xff);
}
