/*
 * Copyright (c) 2001-2002, David Janssens
 * Copyright (c) 2002-2004, Yannick Verschueren
 * Copyright (c) 2002-2004, Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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

#include "tcd.h"
#include "int.h"
#include "t1.h"
#include "t2.h"
#include "dwt.h"
#include "mct.h"
#include <setjmp.h>
#include <float.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static tcd_image_t tcd_image;

static j2k_image_t *tcd_img;
static j2k_cp_t *tcd_cp;

static tcd_tile_t *tcd_tile;
static j2k_tcp_t *tcd_tcp;
static int tcd_tileno;

static tcd_tile_t *tile;
static tcd_tilecomp_t *tilec;
static tcd_resolution_t *res;
static tcd_band_t *band;
static tcd_precinct_t *prc;
static tcd_cblk_t *cblk;

extern jmp_buf j2k_error;

void tcd_dump(tcd_image_t * img, int curtileno)
{
  int tileno, compno, resno, bandno, precno, cblkno;
  (void)curtileno;
  fprintf(stdout, "image {\n");
  fprintf(stdout, "  tw=%d, th=%d x0=%d x1=%d y0=%d y1=%d\n", img->tw,
	  img->th, tcd_img->x0, tcd_img->x1, tcd_img->y0, tcd_img->y1);
  for (tileno = 0; tileno < img->th * img->tw; tileno++) {
    tcd_tile_t *tile = &tcd_image.tiles[tileno];
    fprintf(stdout, "  tile {\n");
    fprintf(stdout, "    x0=%d, y0=%d, x1=%d, y1=%d, numcomps=%d\n",
	    tile->x0, tile->y0, tile->x1, tile->y1, tile->numcomps);
    for (compno = 0; compno < tile->numcomps; compno++) {
      tcd_tilecomp_t *tilec = &tile->comps[compno];
      fprintf(stdout, "    tilec {\n");
      fprintf(stdout,
	      "      x0=%d, y0=%d, x1=%d, y1=%d, numresolutions=%d\n",
	      tilec->x0, tilec->y0, tilec->x1, tilec->y1,
	      tilec->numresolutions);
      for (resno = 0; resno < tilec->numresolutions; resno++) {
	tcd_resolution_t *res = &tilec->resolutions[resno];
	fprintf(stdout, "\n   res {\n");
	fprintf(stdout,
		"          x0=%d, y0=%d, x1=%d, y1=%d, pw=%d, ph=%d, numbands=%d\n",
		res->x0, res->y0, res->x1, res->y1, res->pw, res->ph,
		res->numbands);
	for (bandno = 0; bandno < res->numbands; bandno++) {
	  tcd_band_t *band = &res->bands[bandno];
	  fprintf(stdout, "        band {\n");
	  fprintf(stdout,
		  "          x0=%d, y0=%d, x1=%d, y1=%d, stepsize=%f, numbps=%d\n",
		  band->x0, band->y0, band->x1, band->y1,
		  band->stepsize, band->numbps);
	  for (precno = 0; precno < res->pw * res->ph; precno++) {
	    tcd_precinct_t *prec = &band->precincts[precno];
	    fprintf(stdout, "          prec {\n");
	    fprintf(stdout,
		    "            x0=%d, y0=%d, x1=%d, y1=%d, cw=%d, ch=%d\n",
		    prec->x0, prec->y0, prec->x1, prec->y1,
		    prec->cw, prec->ch);
	    for (cblkno = 0; cblkno < prec->cw * prec->ch; cblkno++) {
	      tcd_cblk_t *cblk = &prec->cblks[cblkno];
	      fprintf(stdout, "            cblk {\n");
	      fprintf(stdout,
		      "              x0=%d, y0=%d, x1=%d, y1=%d\n",
		      cblk->x0, cblk->y0, cblk->x1, cblk->y1);
	      fprintf(stdout, "            }\n");
	    }
	    fprintf(stdout, "          }\n");
	  }
	  fprintf(stdout, "        }\n");
	}
	fprintf(stdout, "      }\n");
      }
      fprintf(stdout, "    }\n");
    }
    fprintf(stdout, "  }\n");
  }
  fprintf(stdout, "}\n");
}

void tcd_malloc_encode(j2k_image_t * img, j2k_cp_t * cp, int curtileno)
{
  int tileno, compno, resno, bandno, precno, cblkno;
  tcd_img = img;
  tcd_cp = cp;
  tcd_image.tw = cp->tw;
  tcd_image.th = cp->th;
  tcd_image.tiles = (tcd_tile_t *) malloc(sizeof(tcd_tile_t));

  for (tileno = 0; tileno < 1; tileno++) {
    j2k_tcp_t *tcp = &cp->tcps[curtileno];
    int j;
    /* cfr p59 ISO/IEC FDIS15444-1 : 2000 (18 august 2000) */
    int p = curtileno % cp->tw;	/* si numerotation matricielle .. */
    int q = curtileno / cp->tw;	/* .. coordonnees de la tile (q,p) q pour ligne et p pour colonne */
    /* tcd_tile_t *tile=&tcd_image.tiles[tileno]; */
    tile = tcd_image.tiles;
    /* 4 borders of the tile rescale on the image if necessary */
    tile->x0 = int_max(cp->tx0 + p * cp->tdx, img->x0);
    tile->y0 = int_max(cp->ty0 + q * cp->tdy, img->y0);
    tile->x1 = int_min(cp->tx0 + (p + 1) * cp->tdx, img->x1);
    tile->y1 = int_min(cp->ty0 + (q + 1) * cp->tdy, img->y1);
    tile->numcomps = img->numcomps;
    /* tile->PPT=img->PPT;  */
    /* Modification of the RATE >> */
    for (j = 0; j < tcp->numlayers; j++) {
      tcp->rates[j] = tcp->rates[j] ? int_ceildiv(tile->numcomps * (tile->x1 - tile->x0) * (tile->y1 - tile->y0) * img->comps[0].prec, (tcp->rates[j] * 8 * img->comps[0].dx * img->comps[0].dy)) : 0;   /*Mod antonin losslessbug*/
      if (tcp->rates[j]) {
	if (j && tcp->rates[j] < tcp->rates[j - 1] + 10) {
	  tcp->rates[j] = tcp->rates[j - 1] + 20;
	} else {
	  if (!j && tcp->rates[j] < 30)
	    tcp->rates[j] = 30;
	}
      }
    }
    /* << Modification of the RATE */

    tile->comps =
      (tcd_tilecomp_t *) malloc(img->numcomps * sizeof(tcd_tilecomp_t));
    for (compno = 0; compno < tile->numcomps; compno++) {
      j2k_tccp_t *tccp = &tcp->tccps[compno];
      /* tcd_tilecomp_t *tilec=&tile->comps[compno]; */
      tilec = &tile->comps[compno];
      /* border of each tile component (global) */
      tilec->x0 = int_ceildiv(tile->x0, img->comps[compno].dx);

      tilec->y0 = int_ceildiv(tile->y0, img->comps[compno].dy);
      tilec->x1 = int_ceildiv(tile->x1, img->comps[compno].dx);
      tilec->y1 = int_ceildiv(tile->y1, img->comps[compno].dy);

      tilec->data =
	(int *) malloc((tilec->x1 - tilec->x0) *
		       (tilec->y1 - tilec->y0) * sizeof(int));
      tilec->numresolutions = tccp->numresolutions;

      tilec->resolutions =
	(tcd_resolution_t *) malloc(tilec->numresolutions *
				    sizeof(tcd_resolution_t));

      for (resno = 0; resno < tilec->numresolutions; resno++) {
	int pdx, pdy;
	int levelno = tilec->numresolutions - 1 - resno;
	int tlprcxstart, tlprcystart, brprcxend, brprcyend;
	int tlcbgxstart, tlcbgystart, brcbgxend, brcbgyend;
	int cbgwidthexpn, cbgheightexpn;
	int cblkwidthexpn, cblkheightexpn;
	/* tcd_resolution_t *res=&tilec->resolutions[resno]; */

	res = &tilec->resolutions[resno];

	/* border for each resolution level (global) */
	res->x0 = int_ceildivpow2(tilec->x0, levelno);
	res->y0 = int_ceildivpow2(tilec->y0, levelno);
	res->x1 = int_ceildivpow2(tilec->x1, levelno);
	res->y1 = int_ceildivpow2(tilec->y1, levelno);

	res->numbands = resno == 0 ? 1 : 3;
	/* p. 35, table A-23, ISO/IEC FDIS154444-1 : 2000 (18 august 2000) */
	if (tccp->csty & J2K_CCP_CSTY_PRT) {
	  pdx = tccp->prcw[resno];
	  pdy = tccp->prch[resno];
	} else {
	  pdx = 15;
	  pdy = 15;
	}
	/* p. 64, B.6, ISO/IEC FDIS15444-1 : 2000 (18 august 2000)  */
	tlprcxstart = int_floordivpow2(res->x0, pdx) << pdx;
	tlprcystart = int_floordivpow2(res->y0, pdy) << pdy;
	brprcxend = int_ceildivpow2(res->x1, pdx) << pdx;
	brprcyend = int_ceildivpow2(res->y1, pdy) << pdy;

	res->pw = (brprcxend - tlprcxstart) >> pdx;
	res->ph = (brprcyend - tlprcystart) >> pdy;

	if (resno == 0) {
	  tlcbgxstart = tlprcxstart;
	  tlcbgystart = tlprcystart;
	  brcbgxend = brprcxend;
	  brcbgyend = brprcyend;
	  cbgwidthexpn = pdx;
	  cbgheightexpn = pdy;
	} else {
	  tlcbgxstart = int_ceildivpow2(tlprcxstart, 1);
	  tlcbgystart = int_ceildivpow2(tlprcystart, 1);
	  brcbgxend = int_ceildivpow2(brprcxend, 1);
	  brcbgyend = int_ceildivpow2(brprcyend, 1);
	  cbgwidthexpn = pdx - 1;
	  cbgheightexpn = pdy - 1;
	}

	cblkwidthexpn = int_min(tccp->cblkw, cbgwidthexpn);
	cblkheightexpn = int_min(tccp->cblkh, cbgheightexpn);

	for (bandno = 0; bandno < res->numbands; bandno++) {
	  int x0b, y0b, i;
	  int gain, numbps;
	  j2k_stepsize_t *ss;
	  band = &res->bands[bandno];
	  band->bandno = resno == 0 ? 0 : bandno + 1;
	  x0b = (band->bandno == 1) || (band->bandno == 3) ? 1 : 0;
	  y0b = (band->bandno == 2) || (band->bandno == 3) ? 1 : 0;

	  if (band->bandno == 0) {
	    /* band border (global) */
	    band->x0 = int_ceildivpow2(tilec->x0, levelno);
	    band->y0 = int_ceildivpow2(tilec->y0, levelno);
	    band->x1 = int_ceildivpow2(tilec->x1, levelno);
	    band->y1 = int_ceildivpow2(tilec->y1, levelno);
	  } else {
	    /* band border (global) */
	    band->x0 =
	      int_ceildivpow2(tilec->x0 -
			      (1 << levelno) * x0b, levelno + 1);
	    band->y0 =
	      int_ceildivpow2(tilec->y0 -
			      (1 << levelno) * y0b, levelno + 1);
	    band->x1 =
	      int_ceildivpow2(tilec->x1 -
			      (1 << levelno) * x0b, levelno + 1);
	    band->y1 =
	      int_ceildivpow2(tilec->y1 -
			      (1 << levelno) * y0b, levelno + 1);

	  }

	  ss = &tccp->stepsizes[resno ==
				0 ? 0 : 3 * (resno - 1) + bandno + 1];
	  gain =
	    tccp->qmfbid ==
	    0 ? dwt_getgain_real(band->bandno) : dwt_getgain(band->bandno);
	  numbps = img->comps[compno].prec + gain;
   band->stepsize = (float)((1.0 + ss->mant / 2048.0) * pow(2.0, numbps - ss->expn));
	  band->numbps = ss->expn + tccp->numgbits - 1;	/* WHY -1 ? */

	  band->precincts =
	    (tcd_precinct_t *) malloc(3 * res->pw * res->ph *
				      sizeof(tcd_precinct_t));

	  for (i = 0; i < res->pw * res->ph * 3; i++) {
	    band->precincts[i].imsbtree = NULL;
	    band->precincts[i].incltree = NULL;
	  }

	  for (precno = 0; precno < res->pw * res->ph; precno++) {
	    int tlcblkxstart, tlcblkystart, brcblkxend, brcblkyend;
	    int cbgxstart =
	      tlcbgxstart + (precno % res->pw) * (1 << cbgwidthexpn);
	    int cbgystart =
	      tlcbgystart + (precno / res->pw) * (1 << cbgheightexpn);
	    int cbgxend = cbgxstart + (1 << cbgwidthexpn);
	    int cbgyend = cbgystart + (1 << cbgheightexpn);
	    /* tcd_precinct_t *prc=&band->precincts[precno]; */
	    prc = &band->precincts[precno];
	    /* precinct size (global) */
	    prc->x0 = int_max(cbgxstart, band->x0);
	    prc->y0 = int_max(cbgystart, band->y0);
	    prc->x1 = int_min(cbgxend, band->x1);
	    prc->y1 = int_min(cbgyend, band->y1);

	    tlcblkxstart =
	      int_floordivpow2(prc->x0, cblkwidthexpn) << cblkwidthexpn;
	    tlcblkystart =
	      int_floordivpow2(prc->y0, cblkheightexpn) << cblkheightexpn;
	    brcblkxend =
	      int_ceildivpow2(prc->x1, cblkwidthexpn) << cblkwidthexpn;
	    brcblkyend =
	      int_ceildivpow2(prc->y1, cblkheightexpn) << cblkheightexpn;
	    prc->cw = (brcblkxend - tlcblkxstart) >> cblkwidthexpn;
	    prc->ch = (brcblkyend - tlcblkystart) >> cblkheightexpn;

	    prc->cblks =
	      (tcd_cblk_t *) malloc((prc->cw * prc->ch) *
				    sizeof(tcd_cblk_t));
	    prc->incltree = tgt_create(prc->cw, prc->ch);
	    prc->imsbtree = tgt_create(prc->cw, prc->ch);

	    for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
	      int cblkxstart =
		tlcblkxstart + (cblkno % prc->cw) * (1 << cblkwidthexpn);
	      int cblkystart =
		tlcblkystart + (cblkno / prc->cw) * (1 << cblkheightexpn);
	      int cblkxend = cblkxstart + (1 << cblkwidthexpn);
	      int cblkyend = cblkystart + (1 << cblkheightexpn);

	      cblk = &prc->cblks[cblkno];
	      /* code-block size (global) */
	      cblk->x0 = int_max(cblkxstart, prc->x0);
	      cblk->y0 = int_max(cblkystart, prc->y0);
	      cblk->x1 = int_min(cblkxend, prc->x1);
	      cblk->y1 = int_min(cblkyend, prc->y1);
	    }
	  }
	}
      }
    }
  }
  /* tcd_dump(&tcd_image,curtileno); */
}

void tcd_free_encode(j2k_image_t * img, j2k_cp_t * cp, int curtileno)
{
  int tileno, compno, resno, bandno, precno;
  (void)curtileno;
  tcd_img = img;
  tcd_cp = cp;
  tcd_image.tw = cp->tw;
  tcd_image.th = cp->th;
  for (tileno = 0; tileno < 1; tileno++) {
    /* j2k_tcp_t *tcp=&cp->tcps[curtileno]; */
    tile = tcd_image.tiles;
    for (compno = 0; compno < tile->numcomps; compno++) {
      tilec = &tile->comps[compno];
      for (resno = 0; resno < tilec->numresolutions; resno++) {
	res = &tilec->resolutions[resno];
	for (bandno = 0; bandno < res->numbands; bandno++) {
	  band = &res->bands[bandno];
	  for (precno = 0; precno < res->pw * res->ph; precno++) {
	    prc = &band->precincts[precno];

	    if (prc->incltree != NULL)
	      tgt_destroy(prc->incltree);
	    if (prc->imsbtree != NULL)
	      tgt_destroy(prc->imsbtree);
	    free(prc->cblks);
	  }			/* for (precno */
	  free(band->precincts);
	}			/* for (bandno */
      }				/* for (resno */
      free(tilec->resolutions);
    }				/* for (compno */
    free(tile->comps);
  }				/* for (tileno */
  free(tcd_image.tiles);
}

void tcd_init_encode(j2k_image_t * img, j2k_cp_t * cp, int curtileno)
{
  int tileno, compno, resno, bandno, precno, cblkno;

  for (tileno = 0; tileno < 1; tileno++) {
    j2k_tcp_t *tcp = &cp->tcps[curtileno];
    int j;
    /*              int previous_x0, previous_x1, previous_y0, previous_y1;*/
    /* cfr p59 ISO/IEC FDIS15444-1 : 2000 (18 august 2000) */
    int p = curtileno % cp->tw;
    int q = curtileno / cp->tw;
    tile = tcd_image.tiles;

    /* 4 borders of the tile rescale on the image if necessary */
    tile->x0 = int_max(cp->tx0 + p * cp->tdx, img->x0);
    tile->y0 = int_max(cp->ty0 + q * cp->tdy, img->y0);
    tile->x1 = int_min(cp->tx0 + (p + 1) * cp->tdx, img->x1);
    tile->y1 = int_min(cp->ty0 + (q + 1) * cp->tdy, img->y1);

    tile->numcomps = img->numcomps;
    /* tile->PPT=img->PPT; */
    /* Modification of the RATE >> */
    for (j = 0; j < tcp->numlayers; j++) {
      tcp->rates[j] = tcp->rates[j] ? int_ceildiv(tile->numcomps * (tile->x1 - tile->x0) * (tile->y1 - tile->y0) * img->comps[0].prec, (tcp->rates[j] * 8 * img->comps[0].dx * img->comps[0].dy)) : 0;   /*Mod antonin losslessbug*/
      if (tcp->rates[j]) {
	if (j && tcp->rates[j] < tcp->rates[j - 1] + 10) {
	  tcp->rates[j] = tcp->rates[j - 1] + 20;
	} else {
	  if (!j && tcp->rates[j] < 30)
	    tcp->rates[j] = 30;
	}
      }
    }
    /* << Modification of the RATE */
    /* tile->comps=(tcd_tilecomp_t*)realloc(tile->comps,img->numcomps*sizeof(tcd_tilecomp_t)); */
    for (compno = 0; compno < tile->numcomps; compno++) {
      j2k_tccp_t *tccp = &tcp->tccps[compno];
      /* int realloc_op; */

      tilec = &tile->comps[compno];
      /* border of each tile component (global) */
      tilec->x0 = int_ceildiv(tile->x0, img->comps[compno].dx);
      tilec->y0 = int_ceildiv(tile->y0, img->comps[compno].dy);
      tilec->x1 = int_ceildiv(tile->x1, img->comps[compno].dx);
      tilec->y1 = int_ceildiv(tile->y1, img->comps[compno].dy);

      tilec->data =
	(int *) malloc((tilec->x1 - tilec->x0) *
		       (tilec->y1 - tilec->y0) * sizeof(int));
      tilec->numresolutions = tccp->numresolutions;
      /* tilec->resolutions=(tcd_resolution_t*)realloc(tilec->resolutions,tilec->numresolutions*sizeof(tcd_resolution_t)); */
      for (resno = 0; resno < tilec->numresolutions; resno++) {
	int pdx, pdy;
	int levelno = tilec->numresolutions - 1 - resno;
	int tlprcxstart, tlprcystart, brprcxend, brprcyend;
	int tlcbgxstart, tlcbgystart, brcbgxend, brcbgyend;
	int cbgwidthexpn, cbgheightexpn;
	int cblkwidthexpn, cblkheightexpn;

	res = &tilec->resolutions[resno];
	/* border for each resolution level (global) */
	res->x0 = int_ceildivpow2(tilec->x0, levelno);
	res->y0 = int_ceildivpow2(tilec->y0, levelno);
	res->x1 = int_ceildivpow2(tilec->x1, levelno);
	res->y1 = int_ceildivpow2(tilec->y1, levelno);

	res->numbands = resno == 0 ? 1 : 3;
	/* p. 35, table A-23, ISO/IEC FDIS154444-1 : 2000 (18 august 2000) */
	if (tccp->csty & J2K_CCP_CSTY_PRT) {
	  pdx = tccp->prcw[resno];
	  pdy = tccp->prch[resno];
	} else {
	  pdx = 15;
	  pdy = 15;
	}
	/* p. 64, B.6, ISO/IEC FDIS15444-1 : 2000 (18 august 2000)  */
	tlprcxstart = int_floordivpow2(res->x0, pdx) << pdx;
	tlprcystart = int_floordivpow2(res->y0, pdy) << pdy;
	brprcxend = int_ceildivpow2(res->x1, pdx) << pdx;
	brprcyend = int_ceildivpow2(res->y1, pdy) << pdy;

	res->pw = (brprcxend - tlprcxstart) >> pdx;
	res->ph = (brprcyend - tlprcystart) >> pdy;

	if (resno == 0) {
	  tlcbgxstart = tlprcxstart;
	  tlcbgystart = tlprcystart;
	  brcbgxend = brprcxend;
	  brcbgyend = brprcyend;
	  cbgwidthexpn = pdx;
	  cbgheightexpn = pdy;
	} else {
	  tlcbgxstart = int_ceildivpow2(tlprcxstart, 1);
	  tlcbgystart = int_ceildivpow2(tlprcystart, 1);
	  brcbgxend = int_ceildivpow2(brprcxend, 1);
	  brcbgyend = int_ceildivpow2(brprcyend, 1);
	  cbgwidthexpn = pdx - 1;
	  cbgheightexpn = pdy - 1;
	}

	cblkwidthexpn = int_min(tccp->cblkw, cbgwidthexpn);
	cblkheightexpn = int_min(tccp->cblkh, cbgheightexpn);

	for (bandno = 0; bandno < res->numbands; bandno++) {
	  int x0b, y0b;
	  int gain, numbps;
	  j2k_stepsize_t *ss;
	  band = &res->bands[bandno];
	  band->bandno = resno == 0 ? 0 : bandno + 1;
	  x0b = (band->bandno == 1) || (band->bandno == 3) ? 1 : 0;
	  y0b = (band->bandno == 2) || (band->bandno == 3) ? 1 : 0;

	  if (band->bandno == 0) {
	    /* band border */
	    band->x0 = int_ceildivpow2(tilec->x0, levelno);
	    band->y0 = int_ceildivpow2(tilec->y0, levelno);
	    band->x1 = int_ceildivpow2(tilec->x1, levelno);
	    band->y1 = int_ceildivpow2(tilec->y1, levelno);
	  } else {
	    band->x0 =
	      int_ceildivpow2(tilec->x0 -
			      (1 << levelno) * x0b, levelno + 1);
	    band->y0 =
	      int_ceildivpow2(tilec->y0 -
			      (1 << levelno) * y0b, levelno + 1);
	    band->x1 =
	      int_ceildivpow2(tilec->x1 -
			      (1 << levelno) * x0b, levelno + 1);
	    band->y1 =
	      int_ceildivpow2(tilec->y1 -
			      (1 << levelno) * y0b, levelno + 1);
	  }

	  ss = &tccp->stepsizes[resno ==
				0 ? 0 : 3 * (resno - 1) + bandno + 1];
	  gain =
	    tccp->qmfbid ==
	    0 ? dwt_getgain_real(band->bandno) : dwt_getgain(band->bandno);
	  numbps = img->comps[compno].prec + gain;
          band->stepsize = (float)((1.0 + ss->mant / 2048.0) * pow(2.0, numbps - ss->expn));
	  band->numbps = ss->expn + tccp->numgbits - 1;	/* WHY -1 ? */

	  for (precno = 0; precno < res->pw * res->ph; precno++) {
	    int tlcblkxstart, tlcblkystart, brcblkxend, brcblkyend;
	    int cbgxstart =
	      tlcbgxstart + (precno % res->pw) * (1 << cbgwidthexpn);
	    int cbgystart =
	      tlcbgystart + (precno / res->pw) * (1 << cbgheightexpn);
	    int cbgxend = cbgxstart + (1 << cbgwidthexpn);
	    int cbgyend = cbgystart + (1 << cbgheightexpn);

	    prc = &band->precincts[precno];
	    /* precinct size (global) */
	    prc->x0 = int_max(cbgxstart, band->x0);
	    prc->y0 = int_max(cbgystart, band->y0);
	    prc->x1 = int_min(cbgxend, band->x1);
	    prc->y1 = int_min(cbgyend, band->y1);

	    tlcblkxstart =
	      int_floordivpow2(prc->x0, cblkwidthexpn) << cblkwidthexpn;
	    tlcblkystart =
	      int_floordivpow2(prc->y0, cblkheightexpn) << cblkheightexpn;
	    brcblkxend =
	      int_ceildivpow2(prc->x1, cblkwidthexpn) << cblkwidthexpn;
	    brcblkyend =
	      int_ceildivpow2(prc->y1, cblkheightexpn) << cblkheightexpn;
	    prc->cw = (brcblkxend - tlcblkxstart) >> cblkwidthexpn;
	    prc->ch = (brcblkyend - tlcblkystart) >> cblkheightexpn;

	    free(prc->cblks);
	    prc->cblks =
	      (tcd_cblk_t *) malloc(prc->cw * prc->ch *
				    sizeof(tcd_cblk_t));

	    if (prc->incltree != NULL)
	      tgt_destroy(prc->incltree);
	    if (prc->imsbtree != NULL)
	      tgt_destroy(prc->imsbtree);

	    prc->incltree = tgt_create(prc->cw, prc->ch);
	    prc->imsbtree = tgt_create(prc->cw, prc->ch);

	    for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
	      int cblkxstart =
		tlcblkxstart + (cblkno % prc->cw) * (1 << cblkwidthexpn);
	      int cblkystart =
		tlcblkystart + (cblkno / prc->cw) * (1 << cblkheightexpn);
	      int cblkxend = cblkxstart + (1 << cblkwidthexpn);
	      int cblkyend = cblkystart + (1 << cblkheightexpn);
	      cblk = &prc->cblks[cblkno];

	      /* code-block size (global) */
	      cblk->x0 = int_max(cblkxstart, prc->x0);
	      cblk->y0 = int_max(cblkystart, prc->y0);
	      cblk->x1 = int_min(cblkxend, prc->x1);
	      cblk->y1 = int_min(cblkyend, prc->y1);

	    }
	  }
	}
      }
    }
  }
  /* tcd_dump(&tcd_image,0); */
}

void tcd_init(j2k_image_t * img, j2k_cp_t * cp)
{
  int tileno, compno, resno, bandno, precno, cblkno, i, j, p, q;
  unsigned int x0 = 0, y0 = 0, x1 = 0, y1 = 0, w, h;
  tcd_img = img;
  tcd_cp = cp;
  tcd_image.tw = cp->tw;
  tcd_image.th = cp->th;
  tcd_image.tiles =
    (tcd_tile_t *) malloc(cp->tw * cp->th * sizeof(tcd_tile_t));

  /*for (tileno = 0; tileno < cp->tw * cp->th; tileno++) {
     j2k_tcp_t *tcp = &cp->tcps[tileno];
     tcd_tile_t *tile = &tcd_image.tiles[tileno]; */

  for (i = 0; i < cp->tileno_size; i++) {
    j2k_tcp_t *tcp = &cp->tcps[cp->tileno[i]];
    tcd_tile_t *tile = &tcd_image.tiles[cp->tileno[i]];
    tileno = cp->tileno[i];


    /*              int previous_x0, previous_x1, previous_y0, previous_y1;*/
    /* cfr p59 ISO/IEC FDIS15444-1 : 2000 (18 august 2000) */
    p = tileno % cp->tw;	/* si numerotation matricielle .. */
    q = tileno / cp->tw;	/* .. coordonnees de la tile (q,p) q pour ligne et p pour colonne */

    /* 4 borders of the tile rescale on the image if necessary */
    tile->x0 = int_max(cp->tx0 + p * cp->tdx, img->x0);
    tile->y0 = int_max(cp->ty0 + q * cp->tdy, img->y0);
    tile->x1 = int_min(cp->tx0 + (p + 1) * cp->tdx, img->x1);
    tile->y1 = int_min(cp->ty0 + (q + 1) * cp->tdy, img->y1);

    tile->numcomps = img->numcomps;
    tile->comps =
      (tcd_tilecomp_t *) malloc(img->numcomps * sizeof(tcd_tilecomp_t));
    for (compno = 0; compno < tile->numcomps; compno++) {
      j2k_tccp_t *tccp = &tcp->tccps[compno];
      tcd_tilecomp_t *tilec = &tile->comps[compno];
      /* border of each tile component (global) */
      tilec->x0 = int_ceildiv(tile->x0, img->comps[compno].dx);
      tilec->y0 = int_ceildiv(tile->y0, img->comps[compno].dy);
      tilec->x1 = int_ceildiv(tile->x1, img->comps[compno].dx);
      tilec->y1 = int_ceildiv(tile->y1, img->comps[compno].dy);

      tilec->data =
	(int *) malloc((tilec->x1 - tilec->x0) *
		       (tilec->y1 - tilec->y0) * sizeof(int));
      tilec->numresolutions = tccp->numresolutions;
      tilec->resolutions =
	(tcd_resolution_t *) malloc(tilec->numresolutions *
				    sizeof(tcd_resolution_t));
      for (resno = 0; resno < tilec->numresolutions; resno++) {
	int pdx, pdy;
	int levelno = tilec->numresolutions - 1 - resno;
	int tlprcxstart, tlprcystart, brprcxend, brprcyend;
	int tlcbgxstart, tlcbgystart, brcbgxend, brcbgyend;
	int cbgwidthexpn, cbgheightexpn;
	int cblkwidthexpn, cblkheightexpn;
	tcd_resolution_t *res = &tilec->resolutions[resno];

	/* border for each resolution level (global) */
	res->x0 = int_ceildivpow2(tilec->x0, levelno);
	res->y0 = int_ceildivpow2(tilec->y0, levelno);
	res->x1 = int_ceildivpow2(tilec->x1, levelno);
	res->y1 = int_ceildivpow2(tilec->y1, levelno);

	res->numbands = resno == 0 ? 1 : 3;
	/* p. 35, table A-23, ISO/IEC FDIS154444-1 : 2000 (18 august 2000) */
	if (tccp->csty & J2K_CCP_CSTY_PRT) {
	  pdx = tccp->prcw[resno];
	  pdy = tccp->prch[resno];
	} else {
	  pdx = 15;
	  pdy = 15;
	}
	/* p. 64, B.6, ISO/IEC FDIS15444-1 : 2000 (18 august 2000)  */
	tlprcxstart = int_floordivpow2(res->x0, pdx) << pdx;
	tlprcystart = int_floordivpow2(res->y0, pdy) << pdy;
	brprcxend = int_ceildivpow2(res->x1, pdx) << pdx;
	brprcyend = int_ceildivpow2(res->y1, pdy) << pdy;
 res->pw = (res->x0 == res->x1) ? 0 : ((brprcxend - tlprcxstart) >> pdx);   /* Mod Antonin : sizebug1*/
 res->ph = (res->y0 == res->y1) ? 0 : ((brprcyend - tlprcystart) >> pdy);   /* Mod Antonin : sizebug1*/

	if (resno == 0) {
	  tlcbgxstart = tlprcxstart;
	  tlcbgystart = tlprcystart;
	  brcbgxend = brprcxend;
	  brcbgyend = brprcyend;
	  cbgwidthexpn = pdx;
	  cbgheightexpn = pdy;
	} else {
	  tlcbgxstart = int_ceildivpow2(tlprcxstart, 1);
	  tlcbgystart = int_ceildivpow2(tlprcystart, 1);
	  brcbgxend = int_ceildivpow2(brprcxend, 1);
	  brcbgyend = int_ceildivpow2(brprcyend, 1);
	  cbgwidthexpn = pdx - 1;
	  cbgheightexpn = pdy - 1;
	}

	cblkwidthexpn = int_min(tccp->cblkw, cbgwidthexpn);
	cblkheightexpn = int_min(tccp->cblkh, cbgheightexpn);

	for (bandno = 0; bandno < res->numbands; bandno++) {
	  int x0b, y0b;
	  int gain, numbps;
	  j2k_stepsize_t *ss;
	  tcd_band_t *band = &res->bands[bandno];
	  band->bandno = resno == 0 ? 0 : bandno + 1;
	  x0b = (band->bandno == 1) || (band->bandno == 3) ? 1 : 0;
	  y0b = (band->bandno == 2) || (band->bandno == 3) ? 1 : 0;

	  if (band->bandno == 0) {
	    /* band border (global) */
	    band->x0 = int_ceildivpow2(tilec->x0, levelno);
	    band->y0 = int_ceildivpow2(tilec->y0, levelno);
	    band->x1 = int_ceildivpow2(tilec->x1, levelno);
	    band->y1 = int_ceildivpow2(tilec->y1, levelno);
	  } else {
	    /* band border (global) */
	    band->x0 =
	      int_ceildivpow2(tilec->x0 -
			      (1 << levelno) * x0b, levelno + 1);
	    band->y0 =
	      int_ceildivpow2(tilec->y0 -
			      (1 << levelno) * y0b, levelno + 1);
	    band->x1 =
	      int_ceildivpow2(tilec->x1 -
			      (1 << levelno) * x0b, levelno + 1);
	    band->y1 =
	      int_ceildivpow2(tilec->y1 -
			      (1 << levelno) * y0b, levelno + 1);
	  }

	  ss = &tccp->stepsizes[resno ==
				0 ? 0 : 3 * (resno - 1) + bandno + 1];
          gain =
	    tccp->qmfbid ==
	    0 ? dwt_getgain_real(band->bandno) : dwt_getgain(band->bandno);
	  numbps = img->comps[compno].prec + gain;
          band->stepsize = (float)((1.0 + ss->mant / 2048.0) * pow(2.0, numbps - ss->expn));
	  band->numbps = ss->expn + tccp->numgbits - 1;	/* WHY -1 ? */

	  band->precincts =
	    (tcd_precinct_t *) malloc(res->pw * res->ph *
				      sizeof(tcd_precinct_t));

	  for (precno = 0; precno < res->pw * res->ph; precno++) {
	    int tlcblkxstart, tlcblkystart, brcblkxend, brcblkyend;
	    int cbgxstart =
	      tlcbgxstart + (precno % res->pw) * (1 << cbgwidthexpn);
	    int cbgystart =
	      tlcbgystart + (precno / res->pw) * (1 << cbgheightexpn);
	    int cbgxend = cbgxstart + (1 << cbgwidthexpn);
	    int cbgyend = cbgystart + (1 << cbgheightexpn);
	    tcd_precinct_t *prc = &band->precincts[precno];
	    /* precinct size (global) */
	    prc->x0 = int_max(cbgxstart, band->x0);
	    prc->y0 = int_max(cbgystart, band->y0);
	    prc->x1 = int_min(cbgxend, band->x1);
	    prc->y1 = int_min(cbgyend, band->y1);

	    tlcblkxstart =
	      int_floordivpow2(prc->x0, cblkwidthexpn) << cblkwidthexpn;
	    tlcblkystart =
	      int_floordivpow2(prc->y0, cblkheightexpn) << cblkheightexpn;
	    brcblkxend =
	      int_ceildivpow2(prc->x1, cblkwidthexpn) << cblkwidthexpn;
	    brcblkyend =
	      int_ceildivpow2(prc->y1, cblkheightexpn) << cblkheightexpn;
	    prc->cw = (brcblkxend - tlcblkxstart) >> cblkwidthexpn;
	    prc->ch = (brcblkyend - tlcblkystart) >> cblkheightexpn;

	    prc->cblks =
	      (tcd_cblk_t *) malloc(prc->cw * prc->ch *
				    sizeof(tcd_cblk_t));

	    prc->incltree = tgt_create(prc->cw, prc->ch);
	    prc->imsbtree = tgt_create(prc->cw, prc->ch);

	    for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
	      int cblkxstart =
		tlcblkxstart + (cblkno % prc->cw) * (1 << cblkwidthexpn);
	      int cblkystart =
		tlcblkystart + (cblkno / prc->cw) * (1 << cblkheightexpn);
	      int cblkxend = cblkxstart + (1 << cblkwidthexpn);
	      int cblkyend = cblkystart + (1 << cblkheightexpn);
	      tcd_cblk_t *cblk = &prc->cblks[cblkno];
	      /* code-block size (global) */
	      cblk->x0 = int_max(cblkxstart, prc->x0);
	      cblk->y0 = int_max(cblkystart, prc->y0);
	      cblk->x1 = int_min(cblkxend, prc->x1);
	      cblk->y1 = int_min(cblkyend, prc->y1);
	    }
	  }
	}
      }
    }
  }
  /*tcd_dump(&tcd_image,0);*/


  /* Allocate place to store the data decoded = final image */
  /* Place limited by the tile really present in the codestream */


  for (i = 0; i < img->numcomps; i++) {
    for (j = 0; j < cp->tileno_size; j++) {
      tileno = cp->tileno[j];
      x0 = j == 0 ? tcd_image.tiles[tileno].comps[i].x0 : int_min(x0,
								  (unsigned int) 
								  tcd_image.
								  tiles
								  [tileno].
								  comps
								  [i].x0);
      y0 =
	j == 0 ? tcd_image.tiles[tileno].comps[i].y0 : int_min(y0,
							       (unsigned int) 
							       tcd_image.
							       tiles
							       [tileno].
							       comps[i].
							       y0);
      x1 =
	j == 0 ? tcd_image.tiles[tileno].comps[i].x1 : int_max(x1,
							       (unsigned int) 
							       tcd_image.
							       tiles
							       [tileno].
							       comps[i].
							       x1);
      y1 =
	j == 0 ? tcd_image.tiles[tileno].comps[i].y1 : int_max(y1,
							       (unsigned int) 
							       tcd_image.
							       tiles
							       [tileno].
							       comps[i].
							       y1);
    }

    w = x1 - x0;

    h = y1 - y0;
    img->comps[i].data = (int *) calloc(w * h, sizeof(int));
    img->comps[i].w = w;
    img->comps[i].h = h;
    img->comps[i].x0 = x0;
    img->comps[i].y0 = y0;
  }
}

void tcd_makelayer_fixed(int layno, int final)
{
  int compno, resno, bandno, precno, cblkno;
  int value;         /*, matrice[tcd_tcp->numlayers][tcd_tile->comps[0].numresolutions][3];*/
  int matrice[10][10][3];
  int i, j, k;

  /*matrice=(int*)malloc(tcd_tcp->numlayers*tcd_tile->comps[0].numresolutions*3*sizeof(int)); */

  for (compno = 0; compno < tcd_tile->numcomps; compno++) {
    tcd_tilecomp_t *tilec = &tcd_tile->comps[compno];
    for (i = 0; i < tcd_tcp->numlayers; i++) {
      for (j = 0; j < tilec->numresolutions; j++) {
	for (k = 0; k < 3; k++) {
	  matrice[i][j][k] =
	    (int) (tcd_cp->
		   matrice[i * tilec->numresolutions * 3 +
			   j * 3 +
			   k] *
		   (float) (tcd_img->comps[compno].prec / 16.0));
    }}}

    for (resno = 0; resno < tilec->numresolutions; resno++) {
      tcd_resolution_t *res = &tilec->resolutions[resno];
      for (bandno = 0; bandno < res->numbands; bandno++) {
	tcd_band_t *band = &res->bands[bandno];
	for (precno = 0; precno < res->pw * res->ph; precno++) {
	  tcd_precinct_t *prc = &band->precincts[precno];
	  for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
	    tcd_cblk_t *cblk = &prc->cblks[cblkno];
	    tcd_layer_t *layer = &cblk->layers[layno];
	    int n;
	    int imsb = tcd_img->comps[compno].prec - cblk->numbps;	/* number of bit-plan equal to zero */
	    /* Correction of the matrix of coefficient to include the IMSB information */

	    if (layno == 0) {
	      value = matrice[layno][resno][bandno];
	      if (imsb >= value)
		value = 0;
	      else
		value -= imsb;
	    } else {
	      value =
		matrice[layno][resno][bandno] -
		matrice[layno - 1][resno][bandno];
	      if (imsb >= matrice[layno - 1][resno][bandno]) {
		value -= (imsb - matrice[layno - 1][resno][bandno]);
		if (value < 0)
		  value = 0;
	      }
	    }

	    if (layno == 0)
	      cblk->numpassesinlayers = 0;

	    n = cblk->numpassesinlayers;
	    if (cblk->numpassesinlayers == 0) {
	      if (value != 0)
		n = 3 * value - 2 + cblk->numpassesinlayers;
	      else
		n = cblk->numpassesinlayers;
	    } else
	      n = 3 * value + cblk->numpassesinlayers;

	    layer->numpasses = n - cblk->numpassesinlayers;

	    if (!layer->numpasses)
	      continue;

	    if (cblk->numpassesinlayers == 0) {
	      layer->len = cblk->passes[n - 1].rate;
	      layer->data = cblk->data;
	    } else {
	      layer->len =
		cblk->passes[n - 1].rate -
		cblk->passes[cblk->numpassesinlayers - 1].rate;
	      layer->data =
		cblk->data +
		cblk->passes[cblk->numpassesinlayers - 1].rate;
	    }
	    if (final)
	      cblk->numpassesinlayers = n;
	  }
	}
      }
    }
  }
}

void tcd_rateallocate_fixed()
{
  int layno;

  for (layno = 0; layno < tcd_tcp->numlayers; layno++) {
    tcd_makelayer_fixed(layno, 1);
  }
}

void tcd_makelayer(int layno, double thresh, int final)
{
  int compno, resno, bandno, precno, cblkno, passno;

  tcd_tile->distolayer[layno] = 0;   /*add fixed_quality*/

  for (compno = 0; compno < tcd_tile->numcomps; compno++) {
    tcd_tilecomp_t *tilec = &tcd_tile->comps[compno];
    for (resno = 0; resno < tilec->numresolutions; resno++) {
      tcd_resolution_t *res = &tilec->resolutions[resno];
      for (bandno = 0; bandno < res->numbands; bandno++) {
	tcd_band_t *band = &res->bands[bandno];
	for (precno = 0; precno < res->pw * res->ph; precno++) {
	  tcd_precinct_t *prc = &band->precincts[precno];
	  for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
	    tcd_cblk_t *cblk = &prc->cblks[cblkno];
	    tcd_layer_t *layer = &cblk->layers[layno];
	    int n;

	    if (layno == 0) {
	      cblk->numpassesinlayers = 0;
	    }
	    n = cblk->numpassesinlayers;
	    for (passno = cblk->numpassesinlayers;
		 passno < cblk->totalpasses; passno++) {
	      int dr;
	      double dd;
	      tcd_pass_t *pass = &cblk->passes[passno];
	      if (n == 0) {
		dr = pass->rate;
		dd = pass->distortiondec;
	      } else {
		dr = pass->rate - cblk->passes[n - 1].rate;
		dd = pass->distortiondec - cblk->passes[n -
							1].distortiondec;
	      }
	      if (!dr) {
		if (dd)
		  n = passno + 1;
		continue;
	      }

	      if (dd / dr >= thresh)
		n = passno + 1;
	    }
	    layer->numpasses = n - cblk->numpassesinlayers;

	    if (!layer->numpasses) {
	      layer->disto = 0;
	      continue;
	    }

	    if (cblk->numpassesinlayers == 0) {
	      layer->len = cblk->passes[n - 1].rate;
	      layer->data = cblk->data;
	      layer->disto = cblk->passes[n - 1].distortiondec;
	    } else {
	      layer->len = cblk->passes[n - 1].rate -
		cblk->passes[cblk->numpassesinlayers - 1].rate;
	      layer->data =
		cblk->data +
		cblk->passes[cblk->numpassesinlayers - 1].rate;
	      layer->disto =
		cblk->passes[n - 1].distortiondec -
		cblk->passes[cblk->numpassesinlayers - 1].distortiondec;
	    }

     tcd_tile->distolayer[layno] += layer->disto;   /*add fixed_quality*/

	    if (final)
	      cblk->numpassesinlayers = n;
	  }
	}
      }
    }
  }
}

void tcd_rateallocate(unsigned char *dest, int len, info_image * info_IM)
{
  int compno, resno, bandno, precno, cblkno, passno, layno;
  double min, max;
  double cumdisto[100];      /*add fixed_quality*/
  const double K = 1;      /* 1.1; //add fixed_quality*/

  double maxSE = 0;
  min = DBL_MAX;
  max = 0;

  tcd_tile->nbpix = 0;      /*add fixed_quality*/

  for (compno = 0; compno < tcd_tile->numcomps; compno++) {
    tcd_tilecomp_t *tilec = &tcd_tile->comps[compno];

    tilec->nbpix = 0;
    for (resno = 0; resno < tilec->numresolutions; resno++) {
      tcd_resolution_t *res = &tilec->resolutions[resno];
      for (bandno = 0; bandno < res->numbands; bandno++) {
	tcd_band_t *band = &res->bands[bandno];
	for (precno = 0; precno < res->pw * res->ph; precno++) {
	  tcd_precinct_t *prc = &band->precincts[precno];
	  for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
	    tcd_cblk_t *cblk = &prc->cblks[cblkno];
	    for (passno = 0; passno < cblk->totalpasses; passno++) {
	      tcd_pass_t *pass = &cblk->passes[passno];
	      int dr;
	      double dd, rdslope;
	      if (passno == 0) {
		dr = pass->rate;
		dd = pass->distortiondec;
	      } else {
		dr = pass->rate - cblk->passes[passno - 1].rate;
		dd = pass->distortiondec -
		  cblk->passes[passno - 1].distortiondec;
	      }
	      if (dr == 0) {
		continue;
	      }

	      rdslope = dd / dr;

	      if (rdslope < min) {
		min = rdslope;
	      }
	      if (rdslope > max) {
		max = rdslope;
	      }
	    }			/* passno */

     tcd_tile->nbpix += ((cblk->x1 - cblk->x0) * (cblk->y1 - cblk->y0));   /*add fixed_quality*/

     tilec->nbpix += ((cblk->x1 - cblk->x0) * (cblk->y1 - cblk->y0));   /*add fixed_quality*/

	  }			/* cbklno */
	}			/* precno */
      }				/* bandno */
    }				/* resno */
    maxSE += (((double)(1 << tcd_img->comps[compno].prec) - 1.0) * ((double)(1 << tcd_img->comps[compno].prec) -1.0)) * ((double)(tilec->nbpix));
  }				/* compno */

  /* add antonin index */
  if (info_IM->index_on) {
    info_tile *info_TL = &info_IM->tile[tcd_tileno];
    info_TL->nbpix = tcd_tile->nbpix;
    info_TL->distotile = tcd_tile->distotile;
    info_TL->thresh =
      (double *) malloc(tcd_tcp->numlayers * sizeof(double));
  }
  /* dda */

  for (layno = 0; layno < tcd_tcp->numlayers; layno++) {
    volatile double lo = min;
    volatile double hi = max;
    volatile int success = 0;
    volatile int maxlen = tcd_tcp->rates[layno] ? int_min(tcd_tcp->rates[layno], len) : len;   /*Mod antonin losslessbug*/
    volatile double goodthresh;
    volatile int i;
    double distotarget;      /*add fixed_quality*/

    distotarget = tcd_tile->distotile - ((K * maxSE) / pow(10, tcd_tcp->distoratio[layno] / 10));   /* add fixed_quality*/
    
    if ((tcd_tcp->rates[layno]) || (tcd_cp->disto_alloc==0)) {
      for (i = 0; i < 32; i++) {
	volatile double thresh = (lo + hi) / 2;
	int l = 0;
 double distoachieved = 0;   /* add fixed_quality*/

	tcd_makelayer(layno, thresh, 0);

 if (tcd_cp->fixed_quality) {   /* add fixed_quality*/
	  distoachieved =
	    layno ==
	    0 ? tcd_tile->distolayer[0] : cumdisto[layno - 1] +
	    tcd_tile->distolayer[layno];
	  if (distoachieved < distotarget) {
	    hi = thresh;
	    continue;
	  }
	  lo = thresh;
	} else {
	  l =
	    t2_encode_packets(tcd_img, tcd_cp, tcd_tileno, tcd_tile,
			      layno + 1, dest, maxlen, info_IM);
	  /* fprintf(stderr, "rate alloc: len=%d, max=%d\n", l, maxlen); */
	  if (l == -999) {
	    lo = thresh;
	    continue;
	  }
	  hi = thresh;
	}

	success = 1;
	goodthresh = thresh;
      }
    } else {
      success = 1;
      goodthresh = min;
    }

    if (!success) {
      longjmp(j2k_error, 1);
    }

    if (info_IM->index_on) {	/* Threshold for Marcela Index */
      info_IM->tile[tcd_tileno].thresh[layno] = goodthresh;
    }
    tcd_makelayer(layno, goodthresh, 1);

    cumdisto[layno] = layno == 0 ? tcd_tile->distolayer[0] : cumdisto[layno - 1] + tcd_tile->distolayer[layno];   /* add fixed_quality*/
  }
}

int
tcd_encode_tile_pxm(int tileno, unsigned char *dest, int len,
		    info_image * info_IM)
{
  int compno;
  int l, i, npck=0;
  clock_t time7;
  tcd_tile_t *tile;
  j2k_tcp_t *tcp = &tcd_cp->tcps[0];
  j2k_tccp_t *tccp = &tcp->tccps[0];
  
  tcd_tileno = tileno;
  tcd_tile = tcd_image.tiles;
  tcd_tcp = &tcd_cp->tcps[tileno];
  tile = tcd_tile;
  /* INDEX >> "Precinct_nb_X et Precinct_nb_Y" */
  if (info_IM->index_on) {
    tcd_tilecomp_t *tilec_idx = &tile->comps[0];   /*Based on Component 0*/
    
    for (i = 0; i < tilec_idx->numresolutions; i++) {
      
      tcd_resolution_t *res_idx = &tilec_idx->resolutions[i];
      
      info_IM->tile[tileno].pw[i] = res_idx->pw;
      info_IM->tile[tileno].ph[i] = res_idx->ph;
      
      npck+=res_idx->pw * res_idx->ph;
      
      info_IM->tile[tileno].pdx[i] = tccp->prcw[i];
      info_IM->tile[tileno].pdy[i] = tccp->prch[i];
      
    }
    info_IM->tile[tileno].packet = (info_packet *) calloc(info_IM->Comp * info_IM->Layer * npck, sizeof(info_packet));
  }
  /* << INDEX */

/*---------------TILE-------------------*/

  time7 = clock();

  for (compno = 0; compno < tile->numcomps; compno++) {
    FILE *src;
    char tmp[256];
    int k;
    unsigned char elmt;
    int i, j;
    int tw, w;
    tcd_tilecomp_t *tilec = &tile->comps[compno];
    int adjust =
      tcd_img->comps[compno].sgnd ? 0 : 1 << (tcd_img->comps[compno].
					      prec - 1);
    int offset_x, offset_y;

    offset_x = int_ceildiv(tcd_img->x0, tcd_img->comps[compno].dx);
    offset_y = int_ceildiv(tcd_img->y0, tcd_img->comps[compno].dy);
    tw = tilec->x1 - tilec->x0;
    w = int_ceildiv(tcd_img->x1 - tcd_img->x0, tcd_img->comps[compno].dx);
    sprintf(tmp, "Compo%d", compno);	/* component file */
    src = fopen(tmp, "rb");
    if (!src) {
      fprintf(stderr, "failed to open %s for reading\n", tmp);
      return 1;
    }

    /* read the Compo file to extract data of the tile */
    k = 0;
    fseek(src, (tilec->x0 - offset_x) + (tilec->y0 - offset_y) * w,
	  SEEK_SET);
    k = (tilec->x0 - offset_x) + (tilec->y0 - offset_y) * w;
    for (j = tilec->y0; j < tilec->y1; j++) {
      for (i = tilec->x0; i < tilec->x1; i++) {
	if (tcd_tcp->tccps[compno].qmfbid == 1) {
	  elmt = fgetc(src);
	  tilec->data[i - tilec->x0 + (j - tilec->y0) * tw] =
	    elmt - adjust;
	  k++;
	} else if (tcd_tcp->tccps[compno].qmfbid == 0) {
	  elmt = fgetc(src);
	  tilec->data[i - tilec->x0 + (j - tilec->y0) * tw] =
	    (elmt - adjust) << 13;
	  k++;
	}
      }
      fseek(src, (tilec->x0 - offset_x) + (j + 1 - offset_y) * w - k,
	    SEEK_CUR);
      k = tilec->x0 - offset_x + (j + 1 - offset_y) * w;

    }
    fclose(src);
  }

/*----------------MCT-------------------*/

  if (tcd_tcp->mct) {
    if (tcd_tcp->tccps[0].qmfbid == 0) {
      mct_encode_real(tile->comps[0].data, tile->comps[1].data,
		      tile->comps[2].data,
		      (tile->comps[0].x1 -
		       tile->comps[0].x0) * (tile->comps[0].y1 -
					     tile->comps[0].y0));
    } else {
      mct_encode(tile->comps[0].data, tile->comps[1].data,
		 tile->comps[2].data,
		 (tile->comps[0].x1 -
		  tile->comps[0].x0) * (tile->comps[0].y1 -
					tile->comps[0].y0));
    }
  }
/*----------------DWT---------------------*/

/* mod Ive*/
for (compno = 0; compno < tile->numcomps; compno++) {
  tcd_tilecomp_t *tilec = &tile->comps[compno];
  if (tcd_tcp->tccps[compno].qmfbid == 1) {
    dwt_encode(tilec);
  } else if (tcd_tcp->tccps[compno].qmfbid == 0) {
    dwt_encode_real(tilec);
  }
}
/* /mod Ive*/
/*------------------TIER1-----------------*/

  t1_init_luts();
  t1_encode_cblks(tile, tcd_tcp);

/*-----------RATE-ALLOCATE------------------*/
  info_IM->index_write = 0;	/* INDEX     */

  if (tcd_cp->disto_alloc || tcd_cp->fixed_quality)   /* mod fixed_quality*/
    /* Normal Rate/distortion allocation */
    tcd_rateallocate(dest, len, info_IM);
  else
    /* Fixed layer allocation */
    tcd_rateallocate_fixed();

/*--------------TIER2------------------*/
  info_IM->index_write = 1;	/* INDEX     */
  l = t2_encode_packets(tcd_img, tcd_cp, tileno, tile,
			tcd_tcp->numlayers, dest, len, info_IM);
/*---------------CLEAN-------------------*/

  time7 = clock() - time7;
  fprintf(stdout,"total:     %ld.%.3ld s\n", time7 / CLOCKS_PER_SEC,
  (time7 % (int)CLOCKS_PER_SEC) * 1000 / CLOCKS_PER_SEC);

  /* cleaning memory */
  for (compno = 0; compno < tile->numcomps; compno++) {
    tilec = &tile->comps[compno];
    free(tilec->data);
  }

  return l;
}

int
tcd_encode_tile_pgx(int tileno, unsigned char *dest, int len,
		    info_image * info_IM)
{
  int compno;
  int l, i, npck=0;
  clock_t time;
  tcd_tile_t *tile;
  j2k_tcp_t *tcp = &tcd_cp->tcps[0];
  j2k_tccp_t *tccp = &tcp->tccps[0];
  
  tcd_tileno = tileno;
  tcd_tile = tcd_image.tiles;
  tcd_tcp = &tcd_cp->tcps[tileno];
  tile = tcd_tile;
  /* INDEX >> "Precinct_nb_X et Precinct_nb_Y" */
  if (info_IM->index_on) {
    tcd_tilecomp_t *tilec_idx = &tile->comps[0];   /*Based on Component 0*/
    
    for (i = 0; i < tilec_idx->numresolutions; i++) {
      
      tcd_resolution_t *res_idx = &tilec_idx->resolutions[i];
      
      info_IM->tile[tileno].pw[i] = res_idx->pw;
      info_IM->tile[tileno].ph[i] = res_idx->ph;
      
      npck+=res_idx->pw * res_idx->ph;
      
      info_IM->tile[tileno].pdx[i] = tccp->prcw[i];
      info_IM->tile[tileno].pdy[i] = tccp->prch[i];
      
    }
    info_IM->tile[tileno].packet = (info_packet *) calloc(info_IM->Comp * info_IM->Layer * npck, sizeof(info_packet));
  }
  /* << INDEX */
/*---------------TILE-------------------*/
  time = clock();

  for (compno = 0; compno < tile->numcomps; compno++) {
    FILE *src;
    char tmp[256];
    int k;
    int elmt;
    int i, j;
    int tw, w;
    tcd_tilecomp_t *tilec = &tile->comps[compno];
    int adjust =
      tcd_img->comps[compno].sgnd ? 0 : 1 << (tcd_img->comps[compno].
					      prec - 1);
    int offset_x, offset_y;

    offset_x = int_ceildiv(tcd_img->x0, tcd_img->comps[compno].dx);
    offset_y = int_ceildiv(tcd_img->y0, tcd_img->comps[compno].dy);
    tw = tilec->x1 - tilec->x0;
    w = int_ceildiv(tcd_img->x1 - tcd_img->x0, tcd_img->comps[compno].dx);
    sprintf(tmp, "bandtile%d", tileno / tcd_cp->tw + 1);	/* bandtile file opening */
    src = fopen(tmp, "rb");
    if (!src) {
      fprintf(stderr, "failed to open %s for reading\n", tmp);
      return 1;
    }
    /* Extract data from bandtile file limited to the current tile */
    k = 0;
    while (k < tilec->x0 - offset_x) {
      k++;
      fscanf(src, "%d", &elmt);
    }

    for (j = 0; j < tilec->y1 - tilec->y0; j++) {
      for (i = tilec->x0; i < tilec->x1; i++) {
	if (tcd_tcp->tccps[compno].qmfbid == 1) {
	  fscanf(src, "%d", &elmt);
	  tilec->data[i - tilec->x0 + (j) * tw] = elmt - adjust;
	  k++;
	} else if (tcd_tcp->tccps[compno].qmfbid == 0) {
	  fscanf(src, "%d", &elmt);
	  tilec->data[i - tilec->x0 + (j) * tw] = (elmt - adjust) << 13;
	  k++;
	}
      }
      while (k < tilec->x0 - offset_x + (j + 1) * w) {
	k++;
	fscanf(src, "%d", &elmt);
      }
    }
    fclose(src);
  }

/*----------------MCT-------------------*/

  if (tcd_tcp->mct) {
    if (tcd_tcp->tccps[0].qmfbid == 0) {
      mct_encode_real(tile->comps[0].data, tile->comps[1].data,
		      tile->comps[2].data,
		      (tile->comps[0].x1 -
		       tile->comps[0].x0) * (tile->comps[0].y1 -
					     tile->comps[0].y0));
    } else {
      mct_encode(tile->comps[0].data, tile->comps[1].data,
		 tile->comps[2].data,
		 (tile->comps[0].x1 -
		  tile->comps[0].x0) * (tile->comps[0].y1 -
					tile->comps[0].y0));
    }
  }

/*----------------DWT---------------------*/

/* mod Ive*/
for (compno = 0; compno < tile->numcomps; compno++) {
  tcd_tilecomp_t *tilec = &tile->comps[compno];
  if (tcd_tcp->tccps[compno].qmfbid == 1) {
    dwt_encode(tilec);
  } else if (tcd_tcp->tccps[compno].qmfbid == 0) {
    dwt_encode_real(tilec);
  }
}
/* /mod Ive*/

/*------------------TIER1-----------------*/

  t1_init_luts();
  t1_encode_cblks(tile, tcd_tcp);

/*-----------RATE-ALLOCATE------------------*/

  info_IM->index_write = 0;	/* INDEX */

  if (tcd_cp->disto_alloc || tcd_cp->fixed_quality)   /* mod fixed_quality*/

    /* Normal Rate/distortion allocation */

    tcd_rateallocate(dest, len, info_IM);

  else
    /* Fixed layer allocation */

    tcd_rateallocate_fixed();

/*--------------TIER2------------------*/
  info_IM->index_write = 1;	/* INDEX */

  l = t2_encode_packets(tcd_img, tcd_cp, tileno, tile,
			tcd_tcp->numlayers, dest, len, info_IM);

 /*---------------CLEAN-------------------*/
  time = clock() - time;
  fprintf(stdout,"total:     %ld.%.3ld s\n", time / CLOCKS_PER_SEC,
  (time % (int)CLOCKS_PER_SEC) * 1000 / CLOCKS_PER_SEC);

  for (compno = 0; compno < tile->numcomps; compno++) {
    tilec = &tile->comps[compno];
    free(tilec->data);
  }

  return l;
}


int tcd_decode_tile(unsigned char *src, int len, int tileno)
{
  int l;
  int compno;
  int eof = 0;
  clock_t time;
  tcd_tile_t *tile;

  tcd_tileno = tileno;
  tcd_tile = &tcd_image.tiles[tileno];
  tcd_tcp = &tcd_cp->tcps[tileno];
  tile = tcd_tile;

  time = clock();

  fprintf(stdout, "Tile %d of %d decoded in ", tileno + 1,
	  tcd_cp->tw * tcd_cp->th);

	/*--------------TIER2------------------*/

  l = t2_decode_packets(src, len, tcd_img, tcd_cp, tileno, tile);

  if (l == -999) {
    eof = 1;
    fprintf(stderr, "tcd_decode: incomplete bistream\n");
  }

	/*------------------TIER1-----------------*/
  t1_init_luts();
  t1_decode_cblks(tile, tcd_tcp);

	/*----------------DWT---------------------*/

  for (compno = 0; compno < tile->numcomps; compno++) {
    tcd_tilecomp_t *tilec = &tile->comps[compno];
    if (tcd_cp->reduce != 0) {
      tcd_img->comps[compno].resno_decoded =
	tile->comps[compno].numresolutions - tcd_cp->reduce - 1;
    }


    /* mod Ive  */
    if (tcd_tcp->tccps[compno].qmfbid == 1) {
      dwt_decode(tilec, 
                 tilec->numresolutions - 1 - 
                 tcd_img->comps[compno].resno_decoded);
    } else {
      dwt_decode_real(tilec,
		      tilec->numresolutions - 1 -
		      tcd_img->comps[compno].resno_decoded);
    }
    /* /mod Ive*/
    
    if (tile->comps[compno].numresolutions > 0)
      tcd_img->comps[compno].factor =
	tile->comps[compno].numresolutions -
	(tcd_img->comps[compno].resno_decoded + 1);
  }

	/*----------------MCT-------------------*/

  if (tcd_tcp->mct) {
    if (tcd_tcp->tccps[0].qmfbid == 1) {
      mct_decode(tile->comps[0].data, tile->comps[1].data,
		 tile->comps[2].data,
		 (tile->comps[0].x1 -
		  tile->comps[0].x0) * (tile->comps[0].y1 -
					tile->comps[0].y0));
    } else {
      mct_decode_real(tile->comps[0].data, tile->comps[1].data,
		      tile->comps[2].data,
		      (tile->comps[0].x1 -
		       tile->comps[0].x0) * (tile->comps[0].y1 -
					     tile->comps[0].y0));
    }
  }

	/*---------------TILE-------------------*/

  for (compno = 0; compno < tile->numcomps; compno++) {
    tcd_tilecomp_t *tilec = &tile->comps[compno];
    tcd_resolution_t *res =
      &tilec->resolutions[tcd_img->comps[compno].resno_decoded];
    int adjust =
      tcd_img->comps[compno].sgnd ? 0 : 1 << (tcd_img->comps[compno].
					      prec - 1);
    int min =
      tcd_img->comps[compno].
      sgnd ? -(1 << (tcd_img->comps[compno].prec - 1)) : 0;
    int max =
      tcd_img->comps[compno].
      sgnd ? (1 << (tcd_img->comps[compno].prec - 1)) -
      1 : (1 << tcd_img->comps[compno].prec) - 1;

    int tw = tilec->x1 - tilec->x0;
    int w = tcd_img->comps[compno].w;

    int i, j;
    int offset_x = int_ceildivpow2(tcd_img->comps[compno].x0,
				   tcd_img->comps[compno].factor);
    int offset_y = int_ceildivpow2(tcd_img->comps[compno].y0,
				   tcd_img->comps[compno].factor);

    for (j = res->y0; j < res->y1; j++) {
      for (i = res->x0; i < res->x1; i++) {

	int v;
 double tmp = (tilec->data[i - res->x0 + (j - res->y0) * tw])/8192.0;
        int tmp2;
        
	if (tcd_tcp->tccps[compno].qmfbid == 1) {
	  v = tilec->data[i - res->x0 + (j - res->y0) * tw];
	} else {
          tmp2=((int) (floor(fabs(tmp)))) + ((int) floor(fabs(tmp*2))%2);
          v = ((tmp<0)?-tmp2:tmp2);
        }
        
	v += adjust;

	tcd_img->comps[compno].data[(i - offset_x) +
				    (j - offset_y) * w] =
	  int_clamp(v, min, max);
      }
    }
  }

  time = clock() - time;
  fprintf(stdout, "%ld.%.3ld s\n", time / CLOCKS_PER_SEC,
   (time % (int)CLOCKS_PER_SEC) * 1000 / CLOCKS_PER_SEC);



  for (compno = 0; compno < tile->numcomps; compno++) {
    free(tcd_image.tiles[tileno].comps[compno].data);
  }

  if (eof) {
    longjmp(j2k_error, 1);
  }

  return l;
}



void tcd_dec_release()

{

  int tileno,compno,resno,bandno,precno;

  for (tileno=0;tileno<tcd_image.tw*tcd_image.th;tileno++) {

    tcd_tile_t tile=tcd_image.tiles[tileno];

    for (compno=0;compno<tile.numcomps;compno++) {

      tcd_tilecomp_t tilec=tile.comps[compno];

      for (resno=0;resno<tilec.numresolutions;resno++) {

	tcd_resolution_t res=tilec.resolutions[resno];

	for (bandno=0;bandno<res.numbands;bandno++) {

	  tcd_band_t band=res.bands[bandno];

	  for (precno=0;precno<res.ph*res.pw;precno++) {

	    tcd_precinct_t prec=band.precincts[precno];

	    if (prec.cblks!=NULL) free(prec.cblks);

	    if (prec.imsbtree!=NULL) free(prec.imsbtree);

	    if (prec.incltree!=NULL) free(prec.incltree);

	  }

	  if (band.precincts!=NULL) free(band.precincts);

	}

      }

      if (tilec.resolutions!=NULL) free(tilec.resolutions);

    }

    if (tile.comps!=NULL) free(tile.comps);

  }

  if (tcd_image.tiles!=NULL) free(tcd_image.tiles);

}
