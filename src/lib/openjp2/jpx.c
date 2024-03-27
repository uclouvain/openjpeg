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
 * Writes out a fragment table.
 *
 * @param   jp2file     Path to jp2 file being embedded
 * @param   cio         the stream to write data to.
 * @param   jpx         the jpx file codec.
 * @param   p_manager   user event manager.
 *
 * @return true if writing was successful.
*/
static OPJ_BOOL opj_jpx_write_ftbl(const char* jp2file,
                                   opj_jpx_t *jpx,
                                   opj_stream_private_t *cio,
                                   opj_event_mgr_t * p_manager);

/**
 * Searches a given jp2 file for codestream information.
 *
 * @param[in]  jp2file JPEG2000 file to search through for codestream info.
 * @param[in]  p_manager user event manager.
 * @param[out] codestream_offset Byte offset from start of file where the codestream is located
 * @param[out] codestream_length Length of the codestream
 * @returns OPJ_TRUE if successful
 */
static OPJ_BOOL opj_jpx_find_codestream(const char* jp2file,
                                        opj_event_mgr_t * p_manager,
                                        OPJ_UINT32* codestream_offset,
                                        OPJ_UINT32* codestream_length);

/** Write out fragment tables for the linked jp2 files */
OPJ_BOOL opj_jpx_encode(opj_jpx_t *jpx,
                        opj_stream_private_t *stream,
                        opj_event_mgr_t * p_manager)
{
    OPJ_UINT32 index = 0;

    assert(jpx != 00);
    assert(jpx->files != 00);
    assert(stream != 00);
    assert(p_manager != 00);

    /**
     * Iterate over each jp2 file that we're linking to, and add a fragment
     * table box for each one.
     */
    for (index = 0; index < jpx->file_count; index += 1) {
        const char* jp2_fname = jpx->files[index];
        // Current file index is 1-based.
        jpx->current_file_index = index + 1;
        // Write out the fragment table for this jp2 file.
        if (!opj_jpx_write_ftbl(jp2_fname, jpx, stream, p_manager)) {
            opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to write fragment tables\n");
            return OPJ_FALSE;
        }
    }
    puts("Added fragment tables");

    // /**
    //  * Iterate over each file again, this time inserting any
    //  * associations (xml boxes)
    //  */
    // for (index = 0; index < jpx->file_count; index += 1) {
    //     const char* jp2_fname = jpx->files[index];
    //     opj_jpx_write_ftbl(jp2_fname, jpx, stream, p_manager);
    // }

    /** Flush data to output stream */
    if (! opj_stream_flush(stream, p_manager)) {
        return OPJ_FALSE;
    }

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

    /** Flush header to output stream */
    if (! opj_stream_flush(stream, p_manager)) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

OPJ_BOOL opj_jpx_setup_encoder(opj_jpx_t *jpx,
                               opj_cparameters_t *parameters,
                               opj_image_t *image,
                               opj_event_mgr_t * p_manager)
{
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
    OPJ_UINT32 i = 0;
    assert(p_jpx != 00);
    assert(p_options != 00);
    assert(p_manager != 00);
    p_jpx->files = p_options;
    // Count the number of files given in the null terminated list.
    while (p_jpx->files[i] != NULL) { i++; }
    p_jpx->file_count = i;
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

static OPJ_BOOL opj_jpx_write_ftbl(const char* jp2file,
                                   opj_jpx_t *jpx,
                                   opj_stream_private_t *cio,
                                   opj_event_mgr_t * p_manager)
{
    /** A fragment table consists of a fragment table box + a fragment list box. */
    /** A 1-length fragment list box is 24 bytes */
    /** So the whole table box should be 32 bytes */
    OPJ_BYTE ftbl[32];
    OPJ_BYTE* ftbl_ptr = &ftbl[0];
    // FIXME: the specification states offset can be a uint64.
    //        opj_write_bytes only writes uint32s.
    OPJ_UINT32 codestream_offset;
    OPJ_UINT32 codestream_length;
    OPJ_UINT16 file_index = jpx->current_file_index;
    OPJ_BOOL bSuccess = opj_jpx_find_codestream(jp2file, p_manager, &codestream_offset, &codestream_length);
    if (!bSuccess) {
        opj_event_msg(p_manager, EVT_ERROR, "Failed to find codestream information in %s\n", jp2file);
        return OPJ_FALSE;
    }

    /** Write box size */
    opj_write_bytes(ftbl_ptr, 32, 4);
    ftbl_ptr += 4;
    /** Write box type */
    opj_write_bytes(ftbl_ptr, JPX_FTBL, 4);
    ftbl_ptr += 4;
    /** Write out fragment list */
    /** Fragment list box size */
    opj_write_bytes(ftbl_ptr, 24, 4);
    ftbl_ptr += 4;
    /** Fragment list box type */
    opj_write_bytes(ftbl_ptr, JPX_FLST, 4);
    ftbl_ptr += 4;
    /** Fragment list contents */
    // Number of fragments
    opj_write_bytes(ftbl_ptr, 1, 2);
    ftbl_ptr += 2;
    /* Codestream offset */
    opj_write_bytes(ftbl_ptr, 0, 4);
    ftbl_ptr += 4;
    opj_write_bytes(ftbl_ptr, codestream_offset, 4);
    ftbl_ptr += 4;
    /* Codestream length */
    opj_write_bytes(ftbl_ptr, codestream_length, 4);
    ftbl_ptr += 4;
    /* Data Reference index */
    opj_write_bytes(ftbl_ptr, file_index, 2);

    if (opj_stream_write_data(cio, ftbl, 32, p_manager) != 32) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static OPJ_BOOL opj_jpx_find_codestream(const char* jp2file,
                                        opj_event_mgr_t * p_manager,
                                        OPJ_UINT32* codestream_offset,
                                        OPJ_UINT32* codestream_length)
{
    OPJ_BYTE l_data_header [8];
    OPJ_UINT32 box_length = UINT32_MAX;
    OPJ_UINT32 box_type = 0;
    OPJ_OFF_T stream_position = 0;
    assert(jp2file != 00);
    assert(codestream_offset != 00);
    assert(codestream_length != 00);

    // Create read stream for the jp2 file.
    opj_stream_t* stream = opj_stream_create_default_file_stream(jp2file, OPJ_TRUE);
    if (!stream) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to open %s for reading\n", jp2file);
        return OPJ_FALSE;
    }

    /* Iterate over the boxes in the jp2 file until we find the codestream */
    /* Search for box_length 0, this indicates the codestream location. */
    while (box_length != 0 && box_type != JP2_JP2C) {
        // Seek to the next box in the stream.
        if (!opj_stream_seek(stream, stream_position, p_manager)) {
            opj_event_msg(p_manager, EVT_ERROR,
                        "Failed to seek while reading %s\n", jp2file);
            opj_stream_destroy(stream);
            return OPJ_FALSE;
        }

        // Read the box header.
        if (opj_stream_read_data(stream, l_data_header, 8, p_manager) != 8) {
            opj_event_msg(p_manager, EVT_ERROR,
                        "Failed to read box information from %s\n", jp2file);
            opj_stream_destroy(stream);
            return OPJ_FALSE;
        }

        // Parse the box header
        opj_read_bytes(l_data_header, &box_length, 4);
        opj_read_bytes(l_data_header + 4, &box_type, 4);

        // Update the stream location
        stream_position += box_length;
    }

    // If the loop exists, then the stream should be at the codestream position.

    *codestream_offset = opj_stream_tell(stream);
    // Assume the codestream runs until the end of the file.
    *codestream_length = opj_stream_get_number_byte_left(stream);

    opj_stream_destroy(stream);
    return OPJ_TRUE;
}