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
	/* fprintf(stderr, "image {\n"); */
	fprintf(stderr, "  tw=%d, th=%d x0 %d x1 %d\n", img->tw, img->th,
					tcd_img->x0, tcd_img->x1);
	for (tileno = 0; tileno < 1; tileno++) {
		tcd_tile_t *tile = &tcd_image.tiles[curtileno];
		/* fprintf(stderr, "  tile {\n"); */
		/* fprintf(stderr, "    x0=%d, y0=%d, x1=%d, y1=%d, numcomps=%d\n", tile->x0, tile->y0, tile->x1, tile->y1, tile->numcomps); */
		for (compno = 0; compno < tile->numcomps; compno++) {
			tcd_tilecomp_t *tilec = &tile->comps[compno];
			/* fprintf(stderr, "    tilec {\n"); */
			/* fprintf(stderr, "      x0=%d, y0=%d, x1=%d, y1=%d, numresolutions=%d\n", tilec->x0, tilec->y0, tilec->x1, tilec->y1, tilec->numresolutions); */
			for (resno = 0; resno < tilec->numresolutions; resno++) {
				tcd_resolution_t *res = &tilec->resolutions[resno];
				/* fprintf(stderr, "\n   res {\n"); */
				/* fprintf(stderr, "          x0=%d, y0=%d, x1=%d, y1=%d, pw=%d, ph=%d, numbands=%d\n", res->x0, res->y0, res->x1, res->y1, res->pw, res->ph, res->numbands); */
				for (bandno = 0; bandno < res->numbands; bandno++) {
					tcd_band_t *band = &res->bands[bandno];
					/* fprintf(stderr, "        band {\n"); */
					/* fprintf(stderr, "          x0=%d, y0=%d, x1=%d, y1=%d, stepsize=%d, numbps=%d\n", band->x0, band->y0, band->x1, band->y1, band->stepsize, band->numbps); */
					for (precno = 0; precno < res->pw * res->ph; precno++) {
						tcd_precinct_t *prec = &band->precincts[precno];
						/* fprintf(stderr, "          prec {\n"); */
						/* fprintf(stderr, "            x0=%d, y0=%d, x1=%d, y1=%d, cw=%d, ch=%d\n", prec->x0, prec->y0, prec->x1, prec->y1, prec->cw, prec->ch); */
						for (cblkno = 0; cblkno < prec->cw * prec->ch; cblkno++) {
							/* tcd_cblk_t *cblk=&prec->cblks[cblkno]; */
							/* fprintf(stderr, "            cblk {\n"); */
							/* fprintf(stderr, "              x0=%d, y0=%d, x1=%d, y1=%d\n", cblk->x0, cblk->y0, cblk->x1, cblk->y1); */
							/* fprintf(stderr, "            }\n"); */
						}
						/* fprintf(stderr, "          }\n"); */
					}
					/* fprintf(stderr, "        }\n"); */
				}
				/* fprintf(stderr, "      }\n"); */
			}
			/* fprintf(stderr, "    }\n"); */
		}
		/* fprintf(stderr, "  }\n"); */
	}
	/* fprintf(stderr, "}\n"); */
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
			tcp->rates[j] =
				ceil(tile->numcomps * (tile->x1 - tile->x0) *
						 (tile->y1 -
							tile->y0) * img->comps[0].prec / (tcp->rates[j] * 8 *
																								img->comps[0].dx *
																								img->comps[0].dy));
			if (j && tcp->rates[j] < tcp->rates[j - 1] + 10) {
				tcp->rates[j] = tcp->rates[j - 1] + 20;
			} else {
				if (!j && tcp->rates[j] < 30)
					tcp->rates[j] = 30;
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

			/* Special DWT (always this case) */
			tilec->previous_row = 0;
			tilec->previous_col = 0;

			tilec->data =
				(int *) malloc(sizeof(int) * (tilec->x1 - tilec->x0) *
											 (tilec->y1 - tilec->y0));
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

				/* Special DWT (always this case) */
				res->previous_x0 = 0;
				res->previous_y0 = 0;
				res->previous_x1 = 0;
				res->previous_y1 = 0;
				res->cas_row = 0;
				res->cas_col = 0;
				/* << DWT */
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
							int_ceildivpow2(tilec->x0 - (1 << levelno) * x0b,
															levelno + 1);
						band->y0 =
							int_ceildivpow2(tilec->y0 - (1 << levelno) * y0b,
															levelno + 1);
						band->x1 =
							int_ceildivpow2(tilec->x1 - (1 << levelno) * x0b,
															levelno + 1);
						band->y1 =
							int_ceildivpow2(tilec->y1 - (1 << levelno) * y0b,
															levelno + 1);

					}

					ss =
						&tccp->stepsizes[resno ==
														 0 ? 0 : 3 * (resno - 1) + bandno + 1];
					gain =
						tccp->qmfbid ==
						0 ? dwt_getgain_real(band->bandno) : dwt_getgain(band->bandno);
					numbps = img->comps[compno].prec + gain;
					band->stepsize =
						(int) floor((1.0 + ss->mant / 2048.0) *
												pow(2.0, numbps - ss->expn) * 8192.0);
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
					}											/* for (precno */
					free(band->precincts);
				}												/* for (bandno */
			}													/* for (resno */
			free(tilec->resolutions);
		}														/* for (compno */
		free(tile->comps);
	}															/* for (tileno */
	free(tcd_image.tiles);
}

void tcd_init_encode(j2k_image_t * img, j2k_cp_t * cp, int curtileno)
{
	int tileno, compno, resno, bandno, precno, cblkno;

	for (tileno = 0; tileno < 1; tileno++) {
		j2k_tcp_t *tcp = &cp->tcps[curtileno];
		int j;
		int previous_x0, previous_x1, previous_y0, previous_y1;
		/* cfr p59 ISO/IEC FDIS15444-1 : 2000 (18 august 2000) */
		int p = curtileno % cp->tw;
		int q = curtileno / cp->tw;
		tile = tcd_image.tiles;

		/* 4 borders of the tile rescale on the image if necessary */
		tile->x0 = int_max(cp->tx0 + p * cp->tdx, img->x0);
		tile->y0 = int_max(cp->ty0 + q * cp->tdy, img->y0);
		tile->x1 = int_min(cp->tx0 + (p + 1) * cp->tdx, img->x1);
		tile->y1 = int_min(cp->ty0 + (q + 1) * cp->tdy, img->y1);
		/* Special DWT */
		if (p) {
			previous_x0 = int_max(cp->tx0 + (p - 1) * cp->tdx, img->x0);
			previous_x1 = int_min(cp->tx0 + p * cp->tdx, img->x1);
		} else {
			previous_x0 = 0;
			previous_x1 = 0;
		}

		if (q) {
			previous_y0 = int_max(cp->ty0 + (q - 1) * cp->tdy, img->y0);
			previous_y1 = int_min(cp->ty0 + q * cp->tdy, img->y1);
		} else {
			previous_y0 = 0;
			previous_y1 = 0;
		}
		/* << DWT */

		tile->numcomps = img->numcomps;
		/* tile->PPT=img->PPT; */
		/* Modification of the RATE >> */
		for (j = 0; j < tcp->numlayers; j++) {
			tcp->rates[j] =
				ceil(tile->numcomps * (tile->x1 - tile->x0) *
						 (tile->y1 -
							tile->y0) * img->comps[0].prec / (tcp->rates[j] * 8 *
																								img->comps[0].dx *
																								img->comps[0].dy));
			if (j && tcp->rates[j] < tcp->rates[j - 1] + 10) {
				tcp->rates[j] = tcp->rates[j - 1] + 20;
			} else {
				if (!j && tcp->rates[j] < 30)
					tcp->rates[j] = 30;
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

			/* special for DWT */
			if (p) {
				previous_x0 = int_ceildiv(previous_x0, img->comps[compno].dx);
				previous_x1 = int_ceildiv(previous_x1, img->comps[compno].dx);

				tilec->previous_row = 1;
			} else {
				previous_x0 = 0;
				previous_x1 = 0;
				tilec->previous_row = 0;
			}

			if (q) {
				previous_y0 = int_ceildiv(previous_y0, img->comps[compno].dx);
				previous_y1 = int_ceildiv(previous_y1, img->comps[compno].dx);
				tilec->previous_col = 1;
			} else {
				previous_y0 = 0;
				previous_y1 = 0;
				tilec->previous_col = 0;
			}
			/* << DWT */
			tilec->data =
				(int *) malloc(sizeof(int) * (tilec->x1 - tilec->x0) *
											 (tilec->y1 - tilec->y0));
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

				/* special for DWT */
				res->previous_x0 = int_ceildivpow2(previous_x0, levelno);
				res->previous_y0 = int_ceildivpow2(previous_y0, levelno);
				res->previous_x1 = int_ceildivpow2(previous_x1, levelno);
				res->previous_y1 = int_ceildivpow2(previous_y1, levelno);

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
							int_ceildivpow2(tilec->x0 - (1 << levelno) * x0b,
															levelno + 1);
						band->y0 =
							int_ceildivpow2(tilec->y0 - (1 << levelno) * y0b,
															levelno + 1);
						band->x1 =
							int_ceildivpow2(tilec->x1 - (1 << levelno) * x0b,
															levelno + 1);
						band->y1 =
							int_ceildivpow2(tilec->y1 - (1 << levelno) * y0b,
															levelno + 1);
					}

					ss =
						&tccp->stepsizes[resno ==
														 0 ? 0 : 3 * (resno - 1) + bandno + 1];
					gain =
						tccp->qmfbid ==
						0 ? dwt_getgain_real(band->bandno) : dwt_getgain(band->bandno);
					numbps = img->comps[compno].prec + gain;
					band->stepsize =
						(int) floor((1.0 + ss->mant / 2048.0) *
												pow(2.0, numbps - ss->expn) * 8192.0);
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
	int tileno, compno, resno, bandno, precno, cblkno;
	tcd_img = img;
	tcd_cp = cp;
	tcd_image.tw = cp->tw;
	tcd_image.th = cp->th;
	tcd_image.tiles =
		(tcd_tile_t *) malloc(cp->tw * cp->th * sizeof(tcd_tile_t));
	for (tileno = 0; tileno < cp->tw * cp->th; tileno++) {
		j2k_tcp_t *tcp = &cp->tcps[tileno];
		tcd_tile_t *tile = &tcd_image.tiles[tileno];
		int previous_x0, previous_x1, previous_y0, previous_y1;
		/* cfr p59 ISO/IEC FDIS15444-1 : 2000 (18 august 2000) */
		int p = tileno % cp->tw;		/* si numerotation matricielle .. */
		int q = tileno / cp->tw;		/* .. coordonnees de la tile (q,p) q pour ligne et p pour colonne */

		/* 4 borders of the tile rescale on the image if necessary */
		tile->x0 = int_max(cp->tx0 + p * cp->tdx, img->x0);
		tile->y0 = int_max(cp->ty0 + q * cp->tdy, img->y0);
		tile->x1 = int_min(cp->tx0 + (p + 1) * cp->tdx, img->x1);
		tile->y1 = int_min(cp->ty0 + (q + 1) * cp->tdy, img->y1);
		/* Special DWT */
		if (p) {
			previous_x0 = int_max(cp->tx0 + (p - 1) * cp->tdx, img->x0);
			previous_x1 = int_min(cp->tx0 + p * cp->tdx, img->x1);
		} else {
			previous_x0 = 0;
			previous_x1 = 0;
		}

		if (q) {
			previous_y0 = int_max(cp->ty0 + (q - 1) * cp->tdy, img->y0);
			previous_y1 = int_min(cp->ty0 + q * cp->tdy, img->y1);
		} else {
			previous_y0 = 0;
			previous_y1 = 0;
		}
		/* << DWT */

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
			/* special for DWT */
			if (p) {
				previous_x0 = int_ceildiv(previous_x0, img->comps[compno].dx);
				previous_x1 = int_ceildiv(previous_x1, img->comps[compno].dx);

				tilec->previous_row = 1;
			} else {
				previous_x0 = 0;
				previous_x1 = 0;
				tilec->previous_row = 0;
			}

			if (q) {
				previous_y0 = int_ceildiv(previous_y0, img->comps[compno].dx);
				previous_y1 = int_ceildiv(previous_y1, img->comps[compno].dx);
				tilec->previous_col = 1;
			} else {
				previous_y0 = 0;
				previous_y1 = 0;
				tilec->previous_col = 0;
			}
			/* << DWT */
			tilec->data =
				(int *) malloc(sizeof(int) * (tilec->x1 - tilec->x0) *
											 (tilec->y1 - tilec->y0));
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
				/* special for DWT */
				res->previous_x0 = int_ceildivpow2(previous_x0, levelno);
				res->previous_y0 = int_ceildivpow2(previous_y0, levelno);
				res->previous_x1 = int_ceildivpow2(previous_x1, levelno);
				res->previous_y1 = int_ceildivpow2(previous_y1, levelno);
				if (!p)
					res->cas_row = 0;
				if (!q)
					res->cas_col = 0;

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
							int_ceildivpow2(tilec->x0 - (1 << levelno) * x0b,
															levelno + 1);
						band->y0 =
							int_ceildivpow2(tilec->y0 - (1 << levelno) * y0b,
															levelno + 1);
						band->x1 =
							int_ceildivpow2(tilec->x1 - (1 << levelno) * x0b,
															levelno + 1);
						band->y1 =
							int_ceildivpow2(tilec->y1 - (1 << levelno) * y0b,
															levelno + 1);
					}

					ss =
						&tccp->stepsizes[resno ==
														 0 ? 0 : 3 * (resno - 1) + bandno + 1];
					gain =
						tccp->qmfbid ==
						0 ? dwt_getgain_real(band->bandno) : dwt_getgain(band->bandno);
					numbps = img->comps[compno].prec + gain;
					band->stepsize =
						(int) floor((1.0 + ss->mant / 2048.0) *
												pow(2.0, numbps - ss->expn) * 8192.0);
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
	/* tcd_dump(&tcd_image,0); */
}

void tcd_makelayer_fixed(int layno, int matrice_data[2][5][3], int final)
{
	int compno, resno, bandno, precno, cblkno;
	int vector[3], matrice[2][5][3];
	int sum_mat, i, j, k;
	for (compno = 0; compno < tcd_tile->numcomps; compno++) {
		tcd_tilecomp_t *tilec = &tcd_tile->comps[compno];
		for (i = 0; i < 2; i++) {
			for (j = 0; j < 5; j++) {
				for (k = 0; k < 3; k++) {
					matrice[i][j][k] =
						(int) (matrice_data[i][j][k] *
									 (float) (tcd_img->comps[compno].prec / 16.0));
				}
			}
		}
		for (resno = 0; resno < tilec->numresolutions; resno++) {
			tcd_resolution_t *res = &tilec->resolutions[resno];
			for (bandno = 0; bandno < res->numbands; bandno++) {
				tcd_band_t *band = &res->bands[bandno];
				for (precno = 0; precno < res->pw * res->ph; precno++) {
					tcd_precinct_t *prc = &band->precincts[precno];
					for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
						tcd_cblk_t *cblk = &prc->cblks[cblkno];
						tcd_layer_t *layer = &cblk->layers[layno];
						int i, n;
						int imsb = tcd_img->comps[compno].prec - cblk->numbps;	/* number of bit-plan equal to zero */
						/* Correction of the matrix of coefficient to include the IMSB information */
						sum_mat = 0;

						for (i = 0; i < layno + 1; i++) {
							int mat_old;
							if (i != tcd_tcp->numlayers) {	/* -1  must add to have a lossless last quality layer */
								/* different quality (layer) */
								vector[bandno] = matrice[i][resno][bandno];
								sum_mat += vector[bandno];
							} else {
								/* Full quality (layer) */
								vector[bandno] = tcd_img->comps[compno].prec - sum_mat;
							}

							mat_old = vector[bandno];
							if (imsb > 0) {
								if (imsb >= vector[bandno]) {
									vector[bandno] = 0;
								} else {
									vector[bandno] -= imsb;
								}
								imsb -= mat_old;
							}
						}
						if (layno == 0) {
							cblk->numpassesinlayers = 0;
						}
						n = cblk->numpassesinlayers;
						if (cblk->numpassesinlayers == 0) {
							if (vector[bandno] != 0) {
								n = 3 * vector[bandno] - 2 + cblk->numpassesinlayers;
							} else {
								n = cblk->numpassesinlayers;
							}

						} else {
							n = 3 * vector[bandno] + cblk->numpassesinlayers;
						}

						layer->numpasses = n - cblk->numpassesinlayers;

						if (!layer->numpasses) {
							continue;
						}
						if (cblk->numpassesinlayers == 0) {
							layer->len = cblk->passes[n - 1].rate;
							layer->data = cblk->data;
						} else {
							layer->len =
								cblk->passes[n - 1].rate -
								cblk->passes[cblk->numpassesinlayers - 1].rate;
							layer->data =
								cblk->data + cblk->passes[cblk->numpassesinlayers -
																					1].rate;
						}
						if (final) {
							cblk->numpassesinlayers = n;
						}
					}
				}
			}
		}
	}

}

void tcd_rateallocate_fixed(unsigned char *dest, int len,
														info_image * info_IM)
{
	int layno;
	int matrice[2][5][3] = { {{16, 0, 0},
														{16, 16, 16},
														{16, 16, 16},
														{0, 0, 0},
														{0, 0, 0}},

	{{1, 0, 0},
	 {1, 1, 1},
	 {1, 1, 1},
	 {1, 1, 1},
	 {0, 0, 0}}
	};

	for (layno = 0; layno < tcd_tcp->numlayers; layno++) {
		tcd_makelayer_fixed(layno, matrice, 1);
	}
}

void tcd_makelayer(int layno, double thresh, int final)
{
	int compno, resno, bandno, precno, cblkno, passno;
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
								dd =
									pass->distortiondec - cblk->passes[n - 1].distortiondec;
							}
							if (dr == 0) {
								if (dd != 0)
									n = passno + 1;
								continue;
							}
							if (dd / dr > thresh)
								n = passno + 1;
						}
						layer->numpasses = n - cblk->numpassesinlayers;
						if (!layer->numpasses)
							continue;
						if (cblk->numpassesinlayers == 0) {
							layer->len = cblk->passes[n - 1].rate;
							layer->data = cblk->data;
							layer->disto = cblk->passes[n - 1].distortiondec;
						} else {
							layer->len =
								cblk->passes[n - 1].rate -
								cblk->passes[cblk->numpassesinlayers - 1].rate;
							layer->data =
								cblk->data + cblk->passes[cblk->numpassesinlayers -
																					1].rate;
							layer->disto =
								cblk->passes[n - 1].distortiondec -
								cblk->passes[cblk->numpassesinlayers - 1].distortiondec;
						}

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
	min = DBL_MAX;
	max = 0;

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
						for (passno = 0; passno < cblk->totalpasses; passno++) {
							tcd_pass_t *pass = &cblk->passes[passno];
							int dr;
							double dd, rdslope;
							if (passno == 0) {
								dr = pass->rate;
								dd = pass->distortiondec;
							} else {
								dr = pass->rate - cblk->passes[passno - 1].rate;
								dd =
									pass->distortiondec - cblk->passes[passno -
																										 1].distortiondec;
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
						}										/* passno */
					}											/* cbklno */
				}												/* precno */
			}													/* bandno */
		}														/* resno */
	}															/* compno */

	for (layno = 0; layno < tcd_tcp->numlayers; layno++) {
		volatile double lo = min;
		volatile double hi = max;
		volatile int success = 0;
		volatile int maxlen = int_min(tcd_tcp->rates[layno], len);
		volatile double goodthresh;
		volatile int goodlen;
		volatile int i;

		for (i = 0; i < 32; i++) {
			volatile double thresh = (lo + hi) / 2;
			int l;

			tcd_makelayer(layno, thresh, 0);

			l =
				t2_encode_packets(tcd_img, tcd_cp, tcd_tileno, tcd_tile, layno + 1,
													dest, maxlen, info_IM);
			/* fprintf(stderr, "rate alloc: len=%d, max=%d\n", l, maxlen); */
			if (l == -999) {
				lo = thresh;
				continue;
			}

			hi = thresh;
			success = 1;
			goodthresh = thresh;
			goodlen = l;
		}

		if (!success) {
			longjmp(j2k_error, 1);
		}
		tcd_makelayer(layno, goodthresh, 1);
	}
}

int tcd_encode_tile_pxm(int tileno, unsigned char *dest, int len,
												info_image * info_IM)
{
	int compno;
	int l;
	clock_t time7;								/* , time2, time3, time4, time5, time6, time1; */
	tcd_tile_t *tile;
	j2k_tcp_t *tcp = &tcd_cp->tcps[0];
	j2k_tccp_t *tccp = &tcp->tccps[0];

	tcd_tileno = tileno;
	tcd_tile = tcd_image.tiles;
	tcd_tcp = &tcd_cp->tcps[tileno];
	tile = tcd_tile;
	/* INDEX >> "Precinct_nb_X et Precinct_nb_Y" */
	if (info_IM->index_on) {
		tcd_tilecomp_t *tilec_idx = &tile->comps[0];	/* old parser version */
		tcd_resolution_t *res_idx = &tilec_idx->resolutions[0];	/* old parser version */

		info_IM->tile[tileno].pw = res_idx->pw;
		info_IM->tile[tileno].ph = res_idx->ph;

		info_IM->pw = res_idx->pw;	/* old parser version */
		info_IM->ph = res_idx->ph;	/* old parser version */
		info_IM->pdx = 1 << tccp->prcw[tccp->numresolutions - 1];
		info_IM->pdy = 1 << tccp->prch[tccp->numresolutions - 1];
	}
	/* << INDEX */

/*---------------TILE-------------------*/

	time7 = clock();
	/* time1=clock(); */

	for (compno = 0; compno < tile->numcomps; compno++) {
		FILE *src;
		char tmp[256];
		int k;
		unsigned char elmt;
		int i, j;
		int tw, w;
		tcd_tilecomp_t *tilec = &tile->comps[compno];
		int adjust =
			tcd_img->comps[compno].sgnd ? 0 : 1 << (tcd_img->comps[compno].prec -
																							1);
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

	/* time1=clock()-time1;   */
	/* printf("tile encoding times:\n"); */
	/* printf("img->tile: %d.%.3d s\n", time1/CLOCKS_PER_SEC, (time1%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

/*----------------MCT-------------------*/

	/* time2=clock(); */
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
	/* time2=clock()-time2;    */
	/* printf("mct:       %ld.%.3ld s\n", time2/CLOCKS_PER_SEC, (time2%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

/*----------------DWT---------------------*/

	/* time3=clock(); */
	for (compno = 0; compno < tile->numcomps; compno++) {
		tcd_tilecomp_t *tilec = &tile->comps[compno];
		if (tcd_tcp->tccps[compno].qmfbid == 1) {
			dwt_encode(tilec->data, tilec->x1 - tilec->x0, tilec->y1 - tilec->y0,
								 tilec, tilec->numresolutions - 1);
		} else if (tcd_tcp->tccps[compno].qmfbid == 0) {
			dwt_encode_real(tilec->data, tilec->x1 - tilec->x0,
											tilec->y1 - tilec->y0, tilec,
											tilec->numresolutions - 1);
		}
	}
	/* time3=clock()-time3; */
	/* printf("dwt:       %ld.%.3ld s\n", time3/CLOCKS_PER_SEC, (time3%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

/*------------------TIER1-----------------*/

	/* time4=clock(); */
	t1_init_luts();
	t1_encode_cblks(tile, tcd_tcp);
	/* time4=clock()-time4; */
	/* printf("tier 1:    %ld.%.3ld s\n", time4/CLOCKS_PER_SEC, (time4%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

/*-----------RATE-ALLOCATE------------------*/

	info_IM->index_write = 0;			/* INDEX     */
	/* time5=clock(); */
	if (tcd_cp->disto_alloc)
		/* Normal Rate/distortion allocation */
		tcd_rateallocate(dest, len, info_IM);
	else
		/* Fixed layer allocation */
		tcd_rateallocate_fixed(dest, len, info_IM);

	/* time5=clock()-time5; */
	/* printf("ratealloc: %ld.%.3ld s\n", time5/CLOCKS_PER_SEC, (time5%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

/*--------------TIER2------------------*/

	info_IM->index_write = 1;			/* INDEX     */
	/* time6=clock(); */
	l =
		t2_encode_packets(tcd_img, tcd_cp, tileno, tile, tcd_tcp->numlayers,
											dest, len, info_IM);
	/* time6=clock()-time6; */
	/* printf("tier 2:    %ld.%.3ld s\n", time6/CLOCKS_PER_SEC, (time6%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

/*---------------CLEAN-------------------*/

	time7 = clock() - time7;
	printf("total:     %ld.%.3ld s\n", time7 / CLOCKS_PER_SEC,
				 (time7 % CLOCKS_PER_SEC) * 1000 / CLOCKS_PER_SEC);

	/* cleaning memory */
	for (compno = 0; compno < tile->numcomps; compno++) {
		tilec = &tile->comps[compno];
		free(tilec->data);
	}

	return l;
}

int tcd_encode_tile_pgx(int tileno, unsigned char *dest, int len,
												info_image * info_IM)
{
	int compno;
	int l;
	clock_t time7;								/* , time2, time3, time4, time5, time6, time1; */
	tcd_tile_t *tile;
	j2k_tcp_t *tcp = &tcd_cp->tcps[0];
	j2k_tccp_t *tccp = &tcp->tccps[0];

	tcd_tileno = tileno;
	tcd_tile = tcd_image.tiles;
	tcd_tcp = &tcd_cp->tcps[tileno];
	tile = tcd_tile;
	/* INDEX >> "Precinct_nb_X et Precinct_nb_Y" */
	if (info_IM->index_on) {
		tcd_tilecomp_t *tilec_idx = &tile->comps[0];
		tcd_resolution_t *res_idx = &tilec_idx->resolutions[0];
		info_IM->tile[tileno].pw = res_idx->pw;
		info_IM->tile[tileno].ph = res_idx->ph;
		info_IM->pw = res_idx->pw;	/* old parser version */
		info_IM->ph = res_idx->ph;	/* old parser version */
		info_IM->pdx = 1 << tccp->prcw[tccp->numresolutions - 1];
		info_IM->pdy = 1 << tccp->prch[tccp->numresolutions - 1];
	}
	/* << INDEX */
/*---------------TILE-------------------*/
	time7 = clock();
	/* time1=clock(); */
	for (compno = 0; compno < tile->numcomps; compno++) {
		FILE *src;
		char tmp[256];
		int k;
		int elmt;
		int i, j;
		int tw, w;
		tcd_tilecomp_t *tilec = &tile->comps[compno];
		int adjust =
			tcd_img->comps[compno].sgnd ? 0 : 1 << (tcd_img->comps[compno].prec -
																							1);
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

	/* time1=clock()-time1;   */
	/* printf("tile encoding times:\n"); */
	/* printf("img->tile: %d.%.3d s\n", time1/CLOCKS_PER_SEC, (time1%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */
/*----------------MCT-------------------*/
	/* time2=clock(); */
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

	/* time2=clock()-time2;    */
	/* printf("mct:       %d.%.3d s\n", time2/CLOCKS_PER_SEC, (time2%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

/*----------------DWT---------------------*/
	/* time3=clock(); */
	for (compno = 0; compno < tile->numcomps; compno++) {
		tcd_tilecomp_t *tilec = &tile->comps[compno];
		if (tcd_tcp->tccps[compno].qmfbid == 1) {
			dwt_encode(tilec->data, tilec->x1 - tilec->x0, tilec->y1 - tilec->y0,
								 tilec, tilec->numresolutions - 1);
		} else if (tcd_tcp->tccps[compno].qmfbid == 0) {
			dwt_encode_real(tilec->data, tilec->x1 - tilec->x0,
											tilec->y1 - tilec->y0, tilec,
											tilec->numresolutions - 1);
		}
	}
	/* time3=clock()-time3; */
	/* printf("dwt:       %d.%.3d s\n", time3/CLOCKS_PER_SEC, (time3%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

/*------------------TIER1-----------------*/
	/* time4=clock(); */
	t1_init_luts();
	t1_encode_cblks(tile, tcd_tcp);
	/* time4=clock()-time4; */
	/* printf("tier 1:    %d.%.3d s\n", time4/CLOCKS_PER_SEC, (time4%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

/*-----------RATE-ALLOCATE------------------*/
	info_IM->index_write = 0;			/* INDEX */

	/* time5=clock(); */
	tcd_rateallocate(dest, len, info_IM);
	/* time5=clock()-time5; */
	/* printf("ratealloc: %d.%.3d s\n", time5/CLOCKS_PER_SEC, (time5%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

/*--------------TIER2------------------*/
	info_IM->index_write = 1;			/* INDEX */

	/* time6=clock(); */
	l =
		t2_encode_packets(tcd_img, tcd_cp, tileno, tile, tcd_tcp->numlayers,
											dest, len, info_IM);
	/* time6=clock()-time6; */
	/* printf("tier 2:    %d.%.3d s\n", time6/CLOCKS_PER_SEC, (time6%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

 /*---------------CLEAN-------------------*/
	time7 = clock() - time7;
	printf("total:     %ld.%.3ld s\n", time7 / CLOCKS_PER_SEC,
				 (time7 % CLOCKS_PER_SEC) * 1000 / CLOCKS_PER_SEC);

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
	clock_t time1, time2, time3, time4, time5, time6;

	tcd_tile_t *tile;
	tcd_tileno = tileno;
	tcd_tile = &tcd_image.tiles[tileno];
	tcd_tcp = &tcd_cp->tcps[tileno];
	tile = tcd_tile;

	time6 = clock();

	time1 = clock();

	l = t2_decode_packets(src, len, tcd_img, tcd_cp, tileno, tile);
	if (l == -999) {
		eof = 1;
		fprintf(stderr, "tcd_decode: incomplete bistream\n");
	}
	time1 = clock() - time1;
	printf("tile decoding time %d/%d:\n", tileno + 1,
				 tcd_cp->tw * tcd_cp->th);
	/* printf("tier 2:    %ld.%.3ld s\n", time1/CLOCKS_PER_SEC, (time1%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC);    */

	time2 = clock();
	t1_init_luts();
	t1_decode_cblks(tile, tcd_tcp);
	time2 = clock() - time2;
	/* printf("tier 1:    %ld.%.3ld s\n", time2/CLOCKS_PER_SEC, (time2%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

	time3 = clock();
	for (compno = 0; compno < tile->numcomps; compno++) {
		tcd_tilecomp_t *tilec = &tile->comps[compno];
		if (tcd_tcp->tccps[compno].qmfbid == 1) {
			if (tileno % tcd_cp->tw) {
				tcd_tile_t *tile_row = &tcd_image.tiles[tileno - 1];
				tcd_tilecomp_t *tilec_row = &tile_row->comps[compno];
				if (tileno / tcd_cp->tw) {
					tcd_tile_t *tile_col = &tcd_image.tiles[tileno - 1];
					tcd_tilecomp_t *tilec_col = &tile_col->comps[compno];
					dwt_decode(tilec->data, tilec->x1 - tilec->x0,
										 tilec->y1 - tilec->y0, tilec,
										 tilec->numresolutions - 1, tilec_row, tilec_col);
				} else
					dwt_decode(tilec->data, tilec->x1 - tilec->x0,
										 tilec->y1 - tilec->y0, tilec,
										 tilec->numresolutions - 1, tilec_row, tilec);
			} else {
				if (tileno / tcd_cp->tw) {
					tcd_tile_t *tile_col = &tcd_image.tiles[tileno - 1];
					tcd_tilecomp_t *tilec_col = &tile_col->comps[compno];
					dwt_decode(tilec->data, tilec->x1 - tilec->x0,
										 tilec->y1 - tilec->y0, tilec,
										 tilec->numresolutions - 1, tilec, tilec_col);
				} else
					dwt_decode(tilec->data, tilec->x1 - tilec->x0,
										 tilec->y1 - tilec->y0, tilec,
										 tilec->numresolutions - 1, tilec, tilec);
			}
		} else if (tcd_tcp->tccps[compno].qmfbid == 0) {
			if (tileno % tcd_cp->tw) {
				tcd_tile_t *tile_row = &tcd_image.tiles[tileno - 1];
				tcd_tilecomp_t *tilec_row = &tile_row->comps[compno];
				if (tileno / tcd_cp->tw) {
					tcd_tile_t *tile_col = &tcd_image.tiles[tileno - 1];
					tcd_tilecomp_t *tilec_col = &tile_col->comps[compno];
					dwt_decode_real(tilec->data, tilec->x1 - tilec->x0,
													tilec->y1 - tilec->y0, tilec,
													tilec->numresolutions - 1, tilec_row, tilec_col);
				} else
					dwt_decode_real(tilec->data, tilec->x1 - tilec->x0,
													tilec->y1 - tilec->y0, tilec,
													tilec->numresolutions - 1, tilec_row, tilec);
			} else {
				if (tileno / tcd_cp->tw) {
					tcd_tile_t *tile_col = &tcd_image.tiles[tileno - 1];
					tcd_tilecomp_t *tilec_col = &tile_col->comps[compno];
					dwt_decode_real(tilec->data, tilec->x1 - tilec->x0,
													tilec->y1 - tilec->y0, tilec,
													tilec->numresolutions - 1, tilec, tilec_col);
				} else
					dwt_decode_real(tilec->data, tilec->x1 - tilec->x0,
													tilec->y1 - tilec->y0, tilec,
													tilec->numresolutions - 1, tilec, tilec);
			}
		}
	}
	time3 = clock() - time3;
	/* printf("dwt:       %ld.%.3ld s\n", time3/CLOCKS_PER_SEC, (time3%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

	time4 = clock();
	if (tcd_tcp->mct) {
		if (tcd_tcp->tccps[0].qmfbid == 0) {
			mct_decode_real(tile->comps[0].data, tile->comps[1].data,
											tile->comps[2].data,
											(tile->comps[0].x1 -
											 tile->comps[0].x0) * (tile->comps[0].y1 -
																						 tile->comps[0].y0));
		} else {
			mct_decode(tile->comps[0].data, tile->comps[1].data,
								 tile->comps[2].data,
								 (tile->comps[0].x1 -
									tile->comps[0].x0) * (tile->comps[0].y1 -
																				tile->comps[0].y0));
		}
	}
	time4 = clock() - time4;
	/* printf("mct:       %ld.%.3ld s\n", time4/CLOCKS_PER_SEC, (time4%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

	time5 = clock();
	for (compno = 0; compno < tile->numcomps; compno++) {
		tcd_tilecomp_t *tilec = &tile->comps[compno];
		int adjust =
			tcd_img->comps[compno].sgnd ? 0 : 1 << (tcd_img->comps[compno].prec -
																							1);
		int min =
			tcd_img->comps[compno].
			sgnd ? -(1 << (tcd_img->comps[compno].prec - 1)) : 0;
		int max =
			tcd_img->comps[compno].
			sgnd ? (1 << (tcd_img->comps[compno].prec - 1)) - 1
					 : (1 << tcd_img->comps[compno].prec) - 1;
		int tw = tilec->x1 - tilec->x0;
		int w =
			int_ceildiv(tcd_img->x1 - tcd_img->x0, tcd_img->comps[compno].dx);
		int i, j;
		int offset_x = int_ceildiv(tcd_img->x0, tcd_img->comps[compno].dx);
		int offset_y = int_ceildiv(tcd_img->y0, tcd_img->comps[compno].dy);

		for (j = tilec->y0; j < tilec->y1; j++) {
			for (i = tilec->x0; i < tilec->x1; i++) {
				int v;
				if (tcd_tcp->tccps[compno].qmfbid == 1) {
					v = tilec->data[i - tilec->x0 + (j - tilec->y0) * tw];
				} else {  /* if (tcd_tcp->tccps[compno].qmfbid == 0) */
					v = tilec->data[i - tilec->x0 + (j - tilec->y0) * tw] >> 13;
				}
				v += adjust;
				/* tcd_img->comps[compno].data[i+j*w]=int_clamp(v, min, max); */
				tcd_img->comps[compno].data[(i - offset_x) + (j - offset_y) * w] = int_clamp(v, min, max);	/* change ! */
			}
		}
	}
	time5 = clock() - time5;
	/* printf("tile->img: %ld.%.3ld s\n", time5/CLOCKS_PER_SEC, (time5%CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC); */

	time6 = clock() - time6;
	printf("total:     %ld.%.3ld s\n\n", time6 / CLOCKS_PER_SEC,
				 (time6 % CLOCKS_PER_SEC) * 1000 / CLOCKS_PER_SEC);

	if (eof) {
		longjmp(j2k_error, 1);
	}

	return l;
}
