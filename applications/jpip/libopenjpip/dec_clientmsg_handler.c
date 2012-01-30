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
#include "dec_clientmsg_handler.h"
#include "ihdrbox_manager.h"
#include "jpipstream_manager.h"
#include "jp2k_encoder.h"

void handle_JPIPstreamMSG( SOCKET connected_socket, cachelist_param_t *cachelist,
			   Byte_t **jpipstream, int *streamlen, msgqueue_param_t *msgqueue)
{
  Byte_t *newjpipstream;
  int newstreamlen = 0;
  cache_param_t *cache;
  char *target, *tid, *cid;
  metadatalist_param_t *metadatalist;
  
  newjpipstream = receive_JPIPstream( connected_socket, &target, &tid, &cid, &newstreamlen);

  fprintf( stderr, "newjpipstream length: %d\n", newstreamlen);
  
  parse_JPIPstream( newjpipstream, newstreamlen, *streamlen, msgqueue);

  *jpipstream = update_JPIPstream( newjpipstream, newstreamlen, *jpipstream, streamlen);
  free( newjpipstream);

  metadatalist = gene_metadatalist();
  parse_metamsg( msgqueue, *jpipstream, *streamlen, metadatalist);

  /* cid registration*/
  if( target != NULL){
    if((cache = search_cache( target, cachelist))){
      if( tid != NULL)
	update_cachetid( tid, cache);
      if( cid != NULL)
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

  if( target)    free( target);
  if( tid)    free( tid);
  if( cid)    free( cid);

  response_signal( connected_socket, true);
}

void handle_PNMreqMSG( SOCKET connected_socket, Byte_t *jpipstream, msgqueue_param_t *msgqueue, cachelist_param_t *cachelist)
{
  Byte_t *pnmstream;
  ihdrbox_param_t *ihdrbox;
  char *CIDorTID, tmp[10];
  cache_param_t *cache;
  int fw, fh;
  
  CIDorTID = receive_string( connected_socket);
  
  if(!(cache = search_cacheBycid( CIDorTID, cachelist)))
    if(!(cache = search_cacheBytid( CIDorTID, cachelist))){
      free( CIDorTID);
      return;
    }
  
  free( CIDorTID);

  receive_line( connected_socket, tmp);
  fw = atoi( tmp);

  receive_line( connected_socket, tmp);
  fh = atoi( tmp);

  ihdrbox = NULL;
  pnmstream = jpipstream_to_pnm( jpipstream, msgqueue, cache->csn, fw, fh, &ihdrbox);

  send_PNMstream( connected_socket, pnmstream, ihdrbox->width, ihdrbox->height, ihdrbox->nc, ihdrbox->bpc > 8 ? 255 : (1 << ihdrbox->bpc) - 1);

  free( ihdrbox);
  free( pnmstream);
}

void handle_XMLreqMSG( SOCKET connected_socket, Byte_t *jpipstream, cachelist_param_t *cachelist)
{
  char *cid;
  cache_param_t *cache;
  boxcontents_param_t *boxcontents;
  Byte_t *xmlstream;

  cid = receive_string( connected_socket);

  if(!(cache = search_cacheBycid( cid, cachelist))){
    free( cid);
    return;
  }

  free( cid);
  
  boxcontents = cache->metadatalist->last->boxcontents;
  xmlstream = (Byte_t *)malloc( boxcontents->length);
  memcpy( xmlstream, jpipstream+boxcontents->offset, boxcontents->length);
  send_XMLstream( connected_socket, xmlstream, boxcontents->length);
  free( xmlstream);
}

void handle_TIDreqMSG( SOCKET connected_socket, cachelist_param_t *cachelist)
{
  char *target, *tid = NULL;
  cache_param_t *cache;
  int tidlen = 0;

  target = receive_string( connected_socket);
  cache = search_cache( target, cachelist);

  free( target);
  
  if( cache){
    tid = cache->tid;
    tidlen = strlen(tid);
  }
  send_TIDstream( connected_socket, tid, tidlen);
}

void handle_CIDreqMSG( SOCKET connected_socket, cachelist_param_t *cachelist)
{
  char *target, *cid = NULL;
  cache_param_t *cache;
  int cidlen = 0;

  target = receive_string( connected_socket);
  cache = search_cache( target, cachelist);
  
  free( target);

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
  char *cid;

  cid = receive_string( connected_socket);
  remove_cachecid( cid, cachelist);
  response_signal( connected_socket, true);
  
  free( cid);
}

void handle_SIZreqMSG( SOCKET connected_socket, Byte_t *jpipstream, msgqueue_param_t *msgqueue, cachelist_param_t *cachelist)
{
  char *tid, *cid;
  cache_param_t *cache;
  Byte4_t width, height;
  
  tid = receive_string( connected_socket);
  cid = receive_string( connected_socket);
  
  cache = NULL;

  if( tid[0] != '0')
    cache = search_cacheBytid( tid, cachelist);
  
  if( !cache && cid[0] != '0')
    cache = search_cacheBycid( cid, cachelist);

  free( tid);
  free( cid);
  
  width = height = 0;
  if( cache){
    if( !cache->ihdrbox)
      cache->ihdrbox = get_SIZ_from_jpipstream( jpipstream, msgqueue, cache->csn);
    width  = cache->ihdrbox->width;
    height = cache->ihdrbox->height;
  }
  send_SIZstream( connected_socket, width, height);
}

void handle_JP2saveMSG( SOCKET connected_socket, cachelist_param_t *cachelist, msgqueue_param_t *msgqueue, Byte_t *jpipstream)
{
  char *cid;
  cache_param_t *cache;
  Byte_t *jp2stream;
  Byte8_t jp2len;

  cid = receive_string( connected_socket);
  if(!(cache = search_cacheBycid( cid, cachelist))){
    free( cid);
    return;
  }
  
  free( cid);
  
  jp2stream = recons_jp2( msgqueue, jpipstream, cache->csn, &jp2len);

  if( jp2stream){
    save_codestream( jp2stream, jp2len, "jp2");
    free( jp2stream);
  }
}
