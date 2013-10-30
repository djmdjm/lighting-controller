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

#include <string.h>

#include "num_format.h"

const char *
ntod(int n)
{
	static char ret[sizeof(int) * 4 + 2];
	int neg = 0, i = sizeof(ret) - 2;

	if (n == 0)
		return "0";
	if (n < 0) {
		neg = 1;
		n = -n;
	}
	while (n > 0) {
		ret[i--] = "0123456789"[n % 10];
		n /= 10;
	}
	if (neg)
		ret[i--] = '-';
	ret[sizeof(ret) - 1] = '\0';
	return &(ret[i + 1]);
}

const char *
ntoh(unsigned int n, int preamble)
{
	static char ret[sizeof(unsigned int) * 2 + 2 + 1];
	int i = sizeof(ret) - 2;

	if (n == 0)
		return "0";
	while (n > 0) {
		ret[i--] = "0123456789abcdef"[n & 0xf];
		n >>= 4;
	}
	if (preamble) {
		ret[i--] = 'x';
		ret[i--] = '0';
	}
	ret[sizeof(ret) - 1] = '\0';
	return &(ret[i + 1]);
}

char *
rjustify(const char *s, char *buf, unsigned int width)
{
	size_t l = strlen(s);

	if (width > 0) {
		if (l > width - 1)
			l = width - 1;
		memset(buf, ' ', width - 1 - l);
		memcpy(buf + width - 1 - l, s, l);
		buf[width - 1] = '\0';
	}
	return buf;
}

