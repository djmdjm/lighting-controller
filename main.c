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
#include <avr/sleep.h>
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
#include "event-types.h"
#include "mcp23s1x.h"
#include "midi.h"
#include "ui.h"

static int pb_encoder = 1;
static int pb_button = 0;
static int running = 0;

/* Interrupt for the digitial inputs */
ISR(PCINT0_vect)
{
	/* Nothing here - we just need an interrupt to leave sleep. */
}

/* Interrupt for pushbuttons and the rotaty encoder */
ISR(PCINT1_vect)
{
	int pb;

	if (running)
		return; /* no events */

	encoder_interrupt();
	pb = (PINB >> 2) & 1;
	if (pb != pb_encoder) {
		/* NB. encoder button is active-low */
		event_enqueue(EV_BUTTON, 0, !pb, 0, 0);
		pb_encoder = pb;
	}
	pb = (PINB >> 3) & 1;
	if (pb != pb_button) {
		event_enqueue(EV_BUTTON, 1, pb, 0, 0);
		pb_button = pb;
	}
}

static void
sleep_for_interrupt(void)
{
	set_sleep_mode(SLEEP_MODE_IDLE);
	cli();
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
}

static int
evaluate_input(int channel_mode)
{
	/* Digital inputs and encoder are active-high */
	switch (channel_mode) {
	case TRIG_CHAN_1:
		return (PINA & (1<<4)) == 0;
	case TRIG_CHAN_1_NOT:
		return (PINA & (1<<4)) != 0;
	case TRIG_CHAN_2:
		return (PINA & (1<<5)) == 0;
	case TRIG_CHAN_2_NOT:
		return (PINA & (1<<5)) != 0;
	case TRIG_MANUAL:
		return (PINB & (1<<3)) == 0;
	case TRIG_NONE:
	default:
		return 0;
	}
}

static uint32_t
duration_to_cycles(int n, int unit)
{
	switch (unit) {
	case DUR_MICROSEC:
		return n * (F_CPU / 1000000);
	case DUR_MILLISEC:
		return n * (F_CPU / 1000);
	case DUR_SEC:
		return n * F_CPU;
	}
	return 0;
}

static uint32_t
freq_to_cycles(int f, int unit)
{
	switch (unit) {
	case RATE_MHZ:
		return (F_CPU / 1000000) / f;
	case RATE_KHZ:
		return (F_CPU / 1000) / f;
	case RATE_HZ:
		return F_CPU / f;
	case RATE_MILLI_HZ:
		return (F_CPU * 1000) / f;
	}
	return 0;
}

/*
 * prepare_wait() / LONG_WAIT() implement delays accurate to 6 cycles
 * over ranges up to ~1k sec.
 */

struct longwait {
	uint32_t t50m;
	uint16_t t768;
	uint8_t t3;
	uint8_t tiny;
};

static void
dump_longwait(struct longwait *lw) {
	lcd_string(ntod(lw->t50m));
	lcd_char(' ');
	lcd_string(ntod(lw->t768));
	lcd_char(' ');
	lcd_string(ntod(lw->t3));
	lcd_char(' ');
	lcd_string(ntod(lw->tiny));
	lcd_clear_eol();
}

static void
prepare_wait(uint32_t t, struct longwait *lw)
{
	memset(lw, 0, sizeof(*lw));
	/* Tiny delays use nop sled */
	if (t < 40) {
		lw->tiny = 40 - (t < 14 ? 0 : t - 14);
		return;
	}
	/* Take off some cycles for comparisons (determined empirically) */
	t -= 36;
	lw->t50m = t / 50000000; /* ~max 256*3 cycles that fit a u16 */
	t %= 50000000;
	lw->t768 = t / 768; /* Max _delay_loop_1() length = 236*3 cycles */
	t %= 768;
	lw->t3 = t / 3; /* _delay_loop_1() takes 3 cycles per loop */
}

#define LONG_WAIT(lw) do { \
	uint32_t __i; \
	uint16_t __j; \
	\
	if (lw.tiny != 0) { \
		/* Really short delays jump into a nop sled */ \
		__asm__ volatile ( \
			"ldi r31, pm_hi8(1f)" "\n\t" \
			"ldi r30, pm_lo8(1f)" "\n\t" \
			"add r30, %0" "\n\t" \
			"adc r31, __zero_reg__" "\n\t" \
			"ijmp" "\n\t" \
			"1:" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			"nop" "\n\t" \
			: /* no output */ \
			: "l" (lw.tiny) \
			: "r30", "r31" \
		); \
	} else { \
		/* Longer delays use the smallest possible counter */ \
		for (__i = lw.t50m; __i != 0; __i--) \
			__builtin_avr_delay_cycles(50000000); \
		for (__j = lw.t768; __j != 0; __j--) \
			__builtin_avr_delay_cycles(762); /* empirical */ \
		if (lw.t3 != 0) /* time == 0 means sleep for 256*3 cycles! */ \
			_delay_loop_1(lw.t3); \
	} \
} while (0)

static void
timing_test(void)
{
	struct longwait on, off;

	lcd_setup();
	lcd_display(1, 0, 1);

	DDRA = 0x0f;
	memset(&on, 0, sizeof(on));
	memset(&off, 0, sizeof(on));
	prepare_wait(1, &on);
	prepare_wait(804, &off);

	lcd_moveto(0, 0);
	dump_longwait(&on);
	lcd_moveto(0, 1);
	dump_longwait(&off);

	for (;;) {
		PORTA = 0x0f;
		LONG_WAIT(on);
		PORTA = 0;
		LONG_WAIT(off);
	}
}

int
main(void)
{
	int i, ready1, ready2, done;
	uint32_t j, wait1, wait2, on, off, holdoff, cycle_len, duration, ncyc;
	struct longwait wait1_l, wait2_l, on_l, off_l, holdoff_l, cycle_len_l;
	uint8_t off_before_ch2, strobe_out;

	/*
	 * NB. external xtal. To select "write lfuse 0 0x6f"
	 */
	CLKPR = 0x80;
	CLKPR = 0x00; /* 20 MHz */

	if (0)
		timing_test();

	/* Turn all pin change interrupts off by default */
	PCICR = 0x00;
	PCMSK0 = 0x00;
	PCMSK1 = 0x00;
	PCMSK2 = 0x00;
	PCMSK3 = 0x00;

	/* I/O port direction */
	DDRA = 0x0f; /* 0-1: digital out 4-5: digital in */
	DDRB = (1 << 4); /* 0-2: encoder 3: button 4: LED1 */
	DDRD = (1 << 7); /* 7: LED2 */
	PORTB = 0x00;
	PORTD = 0x00;
	PORTA = 0x00;

	lcd_setup();
	lcd_display(1, 0, 1);
	lcd_string("OK ");
	lcd_moveto(0, 0);

	reset_config();
	event_setup();
	encoder_setup();

	/* Enable interrupts for buttons */
	PCMSK1 |= (1 << 2)|(1 << 3);
	PCICR |= (1 << PCIE1);

	sei();

	for (;;) {
		/* Disable interrupts for inputs */
		PCMSK0 &= ~((1 << 4) | (1 << 5));
		PCICR &= ~(1 << PCIE0);

		/* Drain any queued events */
		event_drain();
		running = 0;

		/* Input lights off */
		PORTB &= ~(1 << 4);
		PORTD &= ~(1 << 7);

		/*
		 * Let the user edit the configuration. This ends when they
		 * select "ready".
		 */
		config_edit();

		/* In running mode now */
		cli();
		running = 1; /* Accessed in interrupt handler */
		sei();
		lcd_display(1, 0, 0);

		/* Calculate delays. */
		wait1 = duration_to_cycles(cfg.wait, cfg.wait_unit);
		wait2 = duration_to_cycles(cfg.wait2, cfg.wait_unit);
		on = duration_to_cycles(cfg.on, cfg.on_unit);
		holdoff = duration_to_cycles(cfg.holdoff, cfg.holdoff_unit);
		duration = duration_to_cycles(cfg.len, cfg.len_unit);
		cycle_len = freq_to_cycles(cfg.freq, cfg.freq_unit);
		ncyc = (duration + cycle_len - 1) / cycle_len;

		if (on > cycle_len)
			on = cycle_len - 1;
		if (on == 0 || (cfg.mode == MODE_STROBE &&
		    (on >= cycle_len || ncyc < 1))) {
			lcd_clear();
			lcd_string("INVALID PARAMETERS");
			_delay_ms(5 * 1000);
			cfg.ready = READY_NO;
			continue;
		}
		off = cycle_len - on;

		off_before_ch2 = 0;
		if (cfg.mode == MODE_ONESHOT) {
			if (on < wait2) {
				off_before_ch2 = 1;
				wait2 -= on;
			} else {
				off_before_ch2 = 0;
				on -= wait2;
			}
		}

		/* Turn the input lights on */
		if (cfg.trigger[0] == TRIG_CHAN_1 ||
		    cfg.trigger[0] == TRIG_CHAN_1_NOT ||
		    cfg.trigger[1] == TRIG_CHAN_1 ||
		    cfg.trigger[1] == TRIG_CHAN_1_NOT)
			PORTB |= (1 << 4);
		if (cfg.trigger[0] == TRIG_CHAN_2 ||
		    cfg.trigger[0] == TRIG_CHAN_2_NOT ||
		    cfg.trigger[1] == TRIG_CHAN_2 ||
		    cfg.trigger[1] == TRIG_CHAN_2_NOT)
			PORTD |= (1 << 7);

		/* Enable interrupt for inputs. */
		PCMSK0 |= (1 << 4) | (1 << 5);
		PCICR |= (1 << PCIE0);

		for (done = 0; !done;) {
			lcd_moveto(0, 0);
			lcd_string("** RUNNING: ");
			lcd_string(cfg.mode == MODE_ONESHOT ?
			    "ONESHOT" : "STROBE");
			lcd_clear_eol();

			/* Prepare output value for strobe */
			switch (cfg.output) {
			case OUT_CH1:
				strobe_out = (1 << 1);
				break;
			case OUT_CH2:
				strobe_out = (1 << 0);
				break;
			case OUT_BOTH:
				strobe_out = (1 << 0) | (1 << 1);
				break;
			default:
				strobe_out = 0;
			}

			/* Prepare timer values */
			prepare_wait(wait1, &wait1_l);
			prepare_wait(wait2, &wait2_l);
			prepare_wait(on, &on_l);
			prepare_wait(holdoff, &holdoff_l);
			prepare_wait(off, &off_l);

#ifdef DEBUG_RUN
			lcd_moveto(0, 0);
			dump_longwait(&holdoff_l);
#endif

			/* XXX fudge these times for comparison, etc. costs */
			/* XXX vary sleep mode for delay? */

			/* Wait for input. */
			event_drain();
			sleep_for_interrupt();

			/* Terminate running state on encoder press */
			if ((PINB & (1 << 2)) == 0)
				break;

#ifdef DEBUG_RUN
			lcd_moveto(0, 0);
			lcd_string("** WAKEUP");
			lcd_clear_eol();
#endif

			/* Evaluate trigger inputs */
			ready1 = evaluate_input(cfg.trigger[0]);
			ready2 = evaluate_input(cfg.trigger[1]);
			switch (cfg.combine) {
			case COMBINE_OR:
				if (!(ready1 || ready2))
					continue;
				break;
			case COMBINE_AND:
				if (!(ready1 && ready2))
					continue;
				break;
			case COMBINE_XOR:
				if (!(ready1 ^ ready2))
					continue;
				break;
			case COMBINE_NONE:
			default:
				if (!ready1)
					continue;
				break;
			}

#ifdef DEBUG_RUN
			lcd_moveto(0, 0);
			lcd_string("** TRIGGERED");
			lcd_clear_eol();
#endif

			/* Wait initial delay. */
			LONG_WAIT(wait1_l);

#ifdef DEBUG_RUN
			lcd_moveto(0, 0);
			lcd_string("** GO");
			lcd_clear_eol();
#endif

			if (cfg.mode == MODE_ONESHOT) {
				/* Output on */
				switch (cfg.output) {
				case OUT_CH1:
					PORTA = (1 << 1);
					LONG_WAIT(on_l);
					PORTA = 0;
					break;
				case OUT_CH2:
					PORTA = (1 << 0);
					LONG_WAIT(on_l);
					PORTA = 0;
					break;
				case OUT_BOTH:
					PORTA = (1 << 1);
					if (off_before_ch2) {
						LONG_WAIT(on_l);
						PORTA = 0;
						LONG_WAIT(wait2_l);
						PORTA = (1 << 0);
						LONG_WAIT(on_l);
						PORTA = 0;
					} else {
						LONG_WAIT(wait2_l);
						PORTA = (1 << 0) | (1 << 1);
						LONG_WAIT(on_l);
						PORTA = (1 << 0);
						LONG_WAIT(wait2_l);
						PORTA = 0;
					}
					break;
				}
				if (cfg.holdoff == -1) {
					done = 1;
					break;
				}
				lcd_moveto(0, 0);
				lcd_string("** HOLDOFF");
				lcd_clear_eol();
				LONG_WAIT(holdoff_l);
			} else {
				for (j = 0; j < ncyc; j++) {
					PORTA = strobe_out;
					LONG_WAIT(on_l);
					PORTA = 0;
					LONG_WAIT(off_l);
				}
				/*
				 * If we are not in manual trigger, then
				 * drop back to editor for explicit re-arming.
				 */
				if (cfg.trigger[0] != TRIG_MANUAL &&
				    cfg.trigger[1] != TRIG_MANUAL) {
					done = 1;
					break;
				}
			}
		}
		/* Run completed - back to edit mode */
		cfg.ready = READY_NO;
	}
	/* NOTREACHED */
}
