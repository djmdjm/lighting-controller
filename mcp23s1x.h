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
 * Setting flip_polarity causes the input pins' values to be flipped on read.
 */
void mcp23s1x_set_iodir(uint8_t addr, uint16_t iodir, int flip_polarity);

/* Set the GPIO output pins. Same numbering as mcp23s1x_set_iodir() */
void mcp23s1x_set_gpio(uint8_t addr, uint16_t val);

/*
 * Read the current status of the GPIO pins. Reading the GPIO will clear the
 * contents of the interrupt flag register (see mcp23s1x_get_interrupt_pins()
 * below).
 */
uint16_t mcp23s1x_get_pins(uint8_t addr);

/*
 * Configures interrupts for both ports. 'pins' specifies which input
 * pins should be configured to interrupt. 'mode' specifies the mode for
 * each pin; a set bit indicates that an interrupt should be generated when
 * the value of the pin differs from the corresponding value of the 'def'
 * argument, a clear bit will cause an interrupt to be generated whenever
 * a pin changes state.
 */
void mcp23s1x_set_interrupt(uint8_t addr, uint16_t pins, uint16_t mode,
    uint16_t def);

/*
 * When interrupts are enabled, return the pins that have triggered them.
 * This value is reset by mcp23s1x_get_pins() or mcp23s1x_get_interrupt_value().
 */
uint16_t mcp23s1x_get_interrupt_pins(uint8_t addr);

/*
 * Return the state of the input pins at the time of the last interupt.
 * Reading this value clear the interrupt flag for that pin and resets this
 * register.
 */
uint16_t mcp23s1x_get_interrupt_value(uint8_t addr);

#endif /* MCP23S1X_H */


