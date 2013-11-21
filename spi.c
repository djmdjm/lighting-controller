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

#include "spi.h"

uint8_t
spi_write(uint8_t val)
{
	SPDR = val;
	/* Wait for value to be written. */
	while ((SPSR & (1 << SPIF)) == 0)
		;
	return SPDR;
}

void
spi_setup(void)
{
	/*
	 * MOSI, SCLK are outputs. MISO is an input
	 * Apparently SS needs to be configured as an output or the AVR can
	 * flip to SPI slave mode if it goes low. Waste of a pin!
	 */
	SPI_DDR = (SPI_DDR & ~(1 << SPI_MISO)) |
	    (1 << SPI_MOSI) | (1 << SPI_SCLK) | (1 << SPI_SS);
	SPI_PORT &= ~((1 << SPI_SCLK) || (1 << SPI_MOSI));

	/*
	 * Enable SPI, without interrupts, as master, MSB first,
	 * idle low clock polarity, sample on falling edge of clock and
	 * with SPI running at F_CPU/2.
	 */
	SPSR = (1 << SPI2X);
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPHA);
}

void
spi_clock_phase(int sample_on_leading)
{
	/* SPCR &= ~(1 << SPE); */
	SPCR = (SPCR & ~(1 << CPHA)) |
	    (sample_on_leading ? 0 : (1 << CPHA));
	/* SPCR |= (1 << SPE); */
}

