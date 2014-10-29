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

/* UI for lighting controller */

#ifndef _UI_H
#define _UI_H

#define MODE_ONESHOT	0
#define MODE_STROBE	1
#define MODE_MAX	2

#define READY_NO	0
#define READY_YES	1
#define READY_MAX	2

#define TRIG_NONE	0
#define TRIG_CHAN_1	1
#define TRIG_CHAN_1_NOT	2
#define TRIG_CHAN_2	3
#define TRIG_CHAN_2_NOT	4
#define TRIG_MANUAL	5
#define TRIG_MAX	6

#define OUT_CH1		0
#define OUT_CH2		1
#define OUT_BOTH	2
#define OUT_MAX		3
/* XXX alternate (for stobe) and inverted (for camera control) modes */

#define COMBINE_NONE	0
#define COMBINE_OR	1
#define COMBINE_AND	2
#define COMBINE_XOR	3
#define COMBINE_MAX	4
/* XXX "then" operator. E.g. "input 1 then input 2" */

#define DUR_MICROSEC	0
#define DUR_MILLISEC	1
#define DUR_SEC		2
#define DUR_MAX		3

#define RATE_MHZ	0
#define RATE_KHZ	1
#define RATE_HZ		2
#define RATE_MILLI_HZ	3
#define RATE_MAX	4

/* Main configuration */
struct config {
	int mode;
	int ready;
	int trigger[2];
	int combine;
	int output;
	int wait, wait_unit;
	int wait2, wait2_unit;
	int on, on_unit;
	/* Strobe */
	int freq, freq_unit;
	int len, len_unit;
	/* Oneshot */
	int holdoff, holdoff_unit;
};

/* Display configuration editor. Returns when user selects Ready */
void config_edit(void);

/* Reset configuration to default. */
void reset_config(void);

/* The actual configuration. NB. must be initialised using reset_config() */
extern struct config cfg;

#endif /* _UI_H */

