/*
 * $Id: index_manager.h 53 2011-05-09 16:55:39Z kaori $
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

#ifndef   	INDEX_MANAGER_H_
# define   	INDEX_MANAGER_H_

#include <stdio.h>
#include "bool.h"
#include "byte_manager.h"
#include "faixbox_manager.h"
#include "metadata_manager.h"

//! index parameters
typedef struct index_param{
  metadatalist_param_t *metadatalist; //!< metadata-bin list
  Byte8_t offset;            //!< codestream offset
  Byte8_t length;            //!< codestream length 
  Byte8_t mhead_length;      //!< main header length
  //! A.5.1 Image and tile size (SIZ)
  Byte2_t Rsiz;              //!< capabilities that a decoder needs
  Byte4_t Xsiz;              //!< width of the reference grid
  Byte4_t Ysiz;              //!< height of the reference grid
  Byte4_t XOsiz;             //!< horizontal offset from the origin of
			     //!the reference grid to the left side of the image area
  Byte4_t YOsiz;             //!< vertical offset from the origin of
			     //!the reference grid to the top side of the image area
  Byte4_t XTsiz;             //!< width of one reference tile with
			     //!respect to the reference grid
  Byte4_t YTsiz;             //!< height of one reference tile with
			     //!respect to the reference grid
  Byte4_t XTOsiz;            //!< horizontal offset from the origin of
			     //!the reference grid to the left side of the first tile
  Byte4_t YTOsiz;            //!< vertical offset from the origin of
			     //!the reference grid to the top side of
			     //!the first tile
  Byte4_t XTnum;             //!< number of tiles in horizontal direction
  Byte4_t YTnum;             //!< number of tiles in vertical
			     //!direction
  Byte2_t Csiz;              //!< number of the components in the image

  Byte_t  Ssiz[3];           //!< precision (depth) in bits and sign
			     //!of the component samples
  Byte_t  XRsiz[3];          //!< horizontal separation of a sample of
			     //!component with respect to the reference grid
  Byte_t  YRsiz[3];          //!< vertical separation of a sample of
			     //!component with respect to the reference grid
  faixbox_param_t *tilepart; //!< tile part information from tpix box
  bool mhead_model;          //!< main header model, if sent, 1, else 0
  bool *tp_model;            //!< dynamic array pointer of tile part
			     //!model, if sent, 1, else 0
} index_param_t;


/**
 * parse JP2 file
 * AnnexI: Indexing JPEG2000 files for JPIP
 *
 * @param[in] fd file descriptor of the JP2 file
 * @return       pointer to the generated structure of index parameters
 */
index_param_t * parse_jp2file( int fd);

/**
 * print index parameters
 *
 * @param[in] index index parameters
 */
void print_index( index_param_t index);

/**
 * print cache model
 *
 * @param[in] index index parameters
 */
void print_cachemodel( index_param_t index);


/**
 * delete index
 *
 * @param[in,out] index addressof the index pointer
 */
void delete_index( index_param_t **index);

//! 1-dimensional range parameters
typedef struct range_param{
  Byte4_t minvalue; //!< minimal value
  Byte4_t maxvalue; //!< maximal value
} range_param_t;

/**
 * get horizontal range of the tile in reference grid
 *
 * @param[in] index    index parameters
 * @param[in] tile_xid tile id in x-direction (0<= <XTnum)
 * @param[in] level    decomposition level
 * @return             structured range parameter
 */
range_param_t get_tile_Xrange( index_param_t index, Byte4_t tile_xid, int level);

/**
 * get vertical range of the tile in reference grid
 *
 * @param[in] index    index parameters
 * @param[in] tile_yid tile id in y-direction (0<= <YTnum)
 * @param[in] level    decomposition level
 * @return             structured range parameter
 */
range_param_t get_tile_Yrange( index_param_t index, Byte4_t tile_yid, int level);


#endif 	    /* !INDEX_MANAGER_H_ */
