/*
 * $Id: msgqueue_manager.h 53 2011-05-09 16:55:39Z kaori $
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

#ifndef   	MSGQUEUE_MANAGER_H_
# define   	MSGQUEUE_MANAGER_H_

#include <stdio.h>
#include "bool.h"
#include "byte_manager.h"
#include "target_manager.h"
#include "placeholder_manager.h"

//! message parameters
typedef struct message_param{
  bool    last_byte;          //!< if message contains the last byte of the data-bin
  Byte8_t in_class_id;        //!< in-class identifier A.2.3
  Byte8_t class_id;           //!< class identifiers 
  Byte8_t csn;                //!< index of the codestream
  Byte8_t bin_offset;         //!< offset of the data in this message from the start of the data-bin
  Byte8_t length;             //!< message byte length
  Byte8_t aux;                //!<
  Byte8_t res_offset;         //!< offset in the resource
  placeholder_param_t *phld;  //!< placeholder pointer in index
  struct message_param *next; //!< pointer to the next message
} message_param_t;

//! message queue parameters
typedef struct msgqueue_param{
  message_param_t *first; //!< first message pointer of the list
  message_param_t *last;  //!< last  message pointer of the list
  bool stateless;         //!< if this is a stateless message queue
  target_param_t *target; //!< reference target pointer
} msgqueue_param_t;

/**
 * generate message queue
 *
 * @param[in] stateless if this is a stateless message queue
 * @param[in] target    reference target pointer
 * @return generated message queue pointer
 */
msgqueue_param_t * gene_msgqueue( bool stateless, target_param_t *target);

/**
 * delete message queue
 *
 * @param[in] msgqueue address of the message queue pointer
 */
void delete_msgqueue( msgqueue_param_t **msgqueue);

/**
 * print message queue
 *
 * @param[in] msgqueue message queue pointer
 */
void print_msgqueue( msgqueue_param_t *msgqueue);


/**
 * enqueue main header data-bin into message queue
 *
 * @param[in,out] msgqueue message queue pointer
 */
void enqueue_mainheader( msgqueue_param_t *msgqueue);


/**
 * enqueue tile data-bin into message queue
 *
 * @param[in]     tile_id  tile id starting from 0
 * @param[in]     level    decomposition level
 * @param[in,out] msgqueue message queue pointer
 */
void enqueue_tile( int tile_id, int level, msgqueue_param_t *msgqueue);


/**
 * enqueue Metadata-bin into message queue
 *
 * @param[in]     meta_id  metadata-bin id
 * @param[in,out] msgqueue message queue pointer
 */
void enqueue_metadata( int meta_id, msgqueue_param_t *msgqueue);


/**
 * emit stream from message queue
 *
 * @param[in] msgqueue message queue pointer
 */
void emit_stream_from_msgqueue( msgqueue_param_t *msgqueue);


/**
 * parse JPT-stream to message queue
 *
 * @param[in]     stream       JPT-stream data pointer
 * @param[in]     jptlen       JPT-stream length
 * @param[in]     offset       offset of the JPT-stream from the whole beginning
 * @param[in,out] msgqueue     adding message queue pointer
 */
void parse_stream( Byte_t *stream, Byte8_t jptlen, Byte8_t offset, msgqueue_param_t *msgqueue);

/**
 * parse JPT-stream to message queue
 *
 * @param[in] msgqueue     reference message queue pointer
 * @param[in] stream       JPT-stream data pointer
 * @param[in] jptlen       JPT-stream length
 * @param[in] metadatalist adding metadata list pointer
 */
void parse_metamsg( msgqueue_param_t *msgqueue, Byte_t *stream, Byte8_t jptlen, metadatalist_param_t *metadatalist);


/**
 * reconstruct j2k codestream from message queue
 *
 * @param[in]  msgqueue  message queue pointer
 * @param[in]  stream    original stream
 * @param[in]  csn       codestream number
 * @param[in]  minlev    minimum decomposition level
 * @param[out] j2klen    pointer to the j2k codestream length
 * @return     generated reconstructed j2k codestream
 */
Byte_t * recons_j2k( msgqueue_param_t *msgqueue, Byte_t *stream, Byte8_t csn, int minlev, Byte8_t *j2klen);


/**
 * reconstruct jp2 file codestream from message queue
 *
 * @param[in]  msgqueue  message queue pointer
 * @param[in]  stream    original stream
 * @param[in]  csn       codestream number
 * @param[out] jp2len    pointer to the jp2 codestream length
 * @return     generated reconstructed jp2 codestream
 */
Byte_t * recons_jp2( msgqueue_param_t *msgqueue, Byte_t *stream, Byte8_t csn, Byte8_t *jp2len);


#endif 	    /* !MSGQUEUE_MANAGER_H_ */
