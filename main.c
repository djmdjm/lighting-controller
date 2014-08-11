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
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "rgbled.h"
#include "demux.h"
#include "lcd.h"
#include "num_format.h"
#include "spi.h"
#include "ad56x8.h"
#include "encoder.h"
#include "event.h"
#include "mcp23s1x.h"
#include "midi.h"

int
main(void)
{
	uint8_t rx, ev_type, ev_v1, ev_v2, ev_v3;

	CLKPR = 0x80;
	CLKPR = 0x00; /* 16 MHz */
	/* CLKPR = 0x03; *//* 2 MHz */

	event_setup();
	lcd_setup();
	//lcd_display(1, 1, 1);
	midi_init(0xffff); /* XXX poly */
	spi_setup();
	ad56x8_setup(1);

	DDRC = 0xff; /* XXX gate output */

	/* Prepare serial */
	DDRD |= 1<<3;
	DDRD &= ~(1<<2);
	PORTD |= 1<<2 | 1<<3;

	UCSR1A=0;
	UCSR1B=(1<<RXEN1)|(1<<TXEN1); // Both receiver and transmitter enable
	UCSR1C=3<<UCSZ10; // 8 data bit, a stop, none parity
	UBRR1H = 0;
	/* UBRR1L = 103; */ /* for 9600 baud testing */
	UBRR1L = 31; /* MIDI 31250 baud */

	lcd_moveto(0, 0);
	lcd_string("OK ");

	DDRD |= 1<<6;
	while (1) {
		if ((UCSR1A & (1<<RXC1)) != 0) {
			rx = UDR1;
			if (rx != 0xfe)
				PORTD ^= 1<<6; /* blink on rx */
			midi_in(rx);
		}
		if (event_dequeue(&ev_type, &ev_v1, &ev_v2, &ev_v3)) {
#if 2
			//lcd_clear();
			lcd_moveto(0, 0);
			lcd_string(ntoh(ev_type, 0));
			lcd_string(" ");
			lcd_string(ntoh(ev_v1, 0));
			lcd_string(" ");
			lcd_string(ntoh(ev_v2, 0));
			lcd_string(" ");
			lcd_string(ntoh(ev_v3, 0));
			lcd_string(" ");
#endif
		}
	}
}
