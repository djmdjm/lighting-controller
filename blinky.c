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

/* Lamp/LCD test application */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stddef.h>
#include <stdint.h>

#include "rgbled.h"
#include "demux.h"
#include "lcd.h"
#include "num_format.h"
#include "spi.h"
#include "ad56x8.h"
#include "encoder.h"
#include "event.h"
#include "mcp23s1x.h"

ISR(ENC_VECT)
{
	encoder_interrupt();
}

struct channel_map {
	int addr;
	int pin;
};
struct channel_map channel_a[16] = {
	{ 0x4, 0 }, { 0x4, 4 }, { 0x4, 11 }, { 0x4, 15 },
	{ 0x5, 4 }, { 0x5, 0 }, { 0x5, 15 }, { 0x5, 11 },
	{ 0x4, 1 }, { 0x4, 5 }, { 0x4, 10 }, { 0x4, 14 },
	{ 0x5, 5 }, { 0x5, 1 }, { 0x5, 14 }, { 0x5, 10 },
};
struct channel_map channel_b[8] = {
	{ 0x4, 2 }, { 0x4, 6 }, { 0x4, 9 }, { 0x4, 13 },
	{ 0x5, 6 }, { 0x5, 2 }, { 0x5, 13 }, { 0x5, 9 },
};
struct channel_map channel_c[8] = {
	{ 0x4, 3 }, { 0x4, 7 }, { 0x4, 8 }, { 0x4, 12 },
	{ 0x5, 7 }, { 0x5, 3 }, { 0x5, 12 }, { 0x5, 8 },
};

int
main(void)
{
	uint16_t i, a, b, c, v4, v5, p;
	char buf[3];

	CLKPR = 0x80;
	CLKPR = 0x03; /* 2 MHz */
//#define lcd_setup()
//#define lcd_moveto(a,b)
//#define lcd_string(a)
	lcd_setup();
	spi_setup();
	mcp23s1x_setup();
	sei();

	mcp23s1x_set_iodir(0x4, 0xffff);
	mcp23s1x_set_iodir(0x5, 0xffff);
	mcp23s1x_set_iodir(0x6, 0x00ff);

///* XXX */ DDRD |= 1<<6;
	for (i = a = b = c = 0; ; i++) {
///* XXX */	PORTD = PORTD ^ (1<<6);
		v4  = (channel_a[a].addr == 0x4) ? (1 << channel_a[a].pin) : 0;
		v4 |= (channel_b[b].addr == 0x4) ? (1 << channel_b[b].pin) : 0;
		v4 |= (channel_c[c].addr == 0x4) ? (1 << channel_c[c].pin) : 0;

		v5  = (channel_a[a].addr == 0x5) ? (1 << channel_a[a].pin) : 0;
		v5 |= (channel_b[b].addr == 0x5) ? (1 << channel_b[b].pin) : 0;
		v5 |= (channel_c[c].addr == 0x5) ? (1 << channel_c[c].pin) : 0;

		mcp23s1x_set_gpio(0x4, v4);
		mcp23s1x_set_gpio(0x5, v5);
		//mcp23s1x_set_gpio(0x6, 1 << (i % 5));
		p = mcp23s1x_get_pins(0x6);
		mcp23s1x_set_gpio(0x6, 
			!(p & (1 << 15)) << 0 |
			!(p & (1 << 14)) << 1 |
			!(p & (1 << 13)) << 2 |
			!(p & (1 << 12)) << 3 |
			!(p & (1 << 11)) << 4 |
			((p & (1 << 10)) ? 0x0 : 0xff));

		lcd_moveto(0, 0);
		lcd_string("Analogue Sequence Test");
		lcd_moveto(0, 1);
		lcd_string("a = ");
		lcd_string(rjustify(ntod(a), buf, 3));
		lcd_string(" b = ");
		lcd_string(rjustify(ntod(b), buf, 3));
		lcd_string(" c = ");
		lcd_string(rjustify(ntod(c), buf, 3));

		_delay_ms(107);

		a = (a + 1) & 0xf;
		if ((i % 2) == 0)
			b = (b + 1) & 0x7;
		if ((i % 4) == 0)
			c = (c + 1) & 0x7;
	}
}
