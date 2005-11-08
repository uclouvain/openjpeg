/*
 * Copyright (c) 2002-2003, Antonin Descampe
 * Copyright (c) 2002-2003,  Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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
	
#ifndef __RAW_H
#define __RAW_H
	
/*
 * Return the number of bytes written/read since initialisation
 */ 
int raw_numbytes();


/*
 * Initialize the encoder
 * bp: pointer to the start of the buffer where the bytes will be written
 */ 
void raw_init_enc(unsigned char *bp);


/*
 * Encode a bit
 * d: bit to encode (0 or 1)
 */ 
void raw_encode(int d);


/*
 * Flush the encoder, so that all remaining data is written
 */ 
void raw_flush();


/*
 * Initialize the decoder
 * bp: pointer to the start of the buffer from which the bytes will be read
 * len: length of the input buffer
 */ 
void raw_init_dec(unsigned char *bp, int len);


/*
 * Decode a bit (returns 0 or 1)
 */ 
int raw_decode();


#endif	/* 
 */
