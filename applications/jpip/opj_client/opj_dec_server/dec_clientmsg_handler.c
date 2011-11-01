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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dec_clientmsg_handler.h"
#include "ihdrbox_manager.h"
#include "jpipstream_manager.h"
#include "jp2k_encoder.h"


//! maximum length of channel identifier
#define MAX_LENOFCID 30

/**
 * handle JPT- JPP- stream message
 *
 * @param[in]     connected_socket socket descriptor
 * @param[in]     cachelist        cache list pointer
 * @param[in,out] jpipstream       address of JPT- JPP- stream pointer
 * @param[in,out] streamlen        address of stream length
 * @param[in,out] msgqueue         message queue pointer
 */
void handle_JPIPstreamMSG( SOCKET connected_socket, cachelist_param_t *cachelist, Byte_t **jpipstream, int *streamlen, msgqueue_param_t *msgqueue);

/**
 * handle PNM request message
 *
 * @param[in] connected_socket socket descriptor
 * @param[in] jpipstream       jpipstream pointer
 * @param[in] msgqueue         message queue pointer
 * @param[in] cachelist        cache list pointer
 */
void handle_PNMreqMSG( SOCKET connected_socket, Byte_t *jpipstream, msgqueue_param_t *msgqueue, cachelist_param_t *cachelist);

/**
 * handle XML request message
 *
 * @param[in] connected_socket socket descriptor
 * @param[in] jpipstream       address of caching jpipstream pointer
 * @param[in] cachelist        cache list pointer
 */
void handle_XMLreqMSG( SOCKET connected_socket, Byte_t *jpipstream, cachelist_param_t *cachelist);

/**
 * handle TargetID request message
 *
 * @param[in] connected_socket socket descriptor
 * @param[in] cachelist        cache list pointer
 */
void handle_TIDreqMSG( SOCKET connected_socket, cachelist_param_t *cachelist);

/**
 * handle ChannelID request message
 *
 * @param[in] connected_socket socket descriptor
 * @param[in] cachelist        cache list pointer
 */
void handle_CIDreqMSG( SOCKET connected_socket, cachelist_param_t *cachelist);

/**
 * handle distroy ChannelID message
 *
 * @param[in]     connected_socket socket descriptor
 * @param[in,out] cachelist        cache list pointer
 */
void handle_dstCIDreqMSG( SOCKET connected_socket, cachelist_param_t *cachelist);

/**
 * handle saving JP2 file request message
 *
 * @param[in] connected_socket socket descriptor
 * @param[in] cachelist        cache list pointer
 * @param[in] msgqueue         message queue pointer
 * @param[in] jpipstream       address of caching jpipstream pointer
 */
void handle_JP2saveMSG( SOCKET connected_socket, cachelist_param_t *cachelist, msgqueue_param_t *msgqueue, Byte_t *jpipstream);

bool handle_clientmsg( SOCKET connected_socket, cachelist_param_t *cachelist, Byte_t **jpipstream, int *streamlen, msgqueue_param_t *msgqueue)
{
  bool quit = false;
  msgtype_t msgtype = identify_clientmsg( connected_socket);
  
  switch( msgtype){
  case JPIPSTREAM:
    handle_JPIPstreamMSG( connected_socket, cachelist, jpipstream, streamlen, msgqueue);
    break;
      
  case PNMREQ:
    handle_PNMreqMSG( connected_socket, *jpipstream, msgqueue, cachelist);
    break;
    
  case XMLREQ:
    handle_XMLreqMSG( connected_socket, *jpipstream, cachelist);
    break;

  case TIDREQ:
    handle_TIDreqMSG( connected_socket, cachelist);
    break;
						
  case CIDREQ:
    handle_CIDreqMSG( connected_socket, cachelist);
    break;

  case CIDDST:
    handle_dstCIDreqMSG( connected_socket, cachelist);
    break;

  case JP2SAVE:
    handle_JP2saveMSG( connected_socket, cachelist, msgqueue, *jpipstream);
    break;

  case QUIT:
    quit = true;
    save_codestream( *jpipstream, *streamlen, "jpt");
    break;
  case MSGERROR:
    break;
  }
        
  printf("\t end of the connection\n\n");
  if( close_socket(connected_socket) != 0){
    perror("close");
    return false;
  }
  if( quit)
    return false;

  return true;
}


void handle_JPIPstreamMSG( SOCKET connected_socket, cachelist_param_t *cachelist,
			   Byte_t **jpipstream, int *streamlen, msgqueue_param_t *msgqueue)
{
  Byte_t *newjpipstream;
  int newstreamlen = 0;
  cache_param_t *cache;
  char target[MAX_LENOFTARGET], tid[MAX_LENOFTID], cid[MAX_LENOFCID];
  metadatalist_param_t *metadatalist;
  
  newjpipstream = receive_JPIPstream( connected_socket, target, tid, cid, &newstreamlen);

  parse_JPIPstream( newjpipstream, newstreamlen, *streamlen, msgqueue);

  *jpipstream = update_JPIPstream( newjpipstream, newstreamlen, *jpipstream, streamlen);
  free( newjpipstream);

  metadatalist = gene_metadatalist();
  parse_metamsg( msgqueue, *jpipstream, *streamlen, metadatalist);

  // cid registration
  if( target[0] != 0){
    if((cache = search_cache( target, cachelist))){
      if( tid[0] != 0)
	update_cachetid( tid, cache);
      if( cid[0] != 0)
	add_cachecid( cid, cache);
    }
    else{
      cache = gene_cache( target, msgqueue->last->csn, tid, cid);
      insert_cache_into_list( cache, cachelist);
    }
  }
  else
    cache = search_cacheBycsn( msgqueue->last->csn, cachelist);

  if( cache->metadatalist)
    delete_metadatalist( &cache->metadatalist);
  cache->metadatalist = metadatalist;

  response_signal( connected_socket, true);
}

void handle_PNMreqMSG( SOCKET connected_socket, Byte_t *jpipstream, msgqueue_param_t *msgqueue, cachelist_param_t *cachelist)
{
  Byte_t *pnmstream;
  ihdrbox_param_t *ihdrbox;
  char cid[MAX_LENOFCID], tmp[10];
  cache_param_t *cache;
  int fw, fh;

  receive_line( connected_socket, cid);
  if(!(cache = search_cacheBycid( cid, cachelist)))
    if(!(cache = search_cacheBytid( cid, cachelist)))
      return;

  receive_line( connected_socket, tmp);
  fw = atoi( tmp);
  
  receive_line( connected_socket, tmp);
  fh = atoi( tmp);

  pnmstream = jpipstream_to_pnm( jpipstream, msgqueue, cache->csn, fw, fh, &cache->ihdrbox);
  ihdrbox = cache->ihdrbox;

  send_PNMstream( connected_socket, pnmstream, ihdrbox->width, ihdrbox->height, ihdrbox->nc, ihdrbox->bpc > 8 ? 255 : (1 << ihdrbox->bpc) - 1);

  free( pnmstream);
}

void handle_XMLreqMSG( SOCKET connected_socket, Byte_t *jpipstream, cachelist_param_t *cachelist)
{
  char cid[MAX_LENOFCID];
  cache_param_t *cache;

  receive_line( connected_socket, cid);
  if(!(cache = search_cacheBycid( cid, cachelist)))
    return;
  
  boxcontents_param_t *boxcontents = cache->metadatalist->last->boxcontents;
  Byte_t *xmlstream = (Byte_t *)malloc( boxcontents->length);
  memcpy( xmlstream, jpipstream+boxcontents->offset, boxcontents->length);
  send_XMLstream( connected_socket, xmlstream, boxcontents->length);
  free( xmlstream);
}

void handle_TIDreqMSG( SOCKET connected_socket, cachelist_param_t *cachelist)
{
  char target[MAX_LENOFTARGET], *tid = NULL;
  cache_param_t *cache;
  int tidlen = 0;

  receive_line( connected_socket, target);
  cache = search_cache( target, cachelist);
  
  if( cache){
    tid = cache->tid;
    tidlen = strlen(tid);
  }
  send_TIDstream( connected_socket, tid, tidlen);
}

void handle_CIDreqMSG( SOCKET connected_socket, cachelist_param_t *cachelist)
{
  char target[MAX_LENOFTARGET], *cid = NULL;
  cache_param_t *cache;
  int cidlen = 0;

  receive_line( connected_socket, target);
  cache = search_cache( target, cachelist);
  
  if( cache){
    if( cache->numOfcid > 0){
      cid = cache->cid[ cache->numOfcid-1];
      cidlen = strlen(cid);
    }
  }
  send_CIDstream( connected_socket, cid, cidlen);
}

void handle_dstCIDreqMSG( SOCKET connected_socket, cachelist_param_t *cachelist)
{
  char cid[MAX_LENOFCID];

  receive_line( connected_socket, cid);
  remove_cachecid( cid, cachelist);
  response_signal( connected_socket, true);
}

void handle_JP2saveMSG( SOCKET connected_socket, cachelist_param_t *cachelist, msgqueue_param_t *msgqueue, Byte_t *jpipstream)
{
  char cid[MAX_LENOFCID];
  cache_param_t *cache;
  Byte_t *jp2stream;
  Byte8_t jp2len;

  receive_line( connected_socket, cid);
  if(!(cache = search_cacheBycid( cid, cachelist)))
    return;
  
  jp2stream = recons_jp2( msgqueue, jpipstream, cache->csn, &jp2len);

  if( jp2stream){
    save_codestream( jp2stream, jp2len, "jp2");
    free( jp2stream);
  }
}
