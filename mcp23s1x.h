#ifndef MCP23S18_H
#define MCP23S18_H

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

/* Driver for Microchip MCP23S18 16-port SPI IO multiplexer */

#define MCP23S18_PORT		PORTB
#define MCP23S18_DDR		DDRB
#define MCP23S18_SS		0
#define MCP23S18_INT_PORT	PORTB
#define MCP23S18_INT_PIN	PINB
#define MCP23S18_INT_PCMSK	PCMSK0
#define MCP23S18_INT_PCIE	PCIE0
#define MCP23S18_INT_PIN_A	0
#define MCP23S18_INT_PIN_B	4
#define MCP23S18_INT_A		PCINT0
#define MCP23S18_INT_B		PCINT4

/*
 * Define MCP23S18_INTERRUPT_HANDLER if you want this module to set up the
 * interrupt handler itself. Otherwise, you need to call mcp23s18_interrupt()
 * from a handler elsewhere.
 */
/* #define MCP23S18_INTERRUPT_HANDLER */

/* Set up the MCP23S18 */
void mcp23s18_setup(void);

/*
 * Select port direction for each 8-wire port, with the LSB representing the
 * lowest-numbered GPIO line. A set bit represents output, clear for input.
 */
void mcp23s18_direction(uint8_t cpa_dir, uint8_t gpb_dir);

/*
 * Configure interrupts mode for each port. A set bit requests interupts for
 * that pin, which will be delivered on the corresponding interrupt line.
 * When requesting interrupts, it is also necessary to specify the mode
 * and/or a comparison value for those pins. When the pin's mode bit it set,
 * Interrupts are raised when the input differs from the comparison value.
 * If the pin's mode bit is clear, an interrupt is generated whenever the
 * pin changes state.
 */
void mcp23s18_config_interrupt(uint8_t cpa_int, uint8_t cpa_mode,
    uint8_t cpa_comp, uint8_t cpb_int, uint8_t cpb_mode, uint8_t cpb_comp);

/*
 * The actual interrupt handler. Use only if MCP23S18_INTERRUPT_HANDLER
 * is unset
 */
void mcp23s18_interrupt(void);

#endif /* MCP23S18_H */


