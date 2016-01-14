/*
 * The copyright in this software is being made available under the 2-clauses 
 * BSD License, included below. This software may be subject to other third 
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2002-2016, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2016, OpenJPEG contributors
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

#ifndef __SEGMENTED_STREAM_H
#define __SEGMENTED_STREAM_H

/*
Smart wrapper to low level C array
*/

typedef struct opj_min_buf {
	OPJ_BYTE *buf;		/* internal array*/
	OPJ_UINT16 len;		/* length of array */
} opj_min_buf_t;

/*
Copy all segments, in sequence, into contiguous array
*/
OPJ_BOOL opj_min_buf_vec_copy_to_contiguous_buffer(opj_vec_t* min_buf_vec, OPJ_BYTE* buffer);

/*
Push buffer to back of min buf vector
*/
OPJ_BOOL opj_min_buf_vec_push_back(opj_vec_t* buf_vec, OPJ_BYTE* buf, OPJ_UINT16 len);

/*
Sum lengths of all buffers
*/

OPJ_UINT16 opj_min_buf_vec_get_len(opj_vec_t* min_buf_vec);


/*
Increment buffer offset
*/
void opj_buf_incr_offset(opj_buf_t* buf, OPJ_OFF_T off);

/*
Free buffer and also its internal array if owns_data is true
*/
void opj_buf_free(opj_buf_t* buf);


/*  Segmented Buffer Interface 

A segmented buffer stores a list of buffers, or segments, but can be treated as one single
contiguous buffer.

*/
typedef struct opj_seg_buf
{
	OPJ_SIZE_T data_len;	/* total length of all segments*/
	OPJ_INT32 cur_seg_id;	/* current index into segments vector */
	opj_vec_t segments;		/* vector of segments */

} opj_seg_buf_t;

/*
Wrap existing array and add to the back of the segmented buffer.
*/
OPJ_BOOL opj_seg_buf_push_back(opj_seg_buf_t* seg_buf, OPJ_BYTE* buf, OPJ_SIZE_T len);

/*
Allocate array and add to the back of the segmented buffer
*/
OPJ_BOOL opj_seg_buf_alloc_and_push_back(opj_seg_buf_t* seg_buf, OPJ_SIZE_T len);

/*
Increment offset of current segment
*/
void opj_seg_buf_incr_cur_seg_offset(opj_seg_buf_t* seg_buf, OPJ_OFF_T offset);

/*
Get length of current segment
*/
OPJ_SIZE_T opj_seg_buf_get_cur_seg_len(opj_seg_buf_t* seg_buf);

/*
Get offset of current segment
*/
OPJ_OFF_T opj_seg_buf_get_cur_seg_offset(opj_seg_buf_t* seg_buf);

/*
Treat segmented buffer as single contiguous buffer, and get current pointer
*/
OPJ_BYTE* opj_seg_buf_get_global_ptr(opj_seg_buf_t* seg_buf);

/*
Treat segmented buffer as single contiguous buffer, and get current offset
*/
OPJ_OFF_T opj_seg_buf_get_global_offset(opj_seg_buf_t* seg_buf);

/*
Reset all offsets to zero, and set current segment to beginning of list
*/
void opj_seg_buf_rewind(opj_seg_buf_t* seg_buf);

/*
Copy all segments, in sequence, into contiguous array
*/
OPJ_BOOL opj_seg_buf_copy_to_contiguous_buffer(opj_seg_buf_t* seg_buf, OPJ_BYTE* buffer);

/*
Cleans up internal resources
*/
void	opj_seg_buf_cleanup(opj_seg_buf_t* seg_buf);

/*
Return current pointer, stored in ptr variable, and advance segmented buffer
offset by chunk_len

*/
OPJ_BOOL opj_seg_buf_zero_copy_read(opj_seg_buf_t* seg_buf,
									OPJ_BYTE** ptr,
									OPJ_SIZE_T chunk_len);



#endif