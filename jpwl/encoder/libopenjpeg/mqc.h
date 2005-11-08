/*
 * Copyright (c) 2001-2002, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
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

#ifndef __MQC_H
#define __MQC_H

/*
 * Return the number of bytes written/read since initialisation
 */
int mqc_numbytes();

/*
 * Reset the states of all the context of the coder/decoder
 * (each context is set to a state where 0 and 1 are more or less equiprobable)
 */
void mqc_resetstates();

/*
 * Set the state of a particular context
 * ctxno: number that identifies the context
 * msb: the MSB of the new state of the context
 * prob: number that identifies the probability of the symbols for the new state of the context
 */
void mqc_setstate(int ctxno, int msb, int prob);

/*
 * Initialize the encoder
 * bp: pointer to the start of the buffer where the bytes will be written
 */
void mqc_init_enc(unsigned char *bp);

/*
 * Set the current context used for coding/decoding
 * ctxno: number that identifies the context
 */
void mqc_setcurctx(int ctxno);

/*
 * Encode a bit
 * d: bit to encode (0 or 1)
 */
void mqc_encode(int d);

/*
 * Flush the encoder, so that all remaining data is written
 */
void mqc_flush();

/*
 * BYPASS mode switch
 */
void mqc_bypass_init_enc();

/*
 * BYPASS mode switch
 */
void mqc_bypass_enc(int d);

/*
 * BYPASS mode switch
 */
int mqc_bypass_flush_enc();

/*
 * RESET mode switch
 */
void mqc_reset_enc();

/*
 * RESTART mode switch (TERMALL)
 */
int mqc_restart_enc();

/*
 * RESTART mode switch (TERMALL)
 */
void mqc_restart_init_enc();

/*
 * ERTERM mode switch (PTERM)
 */
void mqc_erterm_enc();

/*
 * SEGMARK mode switch (SEGSYM)
 */
void mqc_segmark_enc();


/*
 * Initialize the decoder
 * bp: pointer to the start of the buffer from which the bytes will be read
 * len: length of the input buffer
 */
void mqc_init_dec(unsigned char *bp, int len);

/*
 * Decode a bit (returns 0 or 1)
 */
int mqc_decode();

#endif
