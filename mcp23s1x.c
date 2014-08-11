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
#include "mcp23s1x.h"

#define MCP23S1X_IODIRA		0x00
#define MCP23S1X_IODIRB		0x01
#define MCP23S1X_IPOLA		0x02
#define MCP23S1X_IPOLB		0x03
#define MCP23S1X_GPINTENA 	0x04
#define MCP23S1X_GPINTENB 	0x05
#define MCP23S1X_DEFVALA	0x06
#define MCP23S1X_DEFVALB	0x07
#define MCP23S1X_INTCONA	0x08
#define MCP23S1X_INTCONB	0x09
#define MCP23S1X_IOCONA		0x0A
#define MCP23S1X_IOCONB		0x0B
#define MCP23S1X_GPPUA		0x0C
#define MCP23S1X_GPPUB		0x0D
#define MCP23S1X_INTFA		0x0E
#define MCP23S1X_INTFB		0x0F
#define MCP23S1X_INTCAPA	0x10
#define MCP23S1X_INTCAPB	0x11
#define MCP23S1X_GPIOA		0x12
#define MCP23S1X_GPIOB		0x13
#define MCP23S1X_OLATA		0x14
#define MCP23S1X_OLATB		0x15

/* IOCON flags */
#define MCP23S1X_IOCON_UNIMP	0	/* Not implemented */
#define MCP23S1X_IOCON_INTPOL	1	/* 1 = INT is active-high */
#define MCP23S1X_IOCON_ODR	2	/* 1 = INT pin is open-drain out */
#define MCP23S1X_IOCON_HAEN	3	/* 1 = enable SPI address pins */
#define MCP23S1X_IOCON_DISSLW	4	/* 0 = I2C SDA slew rate enabled */
#define MCP23S1X_IOCON_SEQOP	5	/* 1 = increment register on SPI r/w */
#define MCP23S1X_IOCON_MIRROR	6	/* 1 = INT pins are connected */
#define MCP23S1X_IOCON_BANK	7	/* 0 = ports in sequential banks */

static void
mcp23s1x_select(void)
{
	spi_clock_phase(1);
	MCP23S1X_PORT |= (1 << MCP23S1X_SS);
	_delay_us(0.1); /* CS disable time 50ns */
	MCP23S1X_PORT &= ~(1 << MCP23S1X_SS);
	_delay_us(0.1); /* CS setup time 50ns */
}

static void
mcp23s1x_deselect(void)
{
	_delay_us(0.1); /* CS hold time 50ns */
	MCP23S1X_PORT |= (1 << MCP23S1X_SS);
}

static uint8_t
mcp23s1x_read8(uint8_t addr, uint8_t reg)
{
	uint8_t r;

	mcp23s1x_select();
	spi_write(0x41 | (addr << 1));
	spi_write(reg);
	r = spi_write(0xff);
	mcp23s1x_deselect();

	return r;
}

static uint16_t
mcp23s1x_read16(uint8_t addr, uint8_t start_reg)
{
	uint16_t r;

	mcp23s1x_select();
	spi_write(0x41 | (addr << 1));
	spi_write(start_reg);
	r = spi_write(0xff) << 8;
	r |= spi_write(0xff);
	mcp23s1x_deselect();

	return r;
}

static void
mcp23s1x_write8(uint8_t addr, uint8_t reg, uint8_t val)
{
	uint8_t r;

	mcp23s1x_select();
	spi_write(0x40 | (addr << 1));
	spi_write(reg);
	spi_write(val);
	mcp23s1x_deselect();
}

static void
mcp23s1x_write16(uint8_t addr, uint8_t start_reg, uint16_t val)
{
	uint8_t r;

	mcp23s1x_select();
	spi_write(0x40 | (addr << 1));
	spi_write(start_reg);
	spi_write((val >> 8) & 0xff);
	spi_write(val & 0xff);
	mcp23s1x_deselect();
}

void
mcp23s1x_set_iodir(uint8_t addr, uint16_t iodir, int flip_polarity)
{
	mcp23s1x_write16(addr, MCP23S1X_IPOLA, flip_polarity ? ~iodir : 0);
	/* MCP23S1x IO direction register is opposite to AVR; 1 = input */
	mcp23s1x_write16(addr, MCP23S1X_IODIRA, ~iodir);
}

void
mcp23s1x_set_gpio(uint8_t addr, uint16_t val)
{
	mcp23s1x_write16(addr, MCP23S1X_GPIOA, val);
}

uint16_t
mcp23s1x_get_pins(uint8_t addr)
{
	return mcp23s1x_read16(addr, MCP23S1X_GPIOA);
}

void
mcp23s1x_set_interrupt(uint8_t addr, uint16_t pins, uint16_t mode, uint16_t def)
{
	mcp23s1x_write16(addr, MCP23S1X_DEFVALA, def & pins);
	mcp23s1x_write16(addr, MCP23S1X_INTCONA, mode & pins);
	mcp23s1x_write16(addr, MCP23S1X_GPINTENA, pins);
}

uint16_t
mcp23s1x_get_interrupt_pins(uint8_t addr)
{
	return mcp23s1x_read16(addr, MCP23S1X_INTFA);
}

uint16_t
mcp23s1x_get_interrupt_value(uint8_t addr)
{
	return mcp23s1x_read16(addr, MCP23S1X_INTCAPA);
}

void
mcp23s1x_setup(void)
{
	int i;

	MCP23S1X_DDR |= (1 << MCP23S1X_SS);
	mcp23s1x_deselect();
	mcp23s1x_select();
	mcp23s1x_deselect();
	mcp23s1x_select();
	mcp23s1x_deselect();

	/* WTF does this need to be addr=0x4 here? HAEN isn't set after PWR */
	mcp23s1x_write8(0x4, MCP23S1X_IOCONA,
	    (1 << MCP23S1X_IOCON_INTPOL) |	/* INT active-high */
	    (1 << MCP23S1X_IOCON_ODR) |		/* INT active driver output */
	    (1 << MCP23S1X_IOCON_HAEN) |	/* Enable hardware addr pins */
	    (0 << MCP23S1X_IOCON_SEQOP) |	/* Enable register increment */
	    (1 << MCP23S1X_IOCON_MIRROR) |	/* Enable INT A/B pin mirror */
	    (0 << MCP23S1X_IOCON_BANK));	/* Sequential GPIO A/B regs */
}

