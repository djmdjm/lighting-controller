#ifndef RGBLED_H
#define RGBLED_H

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

/* Convenience routines for RGB LED directly attached to MCU */

/* XXX implement PWM for in-between colours */

#define RGBLED_DDR	DDRC
#define RGBLED_PORT	PORTC
#define RGBLED_R	7
#define RGBLED_G	6
#define RGBLED_B	5

/* Set up port */
void rgbled_setup(void);

/* Specify colour by component (binary only) */
void rgbled(int r, int g, int b);

/* Specify colour by integer R=0x01 | G=0x02 | B=0x04 */
void rgbled_n(int v);

#endif /* RGBLED_H */
