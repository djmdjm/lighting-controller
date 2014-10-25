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

struct selection {
	size_t n;
	const char *labels[];
};

#define MODE_ONESHOT	0
#define MODE_STROBE	1
#define MODE_MAX	2
static const struct selection modes = {
	MODE_MAX, { "oneshot", "strobe " }
};

#define READY_NO	0
#define READY_YES	1
#define READY_MAX	2
static const struct selection ready = {
	READY_MAX, { " ready ", "*READY*" }
};

#define IN_NONE		0
#define IN_CHAN_1	1
#define IN_CHAN_1_NOT	2
#define IN_CHAN_2	3
#define IN_CHAN_2_NOT	4
#define IN_MANUAL	5
#define IN_MAX		6
static const struct selection inputs = {
	IN_MAX, { "  ", " 1", "!1", " 2", "!2", " M" }
};

#define COMB_NONE	0
#define COMB_OR		1
#define COMB_AND	2
#define COMB_XOR	3
#define COMB_MAX	4
static const struct selection combines = {
	COMB_MAX, { " ", "|", "&", "^" }
};

#define DUR_TICK	0
#define DUR_MICROSEC	1
#define DUR_MILLISEC	2
#define DUR_SEC		3
#define DUR_MAX		4
static const struct selection durations = {
	DUR_MAX, { "cy", "\xe4s", "ms", "s " }
};

#define RATE_MHZ		0
#define RATE_KHZ		1
#define RATE_HZ			2
#define RATE_MILLI_HZ		3
#define RATE_MAX		4
static const struct selection rates = {
	RATE_MAX, { "MHz", "kHz", "Hz ", "mHz" }
};

/* Input configuration for a channel */
struct channel_trigger {
	int input[2];
	int comb;
};

/* Main configuration */
struct config {
	int mode;
	int ready;
	struct channel_trigger ch1, ch2;
	int delay, delay_unit;
	int on, on_unit;
	/* Strobe */
	int freq, freq_unit;
	int len, len_unit;
	/* Oneshot */
	int holdoff, holdoff_unit;
};

static const struct config default_config = {
	MODE_ONESHOT,
	READY_NO,
	/* Channel 1 */
	{ { IN_CHAN_1, IN_NONE }, COMB_NONE },
	/* Channel 2 */
	{ { IN_CHAN_2, IN_NONE }, COMB_NONE },
	/* Delay */
	100, DUR_MILLISEC,
	/* On duration */
	10, DUR_MILLISEC,
	/* Strobe: freq */
	10, RATE_HZ,
	/* Strobe: length */
	1, DUR_SEC,
	/* Oneshot: holdoff time */
	10, DUR_SEC,
};

static struct config cfg;

/* Identifiers for UI inputs */
enum control_id {
	C_MODE,
	C_READY,
	C_CH1_IN1, C_CH1_COMBINE, C_CH1_IN2,
	C_CH2_IN1, C_CH2_COMBINE, C_CH2_IN2,
	C_DELAY, C_DELAY_U,
	C_ON, C_ON_U,
	C_FREQ, C_FREQ_U,
	C_DURATION, C_DURATION_U,
	C_HOLDOFF, C_HOLDOFF_U,
};

/* Identifiers for different types of input */
enum control_type {
	I_LAB,	/* Uneditable label */
	I_SEL,	/* Selection list */
	I_INT,	/* Integer */
	I_OTH,	/* Other: special case */
};

/*
 * UI element and layout/editing information
 * NB. must be increasing X, Y order.
 */
struct control {
	int x, y;
	int id;
	int type;
	size_t int_width;
	char *label;
	int *value;
	const struct selection *selection;
};

/* UI for oneshot mode */
#define NUM_CONTROLS_ONESHOT 20
static const struct control oneshot_controls[NUM_CONTROLS_ONESHOT] = {
	{ 0,  0, -1,		I_LAB, 0, "Mode:", NULL, NULL },
	{ 5,  0, C_MODE,	I_SEL, 0, NULL, &cfg.mode, &modes },
	{ 13, 0, C_READY,	I_SEL, 0, NULL, &cfg.ready, &ready },
	{ 0,  1, -1,		I_LAB, 0, "CH1:", NULL, NULL },
	{ 4,  1, C_CH1_IN1,	I_SEL, 0, NULL, &cfg.ch1.input[0], &inputs },
	{ 6,  1, C_CH1_COMBINE,	I_SEL, 0, NULL, &cfg.ch1.comb, &combines },
	{ 7,  1, C_CH1_IN2,	I_SEL, 0, NULL, &cfg.ch1.input[1], &inputs },
	{ 11, 1, -1,		I_LAB, 0, "CH2:", NULL, NULL },
	{ 15, 1, C_CH2_IN1,	I_SEL, 0, NULL, &cfg.ch2.input[0], &inputs },
	{ 17, 1, C_CH2_COMBINE, I_SEL, 0, NULL, &cfg.ch2.comb, &combines },
	{ 18, 1, C_CH2_IN2,	I_SEL, 0, NULL, &cfg.ch2.input[1], &inputs },
	{ 0,  2, -1,		I_LAB, 0, "Delay:", NULL, NULL },
	{ 6,  2, C_DELAY,	I_INT, 3, NULL, &cfg.delay, NULL },
	{ 9,  2, C_DELAY_U,	I_SEL, 0, NULL, &cfg.delay_unit, &durations },
	{ 12, 2, -1,		I_LAB, 0, "On:", NULL, NULL },
	{ 15, 2, C_ON,		I_INT, 3, NULL, &cfg.on, NULL },
	{ 18, 2, C_ON_U,	I_SEL, 0, NULL, &cfg.on_unit, &durations },
	{ 0,  3, -1,		I_LAB, 0, "Holdoff:", NULL, NULL },
	{ 8,  3, C_HOLDOFF,	I_OTH, 3, NULL, &cfg.holdoff, NULL },
	{ 11, 3, C_HOLDOFF_U,	I_SEL, 0, NULL, &cfg.holdoff_unit, &durations },
};

/* UI for strobe mode */
#define NUM_CONTROLS_STROBE 23
static const struct control strobe_controls[NUM_CONTROLS_STROBE] = {
	{ 0,  0, -1,		I_LAB, 0, "Mode:", NULL, NULL },
	{ 5,  0, C_MODE,	I_SEL, 0, NULL, &cfg.mode, &modes },
	{ 13, 0, C_READY,	I_SEL, 0, NULL, &cfg.ready, &ready },
	{ 0,  1, -1,		I_LAB, 0, "CH1:", NULL, NULL },
	{ 4,  1, C_CH1_IN1,	I_SEL, 0, NULL, &cfg.ch1.input[0], &inputs },
	{ 6,  1, C_CH1_COMBINE,	I_SEL, 0, NULL, &cfg.ch1.comb, &combines },
	{ 7,  1, C_CH1_IN2,	I_SEL, 0, NULL, &cfg.ch1.input[1], &inputs },
	{ 0,  1, -1,		I_LAB, 0, "CH2:", NULL, NULL },
	{ 12, 1, C_CH2_IN1,	I_SEL, 0, NULL, &cfg.ch2.input[0], &inputs },
	{ 14, 1, C_CH2_COMBINE, I_SEL, 0, NULL, &cfg.ch2.comb, &combines },
	{ 15, 1, C_CH2_IN2,	I_SEL, 0, NULL, &cfg.ch2.input[1], &inputs },
	{ 0,  2, -1,		I_LAB, 0, "Delay:", NULL, NULL },
	{ 6,  2, C_DELAY,	I_INT, 3, NULL, &cfg.delay, NULL },
	{ 9,  2, C_DELAY_U,	I_SEL, 0, NULL, &cfg.delay_unit, &durations },
	{ 12, 2, -1,		I_LAB, 0, "On:", NULL, NULL },
	{ 15, 2, C_ON,		I_INT, 3, NULL, &cfg.on, NULL },
	{ 18, 2, C_ON_U,	I_SEL, 0, NULL, &cfg.on_unit, &durations },
	{ 5,  3, -1,		I_LAB, 0, "Freq:", NULL, NULL },
	{ 5,  3, C_FREQ,	I_INT, 3, NULL, &cfg.freq, NULL },
	{ 8,  3, C_FREQ_U,	I_SEL, 0, NULL, &cfg.freq_unit, &rates },
	{ 13, 3, -1,		I_LAB, 0, "L:", NULL, NULL },
	{ 15, 3, C_DURATION,	I_INT, 3, NULL, &cfg.len, NULL },
	{ 18, 3, C_DURATION_U,	I_SEL, 0, NULL, &cfg.len_unit, &durations },
};

/*
 * Returns true if the current control should be skipped due to the
 * config making it irrelevant.
 */
static int
control_skipped(int v)
{
	return (v < 0 ||
	    (v == C_CH1_IN2 && cfg.ch1.comb == COMB_NONE) ||
	    (v == C_CH2_IN2 && cfg.ch2.comb == COMB_NONE) ||
	    (v == C_HOLDOFF_U && cfg.holdoff == -1));
}

/*
 * Look up the index of the next control. 'current' is the index of the
 * current control. This handles controls that are disabled by the current
 * configuration (e.g. units for holdoff when manual holdoff selected).
 * If 'dec' is set then decrement rather than increment the control.
 */
static uint8_t
incdec_control_oneshot(uint8_t current, int dec)
{
	uint8_t next, v;
	const struct control *controls = cfg.mode == MODE_ONESHOT ?
	    oneshot_controls : strobe_controls;
	size_t control_max = cfg.mode == MODE_ONESHOT ?
	    NUM_CONTROLS_ONESHOT : NUM_CONTROLS_STROBE;

	do {
		next = (dec ? (next - 1) : (next + 1)) % control_max;
	} while (control_skipped(controls[next].id));

	return next;
}

static void
draw(uint8_t active)
{
	size_t i;
	int x, cursor_x, cursor_y;
	char nbuf[16];
	const struct control *controls = cfg.mode == MODE_ONESHOT ?
	    oneshot_controls : strobe_controls;
	size_t control_max = cfg.mode == MODE_ONESHOT ?
	    NUM_CONTROLS_ONESHOT : NUM_CONTROLS_STROBE;

	for (i = 0; i < control_max; i++) {
		const struct control *ctrl = &controls[i];
		const struct control *next_ctrl = (i + 1 > control_max) ?
		    &controls[i + 1] : NULL;

		/* Don't draw skipped controls */
		if (ctrl->id != -1 && control_skipped(ctrl->id)) {
			/* Blank characters to the next UI element */
			lcd_getpos(&x, NULL);
			if (next_ctrl == NULL || next_ctrl->y != ctrl->y)
				lcd_clear_eol();
			else if (x < next_ctrl->x)
				lcd_fill(' ', next_ctrl->x - x);
			continue;
		}

		/* Record cursor position for editing */
		if (active == i) {
			cursor_x = ctrl->x;
			cursor_y = ctrl->y;
		}
		/* Draw control */
		lcd_moveto(ctrl->x, ctrl->y);
		switch (ctrl->type) {
		case I_LAB:
			lcd_string(ctrl->label);
			break;
		case I_INT:
			if (ctrl->int_width >= sizeof(nbuf)) {
				lcd_string("BAD INT WIDTH");
				return;
			}
			lcd_string(rjustify(ntod(*ctrl->value),
			    nbuf, ctrl->int_width + 1));
			break;
		case I_SEL:
			if (*ctrl->value < 0 || ctrl->selection == NULL ||
			    (size_t)*ctrl->value > ctrl->selection->n) {
				lcd_string("BAD SELECTION");
				return;
			}
			lcd_string(ctrl->selection->labels[*ctrl->value]);
			break;
		case I_OTH:
			switch (ctrl->id) {
			case C_HOLDOFF:
				if (*ctrl->value == -1)
					lcd_string("MAN");
				else if (ctrl->int_width >= sizeof(nbuf)) {
					lcd_string("BAD INT WIDTH");
					return;
				} else {
					lcd_string(rjustify(ntod(*ctrl->value),
					    nbuf, ctrl->int_width + 1));
				}
				break;
			}
			break;
		}
	}
}

static void
strobe_entry(void)
{
	uint8_t editing;
	uint8_t active;

	lcd_display(1, 0, 1);
	lcd_moveto(0, 0);
	lcd_clear();

	while (1) {
		draw(0);
	}
}

static void
strobe(void)
{
}

static void
oneshot_entry(void)
{
}

static void
oneshot(void)
{
}

int
main(void)
{
	int i, j;

	/*
	 * NB external xtal. To select "write lfuse 0 0x2f"
	 */
	CLKPR = 0x80;
	CLKPR = 0x00; /* 20 MHz */

	/* Turn all pin change interrupts off by default */
	PCICR = 0x00;
	PCMSK0 = 0x00;
	PCMSK1 = 0x00;
	PCMSK2 = 0x00;
	PCMSK3 = 0x00;

	/* I/O port direction */
	DDRA = 0x0f; /* 0-3: out 4-5: in */
	DDRB = (1 << 4);
	DDRD = (1 << 7);
	PORTB = 0x00;
	PORTD = (1 << 7);
	PORTA = 0x02;

	event_setup();
	encoder_setup();
	lcd_setup();
	sei();

	lcd_display(1, 1, 1);
	lcd_moveto(0, 0);
	lcd_string("OK ");

	strobe_entry();

	i = j = 0;
	while (1) {
		//lcd_clear();
		lcd_moveto(0, 0);
		lcd_string("i=");
		lcd_string(ntod(i));
		lcd_clear_eol();

		lcd_moveto(0, 1);
		lcd_string((PINB & (1<<2)) ? "ENC:OFF" : "ENC:ON ");
		lcd_string(" ");
		lcd_string((PINB & (1<<3)) ? "PB:ON " : "PB:OFF");

		lcd_moveto(0, 2);
		lcd_string((PINA & (1<<4)) ? "IN1:OFF" : "IN1:ON ");
		lcd_string(" ");
		lcd_string((PINA & (1<<5)) ? "IN2:OFF" : "IN2:ON ");

		lcd_moveto(0, 3);
		lcd_string("encoder=");
		lcd_string(ntod(encoder_value()));
		lcd_clear_eol();

		lcd_moveto(9, 3);
		_delay_ms(100);

		//if (++j > 500) {
			i++;
			j = 0;
			PORTA ^= 0x0f;
			PORTB ^= (1 << 4);
			PORTD ^= (1 << 7);
		//}
		//_delay_ms(1);
	}
}
