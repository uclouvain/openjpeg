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

#ifndef __TCD_H
#define __TCD_H

#include "j2k.h"
#include "tgt.h"

typedef struct {
  int numpasses;
  int len;
  unsigned char *data;
  int maxpasses;
  int numnewpasses;
  int newlen;
} tcd_seg_t;

typedef struct {
  int rate;
  double distortiondec;
  int term, len;
} tcd_pass_t;

typedef struct {
  int numpasses;		/* Number of passes in the layer */
  int len;			/* len of information */
  double disto;			/* add for index (Cfr. Marcela) */
  unsigned char *data;		/* data */
} tcd_layer_t;

typedef struct {
  int x0, y0, x1, y1;		/* dimension of the code-blocks : left upper corner (x0, y0) right low corner (x1,y1) */
  int numbps;
  int lastbp;			/* Add antonin : quantizbug1 */
  int numlenbits;
  int len;			/* length */
  int numpasses;		/* number of pass already done for the code-blocks */
  int numnewpasses;		/* number of pass added to the code-blocks */
  int numsegs;			/* number of segments */
  tcd_seg_t segs[100];		/* segments informations */
  unsigned char data[8192];	/* Data */
  int numpassesinlayers;	/* number of passes in the layer */
  tcd_layer_t layers[100];	/* layer information */
  int totalpasses;		/* total number of passes */
  tcd_pass_t passes[100];	/* information about the passes */
} tcd_cblk_t;

typedef struct {
  int x0, y0, x1, y1;		/* dimension of the precinct : left upper corner (x0, y0) right low corner (x1,y1) */
  int cw, ch;			/* number of precinct in width and heigth */
  tcd_cblk_t *cblks;		/* code-blocks informations */
  tgt_tree_t *incltree;		/* inclusion tree */
  tgt_tree_t *imsbtree;		/* IMSB tree */
} tcd_precinct_t;

typedef struct {
  int x0, y0, x1, y1;		/* dimension of the subband : left upper corner (x0, y0) right low corner (x1,y1) */
  int bandno;
  tcd_precinct_t *precincts;	/* precinct information */
  int numbps;
  int stepsize;
} tcd_band_t;

typedef struct {
  int x0, y0, x1, y1;		/* dimension of the resolution level : left upper corner (x0, y0) right low corner (x1,y1) */
  int pw, ph;
  int numbands;			/* number sub-band for the resolution level */
  tcd_band_t bands[3];		/* subband information */
} tcd_resolution_t;

typedef struct {
  int x0, y0, x1, y1;		/* dimension of component : left upper corner (x0, y0) right low corner (x1,y1) */
  int numresolutions;		/* number of resolutions level */
  tcd_resolution_t *resolutions;	/* resolutions information */
  int *data;			/* data of the component */
  int nbpix;			/* add fixed_quality */
} tcd_tilecomp_t;

typedef struct {
  int x0, y0, x1, y1;		/* dimension of the tile : left upper corner (x0, y0) right low corner (x1,y1) */
  int numcomps;			/* number of components in tile */
  tcd_tilecomp_t *comps;	/* Components information */
  int nbpix;			/* add fixed_quality */
  double distotile;		/* add fixed_quality */
  double distolayer[100];	/* add fixed_quality */
} tcd_tile_t;

typedef struct {
  int tw, th;			/* number of tiles in width and heigth */
  tcd_tile_t *tiles;		/* Tiles information */
} tcd_image_t;

/*
 * Initialize the tile coder (reuses the memory allocated by tcd_malloc_encode)
 * img: raw image
 * cp: coding parameters
 * curtileno : number that identifies the tile that will be encoded
 */
void tcd_init_encode(j2k_image_t * img, j2k_cp_t * cp, int curtileno);


/*
 * Initialize the tile coder (allocate the memory)
 * img: raw image
 * cp: coding parameters
 * curtileno : number that identifies the tile that will be encoded
 */
void tcd_malloc_encode(j2k_image_t * img, j2k_cp_t * cp, int curtileno);


/*
 * Initialize the tile decoder
 * img: raw image
 * cp: coding parameters
 */
void tcd_init(j2k_image_t * img, j2k_cp_t * cp);


/*
 * Free the memory allocated for encoding
 * img: raw image
 * cp: coding parameters
 * curtileno : number that identifies the tile that will be encoded
 */
void tcd_free_encode(j2k_image_t * img, j2k_cp_t * cp, int curtileno);

/*
 * Encode a tile from the raw image into a buffer, format pnm, pgm or ppm
 * tileno: number that identifies one of the tiles to be encoded
 * dest: destination buffer
 * len: length of destination buffer
 * info_IM: creation of index file
 */
int tcd_encode_tile_pxm(int tileno, unsigned char *dest, int len,
			info_image * info_IM);


/*
 * Encode a tile from the raw image into a buffer, format pgx
 * tileno: number that identifies one of the tiles to be encoded
 * dest: destination buffer
 * len: length of destination buffer
 * info_IM: creation of index file
 */
int tcd_encode_tile_pgx(int tileno, unsigned char *dest, int len,
			info_image * info_IM);

/*
 * Decode a tile from a buffer into a raw image
 * src: source buffer
 * len: length of the source buffer
 * tileno: number that identifies the tile that will be decoded
 */
int tcd_decode_tile(unsigned char *src, int len, int tileno);

#endif
