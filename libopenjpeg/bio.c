/*
 * Copyright (c) 2001-2002, David Janssens
 * Copyright (c) 2003, Yannick Verschueren
 * Copyright (c) 2003,  Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
 *
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

#include "bio.h"
#include <stdio.h>
#include <setjmp.h>

static unsigned char *bio_start;	/* pointer to the start of the buffer */
static unsigned char *bio_end;	/* pointer to the end of the buffer */
static unsigned char *bio_bp;	/* pointer to the present position in the buffer */
static unsigned int bio_buf;	/* temporary place where each byte is read or written */
static int bio_ct;		/* coder : number of bits free to write // decoder : number of bits read */

extern jmp_buf j2k_error;

/*
 * Number of bytes written.
 */
int bio_numbytes()
{
    return bio_bp - bio_start;
}

/*
 * Init encoder.
 *
 * bp  : Output buffer
 * len : Output buffer length 
 */
void bio_init_enc(unsigned char *bp, int len)
{
    bio_start = bp;
    bio_end = bp + len;
    bio_bp = bp;
    bio_buf = 0;
    bio_ct = 8;
}

/*
 * Init decoder.
 * 
 * bp  : Input buffer
 * len : Input buffer length 
 */
void bio_init_dec(unsigned char *bp, int len)
{
    bio_start = bp;
    bio_end = bp + len;
    bio_bp = bp;
    bio_buf = 0;
    bio_ct = 0;
}

/*
 * Write byte. --> function modified to eliminate longjmp !!! 
 *
 */
int bio_byteout()
{
    bio_buf = (bio_buf << 8) & 0xffff;
    bio_ct = bio_buf == 0xff00 ? 7 : 8;
    if (bio_bp >= bio_end)
	return 1;
    *bio_bp++ = bio_buf >> 8;
    return 0;
}

/*
 * Read byte. --> function modified to eliminate longjmp !!
 *
 */
int bio_bytein()
{
    bio_buf = (bio_buf << 8) & 0xffff;
    bio_ct = bio_buf == 0xff00 ? 7 : 8;
    if (bio_bp >= bio_end)
	return 1;
    bio_buf |= *bio_bp++;
    return 0;
}

/*
 * Write bit.
 *
 * b : Bit to write (0 or 1)
 */
void bio_putbit(int b)
{
    if (bio_ct == 0) {
	bio_byteout();
    }
    bio_ct--;
    bio_buf |= b << bio_ct;
}

/*
 * Read bit.
 *
 */
int bio_getbit()
{
    if (bio_ct == 0) {
	bio_bytein();
    }
    bio_ct--;
    return (bio_buf >> bio_ct) & 1;
}

/*
 * Write bits.
 *
 * v : Value of bits
 * n : Number of bits to write 
 */
void bio_write(int v, int n)
{
    int i;
    for (i = n - 1; i >= 0; i--) {
	bio_putbit((v >> i) & 1);
    }
}

/*
 * Read bits.
 * 
 * n : Number of bits to read
 */
int bio_read(int n)
{
    int i, v;
    v = 0;
    for (i = n - 1; i >= 0; i--) {
	v += bio_getbit() << i;
    }
    return v;
}

/*
 * Flush bits. Modified to eliminate longjmp !!
 *
 */
int bio_flush()
{
    bio_ct = 0;
    if (bio_byteout())
	return 1;
    if (bio_ct == 7) {
	bio_ct = 0;

	if (bio_byteout())
	    return 1;
    }
    return 0;
}

/*
 * Passes the ending bits (coming from flushing)
 */
int bio_inalign()
{
    bio_ct = 0;
    if ((bio_buf & 0xff) == 0xff) {
	if (bio_bytein())
	    return 1;
	bio_ct = 0;
    }
    return 0;
}
