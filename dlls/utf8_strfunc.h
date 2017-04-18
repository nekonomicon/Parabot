/*-
 * Copyright 2013 Garrett D'Amore <garrett@damore.org>
 * Copyright 2011 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 2002-2004 Tim J. Robbins
 * All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 * All rights reserved.
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <wctype.h>
#include <locale.h>

static size_t UTF8_mbtowc( wchar_t *pwc, const char *s )
{
	int ch, i, mask, want;
	wchar_t lbound, wch;

	if (s == NULL) {
		s = "";
		want = 1;
		pwc = NULL;
	}

	/*
	 * Determine the number of octets that make up this character
	 * from the first octet, and a mask that extracts the
	 * interesting bits of the first octet. We already know
	 * the character is at least two bytes long.
	 *
	 * We also specify a lower bound for the character code to
	 * detect redundant, non-"shortest form" encodings. For
	 * example, the sequence C0 80 is _not_ a legal representation
	 * of the null character. This enforces a 1-to-1 mapping
	 * between character codes and their multibyte representations.
	 */
	ch = (unsigned char)*s;
	if ((ch & 0x80) == 0) {
		/* Fast path for plain ASCII characters. */
		if (pwc != NULL)
			*pwc = ch;
		return (ch != '\0' ? 1 : 0);
	}
	if ((ch & 0xe0) == 0xc0) {
		mask = 0x1f;
		want = 2;
		lbound = 0x80;
	} else if ((ch & 0xf0) == 0xe0) {
		mask = 0x0f;
		want = 3;
		lbound = 0x800;
	} else if ((ch & 0xf8) == 0xf0) {
		mask = 0x07;
		want = 4;
		lbound = 0x10000;
	} else {
		/*
		 * Malformed input; input is not UTF-8.
		 */
		return ((size_t)-1);
	}

	/*
	 * Decode the octet sequence representing the character in chunks
	 * of 6 bits, most significant first.
	 */
	wch = (unsigned char)*s++ & mask;

	for (i = 1; i < want; i++) {
		if ((*s & 0xc0) != 0x80) {
			/*
			 * Malformed input; bad characters in the middle
			 * of a character.
			 */
			return ((size_t)-1);
		}
		wch <<= 6;
		wch |= *s++ & 0x3f;
	}
	if (i < want) {
		/* Incomplete multibyte sequence. */
		return ((size_t)-2);
	}
	if (wch < lbound) {
		/*
		 * Malformed input; redundant encoding.
		 */
		return ((size_t)-1);
	}
	if ((wch >= 0xd800 && wch <= 0xdfff) || wch > 0x10ffff) {
		/*
		 * Malformed input; invalid code points.
		 */
		return ((size_t)-1);
	}
	if (pwc != NULL)
		*pwc = wch;
	return (wch == L'\0' ? 0 : want);
}

static size_t UTF8_wctomb( char *s, wchar_t wc )
{
	unsigned char lead;
	int i, len;

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);

	/*
	 * Determine the number of octets needed to represent this character.
	 * We always output the shortest sequence possible. Also specify the
	 * first few bits of the first octet, which contains the information
	 * about the sequence length.
	 */
	if ((wc & ~0x7f) == 0) {
		/* Fast path for plain ASCII characters. */
		*s = (char)wc;
		return (1);
	} else if ((wc & ~0x7ff) == 0) {
		lead = 0xc0;
		len = 2;
	} else if ((wc & ~0xffff) == 0) {
		if (wc >= 0xd800 && wc <= 0xdfff) {
			return ((size_t)-1);
		}
		lead = 0xe0;
		len = 3;
	} else if (wc >= 0 && wc <= 0x10ffff) {
		lead = 0xf0;
		len = 4;
	} else {
		return ((size_t)-1);
	}

	/*
	 * Output the octets representing the character in chunks
	 * of 6 bits, least significant last. The first octet is
	 * a special case because it contains the sequence length
	 * information.
	 */
	for( i = len - 1; i > 0; i-- )
	{
		s[i] = (wc & 0x3f) | 0x80;
		wc >>= 6;
	}
	*s = ( wc & 0xff ) | lead;

	return len;
}

static void UTF8_strupr( char* s )
{
	wchar_t wc;
	size_t len;
	setlocale( LC_ALL, "" );
	while( *s )
	{
		UTF8_mbtowc( &wc, s );
		wc = towupper( wc );
		len = UTF8_wctomb( s, wc );
		s += len;
        }
	setlocale( LC_ALL, "C" );
}
