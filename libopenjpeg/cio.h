/*
 * Copyright (c) 2001-2002, David Janssens
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

#ifndef __CIO_H
#define __CIO_H

/* 
 * Number of bytes written.
 *
 * returns number of bytes written
 */
int cio_numbytes();

/*
 * Get position in byte stream.
 *
 * return position in bytes
 */
int cio_tell();

/*
 * Set position in byte stream.
 *
 * pos : position, in number of bytes, from the beginning of the stream
 */
void cio_seek(int pos);

/*
 * Number of bytes left before the end of the stream.
 *
 * Returns the number of bytes before the end of the stream 
 */
int cio_numbytesleft();

/*
 * Get pointer to the current position in the stream.
 *
 * return : pointer to the position
 */
unsigned char *cio_getbp();

/* 
 * Initialize byte IO
 *
 * bp  : destination/source stream
 * len : length of the stream
 */
void cio_init(unsigned char *bp, int len);

/*
 * Write some bytes.
 *
 * v : value to write
 * n : number of bytes to write
 */
void cio_write(unsigned int v, int n);

/*
 * Read some bytes.
 *
 * n : number of bytes to read
 *
 * return : value of the n bytes read
 */
unsigned int cio_read(int n);

/* 
 * Skip some bytes.
 *
 * n : number of bytes to skip
 */
void cio_skip(int n);

/*
 * Read n bytes, copy to buffer
 */
void cio_read_to_buf(unsigned char* buf, int n);/* Glenn Pearson adds */

/*
 * Write n bytes, copy from buffer
 */
void cio_write_from_buf(unsigned char* buf, int n);/* Glenn Pearson adds */

#endif
