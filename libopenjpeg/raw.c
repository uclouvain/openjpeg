/*
 * Copyright (c) 2002-2003, Antonin Descampe
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "raw.h"


unsigned char raw_c;
unsigned int raw_ct, raw_lenmax, raw_len;
unsigned char *raw_bp;
unsigned char *raw_start;
unsigned char *raw_end;

/* <summary> */
/* Return the number of bytes already encoded. */
/* </summary> */
int raw_numbytes()
{
	return raw_bp - raw_start;
}

/* <summary> */
/* Initialize raw-encoder. */
/* </summary> */
/* <param name="bp">Output buffer.</param> */
void raw_init_enc(unsigned char *bp)
{
	raw_bp = bp - 1;
	raw_c = 0;
	raw_ct = 7;
	raw_start = bp;
}

/* <summary> */
/* Encode a symbol using the RAW-coder. */
/* </summary> */
/* <param name="d"> The symbol to be encoded (0 or 1).</param> */
void raw_encode(int d)
{
	/*  raw_c+=d; */

	raw_ct--;
	raw_c += (d << raw_ct);

	if (raw_ct == 0) {
		raw_bp++;
		*raw_bp = raw_c;
		raw_ct = 7;
		if (raw_c == 0xff) {
			raw_ct = 6;
		}
		raw_c = 0;
	}
	/*else 
	   {
	   raw_ct--;
	   raw_c<<=1;
	   } */
}

/* <summary> */
/* Flush encoded data. */
/* </summary> */
void raw_flush()
{
	char first = 1;
	int prev = 1;
	while (raw_ct != 7) {
		raw_encode(first ? 0 : !(prev));
		prev = first ? 0 : !(prev);
		first = 0;
	}
}

/* <summary> */
/* Initialize raw-decoder. */
/* </summary> */
void raw_init_dec(unsigned char *bp, int len)
{
	raw_start = bp;
	raw_lenmax = len;
	raw_len = 0;
	raw_c = 0;
	raw_ct = 0;
}

/* <summary> */
/* Decode a symbol using raw-decoder. Cfr p.506 TAUBMAN */
/* </summary> */
int raw_decode()
{
	int d;
	if (raw_ct == 0) {
		raw_ct = 8;
		if (raw_len == raw_lenmax)
			raw_c = 0xff;
		else {
			if (raw_c == 0xff)
				raw_ct = 7;
			raw_c = *(raw_start + raw_len);
			raw_len++;
		}
	}
	raw_ct--;
	d = (raw_c >> raw_ct) & 0x01;
	return d;
}
