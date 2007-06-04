/*
 * Copyright (c) 2002-2007, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2007, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux and Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * Copyright (c) 2007, Callum Lerwick <seg@haxxed.com>
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

#include "opj_includes.h"
#include "t1_luts.h"

/** @defgroup T1 T1 - Implementation of the tier-1 coding */
/*@{*/

/** @name Local static functions */
/*@{*/

static char t1_getctxno_zc(int f, int orient);
static char t1_getctxno_sc(int f);
static char t1_getctxno_mag(int f);
static char t1_getspb(int f);
static short t1_getnmsedec_sig(int x, int bitpos);
static short t1_getnmsedec_ref(int x, int bitpos);
static void t1_updateflags(flag_t *flagsp, int s, int stride);
/**
Encode significant pass
*/
static void t1_enc_sigpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int orient,
		int bpno,
		int one,
		int *nmsedec,
		char type,
		int vsc);
/**
Decode significant pass
*/
static void t1_dec_sigpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int orient,
		int oneplushalf,
		char type,
		int vsc);
/**
Encode significant pass
*/
static void t1_enc_sigpass(
		opj_t1_t *t1,
		int bpno,
		int orient,
		int *nmsedec,
		char type,
		int cblksty);
/**
Decode significant pass
*/
static void t1_dec_sigpass(
		opj_t1_t *t1,
		int bpno,
		int orient,
		char type,
		int cblksty);
/**
Encode refinement pass
*/
static void t1_enc_refpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int bpno,
		int one,
		int *nmsedec,
		char type,
		int vsc);
/**
Decode refinement pass
*/
static void t1_dec_refpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int poshalf,
		int neghalf,
		char type,
		int vsc);
/**
Encode refinement pass
*/
static void t1_enc_refpass(
		opj_t1_t *t1,
		int bpno,
		int *nmsedec,
		char type,
		int cblksty);
/**
Decode refinement pass
*/
static void t1_dec_refpass(
		opj_t1_t *t1,
		int bpno,
		char type,
		int cblksty);
/**
Encode clean-up pass
*/
static void t1_enc_clnpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int orient,
		int bpno,
		int one,
		int *nmsedec,
		int partial,
		int vsc);
/**
Decode clean-up pass
*/
static void t1_dec_clnpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int orient,
		int oneplushalf,
		int partial,
		int vsc);
/**
Encode clean-up pass
*/
static void t1_enc_clnpass(
		opj_t1_t *t1,
		int bpno,
		int orient,
		int *nmsedec,
		int cblksty);
/**
Decode clean-up pass
*/
static void t1_dec_clnpass(
		opj_t1_t *t1,
		int bpno,
		int orient,
		int cblksty);
static double t1_getwmsedec(
		int nmsedec,
		int compno,
		int level,
		int orient,
		int bpno,
		int qmfbid,
		double stepsize,
		int numcomps);
/**
Encode 1 code-block
@param t1 T1 handle
@param cblk Code-block coding parameters
@param orient
@param compno Component number
@param level
@param qmfbid
@param stepsize
@param cblksty Code-block style
@param numcomps
@param tile
*/
static void t1_encode_cblk(
		opj_t1_t *t1,
		opj_tcd_cblk_t * cblk,
		int orient,
		int compno,
		int level,
		int qmfbid,
		double stepsize,
		int cblksty,
		int numcomps,
		opj_tcd_tile_t * tile);
/**
Decode 1 code-block
@param t1 T1 handle
@param cblk Code-block coding parameters
@param orient
@param roishift Region of interest shifting value
@param cblksty Code-block style
*/
static void t1_decode_cblk(
		opj_t1_t *t1,
		opj_tcd_cblk_t * cblk,
		int orient,
		int roishift,
		int cblksty);

/*@}*/

/*@}*/

/* ----------------------------------------------------------------------- */

static char t1_getctxno_zc(int f, int orient) {
	return lut_ctxno_zc[(orient << 8) | (f & T1_SIG_OTH)];
}

static char t1_getctxno_sc(int f) {
	return lut_ctxno_sc[(f & (T1_SIG_PRIM | T1_SGN)) >> 4];
}

static char t1_getctxno_mag(int f) {
	return lut_ctxno_mag[(f & T1_SIG_OTH) | (((f & T1_REFINE) != 0) << 11)];
}

static char t1_getspb(int f) {
	return lut_spb[(f & (T1_SIG_PRIM | T1_SGN)) >> 4];
}

static short t1_getnmsedec_sig(int x, int bitpos) {
	if (bitpos > T1_NMSEDEC_FRACBITS) {
		return lut_nmsedec_sig[(x >> (bitpos - T1_NMSEDEC_FRACBITS)) & ((1 << T1_NMSEDEC_BITS) - 1)];
	}
	
	return lut_nmsedec_sig0[x & ((1 << T1_NMSEDEC_BITS) - 1)];
}

static short t1_getnmsedec_ref(int x, int bitpos) {
	if (bitpos > T1_NMSEDEC_FRACBITS) {
		return lut_nmsedec_ref[(x >> (bitpos - T1_NMSEDEC_FRACBITS)) & ((1 << T1_NMSEDEC_BITS) - 1)];
	}

    return lut_nmsedec_ref0[x & ((1 << T1_NMSEDEC_BITS) - 1)];
}

static void t1_updateflags(flag_t *flagsp, int s, int stride) {
	flag_t *np = flagsp - stride;
	flag_t *sp = flagsp + stride;

	static const flag_t mod[] = {
		T1_SIG_S,            T1_SIG_N,            T1_SIG_E,            T1_SIG_W,
		T1_SIG_S | T1_SGN_S, T1_SIG_N | T1_SGN_N, T1_SIG_E | T1_SGN_E, T1_SIG_W | T1_SGN_W
	};

	s <<= 2;

	np[-1] |= T1_SIG_SE;
	np[0]  |= mod[s];
	np[1]  |= T1_SIG_SW;

	flagsp[-1] |= mod[s+2];
	flagsp[1]  |= mod[s+3];

	sp[-1] |= T1_SIG_NE;
	sp[0]  |= mod[s+1];
	sp[1]  |= T1_SIG_NW;
}

static void t1_enc_sigpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int orient,
		int bpno,
		int one,
		int *nmsedec,
		char type,
		int vsc)
{
	int v, flag;
	
	opj_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	flag = vsc ? ((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (*flagsp);
	if ((flag & T1_SIG_OTH) && !(flag & (T1_SIG | T1_VISIT))) {
		v = int_abs(*datap) & one ? 1 : 0;
		if (type == T1_TYPE_RAW) {	/* BYPASS/LAZY MODE */
			mqc_setcurctx(mqc, t1_getctxno_zc(flag, orient));	/* ESSAI */
			mqc_bypass_enc(mqc, v);
		} else {
			mqc_setcurctx(mqc, t1_getctxno_zc(flag, orient));
			mqc_encode(mqc, v);
		}
		if (v) {
			v = *datap < 0 ? 1 : 0;
			*nmsedec +=	t1_getnmsedec_sig(int_abs(*datap), bpno + T1_NMSEDEC_FRACBITS);
			if (type == T1_TYPE_RAW) {	/* BYPASS/LAZY MODE */
				mqc_setcurctx(mqc, t1_getctxno_sc(flag));	/* ESSAI */
				mqc_bypass_enc(mqc, v);
			} else {
				mqc_setcurctx(mqc, t1_getctxno_sc(flag));
				mqc_encode(mqc, v ^ t1_getspb(flag));
			}
			t1_updateflags(flagsp, v, t1->flags_stride);
			*flagsp |= T1_SIG;
		}
		*flagsp |= T1_VISIT;
	}
}

static void t1_dec_sigpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int orient,
		int oneplushalf,
		char type,
		int vsc)
{
	int v, flag;
	
	opj_raw_t *raw = t1->raw;	/* RAW component */
	opj_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	flag = vsc ? ((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (*flagsp);
	if ((flag & T1_SIG_OTH) && !(flag & (T1_SIG | T1_VISIT))) {
		if (type == T1_TYPE_RAW) {
			if (raw_decode(raw)) {
				v = raw_decode(raw);	/* ESSAI */
				*datap = v ? -oneplushalf : oneplushalf;
				t1_updateflags(flagsp, v, t1->flags_stride);
				*flagsp |= T1_SIG;
			}
		} else {
			mqc_setcurctx(mqc, t1_getctxno_zc(flag, orient));
			if (mqc_decode(mqc)) {
				mqc_setcurctx(mqc, t1_getctxno_sc(flag));
				v = mqc_decode(mqc) ^ t1_getspb(flag);
				*datap = v ? -oneplushalf : oneplushalf;
				t1_updateflags(flagsp, v, t1->flags_stride);
				*flagsp |= T1_SIG;
			}
		}
		*flagsp |= T1_VISIT;
	}
}				/* VSC and  BYPASS by Antonin */

static void t1_enc_sigpass(
		opj_t1_t *t1,
		int bpno,
		int orient,
		int *nmsedec,
		char type,
		int cblksty)
{
	int i, j, k, one, vsc;
	*nmsedec = 0;
	one = 1 << (bpno + T1_NMSEDEC_FRACBITS);
	for (k = 0; k < t1->h; k += 4) {
		for (i = 0; i < t1->w; ++i) {
			for (j = k; j < k + 4 && j < t1->h; ++j) {
				vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (j == k + 3 || j == t1->h - 1)) ? 1 : 0;
				t1_enc_sigpass_step(
						t1,
						&t1->flags[((j+1) * t1->flags_stride) + i + 1],
						&t1->data[(j * t1->w) + i],
						orient,
						bpno,
						one,
						nmsedec,
						type,
						vsc);
			}
		}
	}
}

static void t1_dec_sigpass(
		opj_t1_t *t1,
		int bpno,
		int orient,
		char type,
		int cblksty)
{
	int i, j, k, one, half, oneplushalf, vsc;
	one = 1 << bpno;
	half = one >> 1;
	oneplushalf = one | half;
	for (k = 0; k < t1->h; k += 4) {
		for (i = 0; i < t1->w; ++i) {
			for (j = k; j < k + 4 && j < t1->h; ++j) {
				vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (j == k + 3 || j == t1->h - 1)) ? 1 : 0;
				t1_dec_sigpass_step(
						t1,
						&t1->flags[((j+1) * t1->flags_stride) + i + 1],
						&t1->data[(j * t1->w) + i],
						orient,
						oneplushalf,
						type,
						vsc);
			}
		}
	}
}				/* VSC and  BYPASS by Antonin */

static void t1_enc_refpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int bpno,
		int one,
		int *nmsedec,
		char type,
		int vsc)
{
	int v, flag;
	
	opj_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	flag = vsc ? ((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (*flagsp);
	if ((flag & (T1_SIG | T1_VISIT)) == T1_SIG) {
		*nmsedec += t1_getnmsedec_ref(int_abs(*datap), bpno + T1_NMSEDEC_FRACBITS);
		v = int_abs(*datap) & one ? 1 : 0;
		if (type == T1_TYPE_RAW) {	/* BYPASS/LAZY MODE */
			mqc_setcurctx(mqc, t1_getctxno_mag(flag));	/* ESSAI */
			mqc_bypass_enc(mqc, v);
		} else {
			mqc_setcurctx(mqc, t1_getctxno_mag(flag));
			mqc_encode(mqc, v);
		}
		*flagsp |= T1_REFINE;
	}
}

static void t1_dec_refpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int poshalf,
		int neghalf,
		char type,
		int vsc)
{
	int v, t, flag;
	
	opj_mqc_t *mqc = t1->mqc;	/* MQC component */
	opj_raw_t *raw = t1->raw;	/* RAW component */
	
	flag = vsc ? ((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (*flagsp);
	if ((flag & (T1_SIG | T1_VISIT)) == T1_SIG) {
		if (type == T1_TYPE_RAW) {
			mqc_setcurctx(mqc, t1_getctxno_mag(flag));	/* ESSAI */
			v = raw_decode(raw);
		} else {
			mqc_setcurctx(mqc, t1_getctxno_mag(flag));
			v = mqc_decode(mqc);
		}
		t = v ? poshalf : neghalf;
		*datap += *datap < 0 ? -t : t;
		*flagsp |= T1_REFINE;
	}
}				/* VSC and  BYPASS by Antonin  */

static void t1_enc_refpass(
		opj_t1_t *t1,
		int bpno,
		int *nmsedec,
		char type,
		int cblksty)
{
	int i, j, k, one, vsc;
	*nmsedec = 0;
	one = 1 << (bpno + T1_NMSEDEC_FRACBITS);
	for (k = 0; k < t1->h; k += 4) {
		for (i = 0; i < t1->w; ++i) {
			for (j = k; j < k + 4 && j < t1->h; ++j) {
				vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (j == k + 3 || j == t1->h - 1)) ? 1 : 0;
				t1_enc_refpass_step(
						t1,
						&t1->flags[((j+1) * t1->flags_stride) + i + 1],
						&t1->data[(j * t1->w) + i],
						bpno,
						one,
						nmsedec,
						type,
						vsc);
			}
		}
	}
}

static void t1_dec_refpass(
		opj_t1_t *t1,
		int bpno,
		char type,
		int cblksty)
{
	int i, j, k, one, poshalf, neghalf;
	int vsc;
	one = 1 << bpno;
	poshalf = one >> 1;
	neghalf = bpno > 0 ? -poshalf : -1;
	for (k = 0; k < t1->h; k += 4) {
		for (i = 0; i < t1->w; ++i) {
			for (j = k; j < k + 4 && j < t1->h; ++j) {
				vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (j == k + 3 || j == t1->h - 1)) ? 1 : 0;
				t1_dec_refpass_step(
						t1,
						&t1->flags[((j+1) * t1->flags_stride) + i + 1],
						&t1->data[(j * t1->w) + i],
						poshalf,
						neghalf,
						type,
						vsc);
			}
		}
	}
}				/* VSC and  BYPASS by Antonin */

static void t1_enc_clnpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int orient,
		int bpno,
		int one,
		int *nmsedec,
		int partial,
		int vsc)
{
	int v, flag;
	
	opj_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	flag = vsc ? ((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (*flagsp);
	if (partial) {
		goto LABEL_PARTIAL;
	}
	if (!(*flagsp & (T1_SIG | T1_VISIT))) {
		mqc_setcurctx(mqc, t1_getctxno_zc(flag, orient));
		v = int_abs(*datap) & one ? 1 : 0;
		mqc_encode(mqc, v);
		if (v) {
LABEL_PARTIAL:
			*nmsedec += t1_getnmsedec_sig(int_abs(*datap), bpno + T1_NMSEDEC_FRACBITS);
			mqc_setcurctx(mqc, t1_getctxno_sc(flag));
			v = *datap < 0 ? 1 : 0;
			mqc_encode(mqc, v ^ t1_getspb(flag));
			t1_updateflags(flagsp, v, t1->flags_stride);
			*flagsp |= T1_SIG;
		}
	}
	*flagsp &= ~T1_VISIT;
}

static void t1_dec_clnpass_step(
		opj_t1_t *t1,
		flag_t *flagsp,
		int *datap,
		int orient,
		int oneplushalf,
		int partial,
		int vsc)
{
	int v, flag;
	
	opj_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	flag = vsc ? ((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (*flagsp);
	if (partial) {
		goto LABEL_PARTIAL;
	}
	if (!(flag & (T1_SIG | T1_VISIT))) {
		mqc_setcurctx(mqc, t1_getctxno_zc(flag, orient));
		if (mqc_decode(mqc)) {
LABEL_PARTIAL:
			mqc_setcurctx(mqc, t1_getctxno_sc(flag));
			v = mqc_decode(mqc) ^ t1_getspb(flag);
			*datap = v ? -oneplushalf : oneplushalf;
			t1_updateflags(flagsp, v, t1->flags_stride);
			*flagsp |= T1_SIG;
		}
	}
	*flagsp &= ~T1_VISIT;
}				/* VSC and  BYPASS by Antonin */

static void t1_enc_clnpass(
		opj_t1_t *t1,
		int bpno,
		int orient,
		int *nmsedec,
		int cblksty)
{
	int i, j, k, one, agg, runlen, vsc;
	
	opj_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	*nmsedec = 0;
	one = 1 << (bpno + T1_NMSEDEC_FRACBITS);
	for (k = 0; k < t1->h; k += 4) {
		for (i = 0; i < t1->w; ++i) {
			if (k + 3 < t1->h) {
				if (cblksty & J2K_CCP_CBLKSTY_VSC) {
					agg = !(MACRO_t1_flags(1 + k,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 1,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 2,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| (MACRO_t1_flags(1 + k + 3,1 + i) 
						& (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW |	T1_SGN_S))) & (T1_SIG | T1_VISIT | T1_SIG_OTH));
				} else {
					agg = !(MACRO_t1_flags(1 + k,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 1,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 2,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 3,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH));
				}
			} else {
				agg = 0;
			}
			if (agg) {
				for (runlen = 0; runlen < 4; ++runlen) {
					if (int_abs(t1->data[((k + runlen)*t1->w) + i]) & one)
						break;
				}
				mqc_setcurctx(mqc, T1_CTXNO_AGG);
				mqc_encode(mqc, runlen != 4);
				if (runlen == 4) {
					continue;
				}
				mqc_setcurctx(mqc, T1_CTXNO_UNI);
				mqc_encode(mqc, runlen >> 1);
				mqc_encode(mqc, runlen & 1);
			} else {
				runlen = 0;
			}
			for (j = k + runlen; j < k + 4 && j < t1->h; ++j) {
				vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (j == k + 3 || j == t1->h - 1)) ? 1 : 0;
				t1_enc_clnpass_step(
						t1,
						&t1->flags[((j+1) * t1->flags_stride) + i + 1],
						&t1->data[(j * t1->w) + i],
						orient,
						bpno,
						one,
						nmsedec,
						agg && (j == k + runlen),
						vsc);
			}
		}
	}
}

static void t1_dec_clnpass(
		opj_t1_t *t1,
		int bpno,
		int orient,
		int cblksty)
{
	int i, j, k, one, half, oneplushalf, agg, runlen, vsc;
	int segsym = cblksty & J2K_CCP_CBLKSTY_SEGSYM;
	
	opj_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	one = 1 << bpno;
	half = one >> 1;
	oneplushalf = one | half;
	for (k = 0; k < t1->h; k += 4) {
		for (i = 0; i < t1->w; ++i) {
			if (k + 3 < t1->h) {
				if (cblksty & J2K_CCP_CBLKSTY_VSC) {
					agg = !(MACRO_t1_flags(1 + k,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 1,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 2,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| (MACRO_t1_flags(1 + k + 3,1 + i) 
						& (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW |	T1_SGN_S))) & (T1_SIG | T1_VISIT | T1_SIG_OTH));
				} else {
					agg = !(MACRO_t1_flags(1 + k,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 1,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 2,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 3,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH));
				}
			} else {
				agg = 0;
			}
			if (agg) {
				mqc_setcurctx(mqc, T1_CTXNO_AGG);
				if (!mqc_decode(mqc)) {
					continue;
				}
				mqc_setcurctx(mqc, T1_CTXNO_UNI);
				runlen = mqc_decode(mqc);
				runlen = (runlen << 1) | mqc_decode(mqc);
			} else {
				runlen = 0;
			}
			for (j = k + runlen; j < k + 4 && j < t1->h; ++j) {
				vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (j == k + 3 || j == t1->h - 1)) ? 1 : 0;
				t1_dec_clnpass_step(
						t1,
						&t1->flags[((j+1) * t1->flags_stride) + i + 1],
						&t1->data[(j * t1->w) + i],
						orient,
						oneplushalf,
						agg && (j == k + runlen),
						vsc);
			}
		}
	}
	if (segsym) {
		int v = 0;
		mqc_setcurctx(mqc, T1_CTXNO_UNI);
		v = mqc_decode(mqc);
		v = (v << 1) | mqc_decode(mqc);
		v = (v << 1) | mqc_decode(mqc);
		v = (v << 1) | mqc_decode(mqc);
		/*
		if (v!=0xa) {
			opj_event_msg(t1->cinfo, EVT_WARNING, "Bad segmentation symbol %x\n", v);
		} 
		*/
	}
}				/* VSC and  BYPASS by Antonin */


/** mod fixed_quality */
static double t1_getwmsedec(
		int nmsedec,
		int compno,
		int level,
		int orient,
		int bpno,
		int qmfbid,
		double stepsize,
		int numcomps)
{
	double w1, w2, wmsedec;
	if (qmfbid == 1) {
		w1 = (numcomps > 1) ? mct_getnorm(compno) : 1.0;
		w2 = dwt_getnorm(level, orient);
	} else {			/* if (qmfbid == 0) */
		w1 = (numcomps > 1) ? mct_getnorm_real(compno) : 1.0;
		w2 = dwt_getnorm_real(level, orient);
	}
	wmsedec = w1 * w2 * stepsize * (1 << bpno);
	wmsedec *= wmsedec * nmsedec / 8192.0;
	
	return wmsedec;
}

static void allocate_buffers(
		opj_t1_t *t1,
		int w,
		int h)
{
	int datasize;
	int flagssize;

	datasize=w * h;
	//fprintf(stderr,"w=%i h=%i datasize=%i flagssize=%i\n",w,h,datasize,flagssize);

	if(datasize > t1->datasize){
		//fprintf(stderr,"Allocating t1->data: datasize=%i\n",datasize);
		free(t1->data);
		t1->data=malloc(datasize * sizeof(int));
		if(!t1->data){
			return;
		}
		t1->datasize=datasize;
	}
	//memset(t1->data,0xff,t1->datasize);
	memset(t1->data,0,datasize * sizeof(int));

	t1->flags_stride=w+2;
	flagssize=t1->flags_stride * (h+2);

	if(flagssize > t1->flagssize){
		//fprintf(stderr,"Allocating t1->flags: flagssize=%i\n",flagssize);
		free(t1->flags);
		t1->flags=malloc(flagssize * sizeof(flag_t));
		if(!t1->flags){
			fprintf(stderr,"Allocating t1->flags FAILED!\n");
			return;
		}
		t1->flagssize=flagssize;
	}
	//memset(t1->flags,0xff,t1->flagssize);
	memset(t1->flags,0,flagssize * sizeof(flag_t));

	t1->w=w;
	t1->h=h;
}

/** mod fixed_quality */
static void t1_encode_cblk(
		opj_t1_t *t1,
		opj_tcd_cblk_t * cblk,
		int orient,
		int compno,
		int level,
		int qmfbid,
		double stepsize,
		int cblksty,
		int numcomps,
		opj_tcd_tile_t * tile)
{
	int i, j;
	int passno;
	int bpno, passtype;
	int max;
	int nmsedec = 0;
	double cumwmsedec = 0.0;
	char type = T1_TYPE_MQ;
	
	opj_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	max = 0;
	for (j = 0; j < t1->h; ++j) {
		for (i = 0; i < t1->w; ++i) {
			max = int_max(max, int_abs(t1->data[(j * t1->w) + i]));
		}
	}
	
	cblk->numbps = max ? (int_floorlog2(max) + 1) - T1_NMSEDEC_FRACBITS : 0;
	
	bpno = cblk->numbps - 1;
	passtype = 2;
	
	mqc_resetstates(mqc);
	mqc_setstate(mqc, T1_CTXNO_UNI, 0, 46);
	mqc_setstate(mqc, T1_CTXNO_AGG, 0, 3);
	mqc_setstate(mqc, T1_CTXNO_ZC, 0, 4);
	mqc_init_enc(mqc, cblk->data);
	
	for (passno = 0; bpno >= 0; ++passno) {
		opj_tcd_pass_t *pass = &cblk->passes[passno];
		int correction = 3;
		type = ((bpno < (cblk->numbps - 4)) && (passtype < 2) && (cblksty & J2K_CCP_CBLKSTY_LAZY)) ? T1_TYPE_RAW : T1_TYPE_MQ;
		
		switch (passtype) {
			case 0:
				t1_enc_sigpass(t1, bpno, orient, &nmsedec, type, cblksty);
				break;
			case 1:
				t1_enc_refpass(t1, bpno, &nmsedec, type, cblksty);
				break;
			case 2:
				t1_enc_clnpass(t1, bpno, orient, &nmsedec, cblksty);
				/* code switch SEGMARK (i.e. SEGSYM) */
				if (cblksty & J2K_CCP_CBLKSTY_SEGSYM)
					mqc_segmark_enc(mqc);
				break;
		}
		
		/* fixed_quality */
		cumwmsedec += t1_getwmsedec(nmsedec, compno, level, orient, bpno, qmfbid, stepsize, numcomps);
		tile->distotile += t1_getwmsedec(nmsedec, compno, level, orient, bpno, qmfbid, stepsize, numcomps);
		
		/* Code switch "RESTART" (i.e. TERMALL) */
		if ((cblksty & J2K_CCP_CBLKSTY_TERMALL)	&& !((passtype == 2) && (bpno - 1 < 0))) {
			if (type == T1_TYPE_RAW) {
				mqc_flush(mqc);
				correction = 1;
				/* correction = mqc_bypass_flush_enc(); */
			} else {			/* correction = mqc_restart_enc(); */
				mqc_flush(mqc);
				correction = 1;
			}
			pass->term = 1;
		} else {
			if (((bpno < (cblk->numbps - 4) && (passtype > 0)) 
				|| ((bpno == (cblk->numbps - 4)) && (passtype == 2))) && (cblksty & J2K_CCP_CBLKSTY_LAZY)) {
				if (type == T1_TYPE_RAW) {
					mqc_flush(mqc);
					correction = 1;
					/* correction = mqc_bypass_flush_enc(); */
				} else {		/* correction = mqc_restart_enc(); */
					mqc_flush(mqc);
					correction = 1;
				}
				pass->term = 1;
			} else {
				pass->term = 0;
			}
		}
		
		if (++passtype == 3) {
			passtype = 0;
			bpno--;
		}
		
		if (pass->term && bpno > 0) {
			type = ((bpno < (cblk->numbps - 4)) && (passtype < 2) && (cblksty & J2K_CCP_CBLKSTY_LAZY)) ? T1_TYPE_RAW : T1_TYPE_MQ;
			if (type == T1_TYPE_RAW)
				mqc_bypass_init_enc(mqc);
			else
				mqc_restart_init_enc(mqc);
		}
		
		pass->distortiondec = cumwmsedec;
		pass->rate = mqc_numbytes(mqc) + correction;	/* FIXME */
		
		/* Code-switch "RESET" */
		if (cblksty & J2K_CCP_CBLKSTY_RESET)
			mqc_reset_enc(mqc);
	}
	
	/* Code switch "ERTERM" (i.e. PTERM) */
	if (cblksty & J2K_CCP_CBLKSTY_PTERM)
		mqc_erterm_enc(mqc);
	else /* Default coding */ if (!(cblksty & J2K_CCP_CBLKSTY_LAZY))
		mqc_flush(mqc);
	
	cblk->totalpasses = passno;

	for (passno = 0; passno<cblk->totalpasses; passno++) {
		opj_tcd_pass_t *pass = &cblk->passes[passno];
		if (pass->rate > mqc_numbytes(mqc))
			pass->rate = mqc_numbytes(mqc);
		/*Preventing generation of FF as last data byte of a pass*/
		if((pass->rate>1) && (cblk->data[pass->rate - 1] == 0xFF)){
			pass->rate--;
		}
		pass->len = pass->rate - (passno == 0 ? 0 : cblk->passes[passno - 1].rate);		
	}
}

static void t1_decode_cblk(
		opj_t1_t *t1,
		opj_tcd_cblk_t * cblk,
		int orient,
		int roishift,
		int cblksty)
{
	int bpno, passtype;
	int segno, passno;
	char type = T1_TYPE_MQ; /* BYPASS mode */
	
	opj_raw_t *raw = t1->raw;	/* RAW component */
	opj_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	allocate_buffers(
			t1,
			cblk->x1 - cblk->x0,
			cblk->y1 - cblk->y0);
	
	bpno = roishift + cblk->numbps - 1;
	passtype = 2;
	
	mqc_resetstates(mqc);
	mqc_setstate(mqc, T1_CTXNO_UNI, 0, 46);
	mqc_setstate(mqc, T1_CTXNO_AGG, 0, 3);
	mqc_setstate(mqc, T1_CTXNO_ZC, 0, 4);
	
	for (segno = 0; segno < cblk->numsegs; ++segno) {
		opj_tcd_seg_t *seg = &cblk->segs[segno];
		
		/* BYPASS mode */
		type = ((bpno <= (cblk->numbps - 1) - 4) && (passtype < 2) && (cblksty & J2K_CCP_CBLKSTY_LAZY)) ? T1_TYPE_RAW : T1_TYPE_MQ;
		if (type == T1_TYPE_RAW) {
			raw_init_dec(raw, seg->data, seg->len);
		} else {
			mqc_init_dec(mqc, seg->data, seg->len);
		}
		
		for (passno = 0; passno < seg->numpasses; ++passno) {
			switch (passtype) {
				case 0:
					t1_dec_sigpass(t1, bpno+1, orient, type, cblksty);
					break;
				case 1:
					t1_dec_refpass(t1, bpno+1, type, cblksty);
					break;
				case 2:
					t1_dec_clnpass(t1, bpno+1, orient, cblksty);
					break;
			}
			
			if ((cblksty & J2K_CCP_CBLKSTY_RESET) && type == T1_TYPE_MQ) {
				mqc_resetstates(mqc);
				mqc_setstate(mqc, T1_CTXNO_UNI, 0, 46);				
				mqc_setstate(mqc, T1_CTXNO_AGG, 0, 3);
				mqc_setstate(mqc, T1_CTXNO_ZC, 0, 4);
			}
			if (++passtype == 3) {
				passtype = 0;
				bpno--;
			}
		}
	}
}

/* ----------------------------------------------------------------------- */

opj_t1_t* t1_create(opj_common_ptr cinfo) {
	opj_t1_t *t1 = (opj_t1_t*) malloc(sizeof(opj_t1_t));
	if(!t1)
		return NULL;

		t1->cinfo = cinfo;
		/* create MQC and RAW handles */
		t1->mqc = mqc_create();
		t1->raw = raw_create();

	t1->datasize=0;
	t1->data=NULL;
	t1->flagssize=0;
	t1->flags=NULL;

	return t1;
}

void t1_destroy(opj_t1_t *t1) {
	if(t1) {
		/* destroy MQC and RAW handles */
		mqc_destroy(t1->mqc);
		raw_destroy(t1->raw);
		free(t1->data);
		free(t1->flags);
		free(t1);
	}
}

void t1_encode_cblks(
		opj_t1_t *t1,
		opj_tcd_tile_t *tile,
		opj_tcp_t *tcp)
{
	int compno, resno, bandno, precno, cblkno;

	tile->distotile = 0;		/* fixed_quality */

	for (compno = 0; compno < tile->numcomps; ++compno) {
		opj_tcd_tilecomp_t *tilec = &tile->comps[compno];

		for (resno = 0; resno < tilec->numresolutions; ++resno) {
			opj_tcd_resolution_t *res = &tilec->resolutions[resno];

			for (bandno = 0; bandno < res->numbands; ++bandno) {
				opj_tcd_band_t *band = &res->bands[bandno];

				for (precno = 0; precno < res->pw * res->ph; ++precno) {
					opj_tcd_precinct_t *prc = &band->precincts[precno];

					for (cblkno = 0; cblkno < prc->cw * prc->ch; ++cblkno) {
						int x, y, w, i, j, orient;
						opj_tcd_cblk_t *cblk = &prc->cblks[cblkno];

						x = cblk->x0 - band->x0;
						y = cblk->y0 - band->y0;
						if (band->bandno & 1) {
							opj_tcd_resolution_t *pres = &tilec->resolutions[resno - 1];
							x += pres->x1 - pres->x0;
						}
						if (band->bandno & 2) {
							opj_tcd_resolution_t *pres = &tilec->resolutions[resno - 1];
							y += pres->y1 - pres->y0;
						}

						allocate_buffers(
								t1,
								cblk->x1 - cblk->x0,
								cblk->y1 - cblk->y0);

						w = tilec->x1 - tilec->x0;
						if (tcp->tccps[compno].qmfbid == 1) {
							for (j = 0; j < t1->h; ++j) {
								for (i = 0; i < t1->w; ++i) {
									t1->data[(j * t1->w) + i] =
										tilec->data[(x + i) + (y + j) * w] << T1_NMSEDEC_FRACBITS;
								}
							}
						} else {		/* if (tcp->tccps[compno].qmfbid == 0) */
							for (j = 0; j < t1->h; ++j) {
								for (i = 0; i < t1->w; ++i) {
									t1->data[(j * t1->w) + i] = 
										fix_mul(
										tilec->data[x + i + (y + j) * w],
										8192 * 8192 / ((int) floor(band->stepsize * 8192))) >> (11 - T1_NMSEDEC_FRACBITS);
								}
							}
						}
						orient = band->bandno;	/* FIXME */
						if (orient == 2) {
							orient = 1;
						} else if (orient == 1) {
							orient = 2;
						}

						t1_encode_cblk(
								t1,
								cblk,
								orient,
								compno,
								tilec->numresolutions - 1 - resno,
								tcp->tccps[compno].qmfbid,
								band->stepsize,
								tcp->tccps[compno].cblksty,
								tile->numcomps,
								tile);

					} /* cblkno */
				} /* precno */
			} /* bandno */
		} /* resno  */
	} /* compno  */
}

void t1_decode_cblks(
		opj_t1_t *t1,
		opj_tcd_tile_t *tile,
		opj_tcp_t *tcp)
{
	int compno, resno, bandno, precno, cblkno;

	for (compno = 0; compno < tile->numcomps; ++compno) {
		opj_tcd_tilecomp_t *tilec = &tile->comps[compno];

		for (resno = 0; resno < tilec->numresolutions; ++resno) {
			opj_tcd_resolution_t *res = &tilec->resolutions[resno];

			for (bandno = 0; bandno < res->numbands; ++bandno) {
				opj_tcd_band_t *band = &res->bands[bandno];

				for (precno = 0; precno < res->pw * res->ph; ++precno) {
					opj_tcd_precinct_t *prc = &band->precincts[precno];

					for (cblkno = 0; cblkno < prc->cw * prc->ch; ++cblkno) {
						int x, y, w, i, j, orient, cblk_w, cblk_h;
						opj_tcd_cblk_t *cblk = &prc->cblks[cblkno];

						orient = band->bandno;	/* FIXME */
						if (orient == 2) {
							orient = 1;
						} else if (orient == 1) {
							orient = 2;
						}
						
						t1_decode_cblk(
								t1,
								cblk,
								orient,
								tcp->tccps[compno].roishift,
								tcp->tccps[compno].cblksty);

						x = cblk->x0 - band->x0;
						y = cblk->y0 - band->y0;
						if (band->bandno & 1) {
							opj_tcd_resolution_t *pres = &tilec->resolutions[resno - 1];
							x += pres->x1 - pres->x0;
						}
						if (band->bandno & 2) {
							opj_tcd_resolution_t *pres = &tilec->resolutions[resno - 1];
							y += pres->y1 - pres->y0;
						}

						cblk_w = cblk->x1 - cblk->x0;
						cblk_h = cblk->y1 - cblk->y0;

						if (tcp->tccps[compno].roishift) {
							int thresh = 1 << tcp->tccps[compno].roishift;
							for (j = 0; j < cblk_h; ++j) {
								for (i = 0; i < cblk_w; ++i) {
									int val = t1->data[(j * t1->w) + i];
									int mag = int_abs(val);
									if (mag >= thresh) {
										mag >>= tcp->tccps[compno].roishift;
										t1->data[(j * t1->w) + i] = val < 0 ? -mag : mag;
									}
								}
							}
						}
						
						w = tilec->x1 - tilec->x0;
						if (tcp->tccps[compno].qmfbid == 1) {
							for (j = 0; j < cblk_h; ++j) {
								for (i = 0; i < cblk_w; ++i) {
									tilec->data[x + i + (y + j) * w] = t1->data[(j * t1->w) + i]/2;
								}
							}
						} else {		/* if (tcp->tccps[compno].qmfbid == 0) */
							for (j = 0; j < cblk_h; ++j) {
								for (i = 0; i < cblk_w; ++i) {
									if (t1->data[(j * t1->w) + i] >> 1 == 0) {
										tilec->data[x + i + (y + j) * w] = 0;
									} else {
										double tmp = (double)(t1->data[(j * t1->w) + i] * band->stepsize * 4096.0);
										int tmp2 = ((int) (floor(fabs(tmp)))) + ((int) floor(fabs(tmp*2))%2);									
										tilec->data[x + i + (y + j) * w] = ((tmp<0)?-tmp2:tmp2);
									}
								}
							}
						}
					} /* cblkno */
				} /* precno */
			} /* bandno */
		} /* resno */
	} /* compno */
}

