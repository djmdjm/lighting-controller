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

/*
 * Define EVENT_LOCAL_DEBUG for main() that tests on build host
 * e.g. gcc -ggdb3 -o /tmp/test_event -D EVENT_LOCAL_DEBUG=1 -Wall event.c
 */

#ifdef EVENT_LOCAL_DEBUG
# define ATOMIC_BLOCK(x) if (1)
#else
# include <avr/io.h>
# include <avr/interrupt.h>
# include <avr/sleep.h>
# include <util/atomic.h>
#endif

#include <stddef.h>
#include <string.h>

#include "event.h"

#define EVENT_QUEUE_LEN	64
struct event {
	uint8_t type;
	uint8_t v[3];
};

/* Ring buffer */
static struct event events[EVENT_QUEUE_LEN];
static unsigned int event_ptr, event_used;
static unsigned int event_overflow, event_maxdepth;

void
event_setup(void)
{
	memset(events, '\0', sizeof(events));
	event_ptr = event_used = event_overflow = event_maxdepth = 0;
}

void
event_drain(void)
{
	event_setup();
}

int
event_enqueue(uint8_t type, uint8_t v1, uint8_t v2, uint8_t v3, int important)
{
	int r = 0;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (event_used >= EVENT_QUEUE_LEN) {
			event_overflow = 1;
			if (important)
				event_used--;
		}
		if (event_used < EVENT_QUEUE_LEN) {
			events[event_ptr].type = type;
			events[event_ptr].v[0] = v1;
			events[event_ptr].v[1] = v2;
			events[event_ptr].v[2] = v3;
			if (++event_ptr >= EVENT_QUEUE_LEN)
				event_ptr = 0;
			event_used++;
			if (event_used > event_maxdepth)
				event_maxdepth = event_used;
			r = 1;
		}
	}
	return r;
}

int
event_dequeue(uint8_t *type, uint8_t *v1, uint8_t *v2, uint8_t *v3)
{
	int r = 0, o;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (event_used > 0) {
			o = (EVENT_QUEUE_LEN + event_ptr) - event_used;
			if (o >= EVENT_QUEUE_LEN)
				o -= EVENT_QUEUE_LEN;
			if (type != NULL)
				*type = events[o].type;
			if (v1 != NULL)
				*v1 = events[o].v[0];
			if (v2 != NULL)
				*v2 = events[o].v[1];
			if (v3 != NULL)
				*v3 = events[o].v[2];
			event_used--;
			r = 1;
		}
	}
	return r;
}

int
event_nqueued(void)
{
	int r;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		r = event_used;
	}
	return r;
}

int
event_maxqueued(void)
{
	int r;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		r = event_maxdepth;
	}
	return r;
}

int
event_queue_overflowed(void)
{
	int r;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		r = event_overflow;
	}
	return r;
}

void
event_reset_overflowed(void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		event_overflow = 0;
	}
}

#ifndef EVENT_LOCAL_DEBUG
void
event_sleep(int sleep_mode, uint8_t *type,
    uint8_t *v1, uint8_t *v2, uint8_t *v3)
{
	set_sleep_mode(sleep_mode);
	for (;;) {
		cli();
		if (event_dequeue(type, v1, v2, v3)) {
			sei();
			return;
		}
		sleep_enable();
		sei();
		sleep_cpu();
		sleep_disable();
	}
	/* NOTREACHED */
}
#endif /* EVENT_LOCAL_DEBUG */

#if EVENT_LOCAL_DEBUG
#include <err.h>
#include <stdio.h>
int
main(void)
{
	int x;
	uint8_t ev_type, ev_v1, ev_v2, ev_v3;

	event_setup();
	event_ptr = 3;

	for (x = 0; x < EVENT_QUEUE_LEN; x++) {
		if (event_enqueue(x, x, x % 3, x / 3, 0) != 1)
			errx(1, "%d: enqueue %d",
			    __LINE__, x);
		if (event_nqueued() != x + 1)
			errx(1, "%d: nqueued %d expected %d",
			    __LINE__, event_nqueued(), x + 1);
		if (event_queue_overflowed())
			errx(1, "%d: overflow %d", __LINE__, x);
	}
	if (event_enqueue(234, 32, 56, 78, 0) != 0)
		errx(1, "%d: full enqueue succeeded", __LINE__);
	if (!event_queue_overflowed())
		errx(1, "%d: !overflow", __LINE__);
	event_reset_overflowed();
	if (event_queue_overflowed())
		errx(1, "%d: overflow %d", __LINE__, x);
	for (x = 0; x < EVENT_QUEUE_LEN; x++) {
		if (event_dequeue(&ev_type, &ev_v1, &ev_v2, &ev_v3) != 1)
			errx(1, "%d: dequeue %d", __LINE__, x);
		if (ev_type != x)
			errx(1, "%d: type %d != expected %d",
			    __LINE__, ev_type, x);
		if (ev_v1 != x)
			errx(1, "%d: v1 %d != expected %d",
			    __LINE__, ev_v1, x);
		if (ev_v2 != x % 3)
			errx(1, "%d: v2 %d != expected %d",
			    __LINE__, ev_v2, x % 3);
		if (ev_v3 != x / 3)
			errx(1, "%d: v2 %d != expected %d",
			    __LINE__, ev_v3, x / 3);
		if (event_nqueued() != EVENT_QUEUE_LEN - x - 1)
			errx(1, "%d: nqueued %d expected %d",
			    __LINE__, event_nqueued(),
			    x + 1);
	}
	if (event_dequeue(&ev_type, &ev_v1, &ev_v2, &ev_v3) != 0)
		errx(1, "%d: dequeue empty", __LINE__);
	for (x = 0; x < EVENT_QUEUE_LEN; x++) {
		if (event_enqueue(x, x, x % 5, x / 5, 0) != 1)
			errx(1, "%d: enqueue %d", __LINE__, x);
	}
	if (event_nqueued() != EVENT_QUEUE_LEN)
		errx(1, "%d: nqueued %d expected %d",
		    __LINE__, event_nqueued(), EVENT_QUEUE_LEN);
	if (event_enqueue(123, 210, 45, 67, 1) != 1)
		errx(1, "%d: enqueue important failed", __LINE__);
	if (!event_queue_overflowed())
		errx(1, "%d: !overflow", __LINE__);
	if (event_nqueued() != EVENT_QUEUE_LEN)
		errx(1, "%d: nqueued %d expected %d",
		    __LINE__, event_nqueued(), EVENT_QUEUE_LEN);
	for (x = 1; x < EVENT_QUEUE_LEN; x++) {
		if (event_dequeue(&ev_type, &ev_v1, &ev_v2, &ev_v3) != 1)
			errx(1, "%d: dequeue %d", __LINE__, x);
		if (ev_type != x)
			errx(1, "%d: type %d != expected %d",
			    __LINE__, ev_type, x);
		if (ev_v1 != x)
			errx(1, "%d: v1 %d != expected %d",
			    __LINE__, ev_v1, x);
		if (ev_v2 != x % 5)
			errx(1, "%d: v2 %d != expected %d",
			    __LINE__, ev_v2, x << 1);
		if (ev_v3 != x / 5)
			errx(1, "%d: v3 %d != expected %d",
			    __LINE__, ev_v3, x << 1);
		if (event_nqueued() != EVENT_QUEUE_LEN - x)
			errx(1, "%d: nqueued %d expected %d",
			    __LINE__, event_nqueued(), x);
	}
	if (event_dequeue(&ev_type, &ev_v1, &ev_v2, &ev_v3) != 1)
		errx(1, "%d: dequeue %d", __LINE__, x);
	if (ev_type != 123)
		errx(1, "%d: type %d != expected %d", __LINE__, ev_type, 123);
	if (ev_v1 != 210)
		errx(1, "%d: v1 %d != expected %d", __LINE__, ev_v1, 210);
	if (ev_v2 != 45)
		errx(1, "%d: v2 %d != expected %d", __LINE__, ev_v2, 4567);
	if (ev_v3 != 67)
		errx(1, "%d: v3 %d != expected %d", __LINE__, ev_v3, 67);
	if (event_nqueued() != 0)
		errx(1, "%d: nqueued %d expected %d",
		    __LINE__, event_nqueued(), 0);
	printf("OK\n");
	return 0;
}
#endif
