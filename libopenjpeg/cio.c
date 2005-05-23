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

#include "cio.h"
#include <setjmp.h>
#include <memory.h>

static unsigned char *cio_start;	/* pointer to the start of the stream */
static unsigned char *cio_end;	/* pointer to the end of the stream */
static unsigned char *cio_bp;	/* pointer to the present position */

extern jmp_buf j2k_error;

/* 
 * Number of bytes written.
 */
int cio_numbytes()
{
  return cio_bp - cio_start;
}

/*
 * Get position in byte stream.
 */
int cio_tell()
{
  return cio_bp - cio_start;
}

/*
 * Set position in byte stream.
 *
 * pos : position, in number of bytes, from the beginning of the stream
 */
void cio_seek(int pos)
{
  cio_bp = cio_start + pos;
}

/*
 * Number of bytes left before the end of the stream.
 */
int cio_numbytesleft()
{
  return cio_end - cio_bp;
}

/*
 * Get pointer to the current position in the stream.
 */
unsigned char *cio_getbp()
{
  return cio_bp;
}

/* 
 * Initialize byte IO
 *
 * bp  : destination/source stream
 * len : length of the stream
 */
void cio_init(unsigned char *bp, int len)
{
  cio_start = bp;
  cio_end = bp + len;
  cio_bp = bp;
}

/*
 * Write a byte.
 */
void cio_byteout(unsigned char v)
{
  if (cio_bp >= cio_end)
    longjmp(j2k_error, 1);
  *cio_bp++ = v;

}

/*
 * Read a byte.
 */
unsigned char cio_bytein()
{
  if (cio_bp >= cio_end)
    longjmp(j2k_error, 1);
  return *cio_bp++;
}

/*
 * Write some bytes.
 *
 * v : value to write
 * n : number of bytes to write
 */
void cio_write(unsigned int v, int n)
{
  int i;
  for (i = n - 1; i >= 0; i--) {
    cio_byteout((unsigned char) ((v >> (i << 3)) & 0xff));
  }
}

/*
 * Read some bytes.
 *
 * n : number of bytes to read
 *
 * return : value of the n bytes read
 */
unsigned int cio_read(int n)
{
  int i;
  unsigned int v;
  v = 0;
  for (i = n - 1; i >= 0; i--) {
    v += cio_bytein() << (i << 3);
  }
  return v;
}

/* 
 * Skip some bytes.
 *
 * n : number of bytes to skip
 */
void cio_skip(int n)
{
  cio_bp += n;
}

/* 
 * Read n bytes, copy to buffer
 *
 * n : number of bytes to transfer
 */
void cio_read_to_buf(unsigned char* dest_buf, int n)/* Glenn adds */
{
  if (cio_bp + n > cio_end)
    longjmp(j2k_error, 1);
  memcpy(cio_bp, dest_buf, n);
  cio_bp += n;
}

/* 
 * Write n bytes, copy from buffer
 *
 * n : number of bytes to transfer
 */
void cio_write_from_buf(unsigned char* src_buf, int n)/* Glenn adds */
{
  if (cio_bp + n > cio_end)
    longjmp(j2k_error, 1);
  memcpy(src_buf, cio_bp, n);
  cio_bp += n;
}