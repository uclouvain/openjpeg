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

#include "opj_includes.h"

/* #define DEBUG_SEG_BUF */


OPJ_BOOL opj_min_buf_vec_copy_to_contiguous_buffer(opj_vec_t* min_buf_vec, OPJ_BYTE* buffer) {
	OPJ_INT32 i = 0;
	OPJ_SIZE_T offset = 0;

	if (!buffer || !min_buf_vec)
		return OPJ_FALSE;

	for (i = 0; i < min_buf_vec->size; ++i) {
		opj_min_buf_t* seg = (opj_min_buf_t*)opj_vec_get(min_buf_vec, i);
		if (seg->len)
			memcpy(buffer + offset, seg->buf, seg->len);
		offset += seg->len;
	}
	return OPJ_TRUE;

}

OPJ_BOOL opj_min_buf_vec_push_back(opj_vec_t* buf_vec, OPJ_BYTE* buf, OPJ_UINT16 len) {
	opj_min_buf_t* seg = NULL;
	if (!buf_vec || !buf || !len)
		return OPJ_FALSE;

	if (!buf_vec->data) {
		opj_vec_init(buf_vec,OPJ_TRUE);
	}

	seg = (opj_min_buf_t*)opj_malloc(sizeof(opj_buf_t));
	if (!seg)
		return OPJ_FALSE;

	seg->buf = buf;
	seg->len = len;
	if (!opj_vec_push_back(buf_vec, seg)) {
		opj_free(seg);
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}

OPJ_UINT16 opj_min_buf_vec_get_len(opj_vec_t* min_buf_vec) {
	OPJ_INT32 i = 0;
	OPJ_UINT16 len = 0;
	if (!min_buf_vec || !min_buf_vec->data)
		return 0;
	for (i = 0; i < min_buf_vec->size; ++i) {
		opj_min_buf_t* seg = (opj_min_buf_t*)opj_vec_get(min_buf_vec, i);
		if (seg)
			len += seg->len;
	}
	return len;

}



/*--------------------------------------------------------------------------*/

/*  Segmented Buffer Stream */

static void opj_seg_buf_increment(opj_seg_buf_t * seg_buf)
{
	opj_buf_t* cur_seg = NULL;
	if (seg_buf == NULL ||	seg_buf->cur_seg_id == seg_buf->segments.size-1) {
		return;
	}

	cur_seg = (opj_buf_t*)opj_vec_get(&seg_buf->segments, seg_buf->cur_seg_id);
	if ((OPJ_SIZE_T)cur_seg->offset == cur_seg->len  && 
					seg_buf->cur_seg_id < seg_buf->segments.size -1)	{
		seg_buf->cur_seg_id++;
	}
}

static OPJ_SIZE_T opj_seg_buf_read(opj_seg_buf_t * seg_buf,
									void * p_buffer,
									OPJ_SIZE_T p_nb_bytes)
{
	OPJ_SIZE_T bytes_in_current_segment;
	OPJ_SIZE_T bytes_to_read;
	OPJ_SIZE_T total_bytes_read;
	OPJ_SIZE_T bytes_left_to_read;
	OPJ_SIZE_T bytes_remaining_in_file;

	if (p_buffer == NULL || p_nb_bytes == 0 || seg_buf == NULL)
		return 0;

	/*don't try to read more bytes than are available */
	bytes_remaining_in_file = seg_buf->data_len - (OPJ_SIZE_T)opj_seg_buf_get_global_offset(seg_buf);
	if (p_nb_bytes > bytes_remaining_in_file) {
#ifdef DEBUG_SEG_BUF
		printf("Warning: attempt to read past end of segmented buffer\n");
#endif
		p_nb_bytes = bytes_remaining_in_file;
	}

	total_bytes_read = 0;
	bytes_left_to_read = p_nb_bytes;
	while (bytes_left_to_read > 0 && seg_buf->cur_seg_id < seg_buf->segments.size) {
		opj_buf_t* cur_seg = (opj_buf_t*)opj_vec_get(&seg_buf->segments, seg_buf->cur_seg_id);
		bytes_in_current_segment = (cur_seg->len - (OPJ_SIZE_T)cur_seg->offset);

		bytes_to_read = (bytes_left_to_read < bytes_in_current_segment) ? 
											bytes_left_to_read : bytes_in_current_segment;

		if (p_buffer) {
			memcpy((OPJ_BYTE*)p_buffer + total_bytes_read,cur_seg->buf + cur_seg->offset, bytes_to_read);
		}
		opj_seg_buf_incr_cur_seg_offset(seg_buf,(OPJ_OFF_T)bytes_to_read);

		total_bytes_read	+= bytes_to_read;
		bytes_left_to_read	-= bytes_to_read;


	}
	return total_bytes_read ? total_bytes_read : (OPJ_SIZE_T)-1;
}

/* Disable this method for now, since it is not needed at the moment */
#if 0
static OPJ_OFF_T opj_seg_buf_skip(OPJ_OFF_T p_nb_bytes, opj_seg_buf_t * seg_buf)
{
	OPJ_SIZE_T bytes_in_current_segment;
	OPJ_SIZE_T bytes_remaining;

	if (!seg_buf)
		return p_nb_bytes;

	if (p_nb_bytes + opj_seg_buf_get_global_offset(seg_buf)> (OPJ_OFF_T)seg_buf->data_len) {
#ifdef DEBUG_SEG_BUF		
		printf("Warning: attempt to skip past end of segmented buffer\n");
#endif
		return p_nb_bytes;
	}

	if (p_nb_bytes == 0)
		return 0;

	bytes_remaining = (OPJ_SIZE_T)p_nb_bytes;
	while (seg_buf->cur_seg_id < seg_buf->segments.size && bytes_remaining > 0) {

		opj_buf_t* cur_seg = (opj_buf_t*)opj_vec_get(&seg_buf->segments, seg_buf->cur_seg_id);
		bytes_in_current_segment = 	(OPJ_SIZE_T)(cur_seg->len -cur_seg->offset);

		/* hoover up all the bytes in this segment, and move to the next one */
		if (bytes_in_current_segment > bytes_remaining) {

			opj_seg_buf_incr_cur_seg_offset(seg_buf, bytes_in_current_segment);

			bytes_remaining	-= bytes_in_current_segment;
			cur_seg = (opj_buf_t*)opj_vec_get(&seg_buf->segments, seg_buf->cur_seg_id);
		}
		else /* bingo! we found the segment */
		{
			opj_seg_buf_incr_cur_seg_offset(seg_buf, bytes_remaining);
			return p_nb_bytes;
		}
	}
	return p_nb_bytes;
}
#endif
static OPJ_BOOL opj_seg_buf_add_segment(opj_seg_buf_t* seg_buf, OPJ_BYTE* buf, OPJ_SIZE_T len) {
	opj_buf_t* new_seg = NULL;
	if (!seg_buf)
		return OPJ_FALSE;
	new_seg = (opj_buf_t*)opj_malloc(sizeof(opj_buf_t));
	if (!new_seg)
		return OPJ_FALSE;

	memset(new_seg, 0, sizeof(opj_buf_t));
	new_seg->buf = buf;
	new_seg->len = len;
	if (!opj_vec_push_back(&seg_buf->segments, new_seg)) {
		opj_free(new_seg);
		return OPJ_FALSE;
	}

	seg_buf->cur_seg_id = seg_buf->segments.size - 1;
	seg_buf->data_len += len;
	return OPJ_TRUE;
}

/*--------------------------------------------------------------------------*/

void opj_seg_buf_cleanup(opj_seg_buf_t* seg_buf) {
	OPJ_INT32 i;
	if (!seg_buf)
		return;
	for (i = 0; i < seg_buf->segments.size; ++i) {
		opj_buf_t* seg = (opj_buf_t*)opj_vec_get(&seg_buf->segments, i);
		if (seg) {
			opj_buf_free(seg);
		}
	}
	opj_vec_cleanup(&seg_buf->segments);
}

void opj_seg_buf_rewind(opj_seg_buf_t* seg_buf) {
	OPJ_INT32 i;
	if (!seg_buf)
		return;
	for (i = 0; i < seg_buf->segments.size; ++i) {
		opj_buf_t* seg = (opj_buf_t*)opj_vec_get(&seg_buf->segments, i);
		if (seg) {
			seg->offset = 0;
		}
	}
	seg_buf->cur_seg_id = 0;
}


OPJ_BOOL opj_seg_buf_push_back(opj_seg_buf_t* seg_buf, OPJ_BYTE* buf, OPJ_SIZE_T len) {
	opj_buf_t* seg = NULL;
	if (!seg_buf || !buf || !len)
		return OPJ_FALSE;

	if (!seg_buf->segments.data) {
		/* let segmented buffer free the segment and its internal buffer,
		so don't make vector manage memory  */
		opj_vec_init(&seg_buf->segments, OPJ_FALSE);
	}


	if (!opj_seg_buf_add_segment(seg_buf, buf, len))
		return OPJ_FALSE;

	seg = (opj_buf_t*)opj_vec_back(&seg_buf->segments);
	seg->owns_data = OPJ_FALSE;
	return OPJ_TRUE;
}

OPJ_BOOL opj_seg_buf_alloc_and_push_back(opj_seg_buf_t* seg_buf, OPJ_SIZE_T len) {
	opj_buf_t* seg = NULL;
	OPJ_BYTE* buf = NULL;
	if (!seg_buf || !len)
		return OPJ_FALSE;

	if (!seg_buf->segments.data) {
		/* we want to free the segment and its internal buffer, so don't make vector manage memory*/
		opj_vec_init(&seg_buf->segments, OPJ_FALSE);
	}


	buf = (OPJ_BYTE*)opj_malloc(len);
	if (!buf)
		return OPJ_FALSE;

	if (!opj_seg_buf_add_segment(seg_buf, buf, len)) {
		opj_free(buf);
		return OPJ_FALSE;
	}

	seg = (opj_buf_t*)opj_vec_back(&seg_buf->segments);
	seg->owns_data = OPJ_TRUE;

	return OPJ_TRUE;
}

void opj_seg_buf_incr_cur_seg_offset(opj_seg_buf_t* seg_buf, OPJ_OFF_T offset) {
	opj_buf_t* cur_seg = NULL;
	if (!seg_buf)
		return;
	cur_seg = (opj_buf_t*)(seg_buf->segments.data[seg_buf->cur_seg_id]);
	opj_buf_incr_offset(cur_seg, offset);
	if ((OPJ_SIZE_T)cur_seg->offset == cur_seg->len) {
		opj_seg_buf_increment(seg_buf);
	}

}


/**
* Zero copy read of contiguous chunk from current segment.
* Returns OPJ_FALSE if unable to get a contiguous chunk, OPJ_TRUE otherwise
*/
OPJ_BOOL opj_seg_buf_zero_copy_read(opj_seg_buf_t* seg_buf,
										OPJ_BYTE** ptr,
										OPJ_SIZE_T chunk_len) {
	opj_buf_t* cur_seg = NULL;
	if (!seg_buf)
		return OPJ_FALSE;
	cur_seg = (opj_buf_t*)opj_vec_get(&seg_buf->segments, seg_buf->cur_seg_id);
	if (!cur_seg)
		return OPJ_FALSE;

	if ((OPJ_SIZE_T)cur_seg->offset + chunk_len <= cur_seg->len) {
		*ptr = cur_seg->buf + cur_seg->offset;
		opj_seg_buf_read(seg_buf, NULL, chunk_len);
		return OPJ_TRUE;
	}
	return OPJ_FALSE;
}

OPJ_BOOL opj_seg_buf_copy_to_contiguous_buffer(opj_seg_buf_t* seg_buf, OPJ_BYTE* buffer) {
	OPJ_INT32 i = 0;
	OPJ_SIZE_T offset = 0;

	if (!buffer || !seg_buf)
		return OPJ_FALSE;

	for (i = 0; i < seg_buf->segments.size; ++i) {
		opj_buf_t* seg = (opj_buf_t*)opj_vec_get(&seg_buf->segments, i);
		if (seg->len)
			memcpy(buffer + offset, seg->buf, seg->len);
		offset += seg->len;
	}
	return OPJ_TRUE;

}

OPJ_BYTE* opj_seg_buf_get_global_ptr(opj_seg_buf_t* seg_buf) {
	opj_buf_t* cur_seg = NULL;
	if (!seg_buf)
		return NULL;
	cur_seg = (opj_buf_t*)(seg_buf->segments.data[seg_buf->cur_seg_id]);
	return (cur_seg) ? (cur_seg->buf + cur_seg->offset) : NULL;
}

OPJ_SIZE_T opj_seg_buf_get_cur_seg_len(opj_seg_buf_t* seg_buf) {
	opj_buf_t* cur_seg = NULL;
	if (!seg_buf)
		return 0;
	cur_seg = (opj_buf_t*)(opj_vec_get(&seg_buf->segments, seg_buf->cur_seg_id));
	return (cur_seg) ? (cur_seg->len - (OPJ_SIZE_T)cur_seg->offset) : 0;
}

OPJ_OFF_T opj_seg_buf_get_cur_seg_offset(opj_seg_buf_t* seg_buf) {
	opj_buf_t* cur_seg = NULL;
	if (!seg_buf)
		return 0;
	cur_seg = (opj_buf_t*)(opj_vec_get(&seg_buf->segments, seg_buf->cur_seg_id));
	return (cur_seg) ? (OPJ_OFF_T)(cur_seg->offset) : 0;
}


OPJ_OFF_T opj_seg_buf_get_global_offset(opj_seg_buf_t* seg_buf) {
	OPJ_INT32 i = 0;
	OPJ_OFF_T offset = 0;

	if (!seg_buf)
		return 0;

	for (i = 0; i < seg_buf->cur_seg_id; ++i) {
		opj_buf_t* seg = (opj_buf_t*)opj_vec_get(&seg_buf->segments, i);
		offset += (OPJ_OFF_T)seg->len;
	}
	return offset + opj_seg_buf_get_cur_seg_offset(seg_buf);
}


void opj_buf_incr_offset(opj_buf_t* buf, OPJ_OFF_T off) {
	if (!buf)
		return;
	/*  we allow the offset to move to one location beyond end of buffer segment*/
	if (buf->offset + off > (OPJ_OFF_T)buf->len) {
#ifdef DEBUG_SEG_BUF
		printf("Warning: attempt to increment buffer offset out of bounds\n");
#endif
		buf->offset = (OPJ_OFF_T)buf->len;
	}
	buf->offset += off;
}

void opj_buf_free(opj_buf_t* buffer) {
	if (!buffer)
		return;
	if (buffer->buf && buffer->owns_data)
		opj_free(buffer->buf);
	opj_free(buffer);
}

