/*
* Copyright (c) 2016, Aaron Boxer
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
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