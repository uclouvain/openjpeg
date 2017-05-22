/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2002-2014, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2014, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux
 * Copyright (c) 2003-2014, Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * Copyright (c) 2012, Carl Hetherington
 * Copyright (c) 2017, IntoPIX SA <support@intopix.com>
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

/* CAUTION: the value of those constants must not be changed, otherwise the */
/* optimization of opj_t1_updateflags() will break! */
/* BEGINNING of flags that apply to opj_flag_t */
#define T1_SIG_NE 0x0001U /**< Context orientation : North-East direction */
#define T1_SIG_SE 0x0002U /**< Context orientation : South-East direction */
#define T1_SIG_SW 0x0004U /**< Context orientation : South-West direction */
#define T1_SIG_NW 0x0008U /**< Context orientation : North-West direction */
#define T1_SIG_N 0x0010U  /**< Context orientation : North direction */
#define T1_SIG_E 0x0020U  /**< Context orientation : East direction */
#define T1_SIG_S 0x0040U  /**< Context orientation : South direction */
#define T1_SIG_W 0x0080U  /**< Context orientation : West direction */
#define T1_SIG_OTH (T1_SIG_N|T1_SIG_NE|T1_SIG_E|T1_SIG_SE|T1_SIG_S|T1_SIG_SW|T1_SIG_W|T1_SIG_NW)
#define T1_SIG_PRIM (T1_SIG_N|T1_SIG_E|T1_SIG_S|T1_SIG_W)

#define T1_SGN_N 0x0100U
#define T1_SGN_E 0x0200U
#define T1_SGN_S 0x0400U
#define T1_SGN_W 0x0800U
#define T1_SGN (T1_SGN_N|T1_SGN_E|T1_SGN_S|T1_SGN_W)

#define T1_SIG    0x1000U /**< No longer used by decoder */
#define T1_REFINE 0x2000U /**< No longer used by decoder */
#define T1_VISIT  0x4000U /**< No longer used by decoder */
/* END of flags that apply to opj_flag_t */

#define T1_NUMCTXS_ZC  9
#define T1_NUMCTXS_SC  5
#define T1_NUMCTXS_MAG 3
#define T1_NUMCTXS_AGG 1
#define T1_NUMCTXS_UNI 1

#define T1_CTXNO_ZC  0
#define T1_CTXNO_SC  (T1_CTXNO_ZC+T1_NUMCTXS_ZC)
#define T1_CTXNO_MAG (T1_CTXNO_SC+T1_NUMCTXS_SC)
#define T1_CTXNO_AGG (T1_CTXNO_MAG+T1_NUMCTXS_MAG)
#define T1_CTXNO_UNI (T1_CTXNO_AGG+T1_NUMCTXS_AGG)
#define T1_NUMCTXS   (T1_CTXNO_UNI+T1_NUMCTXS_UNI)

#define T1_NMSEDEC_FRACBITS (T1_NMSEDEC_BITS-1)

#define T1_TYPE_MQ 0    /**< Normal coding using entropy coder */
#define T1_TYPE_RAW 1   /**< No encoding the information is store under raw format in codestream (mode switch RAW)*/

/* Those flags are used by opj_colflag_t */
#define T1_COLFLAG_RBS              4U /* RBS = Row Bit Shift */
#define T1_COLFLAG_SIG_OTHER_ROW_0 (1U << 0U)  /**< This sample has at least one significant neighbour */
#define T1_COLFLAG_SIG_ROW_0       (1U << 1U)  /**< This sample is significant */
#define T1_COLFLAG_VISIT_ROW_0     (1U << 2U)  /**< This sample has been visited */
#define T1_COLFLAG_REFINE_ROW_0    (1U << 3U)  /**< This sample has been refined */
#define T1_COLFLAG_SIG_OTHER_ROW_1 (T1_COLFLAG_SIG_OTHER_ROW_0 << (1U * T1_COLFLAG_RBS))
#define T1_COLFLAG_SIG_ROW_1       (T1_COLFLAG_SIG_ROW_0       << (1U * T1_COLFLAG_RBS))
#define T1_COLFLAG_VISIT_ROW_1     (T1_COLFLAG_VISIT_ROW_0     << (1U * T1_COLFLAG_RBS))
#define T1_COLFLAG_REFINE_ROW_1    (T1_COLFLAG_REFINE_ROW_0    << (1U * T1_COLFLAG_RBS))
#define T1_COLFLAG_SIG_OTHER_ROW_2 (T1_COLFLAG_SIG_OTHER_ROW_0 << (2U * T1_COLFLAG_RBS))
#define T1_COLFLAG_SIG_ROW_2       (T1_COLFLAG_SIG_ROW_0       << (2U * T1_COLFLAG_RBS))
#define T1_COLFLAG_VISIT_ROW_2     (T1_COLFLAG_VISIT_ROW_0     << (2U * T1_COLFLAG_RBS))
#define T1_COLFLAG_REFINE_ROW_2    (T1_COLFLAG_REFINE_ROW_0    << (2U * T1_COLFLAG_RBS))
#define T1_COLFLAG_SIG_OTHER_ROW_3 (T1_COLFLAG_SIG_OTHER_ROW_0 << (3U * T1_COLFLAG_RBS))
#define T1_COLFLAG_SIG_ROW_3       (T1_COLFLAG_SIG_ROW_0       << (3U * T1_COLFLAG_RBS))
#define T1_COLFLAG_VISIT_ROW_3     (T1_COLFLAG_VISIT_ROW_0     << (3U * T1_COLFLAG_RBS))
#define T1_COLFLAG_REFINE_ROW_3    (T1_COLFLAG_REFINE_ROW_0    << (3U * T1_COLFLAG_RBS))


/* BEGINNING of flags that apply to opj_flag_enc_t */
/** We hold the state of individual data points for the T1 encoder using
 *  a single 32-bit flags word to hold the state of 4 data points.  This corresponds
 *  to the 4-point-high columns that the data is processed in.
 *
 *  These #defines declare the layout of a 32-bit flags word.
 *
 *  This is currently done for encoding only.
 *  The values must NOT be changed, otherwise this is going to break a lot of
 *  assumptions.
 */

/* SIGMA: significance state (3 cols x 6 rows)
 * CHI:   state for negative sample value (1 col x 6 rows)
 * MU:    state for visited in refinement pass (1 col x 4 rows)
 * PI:    state for visited in significance pass (1 col * 4 rows)
 */

#define T1_SIGMA_0  (1U << 0)
#define T1_SIGMA_1  (1U << 1)
#define T1_SIGMA_2  (1U << 2)
#define T1_SIGMA_3  (1U << 3)
#define T1_SIGMA_4  (1U << 4)
#define T1_SIGMA_5  (1U << 5)
#define T1_SIGMA_6  (1U << 6)
#define T1_SIGMA_7  (1U << 7)
#define T1_SIGMA_8  (1U << 8)
#define T1_SIGMA_9  (1U << 9)
#define T1_SIGMA_10 (1U << 10)
#define T1_SIGMA_11 (1U << 11)
#define T1_SIGMA_12 (1U << 12)
#define T1_SIGMA_13 (1U << 13)
#define T1_SIGMA_14 (1U << 14)
#define T1_SIGMA_15 (1U << 15)
#define T1_SIGMA_16 (1U << 16)
#define T1_SIGMA_17 (1U << 17)

#define T1_CHI_0    (1U << 18)
#define T1_CHI_0_I  18
#define T1_CHI_1    (1U << 19)
#define T1_CHI_1_I  19
#define T1_MU_0     (1U << 20)
#define T1_PI_0     (1U << 21)
#define T1_CHI_2    (1U << 22)
#define T1_CHI_2_I  22
#define T1_MU_1     (1U << 23)
#define T1_PI_1     (1U << 24)
#define T1_CHI_3    (1U << 25)
#define T1_MU_2     (1U << 26)
#define T1_PI_2     (1U << 27)
#define T1_CHI_4    (1U << 28)
#define T1_MU_3     (1U << 29)
#define T1_PI_3     (1U << 30)
#define T1_CHI_5    (1U << 31)

/** As an example, the bits T1_SIGMA_3, T1_SIGMA_4 and T1_SIGMA_5
 *  indicate the significance state of the west neighbour of data point zero
 *  of our four, the point itself, and its east neighbour respectively.
 *  Many of the bits are arranged so that given a flags word, you can
 *  look at the values for the data point 0, then shift the flags
 *  word right by 3 bits and look at the same bit positions to see the
 *  values for data point 1.
 *
 *  The #defines below help a bit with this; say you have a flags word
 *  f, you can do things like
 *
 *  (f & T1_SIGMA_THIS)
 *
 *  to see the significance bit of data point 0, then do
 *
 *  ((f >> 3) & T1_SIGMA_THIS)
 *
 *  to see the significance bit of data point 1.
 */

#define T1_SIGMA_NW   T1_SIGMA_0
#define T1_SIGMA_N    T1_SIGMA_1
#define T1_SIGMA_NE   T1_SIGMA_2
#define T1_SIGMA_W    T1_SIGMA_3
#define T1_SIGMA_THIS T1_SIGMA_4
#define T1_SIGMA_E    T1_SIGMA_5
#define T1_SIGMA_SW   T1_SIGMA_6
#define T1_SIGMA_S    T1_SIGMA_7
#define T1_SIGMA_SE   T1_SIGMA_8
#define T1_SIGMA_NEIGHBOURS (T1_SIGMA_NW | T1_SIGMA_N | T1_SIGMA_NE | T1_SIGMA_W | T1_SIGMA_E | T1_SIGMA_SW | T1_SIGMA_S | T1_SIGMA_SE)

#define T1_CHI_THIS   T1_CHI_1
#define T1_CHI_THIS_I T1_CHI_1_I
#define T1_MU_THIS    T1_MU_0
#define T1_PI_THIS    T1_PI_0
#define T1_CHI_S      T1_CHI_2

#define T1_LUT_SGN_W (1U << 0)
#define T1_LUT_SIG_N (1U << 1)
#define T1_LUT_SGN_E (1U << 2)
#define T1_LUT_SIG_W (1U << 3)
#define T1_LUT_SGN_N (1U << 4)
#define T1_LUT_SIG_E (1U << 5)
#define T1_LUT_SGN_S (1U << 6)
#define T1_LUT_SIG_S (1U << 7)
/* END of flags that apply to opj_flag_enc_t */

/* ----------------------------------------------------------------------- */

typedef OPJ_UINT16 opj_flag_t;

/** Flags for 4 consecutive rows of a column */
typedef OPJ_UINT16 opj_colflag_t;

typedef OPJ_UINT32 opj_flag_enc_t;

/**
Tier-1 coding (coding of code-block coefficients)
*/
typedef struct opj_t1 {

    /** MQC component */
    opj_mqc_t *mqc;
    /** RAW component */
    opj_raw_t *raw;

    OPJ_INT32  *data;
    /** Flags used by decoder */
    opj_flag_t *flags;
    /** Addition flag array such that colflags[1+0] is for state of col=0,row=0..3,
       colflags[1+1] for col=1, row=0..3, colflags[1+flags_stride] for col=0,row=4..7, ...
       This array avoids too much cache trashing when processing by 4 vertical samples
       as done in the various decoding steps. */
    opj_colflag_t* colflags;
    /** Flags used by encoder */
    opj_flag_enc_t *enc_flags;
    OPJ_UINT32 w;
    OPJ_UINT32 h;
    OPJ_UINT32 datasize;
    OPJ_UINT32 flagssize;
    OPJ_UINT32 flags_stride;
    OPJ_UINT32 colflags_size;
    OPJ_UINT32 data_stride;
    OPJ_BOOL   encoder;
} opj_t1_t;

#define MACRO_t1_flags(x,y) t1->flags[((x)*(t1->flags_stride))+(y)]

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */

/**
Encode the code-blocks of a tile
@param t1 T1 handle
@param tile The tile to encode
@param tcp Tile coding parameters
@param mct_norms  FIXME DOC
@param mct_numcomps Number of components used for MCT
*/
OPJ_BOOL opj_t1_encode_cblks(opj_t1_t *t1,
                             opj_tcd_tile_t *tile,
                             opj_tcp_t *tcp,
                             const OPJ_FLOAT64 * mct_norms,
                             OPJ_UINT32 mct_numcomps);

/**
Decode the code-blocks of a tile
@param t1 T1 handle
@param tilec The tile to decode
@param tccp Tile coding parameters
*/
void opj_t1_decode_cblks(opj_thread_pool_t* tp,
                         volatile OPJ_BOOL* pret,
                         opj_tcd_tilecomp_t* tilec,
                         opj_tccp_t* tccp);



/**
 * Creates a new Tier 1 handle
 * and initializes the look-up tables of the Tier-1 coder/decoder
 * @return a new T1 handle if successful, returns NULL otherwise
*/
opj_t1_t* opj_t1_create(OPJ_BOOL isEncoder);

/**
 * Destroys a previously created T1 handle
 *
 * @param p_t1 Tier 1 handle to destroy
*/
void opj_t1_destroy(opj_t1_t *p_t1);
/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __T1_H */
