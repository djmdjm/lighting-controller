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
#include "mcp23s18.h"

/* LUT for mux wiring */
static const int mux_order[16] = {
	7, 6, 5, 4, 3, 1, 2, 0, 10, 11, 8, 9, 14, 15, 12, 13
};

ISR(ENC_VECT)
{
	encoder_interrupt();
}

int
main(void)
{
	int i, j, x, e;
	uint8_t ev_type;
	uint16_t ev_value;
	char buf[16];

	demux_setup();
	rgbled_setup();
	lcd_setup();
	spi_setup();
	//ad56x8_setup(1);
	encoder_setup();
	mcp23s18_setup();
	sei();

#if 0
	for (x = i = 0;; i++) {
		rgbled_n(i);
		demux_set_line(mux_order[i % 16]);
		lcd_moveto(0, 0);
		lcd_string(rjustify(ntod(i), buf, 7));
		lcd_string(" Julius & Hugo");

		lcd_moveto(0, 1);
		lcd_string(rjustify(ntod(encoder_value()), buf, 7));

		e = event_dequeue(&ev_type, &ev_value);
		while (event_dequeue(NULL, NULL))
			;
		if (e != 0 || x-- == 0) {
			if (e != 0) {
				lcd_moveto(7, 1);
				lcd_string(rjustify(ntoh(ev_type, 0), buf, 3));
				lcd_string(" ");
				lcd_string(rjustify(ntoh(ev_value, 0), buf, 5));
				x = 16;
			} else {
				lcd_moveto(7, 1);
				lcd_string("       ");
			}
		}

		lcd_moveto(15, 1);
		lcd_string(rjustify(ntoh(event_nqueued(), 0), buf, 3));

		lcd_moveto(18, 1);
		lcd_string(rjustify(ntoh(event_maxqueued(), 0), buf, 3));

		for (j = 0; j < 512; j++)
			ad56x8_write_update(-1, (j & 1) ? 0xffff : 0);
		//_delay_ms((60 * 250)/140);
	}
#endif
}
