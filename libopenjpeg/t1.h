/*
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2005, Francois Devaux and Antonin Descampe
 * Copyright (c) 2005, Hervé Drolon, FreeImage Team
 * Copyright (c) 2002-2005, Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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

#ifndef __T1_H
#define __T1_H
/**
@file t1.h
@brief Implementation of the tier-1 coding (coding of code-block coefficients) (T1)

The functions in T1.C have for goal to realize the tier-1 coding operation. The functions
in T1.C are used by some function in TCD.C.
*/

/** @defgroup T1 T1 - Implementation of the tier-1 coding */
/*@{*/

/* ----------------------------------------------------------------------- */
#define T1_NMSEDEC_BITS 7

#define T1_MAXCBLKW 1024  /**< Maximum size of code-block (width) */
#define T1_MAXCBLKH 1024  /**< Maximum size of code-block (heigth) */

#define T1_SIG_NE 0x0001  /**< Context orientation : North-East direction */
#define T1_SIG_SE 0x0002  /**< Context orientation : South-East direction */
#define T1_SIG_SW 0x0004  /**< Context orientation : South-West direction */
#define T1_SIG_NW 0x0008  /**< Context orientation : North-West direction */
#define T1_SIG_N 0x0010   /**< Context orientation : North direction */
#define T1_SIG_E 0x0020   /**< Context orientation : East direction */
#define T1_SIG_S 0x0040   /**< Context orientation : South direction */
#define T1_SIG_W 0x0080   /**< Context orientation : West direction */
#define T1_SIG_OTH (T1_SIG_N|T1_SIG_NE|T1_SIG_E|T1_SIG_SE|T1_SIG_S|T1_SIG_SW|T1_SIG_W|T1_SIG_NW)
#define T1_SIG_PRIM (T1_SIG_N|T1_SIG_E|T1_SIG_S|T1_SIG_W)

#define T1_SGN_N 0x0100
#define T1_SGN_E 0x0200
#define T1_SGN_S 0x0400
#define T1_SGN_W 0x0800
#define T1_SGN (T1_SGN_N|T1_SGN_E|T1_SGN_S|T1_SGN_W)

#define T1_SIG 0x1000
#define T1_REFINE 0x2000
#define T1_VISIT 0x4000

#define T1_NUMCTXS_AGG 1
#define T1_NUMCTXS_ZC 9
#define T1_NUMCTXS_MAG 3
#define T1_NUMCTXS_SC 5
#define T1_NUMCTXS_UNI 1

#define T1_CTXNO_AGG 0
#define T1_CTXNO_ZC (T1_CTXNO_AGG+T1_NUMCTXS_AGG)
#define T1_CTXNO_MAG (T1_CTXNO_ZC+T1_NUMCTXS_ZC)
#define T1_CTXNO_SC (T1_CTXNO_MAG+T1_NUMCTXS_MAG)
#define T1_CTXNO_UNI (T1_CTXNO_SC+T1_NUMCTXS_SC)
#define T1_NUMCTXS (T1_CTXNO_UNI+T1_NUMCTXS_UNI)

#define T1_NMSEDEC_FRACBITS (T1_NMSEDEC_BITS-1)

#define T1_TYPE_MQ 0  /**< Normal coding using entropy coder */
#define T1_TYPE_RAW 1 /**< No encoding the information is store under raw format in codestream (mode switch RAW)*/

/* ----------------------------------------------------------------------- */

/**
Tier-1 coding (coding of code-block coefficients)
*/
typedef struct opj_t1 {
  /** codec context */
  opj_common_ptr cinfo;

  /** MQC component */
  opj_mqc_t *mqc;
  /** RAW component */
  opj_raw_t *raw;

  int lut_ctxno_zc[1024];
  int lut_ctxno_sc[256];
  int lut_ctxno_mag[4096];
  int lut_spb[256];
  int lut_nmsedec_sig[1 << T1_NMSEDEC_BITS];
  int lut_nmsedec_sig0[1 << T1_NMSEDEC_BITS];
  int lut_nmsedec_ref[1 << T1_NMSEDEC_BITS];
  int lut_nmsedec_ref0[1 << T1_NMSEDEC_BITS];

  int data[T1_MAXCBLKH][T1_MAXCBLKW];
  int flags[T1_MAXCBLKH + 2][T1_MAXCBLKH + 2];

} opj_t1_t;

/** @name Local static functions */
/*@{*/
/* ----------------------------------------------------------------------- */
static int t1_getctxno_zc(opj_t1_t *t1, int f, int orient);
static int t1_getctxno_sc(opj_t1_t *t1, int f);
static int t1_getctxno_mag(opj_t1_t *t1, int f);
static int t1_getspb(opj_t1_t *t1, int f);
static int t1_getnmsedec_sig(opj_t1_t *t1, int x, int bitpos);
static int t1_getnmsedec_ref(opj_t1_t *t1, int x, int bitpos);
static void t1_updateflags(int *fp, int s);
/**
Encode significant pass
*/
static void t1_enc_sigpass_step(opj_t1_t *t1, int *fp, int *dp, int orient, int bpno, int one, int *nmsedec, char type, int vsc);
/**
Decode significant pass
*/
static void t1_dec_sigpass_step(opj_t1_t *t1, int *fp, int *dp, int orient, int oneplushalf, char type, int vsc);
/**
Encode significant pass
*/
static void t1_enc_sigpass(opj_t1_t *t1, int w, int h, int bpno, int orient, int *nmsedec, char type, int cblksty);
/**
Decode significant pass
*/
static void t1_dec_sigpass(opj_t1_t *t1, int w, int h, int bpno, int orient, char type, int cblksty);
/**
Encode refinement pass
*/
static void t1_enc_refpass_step(opj_t1_t *t1, int *fp, int *dp, int bpno, int one, int *nmsedec, char type, int vsc);
/**
Decode refinement pass
*/
static void t1_dec_refpass_step(opj_t1_t *t1, int *fp, int *dp, int poshalf, int neghalf, char type, int vsc);
/**
Encode refinement pass
*/
static void t1_enc_refpass(opj_t1_t *t1, int w, int h, int bpno, int *nmsedec, char type, int cblksty);
/**
Decode refinement pass
*/
static void t1_dec_refpass(opj_t1_t *t1, int w, int h, int bpno, char type, int cblksty);
/**
Encode clean-up pass
*/
static void t1_enc_clnpass_step(opj_t1_t *t1, int *fp, int *dp, int orient, int bpno, int one, int *nmsedec, int partial, int vsc);
/**
Decode clean-up pass
*/
static void t1_dec_clnpass_step(opj_t1_t *t1, int *fp, int *dp, int orient, int oneplushalf, int partial, int vsc);
/**
Encode clean-up pass
*/
static void t1_enc_clnpass(opj_t1_t *t1, int w, int h, int bpno, int orient, int *nmsedec, int cblksty);
/**
Decode clean-up pass
*/
static void t1_dec_clnpass(opj_t1_t *t1, int w, int h, int bpno, int orient, int cblksty);
static double t1_getwmsedec(opj_t1_t *t1, int nmsedec, int compno, int level, int orient, int bpno, int qmfbid, double stepsize, int numcomps);
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
static void t1_encode_cblk(opj_t1_t *t1, opj_tcd_cblk_t * cblk, int orient, int compno, int level, int qmfbid, double stepsize, int cblksty, int numcomps, opj_tcd_tile_t * tile);
/**
Decode 1 code-block
@param t1 T1 handle
@param cblk Code-block coding parameters
@param orient
@param roishift Region of interest shifting value
@param cblksty Code-block style
*/
static void t1_decode_cblk(opj_t1_t *t1, opj_tcd_cblk_t * cblk, int orient, int roishift, int cblksty);
static int t1_init_ctxno_zc(int f, int orient);
static int t1_init_ctxno_sc(int f);
static int t1_init_ctxno_mag(int f);
static int t1_init_spb(int f);
/**
Initialize the look-up tables of the Tier-1 coder/decoder
@param t1 T1 handle
*/
static void t1_init_luts(opj_t1_t *t1);

/* ----------------------------------------------------------------------- */
/*@}*/

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */
/**
Create a new T1 handle 
and initialize the look-up tables of the Tier-1 coder/decoder
@return Returns a new T1 handle if successful, returns NULL otherwise
@see t1_init_luts
*/
opj_t1_t* t1_create(opj_common_ptr cinfo);
/**
Destroy a previously created T1 handle
@param t1 T1 handle to destroy
*/
void t1_destroy(opj_t1_t *t1);
/**
Encode the code-blocks of a tile
@param t1 T1 handle
@param tile The tile to encode
@param tcp Tile coding parameters
*/
void t1_encode_cblks(opj_t1_t *t1, opj_tcd_tile_t *tile, opj_tcp_t *tcp);
/**
Decode the code-blocks of a tile
@param t1 T1 handle
@param tile The tile to decode
@param tcp Tile coding parameters
*/
void t1_decode_cblks(opj_t1_t *t1, opj_tcd_tile_t *tile, opj_tcp_t *tcp);
/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __T1_H */
