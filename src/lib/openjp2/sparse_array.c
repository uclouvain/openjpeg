/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2017, IntoPix SA <contact@intopix.com>
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


struct opj_sparse_array_int32 {
    OPJ_UINT32 width;
    OPJ_UINT32 height;
    OPJ_UINT32 block_width;
    OPJ_UINT32 block_height;
    OPJ_UINT32 block_count_hor;
    OPJ_UINT32 block_count_ver;
    OPJ_INT32** data_blocks;
};

opj_sparse_array_int32_t* opj_sparse_array_int32_create(OPJ_UINT32 width,
        OPJ_UINT32 height,
        OPJ_UINT32 block_width,
        OPJ_UINT32 block_height)
{
    opj_sparse_array_int32_t* sa;

    if (width == 0 || height == 0 || block_width == 0 || block_height == 0) {
        return NULL;
    }
    if (block_width > ((OPJ_UINT32)~0U) / block_height / sizeof(OPJ_INT32)) {
        return NULL;
    }

    sa = opj_calloc(1, sizeof(opj_sparse_array_int32_t));
    sa->width = width;
    sa->height = height;
    sa->block_width = block_width;
    sa->block_height = block_height;
    sa->block_count_hor = opj_uint_ceildiv(width, block_width);
    sa->block_count_ver = opj_uint_ceildiv(height, block_height);
    if (sa->block_count_hor > ((OPJ_UINT32)~0U) / sa->block_count_ver) {
        opj_free(sa);
        return NULL;
    }
    sa->data_blocks = opj_calloc(sizeof(OPJ_INT32*),
                                 sa->block_count_hor * sa->block_count_ver);
    if (sa->data_blocks == NULL) {
        opj_free(sa);
        return NULL;
    }

    return sa;
}

void opj_sparse_array_int32_free(opj_sparse_array_int32_t* sa)
{
    if (sa) {
        OPJ_UINT32 i;
        for (i = 0; i < sa->block_count_hor * sa->block_count_ver; i++) {
            if (sa->data_blocks[i]) {
                opj_free(sa->data_blocks[i]);
            }
        }
        opj_free(sa->data_blocks);
        opj_free(sa);
    }
}

OPJ_BOOL opj_sparse_array_is_region_valid(opj_sparse_array_int32_t* sa,
        OPJ_UINT32 x0,
        OPJ_UINT32 y0,
        OPJ_UINT32 x1,
        OPJ_UINT32 y1)
{
    return !(x0 >= sa->width || x1 <= x0 || x1 > sa->width ||
             y0 >= sa->height || y1 <= y0 || y1 > sa->height);
}

static OPJ_BOOL opj_sparse_array_int32_read_or_write(
    opj_sparse_array_int32_t* sa,
    OPJ_UINT32 x0,
    OPJ_UINT32 y0,
    OPJ_UINT32 x1,
    OPJ_UINT32 y1,
    OPJ_INT32* buf,
    OPJ_UINT32 buf_col_stride,
    OPJ_UINT32 buf_line_stride,
    OPJ_BOOL forgiving,
    OPJ_BOOL is_read_op)
{
    OPJ_UINT32 y, block_y;
    OPJ_UINT32 y_incr = 0;
    if (!opj_sparse_array_is_region_valid(sa, x0, y0, x1, y1)) {
        return forgiving;
    }

    block_y = y0 / sa->block_height;
    for (y = y0; y < y1; block_y ++, y += y_incr) {
        OPJ_UINT32 x, block_x;
        OPJ_UINT32 x_incr = 0;
        OPJ_UINT32 block_y_offset;
        y_incr = (y == y0) ? sa->block_height - (y0 % sa->block_height) :
                 sa->block_height;
        block_y_offset = sa->block_height - y_incr;
        y_incr = opj_uint_min(y_incr, y1 - y);
        block_x = x0 / sa->block_width;
        for (x = x0; x < x1; block_x ++, x += x_incr) {
            OPJ_UINT32 j;
            OPJ_UINT32 block_x_offset;
            OPJ_INT32* src_block;
            x_incr = (x == x0) ? sa->block_width - (x0 % sa->block_width) : sa->block_width;
            block_x_offset = sa->block_width - x_incr;
            x_incr = opj_uint_min(x_incr, x1 - x);
            src_block = sa->data_blocks[block_y * sa->block_count_hor + block_x];
            if (is_read_op) {
                if (src_block == NULL) {
                    for (j = 0; j < y_incr; j++) {
                        if (buf_col_stride == 1) {
                            memset(buf + (y - y0 + j) * buf_line_stride + (x - x0) * buf_col_stride,
                                   0,
                                   sizeof(OPJ_INT32) * x_incr);
                        } else {
                            OPJ_UINT32 k;
                            for (k = 0; k < x_incr; k++) {
                                *(buf + (y - y0 + j) * buf_line_stride + (x - x0 + k) * buf_col_stride) = 0;
                            }
                        }
                    }
                } else {
                    for (j = 0; j < y_incr; j++) {
                        if (buf_col_stride == 1) {
                            memcpy(buf + (y - y0 + j) * buf_line_stride + (x - x0) * buf_col_stride,
                                   src_block + (block_y_offset + j) * sa->block_width + block_x_offset,
                                   sizeof(OPJ_INT32) * x_incr);
                        } else {
                            OPJ_UINT32 k;
                            for (k = 0; k < x_incr; k++) {
                                *(buf + (y - y0 + j) * buf_line_stride + (x - x0 + k) * buf_col_stride) =
                                    *(src_block + (block_y_offset + j) * sa->block_width + block_x_offset + k);
                            }
                        }
                    }
                }
            } else {
                if (src_block == NULL) {
                    src_block = opj_calloc(1,
                                           sa->block_width * sa->block_height * sizeof(OPJ_INT32));
                    if (src_block == NULL) {
                        return OPJ_FALSE;
                    }
                    sa->data_blocks[block_y * sa->block_count_hor + block_x] = src_block;
                }

                for (j = 0; j < y_incr; j++) {
                    if (buf_col_stride == 1) {
                        memcpy(src_block + (block_y_offset + j) * sa->block_width + block_x_offset,
                               buf + (y - y0 + j) * buf_line_stride + (x - x0) * buf_col_stride,
                               sizeof(OPJ_INT32) * x_incr);
                    } else {
                        OPJ_UINT32 k;
                        for (k = 0; k < x_incr; k++) {
                            *(src_block + (block_y_offset + j) * sa->block_width + block_x_offset + k) =
                                *(buf + (y - y0 + j) * buf_line_stride + (x - x0 + k) * buf_col_stride);
                        }
                    }
                }
            }
        }
    }

    return OPJ_TRUE;
}

OPJ_BOOL opj_sparse_array_int32_read(opj_sparse_array_int32_t* sa,
                                     OPJ_UINT32 x0,
                                     OPJ_UINT32 y0,
                                     OPJ_UINT32 x1,
                                     OPJ_UINT32 y1,
                                     OPJ_INT32* dest,
                                     OPJ_UINT32 dest_col_stride,
                                     OPJ_UINT32 dest_line_stride,
                                     OPJ_BOOL forgiving)
{
    return opj_sparse_array_int32_read_or_write(sa, x0, y0, x1, y1,
            dest,
            dest_col_stride,
            dest_line_stride,
            forgiving,
            OPJ_TRUE);
}

OPJ_BOOL opj_sparse_array_int32_write(opj_sparse_array_int32_t* sa,
                                      OPJ_UINT32 x0,
                                      OPJ_UINT32 y0,
                                      OPJ_UINT32 x1,
                                      OPJ_UINT32 y1,
                                      const OPJ_INT32* src,
                                      OPJ_UINT32 src_col_stride,
                                      OPJ_UINT32 src_line_stride,
                                      OPJ_BOOL forgiving)
{
    return opj_sparse_array_int32_read_or_write(sa, x0, y0, x1, y1,
            (OPJ_INT32*)src,
            src_col_stride,
            src_line_stride,
            forgiving,
            OPJ_FALSE);
}
