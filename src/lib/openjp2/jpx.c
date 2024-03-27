/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2024, Daniel Garcia Briseno, ADNET Systems Inc, NASA
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

/**
 * Executes the given procedures on the given codec.
 *
 * @param   p_procedure_list    the list of procedures to execute
 * @param   jpx                 the jpeg2000 file codec to execute the procedures on.
 * @param   stream                  the stream to execute the procedures on.
 * @param   p_manager           the user manager.
 *
 * @return  true                if all the procedures were successfully executed.
 */
static OPJ_BOOL opj_jpx_exec(opj_jpx_t * jpx,
                             opj_procedure_list_t * p_procedure_list,
                             opj_stream_private_t *stream,
                             opj_event_mgr_t * p_manager);

OPJ_BOOL opj_jpx_encode(opj_jpx_t *jpx,
                        opj_stream_private_t *stream,
                        opj_event_mgr_t * p_manager)
{
    puts("Called opj_jpx_encode");
    return OPJ_TRUE;
}

OPJ_BOOL opj_jpx_end_compress(opj_jpx_t *jpx,
                              opj_stream_private_t *cio,
                              opj_event_mgr_t * p_manager
                             )
{
    puts("Called opj_jpx_end_compress");
    return OPJ_TRUE;
}

static OPJ_BOOL opj_jpx_setup_header_writing(opj_jpx_t *jpx,
        opj_event_mgr_t * p_manager)
{
    /* preconditions */
    assert(jpx != 00);
    assert(p_manager != 00);

    if (! opj_procedure_list_add_procedure(jpx->jp2->m_procedure_list,
                                           (opj_procedure)opj_jp2_write_jp, p_manager)) {
        return OPJ_FALSE;
    }
    if (! opj_procedure_list_add_procedure(jpx->jp2->m_procedure_list,
                                           (opj_procedure)opj_jp2_write_ftyp, p_manager)) {
        return OPJ_FALSE;
    }

    if (! opj_procedure_list_add_procedure(jpx->jp2->m_procedure_list,
                                           (opj_procedure)opj_jpx_write_rreq, p_manager)) {
        return OPJ_FALSE;
    }

    /* DEVELOPER CORNER, insert your custom procedures */

    return OPJ_TRUE;
}

OPJ_BOOL opj_jpx_start_compress(opj_jpx_t *jpx,
                                opj_stream_private_t *stream,
                                opj_image_t * p_image,
                                opj_event_mgr_t * p_manager
                               )
{
    assert(jpx != 00);
    assert(stream != 00);
    assert(p_manager != 00);

    if (! opj_jpx_setup_header_writing(jpx, p_manager)) {
        return OPJ_FALSE;
    }

    /* write header */
    if (! opj_jp2_exec(jpx->jp2, jpx->jp2->m_procedure_list, stream, p_manager)) {
        return OPJ_FALSE;
    }

    if (! opj_stream_flush(stream, p_manager)) {
        return OPJ_FALSE;
    }

    puts("Called opj_jpx_start_compress. execced");
    return OPJ_TRUE;
}

OPJ_BOOL opj_jpx_setup_encoder(opj_jpx_t *jpx,
                               opj_cparameters_t *parameters,
                               opj_image_t *image,
                               opj_event_mgr_t * p_manager)
{
    puts("Called opj_jpx_setup_encoder");
    assert(jpx != 00);
    assert(parameters != 00);
    assert(p_manager != 00);
    // Need to set brand in ftyp box to "jpx "
    jpx->jp2->brand = JPX_JPX;
    // JPX compatibility list should contain "jpx ", "jp2 ", and "jpxb"
    jpx->jp2->numcl = 3;
    // Allocate memory for the compatibility list.
    jpx->jp2->cl = (OPJ_UINT32*) opj_malloc(jpx->jp2->numcl * sizeof(OPJ_UINT32));
    // Return failure if we couldn't allocate memory for the cl
    if (!jpx->jp2->cl) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Not enough memory when setting up the JPX encoder\n");
        return OPJ_FALSE;
    }
    // Assign the cl values
    jpx->jp2->cl[0] = JPX_JPX;
    jpx->jp2->cl[1] = JP2_JP2;
    jpx->jp2->cl[2] = JPX_JPXB;
    return OPJ_TRUE;
}

OPJ_BOOL opj_jpx_encoder_set_extra_options(
    opj_jpx_t *p_jpx,
    const char* const* p_options,
    opj_event_mgr_t * p_manager)
{
    puts("Called opj_jpx_encoder_set_extra_options");
    p_jpx->files = p_options;
    puts("Set the list of files to be processed");
    return OPJ_TRUE;
}

OPJ_BOOL opj_jpx_set_threads(opj_jpx_t *jpx, OPJ_UINT32 num_threads)
{
    puts("Called opj_jpx_set_threads");
    return OPJ_TRUE;
}

opj_jpx_t* opj_jpx_create(void)
{
    opj_jpx_t *jpx = (opj_jpx_t*)opj_calloc(1, sizeof(opj_jpx_t));
    /* execution list creation */
    jpx->m_procedure_list = opj_procedure_list_create();
    if (! jpx->m_procedure_list) {
        opj_jpx_destroy(jpx);
        return 00;
    }

    jpx->jp2 = opj_jp2_create(OPJ_FALSE);
    if (!jpx->jp2) {
        opj_jpx_destroy(jpx);
        return 00;
    }
    puts("Created opj_jpx_t instance with a procedure list");
    return jpx;
}

void opj_jpx_destroy(opj_jpx_t *jpx)
{
    if (jpx) {
        if (jpx->m_procedure_list) {
            opj_procedure_list_destroy(jpx->m_procedure_list);
        }
        if (jpx->jp2) {
            opj_jp2_destroy(jpx->jp2);
        }
        opj_free(jpx);
    }
}

/**
 * Writes an RREQ box - Reader Requirements box
 *
 * @param   cio         the stream to write data to.
 * @param   jpx         the jpeg2000 file codec.
 * @param   p_manager   the user event manager.
 *
 * @return  true if writing was successful.
 */
OPJ_BOOL opj_jpx_write_rreq(opj_jp2_t *jp2,
                            opj_stream_private_t *cio,
                            opj_event_mgr_t * p_manager)
{
    /**
     * The reader requirements box is a set of feature flags that list all
     * the reader requirements needed to either display the file, or fully
     * understand the file. Some features like reading metadata are needed
     * to fully understand the file, but not needed for displaying the file.
     */

    // For this implementation of jpx support, we embed multiple codestreams
    // in the file. So we set feature flag 2: Contains multiple composition layers
    // jp2 files are only being linked, not embedded, so we set
    // flag 15: Fragmented codestream where not all fragments are within the file
    // but all are in locally accessible files
    // We only need 1 byte to support both features masks
    OPJ_UINT8 mask_length = 1;
    OPJ_UINT8 fully_understand = 0b00000011;
    OPJ_UINT8 display_mask     = 0b00000011;
    OPJ_UINT16 num_flags = 2;
    OPJ_UINT16 flags[2] = {RREQ_FLAG_MULTILAYERED,
                           RREQ_FLAG_REFERENCE_LOCAL_FILES};
    OPJ_UINT8 flag_masks[2] = {0b00000001,  // mask for multilayered
                               0b00000010}; // mask for local files
    OPJ_UINT16 num_vendor_features = 0;
    // Above adds up to 13 bytes. Add 8 bytes for box length and box type.
    OPJ_UINT32 box_length = 21;
    OPJ_BYTE rreq_data[box_length];
    OPJ_BYTE * rreq_data_ptr = &rreq_data[0];
    OPJ_UINT32 i;

    /* preconditions */
    assert(cio != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);

    OPJ_UNUSED(jp2);

    /* write box length */
    opj_write_bytes(rreq_data_ptr, box_length, 4);
    rreq_data_ptr += 4;

    /* writes box type */
    opj_write_bytes(rreq_data_ptr, JPX_RREQ, 4);
    rreq_data_ptr += 4;

    /* write reader requirements */
    /* mask length */
    opj_write_bytes(rreq_data_ptr, mask_length, 1);
    rreq_data_ptr += 1;

    /* fully understand aspects mask */
    opj_write_bytes(rreq_data_ptr, fully_understand, 1);
    rreq_data_ptr += 1;

    /* display contents mask */
    opj_write_bytes(rreq_data_ptr, display_mask, 1);
    rreq_data_ptr += 1;

    /* number of flags */
    opj_write_bytes(rreq_data_ptr, num_flags, 2);
    rreq_data_ptr += 2;

    /* Write out each flag followed by its mask */
    for (i = 0; i < num_flags; i++) {
        opj_write_bytes(rreq_data_ptr, flags[i], 2);
        rreq_data_ptr += 2;
        opj_write_bytes(rreq_data_ptr, flag_masks[i], 1);
        rreq_data_ptr += 1;
    }

    /* Write out vendor features */
    opj_write_bytes(rreq_data_ptr, num_vendor_features, 2);
    rreq_data_ptr += 2;

    if (opj_stream_write_data(cio, rreq_data, box_length, p_manager) != box_length) {
        return OPJ_FALSE;
    }

    puts("All good writing header");
    return OPJ_TRUE;
}

/**
 * Ignored for jpx files. Here for codec compatibility.
 *
 * @param  p_jpx    the jpeg2000 codec.
 * @param p_tile_index  FIXME DOC
 * @param p_data        FIXME DOC
 * @param p_data_size   FIXME DOC
 * @param  p_stream      the stream to write data to.
 * @param  p_manager  the user event manager.
 */
OPJ_BOOL opj_jpx_write_tile(opj_jpx_t *p_jpx,
                            OPJ_UINT32 p_tile_index,
                            OPJ_BYTE * p_data,
                            OPJ_UINT32 p_data_size,
                            opj_stream_private_t *p_stream,
                            opj_event_mgr_t * p_manager)
{
    assert(p_manager != 00);
    opj_event_msg(p_manager, EVT_WARNING,
                      "JPX encoder does not support write tile. Ignoring write tile request.\n");
    return OPJ_TRUE;
}