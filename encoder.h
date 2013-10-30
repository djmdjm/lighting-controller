#ifndef ENC_H
#define ENC_H

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

/* "Driver" for two-ping quadrature rotary encoder */

#define ENC_PORT		PORTB
#define ENC_PIN			PINB
#define ENC_DDR			DDRB
#define ENC_VECT		PCINT0_vect
#define ENC_PCIE		PCIE0
#define ENC_PCMSK		PCMSK0
#define ENC_PIN_A		6
#define ENC_PIN_B		5
#define ENC_INT_A		PCINT6
#define ENC_INT_B		PCINT5
#define ENC_PULSE_PER_DETENT	4

/* Setup to use the rotary encoder */
void encoder_setup(void);

/*
 * Returns the value of the running counter incremented and decremented by
 * the encoder. It is clamped to INT16_MIN <= v <= INT16_MAX.
 */
int16_t encoder_value(void);

/* Sets the running counter value */
void encoder_setvalue(int16_t v);

#endif /* ENC_H */
