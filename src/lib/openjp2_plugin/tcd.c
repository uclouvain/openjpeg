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
 * Copyright (c) 2006-2007, Parvatha Elangovan
 * Copyright (c) 2008, 2011-2012, Centre National d'Etudes Spatiales (CNES), FR 
 * Copyright (c) 2012, CS Systemes d'Information, France
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

static OPJ_BOOL opj_tcd_t1_decode (opj_tcd_t *p_tcd);

static OPJ_BOOL opj_tcd_dwt_decode (opj_tcd_t *p_tcd);

static OPJ_BOOL opj_tcd_mct_decode (opj_tcd_t *p_tcd);

static OPJ_BOOL opj_tcd_dc_level_shift_decode (opj_tcd_t *p_tcd);

static OPJ_BOOL opj_tcd_dc_level_shift_encode ( opj_tcd_t *p_tcd );

static OPJ_BOOL opj_tcd_mct_encode ( opj_tcd_t *p_tcd );

static OPJ_BOOL opj_tcd_dwt_encode ( opj_tcd_t *p_tcd );

static OPJ_BOOL opj_tcd_t1_encode ( opj_tcd_t *p_tcd );




OPJ_BOOL opj_tcd_encode_tile(   opj_tcd_t *p_tcd,
                                OPJ_UINT32 p_tile_no,
                                OPJ_BYTE *p_dest,
                                OPJ_UINT32 * p_data_written,
                                OPJ_UINT32 p_max_length,
                                opj_codestream_info_t *p_cstr_info)
{
		if (!p_tcd)
			return OPJ_FALSE;

        if (p_tcd->cur_tp_num == 0) {

            p_tcd->tcd_tileno = p_tile_no;
            p_tcd->tcp = &p_tcd->cp->tcps[p_tile_no];

            if (! opj_tcd_dc_level_shift_encode(p_tcd)) {
                    return OPJ_FALSE;
            }
            if (! opj_tcd_mct_encode(p_tcd)) {
                    return OPJ_FALSE;
            }
            if (! opj_tcd_dwt_encode(p_tcd)) {
                    return OPJ_FALSE;
            }
            if (! opj_tcd_t1_encode(p_tcd)) {
                    return OPJ_FALSE;
            }
        }
 
        return OPJ_TRUE;
}

OPJ_BOOL opj_tcd_decode_tile(   opj_tcd_t *p_tcd,
                                OPJ_BYTE *p_src,
                                OPJ_UINT32 p_max_length,
                                OPJ_UINT32 p_tile_no,
                                opj_codestream_index_t *p_cstr_index
                                )
{
        p_tcd->tcd_tileno = p_tile_no;
        p_tcd->tcp = &(p_tcd->cp->tcps[p_tile_no]);

        if  (! opj_tcd_t1_decode(p_tcd))
        {
                return OPJ_FALSE;
        }
        if  (! opj_tcd_dwt_decode(p_tcd))
        {
                return OPJ_FALSE;
        }
        if     (! opj_tcd_mct_decode(p_tcd))
        {
                return OPJ_FALSE;
        }
        if  (! opj_tcd_dc_level_shift_decode(p_tcd))
        {
                return OPJ_FALSE;
        }
        return OPJ_TRUE;
}


OPJ_BOOL opj_tcd_t1_decode ( opj_tcd_t *p_tcd )
{
        OPJ_UINT32 compno;
        opj_tcd_tile_t * l_tile = p_tcd->tcd_image->tiles;
        opj_tcd_tilecomp_t* l_tile_comp = l_tile->comps;
        opj_tccp_t * l_tccp = p_tcd->tcp->tccps;
        for (compno = 0; compno < l_tile->numcomps; ++compno) {
            /* The +3 is headroom required by the vectorized DWT */
            if (OPJ_FALSE == opj_t1_decode_cblks(l_tile_comp, l_tccp)) {
                    return OPJ_FALSE;
            }
            ++l_tile_comp;
            ++l_tccp;
        }
        return OPJ_TRUE;
}


OPJ_BOOL opj_tcd_dwt_decode ( opj_tcd_t *p_tcd )
{
        opj_tcd_tile_t * l_tile = p_tcd->tcd_image->tiles;
		OPJ_INT64 compno;
		OPJ_BOOL rc = OPJ_TRUE;
#ifdef _OPENMP
		 #pragma omp parallel default(none) private(compno) shared(p_tcd, l_tile, rc)
		 {
		#pragma omp for
#endif
        for (compno = 0; compno < (OPJ_INT64)l_tile->numcomps; compno++) {
			 opj_tcd_tilecomp_t * l_tile_comp = l_tile->comps + compno;
			opj_tccp_t * l_tccp = p_tcd->tcp->tccps + compno;
			opj_image_comp_t * l_img_comp = p_tcd->image->comps + compno;
            if (l_tccp->qmfbid == 1) {
                    if (! opj_dwt_decode(l_tile_comp, l_img_comp->resno_decoded+1)) {
						rc = OPJ_FALSE;
						continue;
                    }
            }
            else {
                    if (! opj_dwt_decode_real(l_tile_comp, l_img_comp->resno_decoded+1)) {
						rc = OPJ_FALSE;
						continue;
                    }
            }
#ifdef _OPENMP
          }
#endif
		 }
        return rc;
}
OPJ_BOOL opj_tcd_mct_decode ( opj_tcd_t *p_tcd )
{
        opj_tcd_tile_t * l_tile = p_tcd->tcd_image->tiles;
        opj_tcp_t * l_tcp = p_tcd->tcp;
        opj_tcd_tilecomp_t * l_tile_comp = l_tile->comps;
        OPJ_UINT32 l_samples,i;

        if (! l_tcp->mct) {
                return OPJ_TRUE;
        }

        l_samples = (OPJ_UINT32)((l_tile_comp->x1 - l_tile_comp->x0) * (l_tile_comp->y1 - l_tile_comp->y0));

        if (l_tile->numcomps >= 3 ){
                /* testcase 1336.pdf.asan.47.376 */
                if ((l_tile->comps[0].x1 - l_tile->comps[0].x0) * (l_tile->comps[0].y1 - l_tile->comps[0].y0) < (OPJ_INT32)l_samples ||
                    (l_tile->comps[1].x1 - l_tile->comps[1].x0) * (l_tile->comps[1].y1 - l_tile->comps[1].y0) < (OPJ_INT32)l_samples ||
                    (l_tile->comps[2].x1 - l_tile->comps[2].x0) * (l_tile->comps[2].y1 - l_tile->comps[2].y0) < (OPJ_INT32)l_samples) {
                        fprintf(stderr, "Tiles don't all have the same dimension. Skip the MCT step.\n");
                        return OPJ_FALSE;
                }
                else if (l_tcp->mct == 2) {
                    OPJ_BYTE ** l_data;

                    if (! l_tcp->m_mct_decoding_matrix) {
                            return OPJ_TRUE;
                    }

                    l_data = (OPJ_BYTE **) opj_malloc(l_tile->numcomps*sizeof(OPJ_BYTE*));
                    if (! l_data) {
                            return OPJ_FALSE;
                    }

                    for (i=0;i<l_tile->numcomps;++i) {
                            l_data[i] = (OPJ_BYTE*) l_tile_comp->data;
                            ++l_tile_comp;
                    }

                    if (! opj_mct_decode_custom(/* MCT data */
                                                (OPJ_BYTE*) l_tcp->m_mct_decoding_matrix,
                                                /* size of components */
                                                l_samples,
                                                /* components */
                                                l_data,
                                                /* nb of components (i.e. size of pData) */
                                                l_tile->numcomps,
                                                /* tells if the data is signed */
                                                p_tcd->image->comps->sgnd)) {
                            opj_free(l_data);
                            return OPJ_FALSE;
                    }

                    opj_free(l_data);
                }
                else {
                    if (l_tcp->tccps->qmfbid == 1) {
                            opj_mct_decode(     l_tile->comps[0].data,
                                                    l_tile->comps[1].data,
                                                    l_tile->comps[2].data,
                                                    l_samples);
                    }
                    else {
                        opj_mct_decode_real((OPJ_FLOAT32*)l_tile->comps[0].data,
                                            (OPJ_FLOAT32*)l_tile->comps[1].data,
                                            (OPJ_FLOAT32*)l_tile->comps[2].data,
                                            l_samples);
                    }
                }
        }
        else {
                /* FIXME need to use opj_event_msg function */
                fprintf(stderr,"Number of components (%d) is inconsistent with a MCT. Skip the MCT step.\n",l_tile->numcomps);
        }

        return OPJ_TRUE;
}


OPJ_BOOL opj_tcd_dc_level_shift_decode ( opj_tcd_t *p_tcd )
{
        OPJ_UINT32 compno;
        opj_tcd_resolution_t* l_res = 00;
        OPJ_UINT32 l_width,l_height,i,j;
        OPJ_INT32 * l_current_ptr;
        OPJ_INT32 l_min, l_max;
        OPJ_UINT32 l_stride;

        opj_tcd_tile_t *l_tile = p_tcd->tcd_image->tiles;


        for (compno = 0; compno < l_tile->numcomps; compno++) {
		    opj_tcd_tilecomp_t *l_tile_comp = l_tile->comps + compno;
			opj_tccp_t * l_tccp = p_tcd->tcp->tccps + compno;
			opj_image_comp_t * l_img_comp = p_tcd->image->comps + compno;

            l_res = l_tile_comp->resolutions + l_img_comp->resno_decoded;
            l_width = (OPJ_UINT32)(l_res->x1 - l_res->x0);
            l_height = (OPJ_UINT32)(l_res->y1 - l_res->y0);
            l_stride = (OPJ_UINT32)(l_tile_comp->x1 - l_tile_comp->x0) - l_width;

            assert(l_height == 0 || l_width + l_stride <= l_tile_comp->data_size / l_height); /*MUPDF*/

            if (l_img_comp->sgnd) {
                    l_min = -(1 << (l_img_comp->prec - 1));
                    l_max = (1 << (l_img_comp->prec - 1)) - 1;
            }
            else {
				l_min = 0;
                    l_max = (1 << l_img_comp->prec) - 1;
            }

            l_current_ptr = l_tile_comp->data;

            if (l_tccp->qmfbid == 1) {
                for (j=0;j<l_height;++j) {
                    for (i = 0; i < l_width; ++i) {
                            *l_current_ptr = opj_int_clamp(*l_current_ptr + l_tccp->m_dc_level_shift, l_min, l_max);
                            ++l_current_ptr;
                    }
                    l_current_ptr += l_stride;
                }
            }
            else {
                for (j=0;j<l_height;++j) {
                    for (i = 0; i < l_width; ++i) {
                            OPJ_FLOAT32 l_value = *((OPJ_FLOAT32 *) l_current_ptr);
                            *l_current_ptr = opj_int_clamp((OPJ_INT32)lrintf(l_value) + l_tccp->m_dc_level_shift, l_min, l_max); ;
                            ++l_current_ptr;
                    }
                    l_current_ptr += l_stride;
                }
            }
        }
        return OPJ_TRUE;
}
               
OPJ_BOOL opj_tcd_dc_level_shift_encode ( opj_tcd_t *p_tcd )
{
        OPJ_UINT32 compno;
        opj_tcd_tilecomp_t * l_tile_comp = 00;
        opj_tccp_t * l_tccp = 00;
        opj_image_comp_t * l_img_comp = 00;
        opj_tcd_tile_t * l_tile;
        OPJ_UINT32 l_nb_elem,i;
        OPJ_INT32 * l_current_ptr;

        l_tile = p_tcd->tcd_image->tiles;
        l_tile_comp = l_tile->comps;
        l_tccp = p_tcd->tcp->tccps;
        l_img_comp = p_tcd->image->comps;

        for (compno = 0; compno < l_tile->numcomps; compno++) {
                l_current_ptr = l_tile_comp->data;
                l_nb_elem = (OPJ_UINT32)((l_tile_comp->x1 - l_tile_comp->x0) * (l_tile_comp->y1 - l_tile_comp->y0));

                if (l_tccp->qmfbid == 1) {
                    for     (i = 0; i < l_nb_elem; ++i) {
                            *l_current_ptr -= l_tccp->m_dc_level_shift ;
                            ++l_current_ptr;
                    }
                }
                else {
                    for (i = 0; i < l_nb_elem; ++i) {
                            *l_current_ptr = (*l_current_ptr - l_tccp->m_dc_level_shift) << 11 ;
                            ++l_current_ptr;
                    }
                }

                ++l_img_comp;
                ++l_tccp;
                ++l_tile_comp;
        }

        return OPJ_TRUE;
}

OPJ_BOOL opj_tcd_mct_encode ( opj_tcd_t *p_tcd )
{
        opj_tcd_tile_t * l_tile = p_tcd->tcd_image->tiles;
        opj_tcd_tilecomp_t * l_tile_comp = p_tcd->tcd_image->tiles->comps;
        OPJ_UINT32 samples = (OPJ_UINT32)((l_tile_comp->x1 - l_tile_comp->x0) * (l_tile_comp->y1 - l_tile_comp->y0));
        OPJ_UINT32 i;
        OPJ_BYTE ** l_data = 00;
        opj_tcp_t * l_tcp = p_tcd->tcp;

        if(!p_tcd->tcp->mct) {
                return OPJ_TRUE;
        }

        if (p_tcd->tcp->mct == 2) {
            if (! p_tcd->tcp->m_mct_coding_matrix) {
                    return OPJ_TRUE;
            }

        l_data = (OPJ_BYTE **) opj_malloc(l_tile->numcomps*sizeof(OPJ_BYTE*));
                if (! l_data) {
                        return OPJ_FALSE;
                }

                for (i=0;i<l_tile->numcomps;++i) {
                        l_data[i] = (OPJ_BYTE*) l_tile_comp->data;
                        ++l_tile_comp;
                }

                if (! opj_mct_encode_custom(/* MCT data */
                                        (OPJ_BYTE*) p_tcd->tcp->m_mct_coding_matrix,
                                        /* size of components */
                                        samples,
                                        /* components */
                                        l_data,
                                        /* nb of components (i.e. size of pData) */
                                        l_tile->numcomps,
                                        /* tells if the data is signed */
                                        p_tcd->image->comps->sgnd) )
                {
            opj_free(l_data);
                        return OPJ_FALSE;
                }

                opj_free(l_data);
        }
        else if (l_tcp->tccps->qmfbid == 0) {
                opj_mct_encode_real(l_tile->comps[0].data, l_tile->comps[1].data, l_tile->comps[2].data, samples);
        }
        else {
                opj_mct_encode(l_tile->comps[0].data, l_tile->comps[1].data, l_tile->comps[2].data, samples);
        }

        return OPJ_TRUE;
}



OPJ_BOOL opj_tcd_dwt_encode ( opj_tcd_t *p_tcd )
{
       opj_tcd_tile_t * l_tile = p_tcd->tcd_image->tiles;
        OPJ_INT64 compno;
		OPJ_BOOL rc = OPJ_TRUE;
#ifdef _OPENMP
		 #pragma omp parallel default(none) private(compno) shared(p_tcd, l_tile, rc)
		 {
		#pragma omp for
#endif
        for (compno = 0; compno < (OPJ_INT64)l_tile->numcomps; ++compno) {
			   opj_tcd_tilecomp_t * tile_comp = p_tcd->tcd_image->tiles->comps + compno;
			    opj_tccp_t * l_tccp = p_tcd->tcp->tccps + compno;
                if (l_tccp->qmfbid == 1) {
                        if (! opj_dwt_encode(tile_comp)) {
							rc = OPJ_FALSE;
							continue;
                        }
                }
                else if (l_tccp->qmfbid == 0) {
                        if (! opj_dwt_encode_real(tile_comp)) {
							rc = OPJ_FALSE;
							continue;
                        }
                }
        }
#ifdef _OPENMP
		 }
#endif
		 
        return rc;
}

OPJ_BOOL opj_tcd_t1_encode ( opj_tcd_t *p_tcd )
{
    const OPJ_FLOAT64 * l_mct_norms;
    OPJ_UINT32 l_mct_numcomps = 0U;
    opj_tcp_t * l_tcp = p_tcd->tcp;

    if (l_tcp->mct == 1) {
            l_mct_numcomps = 3U;
            /* irreversible encoding */
            if (l_tcp->tccps->qmfbid == 0) {
                    l_mct_norms = opj_mct_get_mct_norms_real();
            }
            else {
                    l_mct_norms = opj_mct_get_mct_norms();
            }
    }
    else {
            l_mct_numcomps = p_tcd->image->numcomps;
            l_mct_norms = (const OPJ_FLOAT64 *) (l_tcp->mct_norms);
    }

	return opj_t1_encode_cblks(p_tcd->tcd_image->tiles, l_tcp, l_mct_norms, l_mct_numcomps);
}
