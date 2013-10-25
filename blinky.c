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

#include <util/delay.h>

#include "rgbled.h"
#include "demux.h"
#include "lcd.h"
#include "num_format.h"

int
main(void)
{
	int i;

	demux_setup();
	rgbled_setup();
	lcd_setup();

	lcd_moveto(0, 0);
	lcd_string("Hello Hugo");

	for (i = 0;; i++) {
		rgbled_n(i & 0xf);
		demux_set_line(i % 16);
		lcd_moveto(0, 1);
		lcd_string(ntod(i));
		lcd_string("       ");
		_delay_ms(80);
	}
}
