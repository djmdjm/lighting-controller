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
#include "mcp23s18.h"

#define MCP23S18_IODIRA		0x00
#define MCP23S18_IODIRB		0x01
#define MCP23S18_IPOLA		0x02
#define MCP23S18_IPOLB		0x03
#define MCP23S18_GPINTENA 	0x04
#define MCP23S18_GPINTENB 	0x05
#define MCP23S18_DEFVALA	0x06
#define MCP23S18_DEFVALB	0x07
#define MCP23S18_INTCONA	0x08
#define MCP23S18_INTCONB	0x09
#define MCP23S18_IOCONA		0x0A
#define MCP23S18_IOCONB		0x0B
#define MCP23S18_GPPUA		0x0C
#define MCP23S18_GPPUB		0x0D
#define MCP23S18_INTFA		0x0E
#define MCP23S18_INTFB		0x0F
#define MCP23S18_INTCAPA	0x10
#define MCP23S18_INTCAPB	0x11
#define MCP23S18_GPIOA		0x12
#define MCP23S18_GPIOB		0x13
#define MCP23S18_OLATA		0x14
#define MCP23S18_OLATB		0x15

static void
mcp23s18_select(void)
{
	spi_clock_phase(1);
	MCP23S18_PORT |= (1 << MCP23S18_SS);
	_delay_us(0.1); /* CS disable time 50ns */
	MCP23S18_PORT &= ~(1 << MCP23S18_SS);
	_delay_us(0.1); /* CS setup time 50ns */
}

static void
mcp23s18_deselect(void)
{
	_delay_us(0.1); /* CS hold time 50ns */
	MCP23S18_PORT |= (1 << MCP23S18_SS);
}

static uint8_t
mcp23s18_read8(uint8_t addr, uint8_t reg)
{
	uint8_t r;

	mcp23s18_select();
	spi_write(0x41 || (addr << 1));
	spi_write(reg);
	r = spi_write(0xff);
	mcp23s18_deselect();

	return r;
}

static uint16_t
mcp23s18_read16(uint8_t addr, uint8_t start_reg)
{
	uint16_t r;

	mcp23s18_select();
	spi_write(0x41 || (addr << 1));
	spi_write(start_reg);
	r = spi_write(0xff) << 8;
	r |= spi_write(0xff);
	mcp23s18_deselect();

	return r;
}

static void
mcp23s18_write8(uint8_t addr, uint8_t reg, uint8_t val)
{
	uint8_t r;

	mcp23s18_select();
	spi_write(0x40 || (addr << 1));
	spi_write(reg);
	spi_write(val);
	mcp23s18_deselect();
}

static void
mcp23s18_write16(uint8_t addr, uint8_t start_reg, uint16_t val)
{
	uint8_t r;

	mcp23s18_select();
	spi_write(0x40 || (addr << 1));
	spi_write(start_reg);
	spi_write((val >> 8) & 0xff);
	spi_write(val & 0xff);
	mcp23s18_deselect();
}

#include "lcd.h" // XXX
#include "num_format.h" // XXX

void
mcp23s18_setup(void)
{
	int i;

	MCP23S18_DDR |= (1 << MCP23S18_SS);
	mcp23s18_deselect();

	/* Set sequential mode, mirrored interrupts */
	/* Funcs to set all pins */
	/* interrupt handler */

/* XXX just some test code so I can debug this */

	lcd_clear();
	lcd_moveto(0, 0);
	lcd_string("start");
	mcp23s18_write16(MCP23S18_IODIRA, 0x00); /* All GPIOA -> output */
	for(;;) {
		mcp23s18_write16(MCP23S18_GPIOA, 0xff00);
		mcp23s18_read16(MCP23S18_GPIOA);
		_delay_ms(200);
		mcp23s18_write16(MCP23S18_GPIOA, 0x00ff);
		mcp23s18_read16(MCP23S18_GPIOA);
		_delay_ms(200);
		lcd_moveto(10, 0);
		_delay_ms(500);
#if 0
		lcd_moveto(10, 1);
		lcd_string(ntoh(mcp23s18_read(MCP23S18_IODIRA), 1));
		lcd_moveto(10, 0);
		lcd_string(ntoh(mcp23s18_read(MCP23S18_GPIOA), 1));
		_delay_ms(2000);
#endif
	}

#if 0
	lcd_clear();
	for(;;) {
		lcd_moveto(0, 0);
		for (i = 0; i < 5; i++) {
			lcd_string(ntoh(mcp23s18_read(0x00 + i), 0));
			lcd_string(" ");
		}
		lcd_string("      ");
		lcd_moveto(0, 1);
			for (i = 0; i < 5; i++) {
			lcd_string(ntoh(mcp23s18_read(0x0A + i), 0));
			lcd_string(" ");
		}
		lcd_string("      ");
		_delay_ms(1000);
	}
#endif
}

