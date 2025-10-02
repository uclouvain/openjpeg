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

#include <limits.h>
#include <stdlib.h>
#include "opj_includes.h"
#ifdef WIN32
#include <windows.h>
#endif

/**
 * Container for a box.
 */
typedef struct box {
    OPJ_UINT32 length;
    OPJ_UINT32 type;
    OPJ_BYTE* contents;
} box_t;

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
 * Returns the absolute path for the given file
 * The buffer returned must be free'd with opj_free
 * @returns NULL on failure, else the absolute path
 */
static char* opj_jpx_get_absolute_path(const char* relative_path);

/**
 * Writes out an association table.
 *
 * @param   jp2file     Path to jp2 file being embedded
 * @param   cio         the stream to write data to.
 * @param   jpx         the jpx file codec.
 * @param   p_manager   user event manager.
 *
 * @return true if writing was successful.
*/
static OPJ_BOOL opj_jpx_write_asoc(const char* jp2file,
                                   opj_jpx_t *jpx,
                                   opj_stream_private_t *cio,
                                   opj_event_mgr_t * p_manager);

/**
 * Writes out the data reference table.
 *
 * @param   jp2file     Path to jp2 file being embedded
 * @param   cio         the stream to write data to.
 * @param   jpx         the jpx file codec.
 * @param   p_manager   user event manager.
 *
 * @return true if writing was successful.
 */
static OPJ_BOOL opj_jpx_write_dtbl(opj_jpx_t *jpx,
                                   opj_stream_private_t *cio,
                                   opj_event_mgr_t * p_manager);

/**
 * Computes the size of the Data Reference box based on the jpx encoder parameters.
 */
static OPJ_UINT32 opj_jpx_compute_dtbl_size(opj_jpx_t *jpx);

/**
 * Computes the size of the url box to hold the given file path.
 * @returns UINT32_MAX on failure, else the url box size
 */
static OPJ_UINT32 opj_jpx_compute_urlbox_size(const char* filepath);

/**
 * Reads a box from the given stream.
 * Assumes that the stream is currently pointing to a box header.
 * The resulting box must be destroyed with opj_jpx_destroy_box.
 * @param[in] l_stream stream to read from
 * @param[in] p_manager user event manager
 * @param[out] box Box struct to store box data.
 */
static OPJ_BOOL opj_jpx_read_box(opj_stream_private_t *l_stream,
                                 opj_event_mgr_t * p_manager,
                                 box_t* box);

/** Frees memory allocated for the given box */
static void opj_jpx_destroy_box(box_t box);

/** Initializes options for the jp2 encoder. */
static OPJ_BOOL opj_jpx_init_jp2_options(opj_jpx_t* jpx,
        opj_event_mgr_t * p_manager);

/**
 * Writes data via a cursor.
 * Each call, cursor will be moved forward by the number of bytes written.
 */
static void opj_jpx_cursor_write(OPJ_BYTE** p_cursor,
                                 OPJ_UINT32 p_value,
                                 OPJ_UINT32 p_nb_bytes);

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

    /*
     * Iterate over each jp2 file that we're linking to, and add a fragment
     * table box for each one.
     */
    /* The jpx spec only allows a u16 for the number of references. */
    /* We should never have more than that. */
    assert(jpx->file_count < UINT16_MAX);
    for (index = 0; index < jpx->file_count; index += 1) {
        const char* jp2_fname = jpx->files[index];

        // Current file index is 1-based.
        jpx->current_file_index = ((OPJ_UINT16)index) + 1;
        // Write out the fragment table for this jp2 file.
        if (!opj_jpx_write_ftbl(jp2_fname, jpx, stream, p_manager)) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Failed to write fragment table boxes\n");
            return OPJ_FALSE;
        }
    }

    /**
     * Iterate over each file again, this time inserting any
     * associations (xml boxes)
     */
    for (index = 0; index < jpx->file_count; index += 1) {
        const char* jp2_fname = jpx->files[index];
        // Current file index is 1-based.
        jpx->current_file_index = (OPJ_UINT16)index + 1;

        // Write out associations for this jp2 file.
        if (!opj_jpx_write_asoc(jp2_fname, jpx, stream, p_manager)) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Failed to write association boxes\n");
            return OPJ_FALSE;
        }
    }

    /**
     * Final step. Create the data reference table for the jp2 files.
     */
    if (!opj_jpx_write_dtbl(jpx, stream, p_manager)) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to write data reference table\n");
        return OPJ_FALSE;
    }

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
    OPJ_UNUSED(jpx);
    OPJ_UNUSED(cio);
    OPJ_UNUSED(p_manager);
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

    if (! opj_procedure_list_add_procedure(jpx->jp2->m_procedure_list,
                                           (opj_procedure)opj_jp2_write_jp2h, p_manager)) {
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

    OPJ_UNUSED(p_image);

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
    while (p_jpx->files[i] != NULL) {
        i++;
    }
    p_jpx->file_count = i;

    // Read the first jp2 file to set the remaining jp2 parameters
    if (!opj_jpx_init_jp2_options(p_jpx, p_manager)) {
        return OPJ_FALSE;
    }
    return OPJ_TRUE;
}

OPJ_BOOL opj_jpx_set_threads(opj_jpx_t *jpx, OPJ_UINT32 num_threads)
{
    OPJ_UNUSED(jpx);
    OPJ_UNUSED(num_threads);
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

    jpx->jp2 = opj_jp2_create(OPJ_TRUE);
    if (!jpx->jp2) {
        opj_jpx_destroy(jpx);
        return 00;
    }
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
                           RREQ_FLAG_REFERENCE_LOCAL_FILES
                          };
    OPJ_UINT8 flag_masks[2] = {0b00000001,  /* mask for multilayered */
                               0b00000010
                              }; /* mask for local files */
    OPJ_UINT16 num_vendor_features = 0;
    /* Above adds up to 13 bytes.Add 8 bytes for box length and box type. */
    OPJ_UINT32 box_length = 21;
    OPJ_BYTE rreq_data[21];
    OPJ_BYTE * cursor = &rreq_data[0];
    OPJ_UINT32 i;

    /* preconditions */
    assert(cio != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);

    OPJ_UNUSED(jp2);

    /* write box length */
    opj_jpx_cursor_write(&cursor, box_length, 4);

    /* writes box type */
    opj_jpx_cursor_write(&cursor, JPX_RREQ, 4);

    /* write reader requirements */
    /* mask length */
    opj_jpx_cursor_write(&cursor, mask_length, 1);

    /* fully understand aspects mask */
    opj_jpx_cursor_write(&cursor, fully_understand, 1);

    /* display contents mask */
    opj_jpx_cursor_write(&cursor, display_mask, 1);

    /* number of flags */
    opj_jpx_cursor_write(&cursor, num_flags, 2);

    /* Write out each flag followed by its mask */
    for (i = 0; i < num_flags; i++) {
        opj_jpx_cursor_write(&cursor, flags[i], 2);
        opj_jpx_cursor_write(&cursor, flag_masks[i], 1);
    }

    /* Write out vendor features */
    opj_jpx_cursor_write(&cursor, num_vendor_features, 2);

    /* Assert there was no overflow. */
    assert((cursor - rreq_data) == 21);

    if (opj_stream_write_data(cio, rreq_data, box_length,
                              p_manager) != box_length) {
        return OPJ_FALSE;
    }

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
    OPJ_UNUSED(p_jpx);
    OPJ_UNUSED(p_tile_index);
    OPJ_UNUSED(p_data);
    OPJ_UNUSED(p_data_size);
    OPJ_UNUSED(p_stream);
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
    OPJ_BYTE* cursor = &ftbl[0];
    // FIXME: the specification states offset can be a uint64.
    //        opj_write_bytes only writes uint32s.
    OPJ_UINT32 codestream_offset;
    OPJ_UINT32 codestream_length;
    OPJ_UINT16 file_index = jpx->current_file_index;
    OPJ_BOOL bSuccess = opj_jpx_find_codestream(jp2file, p_manager,
                        &codestream_offset, &codestream_length);
    if (!bSuccess) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to find codestream information in %s\n", jp2file);
        return OPJ_FALSE;
    }

    /** Write box size */
    opj_jpx_cursor_write(&cursor, 32, 4);
    /** Write box type */
    opj_jpx_cursor_write(&cursor, JPX_FTBL, 4);
    /** Write out fragment list */
    /** Fragment list box size */
    opj_jpx_cursor_write(&cursor, 24, 4);
    /** Fragment list box type */
    opj_jpx_cursor_write(&cursor, JPX_FLST, 4);
    /** Fragment list contents */
    // Number of fragments
    opj_jpx_cursor_write(&cursor, 1, 2);
    /* Codestream offset */
    opj_jpx_cursor_write(&cursor, 0, 4);
    opj_jpx_cursor_write(&cursor, codestream_offset, 4);
    /* Codestream length */
    opj_jpx_cursor_write(&cursor, codestream_length, 4);
    /* Data Reference index */
    opj_jpx_cursor_write(&cursor, file_index, 2);

    if (opj_stream_write_data(cio, ftbl, 32, p_manager) != 32) {
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

/**
 * Does a forward search for the given box type and
 * sets the stream position to the header of that box.
 * On success, the stream will be pointing to the box length.
 *
 * If the box is not found, then the stream position is
 * undefined. If this function returns false, you should use
 * opj_stream_seek to place the stream position back to a known location.
 *
 * @param p_stream The stream to read
 * @param p_manager user event manager
 * @param box_type The box type to search for within the stream
 * @returns OPJ_TRUE if the box is found.
 */
static OPJ_BOOL opj_jpx_find_box(opj_stream_t* p_stream,
                                 opj_event_mgr_t * p_manager,
                                 OPJ_UINT32 box_type)
{
    OPJ_BYTE l_data_header [8];
    OPJ_OFF_T stream_position = 0;
    OPJ_UINT32 lbox = 0;
    OPJ_UINT32 tbox = 0;
    opj_stream_private_t* l_stream = (opj_stream_private_t*) p_stream;


    /* Iterate over the boxes in the jp2 file until we find the desired box */
    while (tbox != JP2_JP2C) {
        // Seek to the next box in the stream.
        if (!opj_stream_seek(l_stream, stream_position, p_manager)) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Failed to seek while reading stream\n");
            return OPJ_FALSE;
        }

        // Read the box header.
        if (opj_stream_read_data(l_stream, l_data_header, 8, p_manager) != 8) {
            // This probably means we reached the end of the file without
            // finding the desired box.
            opj_event_msg(p_manager, EVT_WARNING,
                          "Failed to read box information\n");
            return OPJ_FALSE;
        }

        // Parse the box header
        opj_read_bytes(l_data_header, &lbox, 4);
        opj_read_bytes(l_data_header + 4, &tbox, 4);

        // If we found the box
        if (tbox == box_type) {
            // Set the stream back to the top of the box
            if (!opj_stream_seek(l_stream, stream_position, p_manager)) {
                // If the seek fails, return false.
                opj_event_msg(p_manager, EVT_ERROR,
                              "Failed to seek while reading stream\n");
                return OPJ_FALSE;
            }

            // And return success
            return OPJ_TRUE;
        }

        // Update the stream location
        stream_position += lbox;
    }

    // We reached the codestream without finding the box.
    return OPJ_FALSE;
}

static OPJ_BOOL opj_jpx_find_codestream(const char* jp2file,
                                        opj_event_mgr_t * p_manager,
                                        OPJ_UINT32* codestream_offset,
                                        OPJ_UINT32* codestream_length)
{
    OPJ_BOOL b_found_codestream = OPJ_FALSE;
    OPJ_OFF_T stream_position = 0;
    OPJ_OFF_T bytes_remaining = 0;
    opj_stream_private_t* stream;

    assert(jp2file != 00);
    assert(codestream_offset != 00);
    assert(codestream_length != 00);

    /* Create read stream for the jp2 file. */
    stream = (opj_stream_private_t*) opj_stream_create_default_file_stream(jp2file,
             OPJ_TRUE);
    if (!stream) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to open %s for reading\n", jp2file);
        opj_stream_destroy((opj_stream_t*) stream);
        return OPJ_FALSE;
    }

    // Find the codestream section of the file.
    b_found_codestream = opj_jpx_find_box((opj_stream_t*) stream, p_manager,
                                          JP2_JP2C);
    if (!b_found_codestream) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Couldn't find jp2 codestream in %s\n", jp2file);
        opj_stream_destroy((opj_stream_t*) stream);
        return OPJ_FALSE;
    }

    /* If the codestream was found, then the stream is pointing to the codestream header. */
    /* The actual codestream starts at stream position + 8 and the length of the codestream */
    /* is the remaining bytes - 8. */
    stream_position = opj_stream_tell(stream);

    assert(stream_position < (UINT32_MAX - 8));
    *codestream_offset = (OPJ_UINT32)stream_position + 8;
    /* Assume the codestream runs until the end of the file. */
    bytes_remaining = opj_stream_get_number_byte_left(stream);
    assert(bytes_remaining < UINT32_MAX);
    *codestream_length = (OPJ_UINT32)bytes_remaining - 8;

    opj_stream_destroy((opj_stream_t*) stream);
    return OPJ_TRUE;
}

static OPJ_BOOL opj_jpx_write_asoc(const char* jp2file,
                                   opj_jpx_t *jpx,
                                   opj_stream_private_t *cio,
                                   opj_event_mgr_t * p_manager)
{
    // For the current implementation, only xml boxes are associated with the
    // embedded jp2 files. If there's no xml box, then the association is
    // skipped
    OPJ_BOOL b_has_xml_box = OPJ_FALSE;

    // Create read stream for the jp2 file.
    opj_stream_t* p_stream = opj_stream_create_default_file_stream(jp2file,
                             OPJ_TRUE);
    opj_stream_private_t* l_stream = (opj_stream_private_t*) p_stream;
    if (!p_stream) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to open %s for reading\n", jp2file);
        return OPJ_FALSE;
    }

    b_has_xml_box = opj_jpx_find_box(p_stream, p_manager, JP2_XML);
    // If there is an xml box, then embed it in an association table.
    if (b_has_xml_box) {
        // Read the xml box into memory
        box_t xmlbox;
        if (opj_jpx_read_box(l_stream, p_manager, &xmlbox)) {
            // Association will contain the xmlbox (xmlbox.length), a number list box (16), and the asoc box header (8)
            OPJ_UINT32 asoc_length = xmlbox.length + 16 + 8;
            // Allocate memory for the asoc box
            OPJ_BYTE* asoc = opj_malloc(asoc_length);
            OPJ_BYTE* cursor = asoc;
            OPJ_UINT32 associate_number;
            OPJ_UINT64 i = 0;
            if (asoc) {
                /* Write asoc header */
                opj_jpx_cursor_write(&cursor, asoc_length, 4);
                /* Write asoc box type */
                opj_jpx_cursor_write(&cursor, JPX_ASOC, 4);
                /* Write number list box length */
                opj_jpx_cursor_write(&cursor, 16, 4);
                /* Write number list type */
                opj_jpx_cursor_write(&cursor, JPX_NLST, 4);
                /* Write codestream association */
                associate_number = 0x01000000;
                // current_file_index is 1-based, so need to subtract one since
                // associate numbers are 0 based, not 1 based like others.
                associate_number |= (jpx->current_file_index - 1);
                opj_jpx_cursor_write(&cursor, associate_number, 4);
                /* Write compositing layer association */
                associate_number = 0x02000000;
                associate_number |= (jpx->current_file_index - 1);
                opj_jpx_cursor_write(&cursor, associate_number, 4);
                /* Write xml box header */
                opj_jpx_cursor_write(&cursor, xmlbox.length, 4);
                opj_jpx_cursor_write(&cursor, JP2_XML, 4);
                /* Write xml box contents */
                /* FIXME: is there a better way to dump the buffer than 1 byte at a time?) */
                for (i = 0; i < (xmlbox.length - 8); i++) {
                    opj_jpx_cursor_write(&cursor, xmlbox.contents[i], 1);
                }

                /* Dump contents to stream */
                if (opj_stream_write_data(cio, asoc, asoc_length, p_manager) != asoc_length) {
                    opj_event_msg(p_manager, EVT_WARNING,
                                  "Failed to write association contents to stream. Stream may be corrupted.\n");
                }

                opj_free(asoc);
            } else {
                opj_event_msg(p_manager, EVT_WARNING,
                              "Failed to allocate memory for association box. Skipping.\n", jp2file);
            }

            opj_jpx_destroy_box(xmlbox);
        } else {
            opj_event_msg(p_manager, EVT_WARNING,
                          "Failed to read xmlbox from %s. Skipping.\n", jp2file);
        }
    }

    opj_stream_destroy(p_stream);
    return OPJ_TRUE;
}

static OPJ_BOOL opj_jpx_read_box(opj_stream_private_t *l_stream,
                                 opj_event_mgr_t * p_manager,
                                 box_t* box)
{
    OPJ_BYTE box_header[8];

    assert(l_stream != 00);
    assert(p_manager != 00);

    if (opj_stream_read_data(l_stream, box_header, 8, p_manager) != 8) {
        opj_event_msg(p_manager, EVT_ERROR, "Failed to read box header\n");
        return OPJ_FALSE;
    }

    opj_read_bytes(box_header, &box->length, 4);
    opj_read_bytes(box_header + 4, &box->type, 4);

    box->contents = opj_malloc(box->length);
    if (!box->contents) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to allocate memory for the box\n");
        return OPJ_FALSE;
    }

    if (opj_stream_read_data(l_stream, box->contents, box->length,
                             p_manager) != box->length) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to read all the box contents into memory\n");
        opj_free(box->contents);
        return OPJ_FALSE;
    }

    return OPJ_TRUE;
}

static void opj_jpx_destroy_box(box_t box)
{
    if (box.contents) {
        opj_free(box.contents);
    }
}

static void opj_jpx_cursor_write(OPJ_BYTE** p_cursor,
                                 OPJ_UINT32 p_value,
                                 OPJ_UINT32 p_nb_bytes)
{
    opj_write_bytes(*p_cursor, p_value, p_nb_bytes);
    *p_cursor += p_nb_bytes;
}

static OPJ_BOOL opj_jpx_write_dtbl(opj_jpx_t *jpx,
                                   opj_stream_private_t *cio,
                                   opj_event_mgr_t * p_manager)
{
    OPJ_UINT64 i = 0;
    OPJ_BYTE* dtbl = 00;
    OPJ_BYTE* cursor = 00;
    // Compute the size of the data table.
    OPJ_UINT32 dtbl_size = opj_jpx_compute_dtbl_size(jpx);
    if (dtbl_size == UINT32_MAX) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to compute the data reference table size\n");
        return OPJ_FALSE;
    }

    dtbl = opj_malloc(dtbl_size);
    if (!dtbl) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to allocate memory for the data reference table\n");
        return OPJ_FALSE;
    }

    cursor = dtbl;
    /* Write box header */
    opj_jpx_cursor_write(&cursor, dtbl_size, 4);
    opj_jpx_cursor_write(&cursor, JPX_DTBL, 4);
    /* Write number of references */
    opj_jpx_cursor_write(&cursor, jpx->file_count, 2);
    /* Write data reference boxes */
    for (i = 0; i < jpx->file_count; i++) {
        /* Write box header */
        const char* jp2file = jpx->files[i];
        char* absolute_path = opj_jpx_get_absolute_path(jp2file);
        if (absolute_path == NULL) {
            opj_event_msg(p_manager, EVT_ERROR,
                          "Failed to get absolute path of file %s\n", jp2file);
            opj_free(dtbl);
            return OPJ_FALSE;
        }

        {
            OPJ_UINT32 box_size = opj_jpx_compute_urlbox_size(jpx->files[i]);
            opj_jpx_cursor_write(&cursor, box_size, 4);
            opj_jpx_cursor_write(&cursor, JPX_URL, 4);
            /** Version and Flags are defined to be 0. */
            opj_jpx_cursor_write(&cursor, 0, 4);
            /** Write the local file path url */
            strcpy((char*) cursor, "file://");
            cursor += strlen("file://");
            strcpy((char*) cursor, absolute_path);
            cursor += strlen(absolute_path);
            /* write null terminator */
            opj_jpx_cursor_write(&cursor, 0, 1);
        }

        opj_free(absolute_path);
    }

    /* Assert that there was no heap buffer overflow on dtbl memory. */
    /* The cursor should be exactly the computed size away from the  */
    /* start of the reference table. */
    assert((cursor - dtbl) == dtbl_size);

    if (opj_stream_write_data(cio, dtbl, dtbl_size, p_manager) != dtbl_size) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to write data reference table to output stream\n");
        opj_free(dtbl);
        return OPJ_FALSE;
    }

    opj_free(dtbl);
    return OPJ_TRUE;
}

static OPJ_UINT32 opj_jpx_compute_dtbl_size(opj_jpx_t *jpx)
{
    OPJ_UINT32 counter = 0;
    // The data reference table is made up of a data reference box
    // followed by a number of data entry url boxes.
    // Each data entry url box is the size of a box header + 4 bytes of 0
    // + the length of a null terminated utf8 string.
    // To compute the size of the dtbl, we have to compute the length of
    // all the strings that will be embedded.
    OPJ_UINT32 i = 0;
    for (i = 0; i < jpx->file_count; i++) {
        const char* jp2file = jpx->files[i];
        OPJ_UINT32 urlbox_size = opj_jpx_compute_urlbox_size(jp2file);
        if (urlbox_size == UINT32_MAX) {
            return UINT32_MAX;
        }
        counter += urlbox_size;
    }

    /* Add the data reference box size */
    // header size (8) + num_reference (2)
    counter += 8 + 2;
    return counter;
}

static OPJ_UINT32 opj_jpx_compute_urlbox_size(const char* filepath)
{
    OPJ_SIZE_T counter = 0;
    char* absolute_path = opj_jpx_get_absolute_path(filepath);
    if (!absolute_path) {
        return UINT32_MAX;
    }

    /* Count the size of a data url box */
    /* header size (8) + version size (1) + flag size (3) */
    counter += 8 + 1 + 3;
    /* + variable string size */
    counter += strlen(absolute_path) + strlen("file://") + 1;

    assert(counter <= UINT32_MAX);
    opj_free(absolute_path);
    return (OPJ_UINT32)counter;
}

static OPJ_BOOL opj_jpx_init_jp2_options(opj_jpx_t* jpx,
        opj_event_mgr_t* p_manager)
{
    opj_stream_t* p_stream = NULL;
    opj_image_t* tmp_img = opj_image_create0();
    if (!tmp_img) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to create temporary image buffer\n", jpx->files[0]);
        return OPJ_FALSE;
    }

    // Read the header options from the first jp2 file
    assert(jpx->file_count > 0);

    p_stream = opj_stream_create_default_file_stream(jpx->files[0], OPJ_TRUE);
    if (!p_stream) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to open %s for reading\n", jpx->files[0]);
        return OPJ_FALSE;
    }
    if (!opj_jp2_read_header((opj_stream_private_t*) p_stream, jpx->jp2, &tmp_img,
                             p_manager)) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Failed to read header from %s\n", jpx->files[0]);
        return OPJ_FALSE;
    }

    /* Replace jp2 header with jpx information */
    /* Need to set brand in ftyp box to "jpx " */
    jpx->jp2->brand = JPX_JPX;
    /* JPX compatibility list should contain "jpx ", "jp2 ", and "jpxb" */
    jpx->jp2->numcl = 3;
    /* Allocate memory for the compatibility list. */
    if (jpx->jp2->cl) {
        opj_free(jpx->jp2->cl);
    }
    jpx->jp2->cl = (OPJ_UINT32*) opj_malloc(jpx->jp2->numcl * sizeof(OPJ_UINT32));
    /* Return failure if we couldn't allocate memory for the cl */
    if (!jpx->jp2->cl) {
        opj_event_msg(p_manager, EVT_ERROR,
                      "Not enough memory when setting up the JPX encoder\n");
        return OPJ_FALSE;
    }
    // Assign the cl values
    jpx->jp2->cl[0] = JPX_JPX;
    jpx->jp2->cl[1] = JP2_JP2;
    jpx->jp2->cl[2] = JPX_JPXB;

    opj_image_destroy(tmp_img);
    return OPJ_TRUE;
}

#ifdef _WIN32
static char* opj_jpx_get_absolute_path(const char* relative_path)
{
    /* Maximum path length for windows */
    DWORD buffer_size = 32767;
    DWORD result = 0;
    char* outbuf = opj_calloc(buffer_size, 1);
    if (!outbuf) {
        return NULL;
    }

    result = GetFullPathName(relative_path, buffer_size, outbuf, NULL);
    /* On failure, GetFullPathName will return either 0 */
    /* Or if the buffer is too small, then it returns the required buffer size. */
    if ((result == 0) || (result > buffer_size)) {
        opj_free(outbuf);
        return NULL;
    }

    return outbuf;
}
#else
static char* opj_jpx_get_absolute_path(const char* relative_path)
{
    char* outbuf = NULL;
    char path_buf[PATH_MAX];
    OPJ_SIZE_T path_length;
    if (realpath(relative_path, path_buf) == NULL) {
        return NULL;
    }

    outbuf = opj_malloc(strlen(path_buf) + 1);
    if (!outbuf) {
        return NULL;
    }

    strcpy(outbuf, path_buf);
    return outbuf;
}
#endif