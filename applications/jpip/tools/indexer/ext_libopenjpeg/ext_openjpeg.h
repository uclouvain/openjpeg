/*
 * $Id$
 *
 * Copyright (c) 2002-2011, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2011, Professor Benoit Macq
 * Copyright (c) 2010-2011, Kaori Hagihara
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

#ifndef  EXT_OPENJPEG_H_
# define EXT_OPENJPEG_H_

#include "openjpeg.h"

/* 
 * Decode and Add main header marker information 
 *
 * @param[in]     cio       file input handle
 * @param[in,out] cstr_info codestream information
 */
void add_mainheader_marker_info( opj_cio_t *cio, opj_codestream_info_t *cstr_info);

/* 
 * Encode JP2 stream with index box
 *
 * @param[in] cinfo     codestream information
 * @param[in] cio       file output handle
 * @param[in] j2kstream j2k codestream
 * @param[in] j2klen    length of j2k codestream
 * @param[in] cstr_info codestream information
 * @param[in] image     image data
 * @return              true if succeed
 */
opj_bool idxjp2_encode( opj_cinfo_t *cinfo, opj_cio_t *cio, unsigned char *j2kstream, int j2klen, opj_codestream_info_t cstr_info, opj_image_t *image);

#endif      /* !EXT_OPENJPEGS_H_ */
