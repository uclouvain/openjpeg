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

#define JPX_JPX  0x6a707820 /**< JPX ftyp brand */
#define JPX_JPXB 0x6a707862 /**< JPX baseline   */
#define JPX_RREQ 0x72726571 /**< Reader Requirements box */
#define JPX_FTBL 0x6674626c /**< Fragment table box */
#define JPX_FLST 0x666c7374 /**< Fragment list box */
#define JPX_ASOC 0x61736f63 /**< Association box */
#define JPX_ASOC 0x61736f63 /**< Association box */
#define JPX_NLST 0x6e6c7374 /**< Number List box */
#define JPX_URL  0x75726c20 /**< URL Box */
#define JPX_DTBL 0x6474626c /**< Data Reference Box */


/** ----------- Feature Flags ----------- **/
#define RREQ_FLAG_MULTILAYERED 2
#define RREQ_FLAG_REFERENCE_LOCAL_FILES 15


typedef struct opj_jpx {
    /** JPX depends on jp2 codec for writing out header data */
    opj_jp2_t* jp2;
    /** List of files to be embedded/linked in the jpx file */
    const char *const *files;
    /** Number of files to be linked */
    OPJ_UINT32 file_count;
    /** list of execution procedures */
    struct opj_procedure_list * m_procedure_list;
    /** The current file being processed */
    OPJ_UINT16 current_file_index;
} opj_jpx_t;

/**
Encode an image into a JPEG-2000 jpx file
@param jp2      JP2 compressor handle
@param stream    Output buffer stream
@param p_manager  event manager
@return Returns true if successful, returns false otherwise
*/
OPJ_BOOL opj_jpx_encode(opj_jpx_t *jpx,
                        opj_stream_private_t *stream,
                        opj_event_mgr_t * p_manager);

/**
 * Ends the compression procedures.
 */
OPJ_BOOL opj_jpx_end_compress(opj_jpx_t *jpx,
                              opj_stream_private_t *cio,
                              opj_event_mgr_t * p_manager
                             );

/**
 * Starts a compression scheme, i.e. validates the codec parameters, writes the header.
 *
 * @param  jp2    the jpeg2000 file codec.
 * @param  stream    the stream object.
 * @param  p_image   FIXME DOC
 * @param p_manager FIXME DOC
 *
 * @return true if the codec is valid.
 */
OPJ_BOOL opj_jpx_start_compress(opj_jpx_t *jpx,
                                opj_stream_private_t *stream,
                                opj_image_t * p_image,
                                opj_event_mgr_t * p_manager);

/**
 * Setup the encoder parameters using the current image and using user parameters.
 * Coding parameters are returned in jp2->j2k->cp.
 *
 * @param jp2 JP2 compressor handle
 * @param parameters compression parameters
 * @param image input filled image
 * @param p_manager  FIXME DOC
 * @return OPJ_TRUE if successful, OPJ_FALSE otherwise
*/
OPJ_BOOL opj_jpx_setup_encoder(opj_jpx_t *jpx,
                               opj_cparameters_t *parameters,
                               opj_image_t *image,
                               opj_event_mgr_t * p_manager);

/**
 * Specify extra options for the encoder.
 *
 * @param  p_jp2        the jpeg2000 codec.
 * @param  p_options    options
 * @param  p_manager    the user event manager
 *
 * @see opj_encoder_set_extra_options() for more details.
 */
OPJ_BOOL opj_jpx_encoder_set_extra_options(
    opj_jpx_t *p_jp2,
    const char* const* p_options,
    opj_event_mgr_t * p_manager);

/**
 * Does nothing at the moment.
 *
 * @param jp2 JP2 decompressor handle
 * @param num_threads Number of threads.
 * @return OPJ_TRUE in case of success.
 */
OPJ_BOOL opj_jpx_set_threads(opj_jpx_t *jpx, OPJ_UINT32 num_threads);

/**
 * Creates a jpx file compressor.
 *
 * @return  an empty jpeg2000 file codec.
 */
opj_jpx_t* opj_jpx_create(void);

/**
 * Destroy a JPX handle
 * @param jpx JPX handle to destroy
 */
void opj_jpx_destroy(opj_jpx_t *jpx);

/**
 * Writes an RREQ box - Reader Requirements box
 *
 * @param   cio         the stream to write data to.
 * @param   jp2         the jpeg2000 file codec.
 * @param   p_manager   the user event manager.
 *
 * @return  true if writing was successful.
 */
OPJ_BOOL opj_jpx_write_rreq(opj_jp2_t *jp2,
                            opj_stream_private_t *cio,
                            opj_event_mgr_t * p_manager);

/**
 * Writes an RREQ box - Reader Requirements box
 *
 * @param   cio         the stream to write data to.
 * @param   jp2         the jpeg2000 file codec.
 * @param   p_manager   the user event manager.
 *
 * @return  true if writing was successful.
 */
OPJ_BOOL opj_jpx_write_rreq(opj_jp2_t *jp2,
                            opj_stream_private_t *cio,
                            opj_event_mgr_t * p_manager);

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
                            opj_event_mgr_t * p_manager);