#ifndef SPI_H
#define SPI_H

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

/* Convenience routines for hardware SPI */

#define SPI_PORT	PORTB
#define SPI_DDR		DDRB
#define SPI_MISO	3
#define SPI_MOSI	2
#define SPI_SCLK	1
#define SPI_SS		0

/* Set up port */
void spi_setup(void);

/*
 * By default the SPI clock is configured for devices that sample on the
 * falling edge of the clock cycle. Setting sample_on_leading=1 here will
 * change the polarity for devices that expect samples on the leading edge
 * of SCLK.
 */
void spi_clock_phase(int sample_on_leading);

/* Write a byte; returns the value read from the bus */
uint8_t spi_write(uint8_t val);

#endif /* SPI_H */

