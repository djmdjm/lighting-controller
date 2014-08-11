/*
 * Copyright (c) 2014 Damien Miller <djm@mindrot.org>
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

#include "event.h"
#include "event-types.h"
#include "midi.h"

static uint16_t channel_bitmap = 0;
static uint8_t running_status = 0;
static uint8_t data[2];
static uint8_t ndata = 0;

#ifdef MIDI_LCD_DEBUG
#include "lcd.h"
#include "num_format.h"
# define MIDI_LCD_DUMP() do { \
	int i; \
	lcd_clear(); \
	lcd_moveto(0, 0); \
	lcd_string("r="); lcd_string(ntoh(r, 0)); \
	lcd_string(" st="); lcd_string(ntoh(running_status, 0)); \
	lcd_string(" need="); lcd_string(ntoh(need, 0)); \
	lcd_moveto(0, 1); \
	lcd_string(" n="); lcd_string(ntoh(ndata, 0)); \
	lcd_string(" d="); \
	for (i = 0; i < ndata; i++) \
		lcd_string(ntoh(data[i], 0)); \
	} while (0)
#endif

void
midi_init(uint16_t channel_mask)
{
	channel_bitmap = channel_mask;
}

void
midi_in(uint8_t r)
{
	uint8_t need, chan, status;

	/* handle single-byte status messages */
	switch (r) {
	case 0xf6: /* tune request */
		/* Not implemented */
		return;
	case 0xF7: /* end of sysex */
		/* We don't handle any sysex messages yet */
		running_status = 0;
		return;
	case 0xf8: /* timing clock */
		event_enqueue(EV_MIDI_CLOCK, 3, 0, 0, 0);
		return;
	case 0xfa: /* song start */
		event_enqueue(EV_MIDI_CLOCK, 0, 0, 0, 0);
		return;
	case 0xfb: /* song continue */
		event_enqueue(EV_MIDI_CLOCK, 2, 0, 0, 0);
		return;
	case 0xfc: /* song stop */
		event_enqueue(EV_MIDI_CLOCK, 1, 0, 0, 1);
		return;
	case 0xfe: /* active sensing */
		/* XXX implement all-notes-off on active sensing watchdog? */
		/* XXX needs a timer system */
		return;
	case 0xff: /* system reset */
		event_enqueue(EV_MIDI_RESET, 0, 0, 0, 1);
		return;
	}

	/* handle multi-byte messages */

	/* Is it a status byte? */
	if ((r & 0x80) != 0) {
		running_status = r;
		ndata = 0;
		return;
	}

	if (ndata > sizeof(data))
		return;
	data[ndata++] = r;

	status = running_status & 0xf0;
	chan = running_status & 0x0f;

	/* How many bytes do we need? */
	switch (status) {
	case 0x80: /* note off */
	case 0x90: /* note on */
	case 0xa0: /* poly aftertouch */
	case 0xe0: /* pitch bend */
		need = 2;
		break;
	case 0xc0: /* program change */
	case 0xd0: /* channel aftertouch */
		need = 1;
		break;
	case 0xb0: /* control change */
		if (ndata == 0) {
			need = 0;
			break;
		}
		/*
		 * Accept most of the 8 bit continuous controllers.
		 * Also all notes/sound off and controller reset.
		 */
		if (data[0] < 0x20 || (data[0] >= 0x46 && data[0] < 0x54))
			need = 2;
		else if (data[0] == 0x78 || data[0] == 0x79 || data[0] == 0x7b)
			need = 1;
		else
			need = 0;
		break;
	default:
		need = 0;
		break;
	}

	if (need == 0 || ndata < need)
		return; /* incomplete or unsupported */
	if ((channel_bitmap & (1 << chan)) == 0) {
		/* not interested */
		ndata = 0;
		return;
	}

	/* Parse the message, enqueue it as an event */
	switch (status) {
	case 0x80: /* note off */
	case 0x90: /* note on */
		/* Special case: velocity == 0 means note off */
		if (data[1] == 0)
			status = 0x80;
		event_enqueue(
		    (status == 0x80) ? EV_MIDI_NOTE_OFF : EV_MIDI_NOTE_ON,
		    chan, data[0], data[1], 1);
		break;
	case 0xe0: /* pitch bend */
		event_enqueue(EV_MIDI_PITCH_BEND, chan, data[1], data[0], 0);
		break;
	case 0xd0: /* channel aftertouch */
		event_enqueue(EV_MIDI_ATOUCH_CHAN, chan, data[0], 0, 0);
		break;
	case 0xb0: /* control change */
		switch (data[0]) {
		case 0x78: /* all sound off */
		case 0x7b: /* all notes off */
			event_enqueue(EV_MIDI_ALL_OFF, chan, 0, 0, 1);
			break;
		case 0x79: /* reset controllers */
			event_enqueue(EV_MIDI_CONTROL_RESET, chan, 0, 0, 0);
			break;
		default:   /* one of the supported continuous controllers */
			event_enqueue(EV_MIDI_CONTROLLER, chan,
			    data[0], data[1], 0);
		}
		break;
	default:
	case 0xc0: /* program change */
	case 0xa0: /* poly aftertouch */
		/* Unimplemented */
		return;
	}
	ndata = 0;
}

/* XXX add main() under a flag to unit test */
