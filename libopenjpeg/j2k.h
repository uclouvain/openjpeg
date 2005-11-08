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

#define VERSION "0.0.8"

#ifdef DAVID_WIN32
#ifdef LIBJ2K_EXPORTS
#define LIBJ2K_API __declspec(dllexport)
#else
#define LIBJ2K_API __declspec(dllimport)
#endif
#else
#define LIBJ2K_API
#endif

#ifndef __J2K_H
#define __J2K_H

#define J2K_MAXRLVLS 33
#define J2K_MAXBANDS (3*J2K_MAXRLVLS+1)

#define J2K_CP_CSTY_PRT 0x01
#define J2K_CP_CSTY_SOP 0x02
#define J2K_CP_CSTY_EPH 0x04
#define J2K_CCP_CSTY_PRT 0x01
#define J2K_CCP_CBLKSTY_LAZY 0x01
#define J2K_CCP_CBLKSTY_RESET 0x02
#define J2K_CCP_CBLKSTY_TERMALL 0x04
#define J2K_CCP_CBLKSTY_VSC 0x08
#define J2K_CCP_CBLKSTY_PTERM 0x10
#define J2K_CCP_CBLKSTY_SEGSYM 0x20
#define J2K_CCP_QNTSTY_NOQNT 0
#define J2K_CCP_QNTSTY_SIQNT 1
#define J2K_CCP_QNTSTY_SEQNT 2

typedef struct {
	int dx, dy;										/* XRsiz, YRsiz */
	int prec;											/* precision */
	int bpp;											/* deapth of image in bits */
	int sgnd;											/* signed */
	int *data;										/* image-component data */
} j2k_comp_t;

typedef struct {
	int x0, y0;										/* XOsiz, YOsiz */
	int x1, y1;										/* Xsiz, Ysiz  */
	int numcomps;									/* number of components */
	int index_on;									/* 0 = no index || 1 = index */
	/* int PPT; */
	j2k_comp_t *comps;						/* image-components */
} j2k_image_t;

typedef struct {
	int expn;											/* exponent */
	int mant;											/* mantissa */
} j2k_stepsize_t;

typedef struct {
	int csty;											/* coding style */
	int numresolutions;						/* number of resolutions */
	int cblkw;										/* width of code-blocks */
	int cblkh;										/* height of code-blocks */
	int cblksty;									/* code-block coding style */
	int qmfbid;										/* discrete wavelet transform identifier */
	int qntsty;										/* quantisation style */
	j2k_stepsize_t stepsizes[J2K_MAXBANDS];	/* stepsizes used for quantisation */
	int numgbits;									/* number of guard bits */
	int roishift;									/* Region Of Interest shift */
	int prcw[J2K_MAXRLVLS];				/* Precinct width */
	int prch[J2K_MAXRLVLS];				/* Precinct height */
} j2k_tccp_t;

typedef struct {
	int resno0, compno0;
	int layno1, resno1, compno1;
	int prg;
	int tile;
	char progorder[4];
} j2k_poc_t;

typedef struct {
	int csty;											/* coding style   */
	int prg;											/* progression order */
	int numlayers;								/* number of layers */
	int mct;											/* multi-component transform identifier */
	int rates[100];								/* rates of layers */
	int numpocs;									/* number of progression order changes  */
	j2k_poc_t pocs[32];						/* progression order changes */
	j2k_tccp_t *tccps;						/* tile-component coding parameters */
} j2k_tcp_t;

typedef struct {
	int image_type;								/* 0: PNM, PGM, PPM 1: PGX */
	int disto_alloc;							/* Allocation by rate/distortion */
	int fixed_alloc;							/* Allocation by fixed layer */
	int tx0, ty0;									/* XTOsiz, YTOsiz */
	int tdx, tdy;									/* XTsiz, YTsiz */
	char *comment;								/* comment for coding */
	int tw, th;
	j2k_tcp_t *tcps;							/* tile coding parameters */
} j2k_cp_t;

typedef struct {
	int start_pos, end_pos;				/* start and end position */
	double disto;
} info_packet;									/* Index struct */

typedef struct {
	int num_tile;									/* Number of Tile */
	int start_pos;								/* Start position */
	int end_header;								/* End position of the header */
	int end_pos;									/* End position */
	int pw, ph;										/* number of precinct by tile */
	info_packet *packet;					/* information concerning packets inside tile */
} info_tile;										/* index struct */

typedef struct {
	int index_on;
	double D_max;									/* ADD for Marcela */
	int num;											/* numero of packet */
	int index_write;							/* writing the packet inthe index with t2_encode_packets */
	int Im_w, Im_h;								/* Image width and Height */
	int Prog;											/* progression order */
	int Tile_x, Tile_y;						/* Number of Tile in X and Y */
	int tw, th;
	int Comp;											/* Component numbers */
	int Layer;										/* number of layer */
	int Decomposition;						/* number of decomposition */
	int pw, ph;										/* nombre precinct in X and Y */
	int pdx, pdy;									/* size of precinct in X and Y */
	int Main_head_end;						/* Main header position */
	int codestream_size;					/* codestream's size */
	info_tile *tile;							/* information concerning tiles inside image */
} info_image;										/* index struct */



/* 
 * Encode an image into a JPEG-2000 codestream
 * i: image to encode
 * cp: coding parameters
 * dest: destination buffer
 * len: length of destination buffer
 * index : index file name
 */
LIBJ2K_API int j2k_encode(j2k_image_t * i, j2k_cp_t * cp, char *outfile,
													int len, char *index);

/* LIBJ2K_API int j2k_encode(j2k_image_t *i, j2k_cp_t *cp,unsigned char *dest, int len); */
/*
 * Decode an image from a JPEG-2000 codestream
 * src: source buffer
 * len: length of source buffer
 * i: decode image
 * cp: coding parameters that were used to encode the image
 */
LIBJ2K_API int j2k_decode(unsigned char *src, int len, j2k_image_t ** i,
													j2k_cp_t ** cp);

#endif
