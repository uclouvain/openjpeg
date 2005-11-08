/*
 * Copyright 2004-2005 Andrea Betti and Michele Massarelli
 * Copyright 2004-2005 DIEI, University of Perugia
 *
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


/* questa è un' estensione della libreria dell'openjpeg in grado di implementare
 * la parte 11 dello standard JPEG2000 ossia il JPWL
 */

#ifndef __JPW_H
#define __JPW_H


typedef struct {
	unsigned int JPWL_on;		//indica se usare le funzionalità JPWL e quindi l'EPC nel main header
	unsigned int EPB_on;
	unsigned int ESD_on;
	unsigned int RED_on;
	unsigned int UEP_on;
	unsigned int info_tech_on;
	unsigned long pepb[6];
	unsigned char pesd;
	unsigned char pred;
} JPWL_cp_t;


// inizializza la struttura contenente le info riguardanti l'uso dei marker jpwl

void jpwl_cp_init(JPWL_cp_t *jpwl_cp);

// qui iniziano le funzioni necessarie per la scrittura dei marker

void jpwl_write_EPC();

void jpwl_write_ESD(char Pesd, int notil);

void jpwl_write_RED();

void JPWL_write_EPB(char *buf, unsigned long LDPepb, unsigned long Pepb, unsigned char Depb);

int JPWL_len_EPB(unsigned int LDPepb, unsigned int Pepb, unsigned char Depb);

int pianifica_epb(unsigned int LDPepb, unsigned int Pepb, int *n_epb );

void JPWL_write_Pepbs(long *pepbs, int num_epb);

void get_jpwl_cp(j2k_cp_t *cp);

int uep(int notil, char *bs, char *header, int len_header);

int uep_lay(int notil, char *bs, char *header, int len_header);

void jpwl_write_EPC_fin(unsigned long CL, long *pepbs, int num_epb);

void correggi_esd(int offset, int len_esd, int Pesd);

int jpwl_encode(char *input, char * output, unsigned long CL);

/* qui andrò ad inserire le funzioni necessarie per il crc oppure la codifica RS
 * e le funzioni per le tecniche informative aggiuntive
 */

void ResetCRC();

void UpdateCRC16(char x);

void UpdateCRC32(char x);

char reflectByte(char inbyte);

void reflectCRC32();


//int crc_ccitt(char *buffer, int len);

short int d2pfp(double in);

double pfp2d(short int in);



#endif