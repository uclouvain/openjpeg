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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>

#include "j2k.h"
#include "cio.h"
#include "tcd.h"
#include "dwt.h"
#include "int.h"

#define J2K_MS_SOC 0xff4f
#define J2K_MS_SOT 0xff90
#define J2K_MS_SOD 0xff93
#define J2K_MS_EOC 0xffd9
#define J2K_MS_SIZ 0xff51
#define J2K_MS_COD 0xff52
#define J2K_MS_COC 0xff53
#define J2K_MS_RGN 0xff5e
#define J2K_MS_QCD 0xff5c
#define J2K_MS_QCC 0xff5d
#define J2K_MS_POC 0xff5f
#define J2K_MS_TLM 0xff55
#define J2K_MS_PLM 0xff57
#define J2K_MS_PLT 0xff58
#define J2K_MS_PPM 0xff60
#define J2K_MS_PPT 0xff61
#define J2K_MS_SOP 0xff91
#define J2K_MS_EPH 0xff92
#define J2K_MS_CRG 0xff63
#define J2K_MS_COM 0xff64

#define J2K_STATE_MHSOC 0x0001
#define J2K_STATE_MHSIZ 0x0002
#define J2K_STATE_MH 0x0004
#define J2K_STATE_TPHSOT 0x0008
#define J2K_STATE_TPH 0x0010
#define J2K_STATE_MT 0x0020
#define J2K_STATE_NEOC 0x0040


jmp_buf j2k_error;

static int j2k_state;
static int j2k_curtileno;
static j2k_tcp_t j2k_default_tcp;
static unsigned char *j2k_eot;
static int j2k_sot_start;
static int pos_correction;

static j2k_image_t *j2k_img;
static j2k_cp_t *j2k_cp;

static unsigned char **j2k_tile_data;
static int *j2k_tile_len;

static info_image info_IM;

/* Add Patrick */
void j2k_clean()
{
	int tileno = 0;
	tcd_free_encode(j2k_img, j2k_cp, j2k_curtileno);

	if (info_IM.index_on) {
		for (tileno = 0; tileno < j2k_cp->tw * j2k_cp->th; tileno++) {
			free(info_IM.tile[tileno].packet);
		}
		free(info_IM.tile);
	}
}

/* \Add Patrick */

void j2k_dump_image(j2k_image_t * img)
{
	int compno;
	fprintf(stderr, "image {\n");
	fprintf(stderr, "  x0=%d, y0=%d, x1=%d, y1=%d\n", img->x0, img->y0,
					img->x1, img->y1);
	fprintf(stderr, "  numcomps=%d\n", img->numcomps);
	for (compno = 0; compno < img->numcomps; compno++) {
		j2k_comp_t *comp = &img->comps[compno];
		fprintf(stderr, "  comp %d {\n", compno);
		fprintf(stderr, "    dx=%d, dy=%d\n", comp->dx, comp->dy);
		fprintf(stderr, "    prec=%d\n", comp->prec);
		fprintf(stderr, "    sgnd=%d\n", comp->sgnd);
		fprintf(stderr, "  }\n");
	}
	fprintf(stderr, "}\n");
}

void j2k_dump_cp(j2k_image_t * img, j2k_cp_t * cp)
{
	int tileno, compno, layno, bandno, resno, numbands;
	fprintf(stderr, "coding parameters {\n");
	fprintf(stderr, "  tx0=%d, ty0=%d\n", cp->tx0, cp->ty0);
	fprintf(stderr, "  tdx=%d, tdy=%d\n", cp->tdx, cp->tdy);
	fprintf(stderr, "  tw=%d, th=%d\n", cp->tw, cp->th);
	for (tileno = 0; tileno < cp->tw * cp->th; tileno++) {
		j2k_tcp_t *tcp = &cp->tcps[tileno];
		fprintf(stderr, "  tile %d {\n", tileno);
		fprintf(stderr, "    csty=%x\n", tcp->csty);
		fprintf(stderr, "    prg=%d\n", tcp->prg);
		fprintf(stderr, "    numlayers=%d\n", tcp->numlayers);
		fprintf(stderr, "    mct=%d\n", tcp->mct);
		fprintf(stderr, "    rates=");
		for (layno = 0; layno < tcp->numlayers; layno++) {
			fprintf(stderr, "%d ", tcp->rates[layno]);
		}
		fprintf(stderr, "\n");
		for (compno = 0; compno < img->numcomps; compno++) {
			j2k_tccp_t *tccp = &tcp->tccps[compno];
			fprintf(stderr, "    comp %d {\n", compno);
			fprintf(stderr, "      csty=%x\n", tccp->csty);
			fprintf(stderr, "      numresolutions=%d\n", tccp->numresolutions);
			fprintf(stderr, "      cblkw=%d\n", tccp->cblkw);
			fprintf(stderr, "      cblkh=%d\n", tccp->cblkh);
			fprintf(stderr, "      cblksty=%x\n", tccp->cblksty);
			fprintf(stderr, "      qmfbid=%d\n", tccp->qmfbid);
			fprintf(stderr, "      qntsty=%d\n", tccp->qntsty);
			fprintf(stderr, "      numgbits=%d\n", tccp->numgbits);
			fprintf(stderr, "      roishift=%d\n", tccp->roishift);
			fprintf(stderr, "      stepsizes=");
			numbands =
				tccp->qntsty ==
				J2K_CCP_QNTSTY_SIQNT ? 1 : tccp->numresolutions * 3 - 2;
			for (bandno = 0; bandno < numbands; bandno++) {
				fprintf(stderr, "(%d,%d) ", tccp->stepsizes[bandno].mant,
								tccp->stepsizes[bandno].expn);
			}
			fprintf(stderr, "\n");

			if (tccp->csty & J2K_CCP_CSTY_PRT) {
				fprintf(stderr, "      prcw=");
				for (resno = 0; resno < tccp->numresolutions; resno++) {
					fprintf(stderr, "%d ", tccp->prcw[resno]);
				}
				fprintf(stderr, "\n");
				fprintf(stderr, "      prch=");
				for (resno = 0; resno < tccp->numresolutions; resno++) {
					fprintf(stderr, "%d ", tccp->prch[resno]);
				}
				fprintf(stderr, "\n");
			}
			fprintf(stderr, "    }\n");
		}
		fprintf(stderr, "  }\n");
	}
	fprintf(stderr, "}\n");
}

void j2k_write_soc()
{
	/* fprintf(stderr, "%.8x: SOC\n", cio_tell()); */
	cio_write(J2K_MS_SOC, 2);
}

void j2k_read_soc()
{
	/* fprintf(stderr, "%.8x: SOC\n", cio_tell()-2); */
	j2k_state = J2K_STATE_MHSIZ;
}

void j2k_write_siz()
{
	int i;
	int lenp, len;
	/* fprintf(stderr, "%.8x: SIZ\n", cio_tell()); */
	cio_write(J2K_MS_SIZ, 2);			/* SIZ */
	lenp = cio_tell();
	cio_skip(2);
	cio_write(0, 2);							/* Rsiz (capabilities) */
	cio_write(j2k_img->x1, 4);		/* Xsiz */
	cio_write(j2k_img->y1, 4);		/* Ysiz */
	cio_write(j2k_img->x0, 4);		/* X0siz */
	cio_write(j2k_img->y0, 4);		/* Y0siz */
	cio_write(j2k_cp->tdx, 4);		/* XTsiz */
	cio_write(j2k_cp->tdy, 4);		/* YTsiz */
	cio_write(j2k_cp->tx0, 4);		/* XT0siz */
	cio_write(j2k_cp->ty0, 4);		/* YT0siz */
	cio_write(j2k_img->numcomps, 2);	/* Csiz */
	for (i = 0; i < j2k_img->numcomps; i++) {
		cio_write(j2k_img->comps[i].prec - 1 + (j2k_img->comps[i].sgnd << 7), 1);	/* Ssiz_i */
		cio_write(j2k_img->comps[i].dx, 1);	/* XRsiz_i */
		cio_write(j2k_img->comps[i].dy, 1);	/* YRsiz_i */
	}
	len = cio_tell() - lenp;
	cio_seek(lenp);
	cio_write(len, 2);						/* Lsiz  */
	cio_seek(lenp + len);
}

void j2k_read_siz()
{
	int len, i;
	/* fprintf(stderr, "%.8x: SIZ\n", cio_tell()); */
	len = cio_read(2);
	cio_read(2);
	j2k_img->x1 = cio_read(4);
	j2k_img->y1 = cio_read(4);
	j2k_img->x0 = cio_read(4);
	j2k_img->y0 = cio_read(4);
	j2k_cp->tdx = cio_read(4);
	j2k_cp->tdy = cio_read(4);
	j2k_cp->tx0 = cio_read(4);
	j2k_cp->ty0 = cio_read(4);

	j2k_img->numcomps = cio_read(2);
	j2k_img->comps =
		(j2k_comp_t *) malloc(j2k_img->numcomps * sizeof(j2k_comp_t));
	for (i = 0; i < j2k_img->numcomps; i++) {
		int tmp, w, h;
		tmp = cio_read(1);
		j2k_img->comps[i].prec = (tmp & 0x7f) + 1;
		j2k_img->comps[i].sgnd = tmp >> 7;
		j2k_img->comps[i].dx = cio_read(1);
		j2k_img->comps[i].dy = cio_read(1);
		w = int_ceildiv(j2k_img->x1 - j2k_img->x0, j2k_img->comps[i].dx);
		h = int_ceildiv(j2k_img->y1 - j2k_img->y0, j2k_img->comps[i].dy);
		j2k_img->comps[i].data = (int *) malloc(sizeof(int) * w * h);
	}

	j2k_cp->tw = int_ceildiv(j2k_img->x1 - j2k_cp->tx0, j2k_cp->tdx);
	j2k_cp->th = int_ceildiv(j2k_img->y1 - j2k_cp->ty0, j2k_cp->tdy);
	j2k_cp->tcps =
		(j2k_tcp_t *) calloc(sizeof(j2k_tcp_t), j2k_cp->tw * j2k_cp->th);
	j2k_default_tcp.tccps =
		(j2k_tccp_t *) calloc(sizeof(j2k_tccp_t), j2k_img->numcomps);
	for (i = 0; i < j2k_cp->tw * j2k_cp->th; i++) {
		j2k_cp->tcps[i].tccps =
			(j2k_tccp_t *) calloc(sizeof(j2k_tccp_t), j2k_img->numcomps);
	}
	j2k_tile_data =
		(unsigned char **) calloc(j2k_cp->tw * j2k_cp->th, sizeof(char *));
	j2k_tile_len = (int *) calloc(j2k_cp->tw * j2k_cp->th, sizeof(int));
	j2k_state = J2K_STATE_MH;
}

void j2k_write_com()
{
	unsigned int i;
	int lenp, len;
	char str[256];
	sprintf(str, "%s", j2k_cp->comment);
	/* fprintf(stderr, "%.8x: COM\n", cio_tell()); */
	cio_write(J2K_MS_COM, 2);
	lenp = cio_tell();
	cio_skip(2);
	cio_write(0, 2);
	for (i = 0; i < strlen(str); i++) {
		cio_write(str[i], 1);
	}
	len = cio_tell() - lenp;
	cio_seek(lenp);
	cio_write(len, 2);
	cio_seek(lenp + len);

}

void j2k_read_com()
{
	int len;
	/* fprintf(stderr, "%.8x: COM\n", cio_tell()-2); */
	len = cio_read(2);
	cio_skip(len - 2);
}

void j2k_write_cox(int compno)
{
	int i;
	j2k_tcp_t *tcp;
	j2k_tccp_t *tccp;
	tcp = &j2k_cp->tcps[j2k_curtileno];
	tccp = &tcp->tccps[compno];
	cio_write(tccp->numresolutions - 1, 1);	/* SPcox (D) */
	cio_write(tccp->cblkw - 2, 1);	/* SPcox (E) */
	cio_write(tccp->cblkh - 2, 1);	/* SPcox (F) */
	cio_write(tccp->cblksty, 1);	/* SPcox (G) */
	cio_write(tccp->qmfbid, 1);		/* SPcox (H) */
	if (tccp->csty & J2K_CCP_CSTY_PRT) {
		for (i = 0; i < tccp->numresolutions; i++) {
			cio_write(tccp->prcw[i] + (tccp->prch[i] << 4), 1);	/* SPcox (I_i) */
		}
	}
}

void j2k_read_cox(int compno)
{
	int i;
	j2k_tcp_t *tcp;
	j2k_tccp_t *tccp;
	tcp =
		j2k_state ==
		J2K_STATE_TPH ? &j2k_cp->tcps[j2k_curtileno] : &j2k_default_tcp;
	tccp = &tcp->tccps[compno];
	tccp->numresolutions = cio_read(1) + 1;
	tccp->cblkw = cio_read(1) + 2;
	tccp->cblkh = cio_read(1) + 2;
	tccp->cblksty = cio_read(1);
	tccp->qmfbid = cio_read(1);
	if (tccp->csty & J2K_CP_CSTY_PRT) {
		for (i = 0; i < tccp->numresolutions; i++) {
			int tmp = cio_read(1);
			tccp->prcw[i] = tmp & 0xf;
			tccp->prch[i] = tmp >> 4;
		}
	}
}

void j2k_write_cod()
{
	j2k_tcp_t *tcp;
	int lenp, len;
	/* fprintf(stderr, "%.8x: COD\n", cio_tell()+pos_correction); */
	cio_write(J2K_MS_COD, 2);			/* COD */
	lenp = cio_tell();
	cio_skip(2);
	tcp = &j2k_cp->tcps[j2k_curtileno];
	cio_write(tcp->csty, 1);			/* Scod */
	cio_write(tcp->prg, 1);				/* SGcod (A) */
	cio_write(tcp->numlayers, 2);	/* SGcod (B) */
	cio_write(tcp->mct, 1);				/* SGcod (C) */
	j2k_write_cox(0);
	len = cio_tell() - lenp;
	cio_seek(lenp);
	cio_write(len, 2);						/* Lcod */
	cio_seek(lenp + len);
}

void j2k_read_cod()
{
	int len, i, pos;
	j2k_tcp_t *tcp;
	/* fprintf(stderr, "%.8x: COD\n", cio_tell()-2); */
	tcp =
		j2k_state ==
		J2K_STATE_TPH ? &j2k_cp->tcps[j2k_curtileno] : &j2k_default_tcp;
	len = cio_read(2);
	tcp->csty = cio_read(1);
	tcp->prg = cio_read(1);
	tcp->numlayers = cio_read(2);
	tcp->mct = cio_read(1);
	pos = cio_tell();
	for (i = 0; i < j2k_img->numcomps; i++) {
		tcp->tccps[i].csty = tcp->csty & J2K_CP_CSTY_PRT;
		cio_seek(pos);
		j2k_read_cox(i);
	}
}

void j2k_write_coc(int compno)
{
	j2k_tcp_t *tcp;
	int lenp, len;
	/* fprintf(stderr, "%.8x: COC\n", cio_tell()+pos_correction); */
	cio_write(J2K_MS_COC, 2);			/* COC */
	lenp = cio_tell();
	cio_skip(2);
	tcp = &j2k_cp->tcps[j2k_curtileno];
	cio_write(compno, j2k_img->numcomps <= 256 ? 1 : 2);	/* Ccoc */
	cio_write(tcp->tccps[compno].csty, 1);	/* Scoc */
	j2k_write_cox(compno);
	len = cio_tell() - lenp;
	cio_seek(lenp);
	cio_write(len, 2);						/* Lcoc */
	cio_seek(lenp + len);
}

void j2k_read_coc()
{
	int len, compno;
	j2k_tcp_t *tcp;
	/* fprintf(stderr, "%.8x: COC\n", cio_tell()); */
	tcp =
		j2k_state ==
		J2K_STATE_TPH ? &j2k_cp->tcps[j2k_curtileno] : &j2k_default_tcp;
	len = cio_read(2);
	compno = cio_read(j2k_img->numcomps <= 256 ? 1 : 2);
	tcp->tccps[compno].csty = cio_read(1);
	j2k_read_cox(compno);
}

void j2k_write_qcx(int compno)
{
	j2k_tcp_t *tcp;
	j2k_tccp_t *tccp;
	int bandno, numbands;
	int expn, mant;
	tcp = &j2k_cp->tcps[j2k_curtileno];
	tccp = &tcp->tccps[compno];

	cio_write(tccp->qntsty + (tccp->numgbits << 5), 1);	/* Sqcx */
	numbands =
		tccp->qntsty ==
		J2K_CCP_QNTSTY_SIQNT ? 1 : tccp->numresolutions * 3 - 2;

	for (bandno = 0; bandno < numbands; bandno++) {
		expn = tccp->stepsizes[bandno].expn;
		mant = tccp->stepsizes[bandno].mant;

		if (tccp->qntsty == J2K_CCP_QNTSTY_NOQNT) {
			cio_write(expn << 3, 1);	/* SPqcx_i */
		} else {
			cio_write((expn << 11) + mant, 2);	/* SPqcx_i */
		}
	}

}

void j2k_read_qcx(int compno, int len)
{
	int tmp;
	j2k_tcp_t *tcp;
	j2k_tccp_t *tccp;
	int bandno, numbands;
	tcp =
		j2k_state ==
		J2K_STATE_TPH ? &j2k_cp->tcps[j2k_curtileno] : &j2k_default_tcp;
	tccp = &tcp->tccps[compno];
	tmp = cio_read(1);
	tccp->qntsty = tmp & 0x1f;
	tccp->numgbits = tmp >> 5;
	numbands =
		tccp->qntsty == J2K_CCP_QNTSTY_SIQNT ? 1 : (tccp->qntsty ==
																								J2K_CCP_QNTSTY_NOQNT ? len
																								- 1 : (len - 1) / 2);
	for (bandno = 0; bandno < numbands; bandno++) {
		int expn, mant;
		if (tccp->qntsty == J2K_CCP_QNTSTY_NOQNT) {	/* WHY STEPSIZES WHEN NOQNT ? */
			expn = cio_read(1) >> 3;
			mant = 0;
		} else {
			tmp = cio_read(2);
			expn = tmp >> 11;
			mant = tmp & 0x7ff;
		}
		tccp->stepsizes[bandno].expn = expn;
		tccp->stepsizes[bandno].mant = mant;
	}
}

void j2k_write_qcd()
{
	int lenp, len;
	/* fprintf(stderr, "%.8x: QCD\n", cio_tell()+pos_correction); */
	cio_write(J2K_MS_QCD, 2);			/* QCD */
	lenp = cio_tell();
	cio_skip(2);
	j2k_write_qcx(0);
	len = cio_tell() - lenp;
	cio_seek(lenp);
	cio_write(len, 2);						/* Lqcd */
	cio_seek(lenp + len);
}

void j2k_read_qcd()
{
	int len, i, pos;
	/* fprintf(stderr, "%.8x: QCD\n", cio_tell()-2); */
	len = cio_read(2);
	pos = cio_tell();
	for (i = 0; i < j2k_img->numcomps; i++) {
		cio_seek(pos);
		j2k_read_qcx(i, len - 2);
	}
}

void j2k_write_qcc(int compno)
{
	int lenp, len;
	/* fprintf(stderr, "%.8x: QCC\n", cio_tell()+pos_correction); */
	cio_write(J2K_MS_QCC, 2);
	lenp = cio_tell();
	cio_skip(2);
	cio_write(compno, j2k_img->numcomps <= 256 ? 1 : 2);
	j2k_write_qcx(compno);
	len = cio_tell() - lenp;
	cio_seek(lenp);
	cio_write(len, 2);
	cio_seek(lenp + len);
}

void j2k_read_qcc()
{
	int len, compno;
	/* fprintf(stderr, "%.8x: QCC\n", cio_tell()-2); */
	len = cio_read(2);
	compno = cio_read(j2k_img->numcomps <= 256 ? 1 : 2);
	j2k_read_qcx(compno, len - 2 - (j2k_img->numcomps <= 256 ? 1 : 2));
}

void j2k_write_poc()
{
	int len, numpchgs, i;
	j2k_tcp_t *tcp;
	fprintf(stderr, "%.8x: POC\n", cio_tell() + pos_correction);
	tcp = &j2k_cp->tcps[j2k_curtileno];
	numpchgs = tcp->numpocs;
	cio_write(J2K_MS_POC, 2);
	len = 2 + (5 + 2 * (j2k_img->numcomps <= 256 ? 1 : 2)) * numpchgs;
	cio_write(len, 2);
	for (i = 0; i < numpchgs; i++) {
		j2k_poc_t *poc;
		poc = &tcp->pocs[i];
		cio_write(poc->resno0, 1);
		cio_write(poc->compno0, (j2k_img->numcomps <= 256 ? 1 : 2));
		cio_write(poc->layno1, 2);
		cio_write(poc->resno1, 1);
		cio_write(poc->compno1, (j2k_img->numcomps <= 256 ? 1 : 2));
		cio_write(poc->prg, 1);
	}
}

void j2k_read_poc()
{
	int len, numpchgs, i;
	j2k_tcp_t *tcp;
	fprintf(stderr,
					"WARNING: POC marker segment processing not fully implemented %d\n",
					j2k_curtileno);
	tcp =
		j2k_state ==
		J2K_STATE_TPH ? &j2k_cp->tcps[j2k_curtileno] : &j2k_default_tcp;
	len = cio_read(2);
	numpchgs = (len - 2) / (5 + 2 * (j2k_img->numcomps <= 256 ? 1 : 2));
	for (i = 0; i < numpchgs; i++) {
		j2k_poc_t *poc;
		poc = &tcp->pocs[i];
		poc->resno0 = cio_read(1);
		poc->compno0 = cio_read(j2k_img->numcomps <= 256 ? 1 : 2);
		poc->layno1 = cio_read(2);
		poc->resno1 = cio_read(1);
		poc->compno1 = cio_read(j2k_img->numcomps <= 256 ? 1 : 2);
		poc->prg = cio_read(1);
	}
	tcp->numpocs = numpchgs;
}

void j2k_read_crg()
{
	int len;
	len = cio_read(2);
	fprintf(stderr,
					"WARNING: CRG marker segment processing not implemented\n");
	cio_skip(len - 2);
}

void j2k_read_tlm()
{
	int len;
	len = cio_read(2);
	fprintf(stderr,
					"WARNING: TLM marker segment processing not implemented\n");
	cio_skip(len - 2);
}

void j2k_read_plm()
{
	int len;
	len = cio_read(2);
	fprintf(stderr,
					"WARNING: PLM marker segment processing not implemented\n");
	cio_skip(len - 2);
}

void j2k_read_plt()
{
	int len;
	len = cio_read(2);
	fprintf(stderr,
					"WARNING: PLT marker segment processing not implemented\n");
	cio_skip(len - 2);
}

void j2k_read_ppm()
{
	int len;
	len = cio_read(2);
	fprintf(stderr,
					"WARNING: PPM marker segment processing not implemented\n");
	cio_skip(len - 2);
}

void j2k_read_ppt()
{
	int len;
	len = cio_read(2);
	fprintf(stderr,
					"WARNING: PPT marker segment processing not implemented\n");
	cio_skip(len - 2);
}

void j2k_write_sot()
{
	int lenp, len;
	/* fprintf(stderr, "%.8x: SOT\n", cio_tell()+pos_correction); */
	j2k_sot_start = cio_tell();
	cio_write(J2K_MS_SOT, 2);			/* SOT */
	lenp = cio_tell();
	cio_skip(2);									/* Lsot (further) */
	cio_write(j2k_curtileno, 2);	/* Isot */
	cio_skip(4);									/* Psot (further in j2k_write_sod) */
	cio_write(0, 1);							/* TPsot */
	cio_write(1, 1);							/* TNsot */
	len = cio_tell() - lenp;
	cio_seek(lenp);
	cio_write(len, 2);						/* Lsot */
	cio_seek(lenp + len);
}

void j2k_read_sot()
{
	int len, tileno, totlen, partno, numparts, i;
	j2k_tcp_t *tcp;
	j2k_tccp_t *tmp;
	/* fprintf(stderr, "%.8x: SOT\n", cio_tell()-2); */
	len = cio_read(2);
	tileno = cio_read(2);
	totlen = cio_read(4);
	if (!totlen)
		totlen = cio_numbytesleft() + 8;

	partno = cio_read(1);
	numparts = cio_read(1);
	j2k_curtileno = tileno;
	j2k_eot = cio_getbp() - 12 + totlen;
	j2k_state = J2K_STATE_TPH;
	tcp = &j2k_cp->tcps[j2k_curtileno];
	tmp = tcp->tccps;
	*tcp = j2k_default_tcp;
	tcp->tccps = tmp;
	for (i = 0; i < j2k_img->numcomps; i++) {
		tcp->tccps[i] = j2k_default_tcp.tccps[i];
	}
}

void j2k_write_sod()
{
	int l, layno;
	int totlen;
	j2k_tcp_t *tcp;
	static int j2k_sod_start;

	/* fprintf(stderr, "%.8x: SOD\n", cio_tell()+pos_correction); */
	cio_write(J2K_MS_SOD, 2);
	if (j2k_curtileno == 0) {
		j2k_sod_start = cio_tell() + pos_correction;
	}

	/* INDEX >> */
	if (info_IM.index_on) {
		info_IM.tile[j2k_curtileno].end_header =
			cio_tell() + pos_correction - 1;
		info_IM.tile[j2k_curtileno].packet =
			(info_packet *) malloc(info_IM.Comp * info_IM.Layer *
														 (info_IM.Decomposition +
															1) * 100 * sizeof(info_packet));
	}
	/* << INDEX */

	tcp = &j2k_cp->tcps[j2k_curtileno];
	for (layno = 0; layno < tcp->numlayers; layno++) {
		tcp->rates[layno] -= (j2k_sod_start / (j2k_cp->th * j2k_cp->tw));
		/* fprintf(stderr, "tcp->rates[%d]=%d\n", layno, tcp->rates[layno]); */
	}

	info_IM.num = 0;
	if (j2k_cp->image_type)
		l =
			tcd_encode_tile_pxm(j2k_curtileno, cio_getbp(),
													cio_numbytesleft() - 2, &info_IM);
	else
		l =
			tcd_encode_tile_pgx(j2k_curtileno, cio_getbp(),
													cio_numbytesleft() - 2, &info_IM);

	/* Writing Psot in SOT marker */
	totlen = cio_tell() + l - j2k_sot_start;
	cio_seek(j2k_sot_start + 6);
	cio_write(totlen, 4);
	cio_seek(j2k_sot_start + totlen);
}

void j2k_read_sod()
{
	int len, truncate = 0;
	unsigned char *data;

	/* fprintf(stderr, "%.8x: SOD\n", cio_tell()-2); */
	len = int_min(j2k_eot - cio_getbp(), cio_numbytesleft() + 1);
	if (len == cio_numbytesleft() + 1)
		truncate = 1;								/* Case of a truncate codestream */

	j2k_tile_len[j2k_curtileno] += len;
	data =
		(unsigned char *) realloc(j2k_tile_data[j2k_curtileno],
															j2k_tile_len[j2k_curtileno]);
	memcpy(data, cio_getbp(), len);
	j2k_tile_data[j2k_curtileno] = data;

	cio_skip(len);

	if (!truncate)
		j2k_state = J2K_STATE_TPHSOT;
	else
		j2k_state = J2K_STATE_NEOC;	/* RAJOUTE !! */
}

void j2k_write_rgn(int compno, int tileno)
{
	j2k_tcp_t *tcp = &j2k_cp->tcps[tileno];
	/* fprintf(stderr, "%.8x: RGN\n",cio_tell()+pos_correction); */
	cio_write(J2K_MS_RGN, 2);			/* RGN */
	cio_write(j2k_img->numcomps <= 256 ? 5 : 6, 2);	/* Lrgn */
	cio_write(compno, j2k_img->numcomps <= 256 ? 1 : 2);	/* Crgn */
	cio_write(0, 1);							/* Srgn */
	cio_write(tcp->tccps[compno].roishift, 1);	/* SPrgn */
}

void j2k_read_rgn()
{
	int len, compno, roisty;
	j2k_tcp_t *tcp;
	fprintf(stderr, "%.8x: RGN\n", cio_tell() - 2);
	tcp =
		j2k_state ==
		J2K_STATE_TPH ? &j2k_cp->tcps[j2k_curtileno] : &j2k_default_tcp;
	len = cio_read(2);
	compno = cio_read(j2k_img->numcomps <= 256 ? 1 : 2);
	roisty = cio_read(1);
	tcp->tccps[compno].roishift = cio_read(1);
}

void j2k_write_eoc()
{
	fprintf(stderr, "%.8x: EOC\n", cio_tell() + pos_correction);
	cio_write(J2K_MS_EOC, 2);
}

void j2k_read_eoc()
{
	int tileno;
	/* fprintf(stderr, "%.8x: EOC\n", cio_tell()-2); */
	/* j2k_dump_image(j2k_img); */
	/* j2k_dump_cp(j2k_img, j2k_cp); */

	tcd_init(j2k_img, j2k_cp);
	for (tileno = 0; tileno < j2k_cp->tw * j2k_cp->th; tileno++) {
		tcd_decode_tile(j2k_tile_data[tileno], j2k_tile_len[tileno], tileno);
	}

	j2k_state = J2K_STATE_MT;
	longjmp(j2k_error, 1);

}

void j2k_read_unk()
{
	fprintf(stderr, "warning: unknown marker\n");
}

LIBJ2K_API int j2k_encode(j2k_image_t * img, j2k_cp_t * cp, char *outfile,
													int len, char *index)
{


	int tileno, compno, layno, resno, precno, pack_nb;
	char *dest;
	FILE *INDEX;
	FILE *f;

	if (setjmp(j2k_error)) {
		return 0;
	}

	f = fopen(outfile, "wb");

	if (!f) {
		fprintf(stderr, "failed to open %s for writing\n", outfile);
		return 1;
	}

	dest = (char *) malloc(len);
	cio_init(dest, len);

	j2k_img = img;
	j2k_cp = cp;
	/* j2k_dump_cp(j2k_img, j2k_cp); */

	/* INDEX >> */
	info_IM.index_on = j2k_img->index_on;
	if (info_IM.index_on) {
		info_IM.tile =
			(info_tile *) malloc(j2k_cp->tw * j2k_cp->th * sizeof(info_tile));
		info_IM.Im_w = j2k_img->x1 - j2k_img->x0;
		info_IM.Im_h = j2k_img->y1 - j2k_img->y0;
		info_IM.Prog = (&j2k_cp->tcps[0])->prg;
		/* info_IM.Tile_x=j2k_cp->tw; old version parser */
		/* info_IM.Tile_y=j2k_cp->th; old version parser */
		info_IM.Tile_x = j2k_cp->tdx;	/* new version parser */
		info_IM.Tile_y = j2k_cp->tdy;	/* new version parser */
		info_IM.Comp = j2k_img->numcomps;
		info_IM.Layer = (&j2k_cp->tcps[0])->numlayers;
		info_IM.Decomposition = (&j2k_cp->tcps[0])->tccps->numresolutions - 1;
		info_IM.D_max = 0;					/* ADD Marcela */
	}
	/* << INDEX */

	j2k_write_soc();
	j2k_write_siz();
	j2k_write_cod();
	j2k_write_qcd();

	for (compno = 0; compno < j2k_img->numcomps; compno++) {
		j2k_tcp_t *tcp = &j2k_cp->tcps[0];
		if (tcp->tccps[compno].roishift)
			j2k_write_rgn(compno, 0);
	}
	if (j2k_cp->comment != NULL)
		j2k_write_com();

	/* Writing the main header */
	pos_correction = cio_tell();
	fwrite(dest, 1, cio_tell(), f);

	/* INDEX >> */
	if (info_IM.index_on) {
		info_IM.Main_head_end = cio_tell() - 1;
	}
	/* << INDEX */

	for (tileno = 0; tileno < cp->tw * cp->th; tileno++) {
		fprintf(stderr, "\nTile number %d / %d \n", tileno + 1,
						cp->tw * cp->th);

		/* new dest for each tile  */
		free(dest);
		dest = (char *) malloc(len);
		cio_init(dest, len);
		j2k_curtileno = tileno;
		/* initialisation before tile encoding  */

		if (tileno == 0) {
			tcd_malloc_encode(j2k_img, j2k_cp, j2k_curtileno);
		} else {
			tcd_init_encode(j2k_img, j2k_cp, j2k_curtileno);
		}

		/* INDEX >> */
		if (info_IM.index_on) {
			info_IM.tile[j2k_curtileno].num_tile = j2k_curtileno;
			info_IM.tile[j2k_curtileno].start_pos = cio_tell() + pos_correction;
		}
		/* << INDEX */
		j2k_write_sot();

		for (compno = 1; compno < img->numcomps; compno++) {
			j2k_write_coc(compno);
			j2k_write_qcc(compno);
		}

		if (cp->tcps[tileno].numpocs)
			j2k_write_poc();
		j2k_write_sod();

		/* INDEX >> */
		if (info_IM.index_on) {
			info_IM.tile[j2k_curtileno].end_pos =
				cio_tell() + pos_correction - 1;
		}
		/* << INDEX */

		/*
		   if (tile->PPT)  BAD PPT !!!
		   {
		   FILE *PPT_file;

		   int i;
		   PPT_file=fopen("PPT","rb");
		   fprintf(stderr,"%c%c%c%c",255,97,tile->len_ppt/256,tile->len_ppt%256);
		   for (i=0;i<tile->len_ppt;i++)
		   {
		   unsigned char elmt;
		   fread(&elmt, 1, 1, PPT_file);
		   fwrite(&elmt,1,1,f);
		   }
		   fclose(PPT_file);
		   unlink("PPT");
		   }
		 */

		fwrite(dest, 1, cio_tell(), f);
		pos_correction = cio_tell() + pos_correction;
	}

	free(dest);
	dest = (char *) malloc(len);
	cio_init(dest, len);

	j2k_write_eoc();

	fwrite(dest, 1, 2, f);
	free(dest);
	/* closing file *.j2k */
	fclose(f);

	/* Creation of the index file     */
	if (info_IM.index_on) {
		info_IM.codestream_size = cio_tell() + pos_correction;	/* Correction 14/4/03 suite rmq de Patrick */
		INDEX = fopen(index, "w");

		if (!INDEX) {
			fprintf(stderr, "failed to open %s for writing\n", index);
			return 1;
		}

		fprintf(INDEX, "%d %d\n", info_IM.Im_w, info_IM.Im_h);
		fprintf(INDEX, "%d\n", info_IM.Prog);
		fprintf(INDEX, "%d %d\n", info_IM.Tile_x, info_IM.Tile_y);
		fprintf(INDEX, "%d\n", info_IM.Comp);
		fprintf(INDEX, "%d\n", info_IM.Layer);
		fprintf(INDEX, "%d\n", info_IM.Decomposition);
		fprintf(INDEX, "%d %d\n", info_IM.pdx, info_IM.pdy);
		fprintf(INDEX, "%d\n", info_IM.Main_head_end);
		fprintf(INDEX, "%d\n", info_IM.codestream_size);

		for (tileno = 0; tileno < j2k_cp->tw * j2k_cp->th; tileno++) {
			fprintf(INDEX, "%d %d %d %d\n", info_IM.tile[tileno].num_tile,
							info_IM.tile[tileno].start_pos,
							info_IM.tile[tileno].end_header,
							info_IM.tile[tileno].end_pos);
		}
		for (tileno = 0; tileno < j2k_cp->tw * j2k_cp->th; tileno++) {
			pack_nb = 0;
			if (info_IM.Prog == 0) {	/* LRCP */
				for (layno = 0; layno < info_IM.Layer; layno++) {
					for (resno = 0; resno < info_IM.Decomposition + 1; resno++) {
						for (compno = 0; compno < info_IM.Comp; compno++) {
							for (precno = 0;
									 precno <
									 info_IM.tile[tileno].pw * info_IM.tile[tileno].ph;
									 precno++) {
								/* fprintf(INDEX,"%d %d %d %d %d %d %d %d %.04f\n",pack_nb,tileno,layno,resno,compno,precno,info_IM.tile[tileno].packet[pack_nb].start_pos,info_IM.tile[tileno].packet[pack_nb].end_pos,info_IM.tile[tileno].packet[pack_nb].disto/info_IM.D_max); */
								fprintf(INDEX, "%d %d %d %d %d %d %d %d\n", pack_nb,
												tileno, layno, resno, compno, precno,
												info_IM.tile[tileno].packet[pack_nb].start_pos,
												info_IM.tile[tileno].packet[pack_nb].end_pos);
								pack_nb++;
							}
						}
					}
				}
			} else if (info_IM.Prog == 1) {	/* RLCP */
				for (resno = 0; resno < info_IM.Decomposition + 1; resno++) {
					for (layno = 0; layno < info_IM.Layer; layno++) {
						for (compno = 0; compno < info_IM.Comp; compno++) {
							for (precno = 0; precno < info_IM.pw * info_IM.ph; precno++) {
								/* fprintf(INDEX,"%d %d %d %d %d %d %d %d %.04f\n",pack_nb,tileno,layno,resno,compno,precno,info_IM.tile[tileno].packet[pack_nb].start_pos,info_IM.tile[tileno].packet[pack_nb].end_pos,info_IM.tile[tileno].packet[pack_nb].disto/info_IM.D_max); */
								fprintf(INDEX, "%d %d %d %d %d %d %d %d\n", pack_nb,
												tileno, layno, resno, compno, precno,
												info_IM.tile[tileno].packet[pack_nb].start_pos,
												info_IM.tile[tileno].packet[pack_nb].end_pos);
								pack_nb++;
							}
						}
					}
				}
			} else if (info_IM.Prog == 2) {	/* RPCL */
				for (resno = 0; resno < info_IM.Decomposition + 1; resno++) {
					for (precno = 0; precno < info_IM.pw * info_IM.ph; precno++) {
						for (compno = 0; compno < info_IM.Comp; compno++) {
							for (layno = 0; layno < info_IM.Layer; layno++) {
								/* fprintf(INDEX,"%d %d %d %d %d %d %d %d %.04f\n",pack_nb,tileno,layno,resno,compno,precno,info_IM.tile[tileno].packet[pack_nb].start_pos,info_IM.tile[tileno].packet[pack_nb].end_pos,info_IM.tile[tileno].packet[pack_nb].disto/info_IM.D_max); */
								fprintf(INDEX, "%d %d %d %d %d %d %d %d\n", pack_nb,
												tileno, layno, resno, compno, precno,
												info_IM.tile[tileno].packet[pack_nb].start_pos,
												info_IM.tile[tileno].packet[pack_nb].end_pos);
								pack_nb++;
							}
						}
					}
				}
			} else if (info_IM.Prog == 3) {	/* PCRL */
				for (precno = 0; precno < info_IM.pw * info_IM.ph; precno++) {
					for (compno = 0; compno < info_IM.Comp; compno++) {
						for (resno = 0; resno < info_IM.Decomposition + 1; resno++) {
							for (layno = 0; layno < info_IM.Layer; layno++) {
								/* fprintf(INDEX,"%d %d %d %d %d %d %d %d %.04f\n",pack_nb,tileno,layno,resno,compno,precno,info_IM.tile[tileno].packet[pack_nb].start_pos,info_IM.tile[tileno].packet[pack_nb].end_pos,info_IM.tile[tileno].packet[pack_nb].disto/info_IM.D_max); */
								fprintf(INDEX, "%d %d %d %d %d %d %d %d\n", pack_nb,
												tileno, layno, resno, compno, precno,
												info_IM.tile[tileno].packet[pack_nb].start_pos,
												info_IM.tile[tileno].packet[pack_nb].end_pos);
								pack_nb++;
							}
						}
					}
				}
			} else {									/* CPRL */

				for (compno = 0; compno < info_IM.Comp; compno++) {
					for (precno = 0; precno < info_IM.pw * info_IM.ph; precno++) {
						for (resno = 0; resno < info_IM.Decomposition + 1; resno++) {
							for (layno = 0; layno < info_IM.Layer; layno++) {
								/* fprintf(INDEX,"%d %d %d %d %d %d %d %d %.04f\n",pack_nb,tileno,layno,resno,compno,precno,info_IM.tile[tileno].packet[pack_nb].start_pos,info_IM.tile[tileno].packet[pack_nb].end_pos,info_IM.tile[tileno].packet[pack_nb].disto/info_IM.D_max); */
								fprintf(INDEX, "%d %d %d %d %d %d %d %d\n", pack_nb,
												tileno, layno, resno, compno, precno,
												info_IM.tile[tileno].packet[pack_nb].start_pos,
												info_IM.tile[tileno].packet[pack_nb].end_pos);
								pack_nb++;
							}
						}
					}
				}
			}
		}
		fclose(INDEX);
	}

	j2k_clean();

	return cio_tell();
}

typedef struct {
	int id;
	int states;
	void (*handler) ();
} j2k_dec_mstabent_t;

j2k_dec_mstabent_t j2k_dec_mstab[] = {
	{J2K_MS_SOC, J2K_STATE_MHSOC, j2k_read_soc},
	{J2K_MS_SOT, J2K_STATE_MH | J2K_STATE_TPHSOT, j2k_read_sot},
	{J2K_MS_SOD, J2K_STATE_TPH, j2k_read_sod},
	{J2K_MS_EOC, J2K_STATE_TPHSOT, j2k_read_eoc},
	{J2K_MS_SIZ, J2K_STATE_MHSIZ, j2k_read_siz},
	{J2K_MS_COD, J2K_STATE_MH | J2K_STATE_TPH, j2k_read_cod},
	{J2K_MS_COC, J2K_STATE_MH | J2K_STATE_TPH, j2k_read_coc},
	{J2K_MS_RGN, J2K_STATE_MH | J2K_STATE_TPH, j2k_read_rgn},
	{J2K_MS_QCD, J2K_STATE_MH | J2K_STATE_TPH, j2k_read_qcd},
	{J2K_MS_QCC, J2K_STATE_MH | J2K_STATE_TPH, j2k_read_qcc},
	{J2K_MS_POC, J2K_STATE_MH | J2K_STATE_TPH, j2k_read_poc},
	{J2K_MS_TLM, J2K_STATE_MH, j2k_read_tlm},
	{J2K_MS_PLM, J2K_STATE_MH, j2k_read_plm},
	{J2K_MS_PLT, J2K_STATE_TPH, j2k_read_plt},
	{J2K_MS_PPM, J2K_STATE_MH, j2k_read_ppm},
	{J2K_MS_PPT, J2K_STATE_TPH, j2k_read_ppt},
	{J2K_MS_SOP, 0, 0},
	{J2K_MS_CRG, J2K_STATE_MH, j2k_read_crg},
	{J2K_MS_COM, J2K_STATE_MH | J2K_STATE_TPH, j2k_read_com},
	{0, J2K_STATE_MH | J2K_STATE_TPH, j2k_read_unk}
};

j2k_dec_mstabent_t *j2k_dec_mstab_lookup(int id)
{
	j2k_dec_mstabent_t *e;
	for (e = j2k_dec_mstab; e->id != 0; e++) {
		if (e->id == id) {
			break;
		}
	}
	return e;
}

LIBJ2K_API int j2k_decode(unsigned char *src, int len, j2k_image_t ** img,
													j2k_cp_t ** cp)
{

	if (setjmp(j2k_error)) {
		if (j2k_state != J2K_STATE_MT) {
			fprintf(stderr, "WARNING: incomplete bitstream\n");
			return 0;
		}
		return cio_numbytes();
	}

	j2k_img = (j2k_image_t *) malloc(sizeof(j2k_image_t));
	j2k_cp = (j2k_cp_t *) malloc(sizeof(j2k_cp_t));
	*img = j2k_img;
	*cp = j2k_cp;
	j2k_state = J2K_STATE_MHSOC;
	cio_init(src, len);

	for (;;) {
		j2k_dec_mstabent_t *e;
		int id = cio_read(2);
		if (id >> 8 != 0xff) {
			fprintf(stderr, "%.8x: expected a marker instead of %x\n",
							cio_tell() - 2, id);
			return 0;
		}
		e = j2k_dec_mstab_lookup(id);
		if (!(j2k_state & e->states)) {
			fprintf(stderr, "%.8x: unexpected marker %x\n", cio_tell() - 2, id);
			return 0;
		}
		if (e->handler) {
			(*e->handler) ();
		}
		if (j2k_state == J2K_STATE_NEOC)
			break;										/* RAJOUTE */
	}
	if (j2k_state == J2K_STATE_NEOC)
		j2k_read_eoc();							/* RAJOUTE */

	return 0;
}

#ifdef WIN32
#include <windows.h>

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
											LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif
