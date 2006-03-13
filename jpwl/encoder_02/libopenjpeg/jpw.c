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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <conio.h>
#include <math.h>
#include "j2k.h"
#include "cio.h"
#include "jpw.h"
#include "rs.h"

// JPWL MARKERS 
#define JPWL_MS_EPC 0xff97
#define JPWL_MS_EPB 0xff96
#define JPWL_MS_ESD 0xff98
#define JPWL_MS_RED 0xff99


//stati del codec
#define J2K_STATE_MHSOC 0x0001
#define J2K_STATE_MHSIZ 0x0002
#define J2K_STATE_MH 0x0004
#define J2K_STATE_TPHSOT 0x0008
#define J2K_STATE_TPH 0x0010
#define J2K_STATE_MT 0x0020
#define J2K_STATE_NEOC 0x0040
#define J2K_STATE_MHEPB 0x0080
#define J2K_STATE_TPHEPB 0x0100

//carica la struttura relativa ai parametri JPWL
extern JPWL_cp_t jpwl_cp;
extern int j2k_state;
extern j2k_cp_t *j2k_cp;
extern info_image info_IM;

static long crcSum;
static long pepbs[1000];
static int cont_epb=0;
static int scavalcato=0;

static int CrcT16[256] =
{0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};

static long CrcT32[256] = {0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4};


/* inizializza i parametri relativi al JPWL *
 * disabilitando le funzionalità            */

void jpwl_cp_init(JPWL_cp_t *jpwl_cp){
	jpwl_cp->JPWL_on = 0;
	jpwl_cp->EPB_on = 0;
	jpwl_cp->ESD_on = 0;
	jpwl_cp->RED_on = 0;
	jpwl_cp->info_tech_on = 0;
}

/****************************************************************************************
 *																						*
 *								    Marker JPWL											*
 *																						*
 ****************************************************************************************/

/* Scrive il marker segment EPC
   version 0.1: in questa prima fase questo andrà subito dopo il SIZ */

void jpwl_write_EPC(){
	int lenp, len, i;
	char c, P_EPC = 0;
	cio_write(JPWL_MS_EPC, 2);
	lenp = cio_tell();
	cio_skip(2);
	// calcola il crc
	cio_skip(2);
	// per adesso lascio uno spazio vuoto (16 bit)
	// CL settore di 32bit che indica la lungheza della codestream:
	// per ora metto tutto uguale a zero (info non disponibile)
	cio_write(0, 4);
	// P_EPC indica se nella codestream saranno usati gli altri 
	// marker segment oppure tecniche informative aggiuntive 
	if (jpwl_cp.ESD_on)
		P_EPC += 0x10;
	if (jpwl_cp.RED_on)
		P_EPC += 0x20;
	if (jpwl_cp.EPB_on)
		P_EPC += 0x40;
	if (jpwl_cp.info_tech_on)
		P_EPC += 0x80;
	cio_write(P_EPC, 1);
	//scrivo i Pepb

	if(jpwl_cp.EPB_on)
		JPWL_write_Pepbs(NULL,0);




	/* Al momento non esistono tecniche informative aggiuntive per richiamarle
	 * opereremo in questo modo:
	 * 0) dovrò inserire nella struttura jpwl_cp_t un campo contenente un puntatore
	 *    ad un array che conterrà gli ID delle tecniche informative da usare;
	 * 1) conrollo il campo info_tech_on;
	 * 2) eseguo delle funzioni da inserire nel file jpw.c e jpw.h relative alle 
	 *    tecniche informative desiderate; tali funzioni dovranno preoccuparsi di
	 *    creare i campi corretti nel marker segment.
	 *  if( jpwl_cp->info_tech_on){
	 *      qui inserirò i comandi desiderati
	 *  }
	 */
    len = cio_tell() - lenp;
	cio_seek(lenp);
	cio_write(len, 2);
	// calcola il crc
	cio_skip(-4);
	ResetCRC();
	for (i=0; i<4; i++){
		c = cio_read(1);
		UpdateCRC16(c);	
	}
	cio_skip(2);
	for (i=6; i<(len + 2); i++){
		c = cio_read(1);
		UpdateCRC16(c);	
	}
	cio_seek(lenp + 2);
	cio_write(crcSum, 2);
	cio_seek(lenp + len);

}


void jpwl_write_EPC_fin(unsigned long CL, long *pepbs, int num_epb){
	int lenp, len, i;
	unsigned long len_codestr;
	char c, P_EPC = 0;
	cio_write(JPWL_MS_EPC, 2);
	lenp = cio_tell();
	cio_skip(2);
	// calcola il crc
	cio_skip(2);
	// per adesso lascio uno spazio vuoto (16 bit)
	// CL settore di 32bit che indica la lungheza della codestream:
	// per ora metto tutto uguale a zero (info non disponibile)
	cio_write(0, 4);
	// P_EPC indica se nella codestream saranno usati gli altri 
	// marker segment oppure tecniche informative aggiuntive 
	if (jpwl_cp.ESD_on)
		P_EPC += 0x10;
	if (jpwl_cp.RED_on)
		P_EPC += 0x20;
	if (jpwl_cp.EPB_on)
		P_EPC += 0x40;
	if (jpwl_cp.info_tech_on)
		P_EPC += 0x80;
	cio_write(P_EPC, 1);
	//scrivo i Pepb

	if(jpwl_cp.EPB_on)
		JPWL_write_Pepbs(pepbs, num_epb);




	/* Al momento non esistono tecniche informative aggiuntive per richiamarle
	 * opereremo in questo modo:
	 * 0) dovrò inserire nella struttura jpwl_cp_t un campo contenente un puntatore
	 *    ad un array che conterrà gli ID delle tecniche informative da usare;
	 * 1) conrollo il campo info_tech_on;
	 * 2) eseguo delle funzioni da inserire nel file jpw.c e jpw.h relative alle 
	 *    tecniche informative desiderate; tali funzioni dovranno preoccuparsi di
	 *    creare i campi corretti nel marker segment.
	 *  if( jpwl_cp->info_tech_on){
	 *      qui inserirò i comandi desiderati
	 *  }
	 */
    len = cio_tell() - lenp;
	cio_seek(lenp);
	cio_write(len, 2);
	//scrivo la lunghezza della codestream
	cio_skip(2);
	len_codestr = CL + len + 2;
	cio_write(len_codestr, 4);
	// calcola il crc
	cio_skip(-10);
	ResetCRC();
	for (i=0; i<4; i++){
		c = cio_read(1);
		UpdateCRC16(c);	
	}
	cio_skip(2);
	for (i=6; i<(len + 2); i++){
		c = cio_read(1);
		UpdateCRC16(c);	
	}
	cio_seek(lenp + 2);
	cio_write(crcSum, 2);
	cio_seek(lenp + len);

}



void JPWL_write_EPB(char *buf, unsigned long LDPepb, unsigned long Pepb, unsigned char Depb){
	int *alpha_to, *index_of, *gg, *data_, *bb;
	unsigned long i=0;
	unsigned int len = 11, lenp, scrivi_pb, cont, leggi, tenta=0;
	char pos_epb;
	unsigned data_unprot = 0, RS = 0;
	int n1, n2, k1, k2, j, n=255, k, CRC = 0, go_back;      // 0 = disabilitato, 1 = CRC CCITT 16 bit, 2 = Ethernet CRC 32 bit 
	//prova
	/*FILE *f;
	FILE *fp;*/


	cio_write(JPWL_MS_EPB, 2);
	lenp = cio_tell();
	/* determino len_epb e i parametri di codifica*/
	pos_epb= Depb & 0x3F;
	if ((j2k_state == J2K_STATE_MH) && !(pos_epb)){  // sto scrivendo il primo epb del mh
		
		n1=160;
		k1=64;
		go_back =lenp + 11;
		len += (n1 - k1)*(unsigned int)ceil((double)go_back/k1);
	} else {if ((j2k_state == J2K_STATE_TPH) && !(pos_epb)&& !(scavalcato)){		// primo epb di un tph
				n1=80;
				k1=go_back=25;
				len += 55;
				
			} else {                    // altro epb
				n1=40;
				k1=go_back=13;
				len += 27;
				//tenta=1;
			}
	}
	if (!Pepb) {               // per codificare i dati seguenti uso lo stesso codice di prima
		RS = 1;
		n2 = n1;
		k2 = k1;
	    len +=(unsigned int) ceil((double)LDPepb/k2)*(n2 - k2);

	} else{
		if (Pepb == 0xffffffff)    
			data_unprot=1;
		else {if ((Pepb >> 28)== 1){
				if ((Pepb & 0x00000001)){
					CRC = 2;
					len += 4;
				} else {
					CRC = 1;
					len +=2;
				}
			} else {if ((Pepb >> 28)== 2){
						n2 = ((Pepb & 0x0000ff00)>> 8);
						k2 = (Pepb & 0x000000ff);
						RS = 1;
						len +=(unsigned int) ceil((double)LDPepb/k2)*(n2 - k2);
						}
					}
			}
	}
	cio_write(len, 2);    // Lepb
	cio_write(Depb, 1);		//Depb
	cio_write((LDPepb & 0xffff0000) >> 16, 2);  //LDPepb
    cio_write((LDPepb & 0x0000ffff), 2);
	cio_write((Pepb & 0xffff0000) >> 16, 2);  //Pepb
    cio_write((Pepb & 0x0000ffff), 2);
	//printf("start\n");

	// devi implementare il codice rs per la protezione dell'epb
	// cio_write(0, (int) ceil((lenp+11)*(n1 - k1)/k1));
	if(!(alpha_to=(int *) calloc(256,sizeof(int))))
	     printf("Non può essere allocata memoria per eseguire il programma1\n ");

    if(!(index_of=(int *) calloc(256,sizeof(int))))
  	     printf("Non può essere allocata memoria per eseguire il programma2\n ");
	
	if(!(gg=(int *) calloc((n1 - k1 +1),sizeof(int))))
  	     printf("Non può essere allocata memoria per eseguire il programma3\n ");
	
	k = 255-(n1-k1);

	if(!(data_=(int *) calloc(k, sizeof(int))))
  	     printf("Non può essere allocata memoria per eseguire il programma4\n ");

	if(!(bb=(int *) calloc((n1 - k1),sizeof(int))))
  	     printf("Non può essere allocata memoria per eseguire il programma5\n ");
	
	generate_gf(alpha_to, index_of);

	gen_poly(k, alpha_to, index_of, gg);

	scrivi_pb = cio_tell();

	//prova
	

	cio_skip(-go_back);
	for (cont = 0; cont < (unsigned int) ceil((double)go_back/k1); cont++) {
		for (j=0; j<k1; j++){
			if (j+(cont*k1) <(unsigned int) go_back)
				data_[j]= cio_read(1);
			else data_[j] = 0;		//padding
		}
		for (j=k1; j<k; j++) data_[j] = 0; // padding per la gestione di codici "accorciati"
		
		leggi = cio_tell();
		encode_rs(k,alpha_to, index_of, gg, bb, data_);
		
		cio_seek(scrivi_pb);
		for (j=0; j<(n1-k1); j++)
			cio_write(bb[j], 1);
		scrivi_pb = cio_tell();
		cio_seek(leggi);
	}
	cio_seek(scrivi_pb);
	
	free(alpha_to);
	free(index_of);
	free(gg);
	free(data_);
	free(bb);

	/* protezione dei dati seguenti */
	
	
	if (!data_unprot) {
		if (CRC == 1){			//crc ccitt 16 bit questo è l'algo
			ResetCRC();
			for (i=0; i < LDPepb; i++){
				UpdateCRC16(buf[i]);	
			}
			cio_write(crcSum & 0xffff, 2);
        }
		if (CRC == 2){  
			/*per fare il crc32 occorre invertire i byte in ingresso, invertire il crc calcolato
			  e farne il complemento a 1*/
			ResetCRC();
			for (i=0; i < LDPepb; i++){
				UpdateCRC32(reflectByte(buf[i]));	
			}
			reflectCRC32();
			crcSum ^= 0xffffffff;		
            cio_write((crcSum & 0xffff0000) >> 16, 2);
			cio_write((crcSum & 0x0000ffff), 2);
        } 
		if (RS) {
			//printf("n2, k2: %d %d\n", n2, k2);
			k = 255-(n2-k2);

			if(!(alpha_to=(int *) malloc(256*sizeof(int))))
				printf("Non può essere allocata memoria per eseguire il programma1\n ");

			if(!(index_of=(int *) malloc(256*sizeof(int))))
  				printf("Non può essere allocata memoria per eseguire il programma2\n ");
	
			if(!(gg=(int *) malloc((n2 - k2 +1)*sizeof(int))))
  				printf("Non può essere allocata memoria per eseguire il programma3\n ");	
				
			if(!(data_=(int *) malloc(k*sizeof(int))))
  				printf("Non può essere allocata memoria per eseguire il programma4\n ");

			if(!(bb=(int *) malloc((n2 - k2)*sizeof(int))))
  				printf("Non può essere allocata memoria per eseguire il programma5\n ");
			
			generate_gf(alpha_to, index_of);

			gen_poly(k, alpha_to, index_of, gg);
			//printf("num blocchi: %f\n", ceil((double)LDPepb/k2));
			//printf("resto: %d\n", LDPepb%k2);

			for (cont = 0; cont < (unsigned int)ceil((double)LDPepb/k2); cont++) {

				for (j=0; j<k2; j++){
					if (j+(cont*k2) < LDPepb)
						data_[j]= 0x000000ff & buf[j+(cont*k2)];  //qualcuno dovrà spiegarmi il perchè!!!!
					else data_[j] = 0;		//padding
				}
				
				for (j=k2; j<k; j++) data_[j] = 0; // padding per la gestione di codici "accorciati"
				//prova
				/*if ((cont==(unsigned int)ceil((double)LDPepb/k2)-1)&&(tenta)){
					f=fopen("data.txt","wb");
					for(j=0; j<k; j++) fputc(data_[j], f);
					fclose(f);
				}*/
				
				encode_rs(k, alpha_to, index_of, gg, bb, data_);
				//prova
				/*if ((cont==(unsigned int)ceil((double)LDPepb/k2)-1)&&(tenta)){
					fp=fopen("parity.txt","wb");
					for(j=0; j<(n2-k2); j++) fputc(bb[j], fp);
					fclose(fp);
				}*/
				
				for (j=0; j<(n2-k2); j++){
					cio_write(bb[j], 1);
					}
				
			}

			free(alpha_to);
			free(index_of);
			free(gg);
			free(data_);
			free(bb);

			

			
		}
	}
}


void jpwl_write_ESD(char Pesd, int notil){
	int i, ult, set, nb_pos, nb_sv, metric, mode, cont=0, npix, lev, start, end, temp, len, npack;
	double **dMSE, **dPSNR, maxMSE, **PSNR, M, val, x;

	//nuova versione!!
	cio_write(JPWL_MS_ESD,2);
	set = cio_tell();
	cio_skip(2);   //len
	// faccio solo un esd medio relativo a tutte le componenti
	if(info_IM.Comp < 257)
		cio_write(0,1);		//Cesd
	else cio_write(0x0000,2);
	cio_write(Pesd,1);
	//determino i parametri per scrivere i dati dell'esd
	nb_pos = (((Pesd & 0x02)>>1)==0) ? 2 : 4;
	nb_sv = (((Pesd & 0x04)>>2)==0) ? 1 : 2;
	metric = ((Pesd & 0x38)>>3);
	mode = ((Pesd & 0xC0)>>6);
	M = 65025;
	// creo le matrici che conterranno i valori di deltaMSE e deltaPSNR
	// in realtà spreco un po' di memoria nel caso in cui la scrivo nei tile...
	dMSE = (double **) malloc(j2k_cp->th * j2k_cp->tw * sizeof(double));
	for (i=0; i<j2k_cp->th * j2k_cp->tw; i++)
		dMSE[i]=(double*) malloc(info_IM.num *sizeof(double));
	dPSNR = (double **) malloc(j2k_cp->th * j2k_cp->tw * sizeof(double));
	for (i=0; i<j2k_cp->th * j2k_cp->tw; i++)
		dPSNR[i]=(double*) malloc(info_IM.num *sizeof(double));
	PSNR = (double **) malloc(j2k_cp->th * j2k_cp->tw * sizeof(double));
	for (i=0; i<j2k_cp->th * j2k_cp->tw; i++)
		PSNR[i]=(double*) malloc(info_IM.num * sizeof(double));

	// a questo punto se notil=0 vuol dire che scrivo l'esd nel main header
	// altrimenti notil indica il tile a cui mi riferisco
	ult = (notil == 0) ? j2k_cp->th * j2k_cp->tw : notil;
	if (notil != 0) notil--; 
	
	// calcolo di dPSNR, dMSE e PSNR 
	// nuova routine più snella!!
	for (i = notil; i < ult; i++){
		val = 0;
		npix = info_IM.tile[i].nbpix;
		maxMSE = info_IM.tile[i].distotile / npix;
		for(npack=0; npack < (info_IM.num); npack++){
			dMSE[i][npack] = info_IM.tile[i].packet[npack].disto / npix;
			if (dMSE[i][npack]!=0){
				val += dMSE[i][npack];
				PSNR[i][npack] = (10 * log10(M/(maxMSE - val)));
				x = (npack==0) ? 0 : PSNR[i][npack-1];
				dPSNR[i][npack] = PSNR[i][npack] - x;
			}
			else {
				dPSNR[i][npack] = 0;
				PSNR[i][npack] = PSNR[i][npack-1];
			}
		} 
	}
	
	//ora scrivo i valori di sensibilità in base alla metrica scelta
	switch (metric){
			case 0:
				//relative error sensitivity: i lvelli di sensibilità coincidono con i layer (livello 0 = header) 
				//uso sette livelli di sensibilità relativa
				switch (mode){
					case 0:
						//packet mode
						
						for (i = notil; i < ult; i++){
							for (npack=0; npack < (info_IM.num); npack++){
								if (dPSNR[i][npack]>10)
									cio_write(1, 1);
								else if ((dPSNR[i][npack]<=10)&&(dPSNR[i][npack]>8))
									cio_write(2,1);
								else if ((dPSNR[i][npack]<=8)&&(dPSNR[i][npack]>6))
									cio_write(3,1);
								else if ((dPSNR[i][npack]<=6)&&(dPSNR[i][npack]> 4.5))
									cio_write(4,1);
								else if ((dPSNR[i][npack]<=4.5)&&(dPSNR[i][npack]>3))
									cio_write(5,1);
								else if ((dPSNR[i][npack]<=3)&&(dPSNR[i][npack]>1.7))
									cio_write(6,1);
								else
									//(dPSNR[i][npack]<=1.7)
									cio_write(7,1);
							}	
						}

						break;

					case 1:
						//byte range mode
						for (i = notil; i < ult; i++){
							//inizializzazioni
							lev = 0;
							start = 0;
							end = 1;
							for (npack=0; npack < (info_IM.num); npack++){
								temp=lev;
								//determinazione dei livelli 
								if (dPSNR[i][npack]>10)
									lev = 1;
								else if ((dPSNR[i][npack]<=10)&&(dPSNR[i][npack]>8))
									lev = 2;
								else if ((dPSNR[i][npack]<=8)&&(dPSNR[i][npack]>6))
									lev = 3;
								else if ((dPSNR[i][npack]<=6)&&(dPSNR[i][npack]> 4.5))
									lev = 4;
								else if ((dPSNR[i][npack]<=4.5)&&(dPSNR[i][npack]>3))
									lev = 5;
								else if ((dPSNR[i][npack]<=3)&&(dPSNR[i][npack]>1.7))
									lev = 6;
								else 
									//(dPSNR[i][npack]<=1.7)
									lev = 7;
								if (lev == temp){
									end++;
									continue;
								} else {
									cio_write(info_IM.tile[i].packet[start].start_pos, nb_pos);
									cio_write(info_IM.tile[i].packet[end-1].end_pos, nb_pos);
									cio_write(lev, nb_sv);
									start = end;
									end++;
								}
							}
							cio_write(info_IM.tile[i].packet[start].start_pos, nb_pos);
							cio_write(info_IM.tile[i].packet[end-1].end_pos, nb_pos);
							cio_write(lev, nb_sv);
						} 
						break;

					case 2:
						//packet range mode
						for (i = notil; i < ult; i++){
							//inizializzazioni
							lev = 0;
							start = 0;
							end = 1;
							for (npack=0; npack < (info_IM.num); npack++){
								temp=lev;
								//determinazione dei livelli (catena di if-else-if)
								if (dPSNR[i][npack]>10)
									lev = 1;
								else if ((dPSNR[i][npack]<=10)&&(dPSNR[i][npack]>8))
									lev = 2;
								else if ((dPSNR[i][npack]<=8)&&(dPSNR[i][npack]>6))
									lev = 3;
								else if ((dPSNR[i][npack]<=6)&&(dPSNR[i][npack]> 4.5))
									lev = 4;
								else if ((dPSNR[i][npack]<=4.5)&&(dPSNR[i][npack]>3))
									lev = 5;
								else if ((dPSNR[i][npack]<=3)&&(dPSNR[i][npack]>1.7))
									lev = 6;
								else 
									//(dPSNR[i][npack]<=1.7)
									lev = 7;
								if (lev == temp){
									end++;
									continue;
								} else {
									cio_write(start, nb_pos);
									cio_write(end-1, nb_pos);
									cio_write(lev, nb_sv);
									start = end;
									end++;
								}
							}
							cio_write(start, nb_pos);
							cio_write(end - 1, nb_pos);
							cio_write(lev, nb_sv);
						} 
						
						break;
					default:
						printf("errore\n");
				}
				break;
			case 1:
				// MSE
				switch (mode){
					case 0:
						//packet mode
						for (i = notil; i < ult; i++){
							npix = info_IM.tile[i].nbpix;
							val = info_IM.tile[i].distotile / npix;
							cio_write(d2pfp(val), nb_sv);
							for (npack=0; npack < (info_IM.num); npack++){
								val -= dMSE[i][npack];
								cio_write(d2pfp(val), nb_sv);
							}
						}
						break;
					case 1:
						//byte range mode
						//divido in blocchi da 10 pacchetti (l'ultimo blocco dipende da quanti pacchetti restano
						for (i = notil; i < ult; i++){
							npix = info_IM.tile[i].nbpix;
							val = info_IM.tile[i].distotile / npix;
							start = 0;
							end = 0;
							for (npack=0; npack < (info_IM.num); npack++){
								val -= dMSE[i][npack];
								if (((npack+1) % 10)==0){
									cio_write(info_IM.tile[i].packet[start].start_pos, nb_pos);
									cio_write(info_IM.tile[i].packet[end].end_pos, nb_pos);
									cio_write(d2pfp(val), nb_sv);
									start = end+1;
								}
								end++;
							}
							if (start != end){
								cio_write(info_IM.tile[i].packet[start].start_pos, nb_pos);
								cio_write(info_IM.tile[i].packet[end-1].end_pos, nb_pos);
								cio_write(d2pfp(val), nb_sv);
							}
						}

						break;
					case 2:
						//packet range mode
						//divido in blocchi da 10 pacchetti (l'ultimo blocco dipende da quanti pacchetti restano
						for (i = notil; i < ult; i++){
							npix = info_IM.tile[i].nbpix;
							val = info_IM.tile[i].distotile / npix;
							start = 0;
							end = 0;
							for (npack=0; npack < (info_IM.num); npack++){
								val -= dMSE[i][npack];
								if (((npack+1) % 10)==0){
									cio_write(start, nb_pos);
									cio_write(end, nb_pos);
									cio_write(d2pfp(val), nb_sv);
									start = end+1;
								}
								end++;
							}
							if (start != end){
								cio_write(start, nb_pos);
								cio_write(end-1, nb_pos);
								cio_write(d2pfp(val), nb_sv);
							}
						}
						break;
					default:
						printf("errore\n");
				}

				break;
			case 2:
				// MSE reduction
				switch (mode){
					case 0:
						//packet mode
						for (i = notil; i < ult; i++)
							for (npack=0; npack < (info_IM.num); npack++)
								cio_write(d2pfp(dMSE[i][npack]), nb_sv);
						
						break;
					case 1:
						//byte range mode
						//scrivo la mse reduction media relativa ad un blocco di dieci pacchetti
						for (i = notil; i < ult; i++){
							val = 0;
							start = 0;
							end = 0;
							for (npack=0; npack < (info_IM.num); npack++){
								val += dMSE[i][npack];
								if (((npack+1) % 10)==0){
									val /= 10;
									cio_write(info_IM.tile[i].packet[start].start_pos, nb_pos);
									cio_write(info_IM.tile[i].packet[end].end_pos, nb_pos);
									cio_write(d2pfp(val), nb_sv);
									start = end+1;
									val = 0;
								}
								end++;
							}
							if (start != end){
								val /= npack%10;
								cio_write(info_IM.tile[i].packet[start].start_pos, nb_pos);
								cio_write(info_IM.tile[i].packet[end-1].end_pos, nb_pos);
								cio_write(d2pfp(val), nb_sv);
							}
						}
						break;
					case 2:
						//packet range mode
						//scrivo la mse reduction media relativa ad un blocco di dieci pacchetti
						for (i = notil; i < ult; i++){
							val = 0;
							start = 0;
							end = 0;
							for (npack=0; npack < (info_IM.num); npack++){
								val += dMSE[i][npack];
								if (((npack+1) % 10)==0){
									val /= 10;
									cio_write(start, nb_pos);
									cio_write(end, nb_pos);
									cio_write(d2pfp(val), nb_sv);
									start = end+1;
									val = 0;
								}
								end++;
							}
							if (start != end){
								val /= npack%10;
								cio_write(start, nb_pos);
								cio_write(end-1, nb_pos);
								cio_write(d2pfp(val), nb_sv);
							}
						}
						break;
					default:
						printf("errore\n");
				}

				break;
			case 3:
				//PSNR

				switch (mode){
					case 0:
						//packet mode	
						for (i = notil; i < ult; i++)
							for (npack=0; npack < (info_IM.num); npack++)
								cio_write(d2pfp(PSNR[i][npack]),nb_sv);
						break;
					case 1:
						//byte range mode
						//divido in blocchi da 10 pacchetti (l'ultimo blocco dipende da quanti pacchetti restano
						for (i = notil; i < ult; i++){
							start = 0;
							end = 0;
							for (npack=0; npack < (info_IM.num); npack++){
								if (((npack+1) % 10)==0){
									cio_write(info_IM.tile[i].packet[start].start_pos, nb_pos);
									//printf("start: %d\n",info_IM.tile[i].packet[start].start_pos);
									cio_write(info_IM.tile[i].packet[end].end_pos, nb_pos);
									//printf("end: %d\n", info_IM.tile[i].packet[end].end_pos);
									cio_write(d2pfp(PSNR[i][npack]), nb_sv);
									start = end+1;
								}
								end++;
							}
							if (start != end){
								cio_write(info_IM.tile[i].packet[start].start_pos, nb_pos);
								//printf("start: %d\n",info_IM.tile[i].packet[start].start_pos);
								cio_write(info_IM.tile[i].packet[end-1].end_pos, nb_pos);
								//printf("end: %d\n", info_IM.tile[i].packet[end-1].end_pos);
								cio_write(d2pfp(PSNR[i][end-1]), nb_sv);
							}
						}
						break;
					case 2:
						//packet range mode
						//divido in blocchi da 10 pacchetti (l'ultimo blocco dipende da quanti pacchetti restano
						for (i = notil; i < ult; i++){
							start = 0;
							end = 0;
							for (npack=0; npack < (info_IM.num); npack++){
								if (((npack+1) % 10)==0){
									cio_write(start, nb_pos);
									cio_write(end, nb_pos);
									cio_write(d2pfp(PSNR[i][npack]), nb_sv);
									start = end+1;
								}
								end++;
							}
							if (start != end){
								cio_write(start, nb_pos);
								cio_write(end-1, nb_pos);
								cio_write(d2pfp(PSNR[i][end-1]), nb_sv);
							}
						}

						break;
					default:
						printf("errore\n");
				}

				break;
			case 4:
				//PSNR increases

				switch (mode){
					case 0:
						//packet mode
						for (i = notil; i < ult; i++)
							for (npack=0; npack < (info_IM.num); npack++)
								cio_write(d2pfp(dPSNR[i][npack]), nb_sv);
						break;
					case 1:
						//byte range mode
						//scrivo il psnr increase medio relativo ad un blocco di dieci pacchetti
						for (i = notil; i < ult; i++){
							val = 0;
							start = 0;
							end = 0;
							for (npack=0; npack < (info_IM.num); npack++){
								val += dPSNR[i][npack];
								if (((npack+1) % 10)==0){
									val /= 10;
									cio_write(info_IM.tile[i].packet[start].start_pos, nb_pos);
									cio_write(info_IM.tile[i].packet[end].end_pos, nb_pos);
									cio_write(d2pfp(val), nb_sv);
									start = end+1;
									val = 0;
								}
								end++;
							}
							if (start != end){
								val /= npack%10;
								cio_write(info_IM.tile[i].packet[start].start_pos, nb_pos);
								cio_write(info_IM.tile[i].packet[end-1].end_pos, nb_pos);
								cio_write(d2pfp(val), nb_sv);
							}
						}
						break;
					case 2:
						//packet range mode
						//scrivo il psnr increase medio relativo ad un blocco di dieci pacchetti
						for (i = notil; i < ult; i++){
							val = 0;
							start = 0;
							end = 0;
							for (npack=0; npack < (info_IM.num); npack++){
								val += dPSNR[i][npack];
								if (((npack+1) % 10)==0){
									val /= 10;
									cio_write(start, nb_pos);
									cio_write(end, nb_pos);
									cio_write(d2pfp(val), nb_sv);
									start = end+1;
									val = 0;
								}
								end++;
							}
							if (start != end){
								val /= npack%10;
								cio_write(start, nb_pos);
								cio_write(end-1, nb_pos);
								cio_write(d2pfp(val), nb_sv);
							}
						}
						break;
					default:
						printf("errore\n");
				}

				break;
			case 5:
				//MAXERR
				printf("Opzione MAXERR non supportata\n");
				break;
			default:
				printf("opzione riservata per uso futuro\n");
		}
	for (i=0; i<j2k_cp->th * j2k_cp->tw; i++)
		free(PSNR[i]);
	free(PSNR);
	for (i=0; i<j2k_cp->th * j2k_cp->tw; i++)
		free(dPSNR[i]);
	free(dPSNR);
	for (i=0; i<j2k_cp->th * j2k_cp->tw; i++)
		free(dMSE[i]);
	free(dMSE);

	len = cio_tell() - set;
	cio_seek(set);
	cio_write(len, 2);
	cio_skip(len - 2);
}


int JPWL_len_EPB(unsigned int LDPepb, unsigned int Pepb, unsigned char Depb){
	unsigned int len = 11, n1, n2, k1, k2, lenp;
	char pos_epb;
	
	
	lenp = cio_tell();
	/* determino len_epb e i parametri di codifica*/
	pos_epb= Depb & 0x3F;
	if ((j2k_state == J2K_STATE_MH) && !(pos_epb)){  // sto scrivendo il primo epb del mh
		n1=160;
		k1=64;
		len += (n1 - k1)*(unsigned int)ceil((double)(lenp + 11)/k1);
	} else {if ((j2k_state == J2K_STATE_TPH) && !(pos_epb)){		// primo epb di un tph
				n1=80;
				k1=25;
				len += 55;
			} else {                    // altro epb
				n1=40;
				k1=13;
				len += 27;
			}
	}
	if (!Pepb) {               // per codificare i dati seguenti uso lo stesso codice di prima
		n2 = n1;
		k2 = k1;
	    len +=(unsigned int) ceil((double)LDPepb/k2)*(n2 - k2);

	} else{
		if (Pepb == 0xffffffff)   
			1;

		else {if ((Pepb >> 28)== 1){
				if ((Pepb & 0x00000001)){
					len += 4;
				} else {
					len +=2;
				}
			} else {if ((Pepb >> 28)== 2){
						n2 = ((Pepb & 0x0000ff00)>> 8);
						k2 = (Pepb & 0x000000ff);
						len +=(unsigned int) ceil((double)LDPepb/k2)*(n2 - k2);
						}
					}
			}
	}

	
	return len;

}

void jpwl_write_RED(){
	//questa funzione scrive un marker RED di test vuoto
	cio_write(JPWL_MS_RED,2);
	//già scrivo la len(non metto informazioni nel marker, poichè la codestream sarà corretta)
	cio_write(3,2);
	cio_write(jpwl_cp.pred,1);
}





void JPWL_write_Pepbs(long *pepbs, int num_epb){
	int i, lenp, pos;
	
	cio_write(0,2);			// id relativo all'uso degli epb
	cio_skip(2);
	lenp=cio_tell();
	/*
		Qui in base a come deciderò di far operare l'encoder dovrò trovare il modo di inserire
		i Pepb relativi alla struttura; In questo caso uso una struttura fissa che prevede un 
		solo epb nel main header che usa lo stesso codice relativo al primo epb e più epb per 
		ogni tile part header, il primo che protegge tutto il tph con lo stesso codice relativo 
		al primo epb del tile part,  gli altri a protezione dei dati con un codice rs specificato
	*/
	for (i=0; i<num_epb; i++)
		cio_write(pepbs[i],4);
	// scrivo la Lid
	pos=cio_tell();
	cio_seek(lenp-2);
	cio_write(pos-lenp,2);
	cio_seek(pos);
}

void correggi_esd(int offset, int len_esd, int Pesd){
	//nel caso in cui i dati dell'esd sono posti in byte range mode, devo correggere con l'offset
	//pari alla somma della lunghezza del primo epb e dell'epc
	int nb_pos, nb_sv, start, end, set, p;
	p=cio_tell();
	//mi metto all'inizio dei dati dell'esd
	cio_skip(6);
	//printf("marker: %x\n", cio_read(2));
	//printf("len: %x\n", cio_read(2));
	//printf("info: %x\n", cio_read(2));
	//determino i parametri con cui sono scritti i dati
	nb_pos = (((Pesd & 0x02)>>1)==0) ? 2 : 4;
	nb_sv = (((Pesd & 0x04)>>2)==0) ? 1 : 2;
	//printf("offset: %x\n",offset);
	//printf("len_esd: %d\n", len_esd);
	//procedo alla correzione
	while ((cio_tell()-p)<len_esd) {
		set=cio_tell();
		start = cio_read(nb_pos);
		//printf("start: %d\n", start);
		end = cio_read(nb_pos);
		//printf("end: %d\n", end);
		cio_seek(set);
		cio_write(start + offset, nb_pos);
		cio_write(end + offset, nb_pos);
		cio_skip(nb_sv);
		//printf("pos: %d\n", cio_tell());
	}

}

int pianifica_epb(unsigned int LDPepb, unsigned int Pepb, int *n_epb ){
	int len_max, *num_epb, n2, k2;
	//serve per pianificare il numero di epb necessari per proteggere i dati
	num_epb = n_epb;
	if ((Pepb >> 28)== 2){
		n2 = ((Pepb & 0x0000ff00)>> 8);
		k2 = (Pepb & 0x000000ff);
		len_max = 0xFF80 * k2 / (n2 - k2);
		*num_epb = (int) ceil((double) LDPepb / len_max);
	} else if (!Pepb){
		n2 = 40;
		k2 = 13;
		len_max = 0xFF80 * k2 / (n2 - k2);
		*num_epb = (int) ceil((double) LDPepb / len_max);
	} else {
		*num_epb = 1;
		len_max = LDPepb+2;
	}
	return len_max;
}



int uep(int notil, char *bs, char *header, int len_header){
	//questa funzione ritorna il valore di psot_corr
	double maxMSE, val, dMSE, M;
	int depb, mask, corr=0, len_til, npix, lung=0, i, h=0, k, l=0, npack, start[6], end[6], fa=0, pkt=0, Psot_cor=0, lmax[6], no_epb[6], len[6], set, cont, tot_epb=0;
	char *data;
	double *PSNR; 
	
	PSNR = (double *) malloc(info_IM.num*sizeof(double));
	//printf("uep\n");

	//mi piazzo alla fine di SOT
	set=cio_tell();

	//determino il psnr che si raggiunge con ogni pacchetto
	M = 65025;
	//printf("M: %f\n",M);
	val = 0;
	npix = info_IM.tile[notil-1].nbpix;
	maxMSE = info_IM.tile[notil-1].distotile / npix;
	//printf("max mse:%f\n", maxMSE);
	for(npack=0; npack < (info_IM.num); npack++){
		dMSE = info_IM.tile[notil-1].packet[npack].disto / npix;
		//printf("dmse: %f\n", dMSE);
		if (dMSE!=0){
			val += dMSE;
			PSNR[npack] = (10 * log10(M/(maxMSE - val)));	
		}
		else {
			PSNR[npack] = PSNR[npack-1];
		}
		printf("psnr %d: %f\n", npack, PSNR[npack] );
	}
	
	//a questo punto determino le fasce
	npack=0;
	
	//inizializzo le posizioni
	for (i=0;i<6;i++){
		start[i]=-1;
		end[i]=-1;
	}
	start[0]=0;
	while (npack<info_IM.num){
		//printf("conta: %d\n", npack);
		if (PSNR[npack]<20){
			npack++;
		}
		else if ((PSNR[npack]>=20)&&(PSNR[npack]<23)){
			if (fa!=1){
				fa=1;
				end[0]=npack++;
				start[1]=npack;
			}
			else
				npack++;
		
		}
		else if ((PSNR[npack]>=23)&&(PSNR[npack]<26)){
			if (fa!=2){
				end[fa]=npack++;
				start[2]=npack;
				fa=2;
			}
			else
				npack++;
		
		}
		else if ((PSNR[npack]>=26)&&(PSNR[npack]<35)){
			if (fa!=3){
				end[fa]=npack++;
				start[3]=npack;
				fa=3;
			}
			else
				npack++;
		
		}
		else if ((PSNR[npack]>=35)&&(PSNR[npack]<37)){
			if (fa!=4){
				end[fa]=npack++;
				start[4]=npack;
				fa=4;
			}
			else
				npack++;

		}
		else{
			end[fa]=info_IM.num-1;
			npack=info_IM.num;
			fa=5;
		
		}
	
	}
	// devo mettere la fine all'ultima fascia abilitata
	if (fa!=5)
		end[fa]=info_IM.num-1;

	printf("livello   start    end\n");
	for (i=0;i<6;i++)
		printf("%d\t%d\t%d\n", i, start[i], end[i]);

	for (i=0;i<6;i++)
		printf("pepb[%d]: %x\n", i, jpwl_cp.pepb[i]);

	//determino anticipatamente le lunghezze degli epb
	if (len_header)
		Psot_cor += JPWL_len_EPB((len_header), jpwl_cp.pepb[0], 0) + 2;
	fa=0;
	//devo calcolare le lunghezze degli epb necessari per ogni fascia
	for(i=0;i<5;i++){
		len[i]=0;
		if ((start[i]!=-1)&&(end[i]!=-1)){
			len[i]= (info_IM.tile[notil-1].packet[end[i]].end_pos) - info_IM.tile[notil-1].packet[start[i]].start_pos +1;
			printf("len[%d]: %d\n",i, len[i]);
			if (i==0)
				//poichè non viene considerato il marker SOD
				len[i]+=2;
			lmax[i] = pianifica_epb(len[i], jpwl_cp.pepb[i+1], &no_epb[i]);
			/*printf("lmax[%d]: %d\n", i, lmax[i]);*/
			tot_epb+=no_epb[i];
			/*printf("num epb[%d]: %d\n",i, no_epb[i]);*/
			if(no_epb[i] > 1){
				if ((!(len_header))&&(i==1))
					Psot_cor += JPWL_len_EPB(lmax[i], jpwl_cp.pepb[i+1], 0) + (no_epb[i]-2)*JPWL_len_EPB((lmax[i]), jpwl_cp.pepb[i+1], 0x41) + JPWL_len_EPB((len[i] % lmax[i]), jpwl_cp.pepb[i+1], 0x41);
				else
					Psot_cor += (no_epb[i]-1)*JPWL_len_EPB((lmax[i]), jpwl_cp.pepb[i+1], 0x41) + JPWL_len_EPB((len[i] % lmax[i]), jpwl_cp.pepb[i+1], 0x41);
			}
			else{
				if ((!(len_header))&&(i==1))
					Psot_cor += JPWL_len_EPB((len[i]), jpwl_cp.pepb[i+1], 0x0);
				else
					Psot_cor += JPWL_len_EPB((len[i]), jpwl_cp.pepb[i+1], 0x41);
			}

			Psot_cor += 2*(no_epb[i]);
			fa=i;


		}
	}
	/*printf("fa: %d\n", fa);
	printf("psot_cor: %d\n", Psot_cor);
	printf("tot_epb: %d\n", tot_epb);*/
	/*len[5]=0;
	for(i=0;i<5;i++){
		len[5]+=len[i];
	}*/
	/*printf("lendata: %d\n", len[5]);*/
	cio_seek(set - 6);
	len_til=cio_read(4);
	cio_skip(-4);
	cio_write(Psot_cor + len_til,4);

	cio_seek(set);
	//correzione index
	if (info_IM.index_on){
		corr += Psot_cor;
		info_IM.tile[notil-1].end_header += corr;
		info_IM.tile[notil-1].end_pos += corr;
		if (notil < j2k_cp->tw * j2k_cp->th)
			info_IM.tile[notil].start_pos += corr;
		pack_corr(corr, notil-1);
	}
	//scrivo gli EPB e riporto i buffer
	if (len_header){
		JPWL_write_EPB(header, len_header, jpwl_cp.pepb[0], 0);
		h=1;
		pepbs[cont_epb++]=jpwl_cp.pepb[0];
	}
	for (i = 0; i < len_header; i++)
		cio_write(header[i],1);
	free(header);
	mask = (tot_epb == 1) ? 0x40 : 0x80;
	for(i=0;i<5;i++){
		if ((start[i]!=-1)&&(end[i]!=-1)){
			data = (unsigned char *)malloc(lmax[i]*sizeof(unsigned char));
			for (k=0; k < no_epb[i]; k++){
				cont=0;
				depb = mask | ((l+h)&0x3F);
				
				while (((cont + k*lmax[i])< len[i])&&(cont < lmax[i])){
					data[cont] = bs[cont + k*lmax[i] + lung];
					cont++;
				}
				if ((cont < lmax[i])&&(i==fa))
					depb |= 0x40;
				/*printf("depb[%d]: %x\n",i, depb);*/
				JPWL_write_EPB(data, cont, jpwl_cp.pepb[i+1], depb);
				pepbs[cont_epb++]=jpwl_cp.pepb[i+1];
				if (l==1)
					scavalcato=1;
				l++;
			}
			free(data);
			lung+=len[i];
		}
	}

	/*printf("lung: %d\n", lung );*/
	return Psot_cor;

}






int jpwl_encode(char *input, char * output, unsigned long CL){
	long len_tot, len_primo_epb, len_til, Psot_cor;
	unsigned char *buf1, *buf2, *buf3, *data;
	int h=0,pos, end_siz, i, depb, len_SIZ, notil, len_mar, len_red, mark, set, end_tph, len_data1, len_data2, len_data3, cur_pos, fine_mh, len_epc, corr=0, len_esd=0, lmax, no_epb, cont, mask;
	
	
	// controllo se devo usare gli epb
	if (jpwl_cp.EPB_on){
		
		//parser, serve a determinare le lunghezze degli epb e scrivere quelli nei tile part
		cio_init(input, CL*4);
		len_tot=CL;
		cio_skip(4);
		len_mar = cio_read(2);
		cio_skip(len_mar-2);
		end_siz=cio_tell();
		j2k_state = J2K_STATE_MH;
		mark = cio_read(2);
		//printf("mark: %x\n",mark);
		while (mark!=0xff90){
			len_mar = cio_read(2);
			cio_skip(len_mar-2);
			mark = cio_read(2);
			//printf("mark: %x\n",mark);
		}
		fine_mh=cio_tell()-2;
		cont_epb=1;
		pepbs[0]=jpwl_cp.pepb[0];

		j2k_state = J2K_STATE_TPH;
		for (notil=1; notil <= j2k_cp->tw * j2k_cp->th; notil++){
			Psot_cor = 0;
			//trucco per non fare sbagliare nella scrittura di più di 64 epb
			scavalcato=0;
			cio_skip(4);
			len_til = cio_read(4);
			cio_skip(2);
			set = cio_tell();
			mark = cio_read(2);
			//printf("mark: %x\n",mark);
			while (mark!=0xff93){
				len_mar = cio_read(2);
				cio_skip(len_mar-2);
				mark = cio_read(2);
				//printf("mark: %x\n",mark);
			}
			end_tph = cio_tell()-2;
			len_data1= end_tph - set;
			len_data2= len_til+set-end_tph-12;
			/*printf("lendata2: %d\n",len_data2);*/
			buf1 = (unsigned char *) malloc(len_data1*sizeof(unsigned char));
			buf2 = (unsigned char *) malloc(len_data2*sizeof(unsigned char));
		    cio_seek(set);
			for (i=0; i<len_data1; i++)
				buf1[i]=cio_read(1);
			for (i=0; i<len_data2; i++)
				buf2[i]=cio_read(1);
			cur_pos=cio_tell();
			len_data3 = len_tot-cur_pos;
			buf3 = (unsigned char *) malloc((len_data3)*sizeof(unsigned char));
			for (i=0; i< len_data3; i++)
				buf3[i]=cio_read(1);
			//uep
			if (jpwl_cp.UEP_on){
				cio_seek(set);
				Psot_cor=uep(notil,buf2,buf1,len_data1);
				//free(buf1);
				len_tot += Psot_cor;
			}
			else{
				//correzione Psot
				if (len_data1)
					Psot_cor += JPWL_len_EPB((len_data1), jpwl_cp.pepb[0], 0) + 2;
				lmax = pianifica_epb(len_data2, jpwl_cp.pepb[1], &no_epb);
				//printf("num epb: %d\n", no_epb);
				if(no_epb > 1){
					if (len_data1)
						Psot_cor += (no_epb-1)*JPWL_len_EPB((lmax), jpwl_cp.pepb[1], 0x41) + JPWL_len_EPB((len_data2 % lmax), jpwl_cp.pepb[1], 0x41);
					else
						Psot_cor += JPWL_len_EPB(lmax, jpwl_cp.pepb[1], 0) + (no_epb-2)*JPWL_len_EPB((lmax), jpwl_cp.pepb[1], 0x41) + JPWL_len_EPB((len_data2 % lmax), jpwl_cp.pepb[1], 0x41);
				}
				else{
					if (len_data1)
						Psot_cor += JPWL_len_EPB((len_data2), jpwl_cp.pepb[1], 0x41);
					else
						Psot_cor += JPWL_len_EPB((len_data2), jpwl_cp.pepb[1], 0x0);
				}
				Psot_cor += 2*(no_epb);
				len_tot += Psot_cor;
				//Psot_cor += len_til;
				cio_seek(set - 6);
				cio_write(Psot_cor + len_til,4);

				cio_seek(set);
				//correzione index
				if (info_IM.index_on){
					corr += Psot_cor;
					info_IM.tile[notil-1].end_header += corr;
					info_IM.tile[notil-1].end_pos += corr;
					if (notil < j2k_cp->tw * j2k_cp->th)
						info_IM.tile[notil].start_pos += corr;
					pack_corr(corr, notil-1);
				}
				//scrivo gli EPB e riporto i buffer
				if (len_data1){
					JPWL_write_EPB(buf1, len_data1, jpwl_cp.pepb[0], 0);
					h=1;
					pepbs[cont_epb++]=jpwl_cp.pepb[0];
				}
				for (i = 0; i < len_data1; i++)
					cio_write(buf1[i],1);
				free(buf1);
				mask = (no_epb == 1) ? 0x40 : 0x80;
				data = (unsigned char *)malloc(lmax*sizeof(unsigned char));
				for (i=0; i < no_epb; i++){
					cont=0;
					depb = mask | ((i+h)&0x3F);
					while (((cont + i*lmax)< len_data2)&&(cont < lmax)){
						data[cont] = buf2[cont + i*lmax];
						cont++;
					}
					if (cont < lmax)
						depb |= 0x40;
					JPWL_write_EPB(data, cont, jpwl_cp.pepb[1], depb);
					pepbs[cont_epb++]=jpwl_cp.pepb[1];
					if (i==1)
						scavalcato=1;
				}
				free(data);
			}
			for (i = 0; i < len_data2; i++)
				cio_write(buf2[i],1);
			cur_pos=cio_tell();
			free(buf2);
			for (i=0; i < len_data3; i++)
				cio_write(buf3[i],1);
			free(buf3);
			cio_seek(cur_pos);
			mark = cio_read(2);

		}

		//ora mi trovo alla fine della codestream
		len_tot=cio_tell();
		// controllo se devo inserire l'esd (non lo devo inserire qui se è attivo il packet range mode)
		if ((jpwl_cp.ESD_on)&(!(((jpwl_cp.pesd & 0xC0)>>6)==2))){
			cio_seek(end_siz);
			len_data1 = len_tot - end_siz;
			buf1 = (unsigned char *) malloc(len_data1 * sizeof(unsigned char));
			for (i = 0; i < len_data1; i++){
				buf1[i] = cio_read(1);
			}
			//printf("%x%x\n", buf1[len_tot - set - 2], buf1[len_tot - set -1]);
			cio_seek(end_siz);
			jpwl_write_ESD(jpwl_cp.pesd, 0);
			len_esd = cio_tell() - end_siz;
			//printf("len esd: %d\n",len_esd);
			len_tot += len_esd;
			for (i = 0; i < len_data1; i++){
				cio_write(buf1[i],1);
			}
			free(buf1);
		}

		//scrivo l'epc ed il primo epb
		cio_init(output, len_tot*2);
		j2k_state = J2K_STATE_MH;
		for (i=0; i< 6; i++){
			output[i]=input[i];
		}
		cio_skip(4);
		len_SIZ = cio_read(2);
		for (i=6; i< 4+len_SIZ; i++){
		output[i]=input[i];
		}
		cio_skip(len_SIZ-2);
		set = cio_tell();
		len_epc = 15 + cont_epb*4;
		len_primo_epb=JPWL_len_EPB(fine_mh - set + len_epc, jpwl_cp.pepb[0], 0);
		len_red=(jpwl_cp.RED_on)? 5 : 0; 
		if (info_IM.index_on){
			corr=len_epc + len_primo_epb + 2 + len_esd + len_red;
			info_IM.Main_head_end += corr;
			//printf("fine_mh: %d\n",info_IM.Main_head_end);
			//printf("%d\n", info_IM.tile[0].start_pos);
			info_IM.tile[0].start_pos += corr;
			for (i=0; i < j2k_cp->th * j2k_cp->tw; i++){
				info_IM.tile[i].end_header += corr;
				info_IM.tile[i].end_pos += corr;
				if (i <  j2k_cp->tw * j2k_cp->th)
					info_IM.tile[i+1].start_pos += corr;
				pack_corr(corr, i);
			} 
		} 

		jpwl_write_EPC_fin(len_tot+len_primo_epb+2+len_esd+len_red, pepbs, cont_epb);
		//controllo se devo inserire la red (solo test)
		if (jpwl_cp.RED_on) 
			jpwl_write_RED();
		pos=cio_tell();
		//scrivo da input il main header
		for (i=4+len_SIZ; i<fine_mh+len_esd; i++){
			cio_write(input[i],1);
		}
		cur_pos = cio_tell();
		//se l'esd è presente ed è in byte range mode devo correggere la posizioni
		if (jpwl_cp.ESD_on && (((jpwl_cp.pesd & 0xC0)>>6)==1) ){
			cio_seek(pos);
			correggi_esd(corr, len_esd, jpwl_cp.pesd);
		}
		
		len_data1=cur_pos-set;
		buf1= (unsigned char *) malloc((len_data1)*sizeof(unsigned char));
		cio_seek(set);
		for (i=0; i< len_data1; i++){
			buf1[i]=cio_read(1);
		}
		cio_seek(set);
		JPWL_write_EPB(buf1, len_data1, jpwl_cp.pepb[0], 0x40);
		for (i = 0; i < len_data1; i++)
			cio_write(buf1[i],1);
		free(buf1);
		for (i = fine_mh+len_esd; i < len_tot; i++)
			cio_write(input[i],1);
		
		
	}
	
	
	// se devo inserire l'epc da solo lo posiziono subito dopo SIZ
	else {
		len_esd=0;
		cio_init(output, CL*2);
		for (i=0; i< 6; i++){
			output[i]=input[i];
		}
		cio_skip(4);
		len_SIZ = cio_read(2);
		for (i=6; i< 4+len_SIZ; i++){
			output[i]=input[i];
		}
		cio_skip(len_SIZ-2);
		set = cio_tell();
		// controllo se devo inserire l'esd 
		if ((jpwl_cp.ESD_on)&(!(((jpwl_cp.pesd & 0xC0)>>6)==2))){
			jpwl_write_ESD(jpwl_cp.pesd, 0);
			len_esd = cio_tell() - set;
			cio_seek(set);
			buf1 = (unsigned char *) malloc(len_esd * sizeof(unsigned char));
			for (i = 0; i < len_esd; i++){
				buf1[i] = cio_read(1);
			}
			cio_seek(set);
			len_red=(jpwl_cp.RED_on)? 5 : 0;
			corr=len_esd+11+len_red;
			//correzione delle posizioni dei pack nella struttura info image da fare nel caso di esd nel main header
			if (info_IM.index_on){
				info_IM.Main_head_end += corr;
				//printf("fine_mh: %d\n",info_IM.Main_head_end);
				info_IM.tile[0].start_pos += corr;
				for (i=0; i < j2k_cp->th * j2k_cp->tw; i++){
					info_IM.tile[i].end_header += corr;
					info_IM.tile[i].end_pos += corr;
					if (i <  j2k_cp->tw * j2k_cp->th)
						info_IM.tile[i+1].start_pos += corr;
					pack_corr(corr, i);
				} 
			} 
		}
		jpwl_write_EPC_fin(CL + len_esd, NULL, 0);
		//controllo se devo inserire la red (solo test)
		if (jpwl_cp.RED_on) 
			jpwl_write_RED();
		set=cio_tell();
		if (jpwl_cp.ESD_on){
			for (i = 0; i < len_esd; i++){
					cio_write(buf1[i],1);
				}
			free(buf1);
			if (((jpwl_cp.pesd & 0xC0)>>6)==1) {
				cio_seek(set);
				correggi_esd(corr, len_esd, jpwl_cp.pesd);
			}
		}
		for (i=4+len_SIZ; i < CL; i++){
			cio_write(input[i],1);
		}
	}

	
	return cio_tell();

}

void get_jpwl_cp(j2k_cp_t *cp){
	char risp, scelta,choose;
	int out = 0;
	unsigned long n;
	j2k_cp_t *param;
	
	param = cp;
	printf("Hai scelto di attivare le tecniche jpwl. Verra' inserito l'EPC\n");
	if (param->intermed_file){
		printf("Poiche' e' attiva l'opzione intermed file non sara' possibile avere nell'EPC l'informazione sulla");
		printf("lunghezza della codestream e gli eventuali ESD saranno inseriti nei tile part header.\n\n\n"); 
	}
	printf("Vuoi usare gli EPB (s/n)?: ");
	risp = (char) _getche();
	printf("\n\n\n");
	do {
		switch (risp){
			case 's':
				jpwl_cp.EPB_on = 1;
				do{
					printf("UEP oppure EEP (u,e)?: ");
					choose = (char) _getche();
					printf("\n\n\n");
				}while(!strchr("ue",choose));
				if (choose=='e'){
						do{
							printf("Impostazioni EPB che proteggono gli header\n");
							printf("1) Usa il codice RS utilizzato per proteggere l'EPB stesso;\n");
							printf("2) Specifica un codice RS(n,32);\n");
							printf("3) Usa il CRC-16(CCITT);\n");
							printf("4) Usa il CRC-32(ETHERNET);\n");
							printf("5) Nessuna protezione.\n\n\n");
							scelta = (char) _getche();
							printf("\n\n");
						}while(!strchr("12345",scelta));
						if (scelta == '1')
							jpwl_cp.pepb[0]=0x00000000;
						if (scelta == '2'){
							do {
							printf("Inserisci il valore di n desiderato (k=32 da standard; n compreso tra 37 e 128): ");
							scanf("%d", &n);
							printf("\n\n\n");
							} while ((n<37)||(n>128));
							jpwl_cp.pepb[0] = ( n & 0x000000FF)<<8;
							jpwl_cp.pepb[0] |= 0x20000020;
						}
						if (scelta == '3')
							jpwl_cp.pepb[0] = 0x10000000;
						if (scelta == '4')
							jpwl_cp.pepb[0] = 0x10000001;
						if (scelta == '5')
							jpwl_cp.pepb[0] = 0xFFFFFFFF;
						printf("pepb1: %x\n", jpwl_cp.pepb[0]);
						do{
							printf("Impostazioni EPB che proteggono i dati\n");
							printf("1) Usa il codice RS utilizzato per proteggere l'EPB stesso;\n");
							printf("2) Specifica un codice RS(n,32);\n");
							printf("3) Usa il CRC-16(CCITT);\n");
							printf("4) Usa il CRC-32(ETHERNET);\n");
							printf("5) Nessuna protezione.\n\n\n");
							scelta = (char) _getche();
							printf("\n\n");
						}while(!strchr("12345",scelta));
						if (scelta == '1')
							jpwl_cp.pepb[1]=0x00000000;
						if (scelta == '2'){
							do {
							printf("Inserisci il valore di n desiderato (k=32 da standard; n compreso tra 37 e 128): ");
							scanf("%d", &n);
							printf("\n\n\n");
							} while ((n<37)||(n>128));
							jpwl_cp.pepb[1] = ( n & 0x000000FF)<<8;
							jpwl_cp.pepb[1] |= 0x20000020;
						}
						if (scelta == '3')
							jpwl_cp.pepb[1] = 0x10000000;
						if (scelta == '4')
							jpwl_cp.pepb[1] = 0x10000001;
						if (scelta == '5')
							jpwl_cp.pepb[1] = 0xFFFFFFFF;
						printf("pepb2: %x\n", jpwl_cp.pepb[1]);
						out=1;
				}
				if (choose=='u'){
						param->index_on = 1;
						jpwl_cp.UEP_on = 1;
						jpwl_cp.pepb[0] = 0x20008020;
						jpwl_cp.pepb[1] = 0x20002B20;
						jpwl_cp.pepb[2] = 0x20002B20;
						jpwl_cp.pepb[3] = 0x20002B20;
						jpwl_cp.pepb[4] = 0x20002520;
						jpwl_cp.pepb[5] = 0x20002520;
						out=1;
				}

				break;
			case 'n':
				out=1;
				break;
			default:
				printf("Errore nell'immissione della risposta.\n");
				printf("Vuoi usare gli EPB (s/n)?: ");
				risp = (char) _getche();
				printf("\n\n\n");
		}
	}while (!out);
	out = 0;
	printf("Vuoi usare l'ESD (s/n)?: ");
	risp = (char) _getche();
	printf("\n\n\n");
	do {
		switch(risp){
			case 's':
				jpwl_cp.ESD_on = 1;
				param->index_on = 1;
				jpwl_cp.pesd = 0x01;
				do{
					printf("Scegli la modalita' di rappresentazione:\n");
					printf("1) Packet mode;\n");
					printf("2) Byte range mode;\n");
					printf("3) Packet range mode.\n");
					scelta = (char) _getche();
					printf("\n\n");
				}while(!strchr("123",scelta));
				if (scelta =='1')
					jpwl_cp.pesd |= 0x00;
				if (scelta =='2')
					jpwl_cp.pesd |= 0x42;
				if (scelta =='3')
					jpwl_cp.pesd |= 0x80;
				do{
					printf("Scegli la metrica:\n");
					printf("1) Relative error sensitivity;\n");
					printf("2) MSE;\n");
					printf("3) MSE reduction;\n");
					printf("4) PSNR;\n");
					printf("5) PSNR increase.\n");
					scelta = (char) _getche();
					printf("\n\n");
				}while(!strchr("12345",scelta));
				if (scelta =='1')
					jpwl_cp.pesd |= 0x00;
				if (scelta =='2')
					jpwl_cp.pesd |= 0x0C;
				if (scelta =='3')
					jpwl_cp.pesd |= 0x14;
				if (scelta =='4')
					jpwl_cp.pesd |= 0x1C;
				if (scelta =='5')
					jpwl_cp.pesd |= 0x24;
				printf("pesd: %x\n", jpwl_cp.pesd);
				out=1;
				break;
			case 'n':
				out=1;
				break;
			default:
				printf("Errore nell'immissione della risposta.\n");
				printf("Vuoi usare l'ESD (s/n)?: ");
				risp = (char) _getche();
				printf("\n\n\n");
		}
	}while (!out);
	out = 0;
	printf("Vuoi usare la RED (s/n)?: ");
	risp = (char) _getche();
	printf("\n\n\n");
	do {
		switch(risp){
			case 's':
				jpwl_cp.RED_on = 1;
				//suppongo che la codestream sia corretta => b0=0 e b5b4b3=000
				jpwl_cp.pred = 0x00;
				do{
					printf("Scegli la modalita' di rappresentazione:\n");
					printf("1) Packet mode;\n");
					printf("2) Byte range mode;\n");
					printf("3) Packet range mode.\n");
					scelta = (char) _getche();
					printf("\n\n");
				}while(!strchr("123",scelta));
				if (scelta =='1')
					jpwl_cp.pred |= 0x00;
				if (scelta =='2')
					jpwl_cp.pred |= 0x42;
				if (scelta =='3')
					jpwl_cp.pred |= 0x80;
				printf("pred: %x\n", jpwl_cp.pred);
				out=1;
				break;
			case 'n':
				out=1;
				break;
			default:
				printf("Errore nell'immissione della risposta.\n");
				printf("Vuoi usare la RED (s/n)?: ");
				risp = (char) _getche();
				printf("\n\n\n");
		}
	}while (!out);

}



void ResetCRC()
{
	crcSum = 0xffffffff;
}


void UpdateCRC16(char x)
{
	int tmp;
	tmp = ((x ^ (crcSum >> 8)) & 0xff);
	crcSum = ((crcSum << 8) ^ CrcT16[tmp]) & 0xffff;
}

void UpdateCRC32(char x)
{
	int tmp;
	tmp = ((x ^ (crcSum >> 24)) & 0xff);
	crcSum = ((crcSum << 8) ^ CrcT32[tmp]);
}
/* funzioni per l'inversione dei byte e del crc32 */

char reflectByte(char inbyte)
{
  // reflect one byte

  unsigned char outbyte=0;
  unsigned char i=0x01;
  unsigned char j;

  for (j=0x080; j; j>>=1) 
  {
    if (inbyte & i) outbyte|=j;
    i<<=1;
  }
  
  return outbyte;
}


void reflectCRC32()
{
 

  unsigned long outcrc=0;
  unsigned long i=0x00000001;
  unsigned long j;

  for (j=0x80000000; j; j>>=1) 
  {
    if (crcSum & i) 
		outcrc|=j;
    i=i<<1;
  }
  crcSum = outcrc;
}

/*
int crc_ccitt(char *buffer, int len){
	char c;
	int i;
	ResetCRC();
	for (i=0; i<len; i++) {
		c= *buffer;
		UpdateCRC(c);
		buffer++;
	}
	return crcSum;
}

*/


//funzioni di conversione allo pseudo floating point format 
short int d2pfp(double in){
	long *punt, data;
	short int mant, ex, out;
	punt = &in;
	punt++;
	data = *punt;
	mant = (data>>9) & 0x07FF;
	ex = (data>>20) & 0x07FF;
	out = ex+15;
	out = out << 11;
	out &= 0x0000F800;
	out |= mant;
	return out;
}


double pfp2d(short int in){
	double out;
	short int ex, mant;
	mant = in & 0x07FF;
	ex = (in >> 11) & 0x001F;
	out = pow(2,ex-15)*(1+(mant/pow(2,11)));
	return out;
}
