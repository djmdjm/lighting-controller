#ifndef MCP23S1X_H
#define MCP23S1X_H

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

/* Driver for Microchip MCP23S1x 16-port SPI IO multiplexer */

#define MCP23S1X_PORT		PORTB
#define MCP23S1X_DDR		DDRB
#define MCP23S1X_SS		0

#if 0
#define MCP23S1X_INT_PORT	PORTB
#define MCP23S1X_INT_PIN	PINB
#define MCP23S1X_INT_PCMSK	PCMSK0
#define MCP23S1X_INT_PCIE	PCIE0
#define MCP23S1X_INT_PIN_A	0
#define MCP23S1X_INT_PIN_B	4
#define MCP23S1X_INT_A		PCINT0
#define MCP23S1X_INT_B		PCINT4
#endif

/*
 * Define MCP23S1X_INTERRUPT_HANDLER if you want this module to set up the
 * interrupt handler itself. Otherwise, you need to call mcp23s1x_interrupt()
 * from a handler elsewhere.
 */
/* #define MCP23S1X_INTERRUPT_HANDLER */

/* Set up the MCP23S1X */
void mcp23s1x_setup(void);

/*
 * Select port direction for both 8-wire ports, with the MSB representing
 * GPA7, descending to GPA0, then descending from GPB7 to the LSB representing
 * GPB0. A set bit represents output.
 */
void mcp23s1x_set_iodir(uint8_t addr, uint16_t iodir);

/* Set the GPIO output pins. Same numbering as mcp23s1x_set_iodir() */
void mcp23s1x_set_gpio(uint8_t addr, uint16_t val);

/* Read the current status of the GPIO pins */
uint16_t mcp23s1x_get_pins(uint8_t addr);

#endif /* MCP23S1X_H */


