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

#ifndef  J2K_TO_IDXJP2_H_
# define J2K_TO_IDXJP2_H_

#include "openjpeg.h"

/* 
 * Decode j2k codestream
 *
 * @param[in]  j2kstream j2k codestream
 * @param[in]  j2klen    length of j2k codestream
 * @param[out] cstr_info codestream information
 * @return               image data
 */
opj_image_t * decode_j2k( unsigned char *j2kstream, int j2klen, opj_codestream_info_t *cstr_info);


/* 
 * Write a JP2 file with index box
 *
 * @param[in] filename  file name
 * @param[in] j2kstream j2k codestream
 * @param[in] j2klen    length of j2k codestream
 * @param[in] image     image data
 * @param[in] cstr_info codestream information
 */
void fwrite_idxjp2( char filename[], unsigned char *j2kstream, int j2klen, opj_image_t *image, opj_codestream_info_t cstr_info);

#endif      /* !J2K_TO_IDXJP2S_H_ */
