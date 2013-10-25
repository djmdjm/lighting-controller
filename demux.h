#ifndef DEMUX_H
#define DEMUX_H

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

/* Convenience routines for CD4145BE 4-16 line demultiplexer */

#define DEMUX_DDR	DDRF
#define DEMUX_PORT	PORTF
#define DEMUX_D0	2
#define DEMUX_D1	3
#define DEMUX_D2	4
#define DEMUX_D3	5
#define DEMUX_INHIBIT	6
#define DEMUX_STROBE	7

void demux_set_line(int val);
void demux_inhibit(void);
void demux_setup(void);

#endif /* DEMUX_H */
