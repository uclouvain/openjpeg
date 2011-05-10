/*
 * $Id: opj_dec_server.c 54 2011-05-10 13:22:47Z kaori $
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

/*! \file
 *  \brief opj_dec_server is a server to decode JPT-stream and communicate locally with JPIP client, which is coded in java.
 *
 *  \section impinst Implementing instructions
 *  Launch opj_dec_server from a terminal in the same machine as JPIP client image viewers. \n
 *   % ./opj_dec_server \n
 *  Keep it alive as long as image viewers are open.\n
 *
 *  To quite the opj_dec_server, send a message "quit" through the telnet.\n
 *   % telnet localhost 5000\n
 *     quit\n
 *  Be sure all image viewers are closed.\n
 *  Cache file in JPT format is stored in the working directly before it quites.
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "byte_manager.h"
#include "msgqueue_manager.h"
#include "ihdrbox_manager.h"
#include "imgsock_manager.h"
#include "jptstream_manager.h"
#include "cache_manager.h"

#ifdef _WIN32
WSADATA initialisation_win32;
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif //_WIN32

//! maximum length of target name
#define MAX_LENOFTARGET 128

//! maximum length of channel identifier
#define MAX_LENOFCID 30

/**
 * handle JPTstream message
 *
 * @param[in]     connected_socket socket descriptor
 * @param[in]     cachelist        cache list pointer
 * @param[in,out] jptstream        address of jptstream pointer
 * @param[in,out] jptlen           address of jptstream length
 * @param[in,out] msgqueue         message queue pointer
 */
void handle_JPTstreamMSG( SOCKET connected_socket, cachelist_param_t *cachelist, Byte_t **jptstream, int *jptlen, msgqueue_param_t *msgqueue);

/**
 * handle PNM request message
 *
 * @param[in] connected_socket socket descriptor
 * @param[in] jptstream        jptstream pointer
 * @param[in] msgqueue         message queue pointer
 * @param[in] cachelist        cache list pointer
 */
void handle_PNMreqMSG( SOCKET connected_socket, Byte_t *jptstream, msgqueue_param_t *msgqueue, cachelist_param_t *cachelist);

/**
 * handle XML request message
 *
 * @param[in] connected_socket socket descriptor
 * @param[in] jptstream        address of caching jptstream pointer
 * @param[in] cachelist        cache list pointer
 */
void handle_XMLreqMSG( SOCKET connected_socket, Byte_t *jptstream, cachelist_param_t *cachelist);

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
 * @param[in] jptstream        address of caching jptstream pointer
 */
void handle_JP2saveMSG( SOCKET connected_socket, cachelist_param_t *cachelist, msgqueue_param_t *msgqueue, Byte_t *jptstream);

int main(int argc, char *argv[]){

  SOCKET connected_socket;
  struct sockaddr_in peer_sin;
  Byte_t *jptstream = NULL;
  int jptlen = 0;
  msgqueue_param_t *msgqueue = gene_msgqueue( true, NULL);
  bool quit = false;

#ifdef _WIN32
  int erreur = WSAStartup(MAKEWORD(2,2),&initialisation_win32);
  if( erreur!=0)
    fprintf( stderr, "Erreur initialisation Winsock error : %d %d\n",erreur,WSAGetLastError());
  else
    printf( "Initialisation Winsock\n");
#endif //_WIN32
  
  int listening_socket = open_listeningsocket();
  
  int addrlen = sizeof(peer_sin);

  cachelist_param_t *cachelist = gene_cachelist();
  
  while(( connected_socket = accept(listening_socket, (struct sockaddr *)&peer_sin, &addrlen))!=-1 ){
    msgtype_t msgtype = identify_clientmsg( connected_socket);
    
    switch( msgtype){
    case JPTSTREAM:
      handle_JPTstreamMSG( connected_socket, cachelist, &jptstream, &jptlen, msgqueue);
      break;
      
    case PNMREQ:
      handle_PNMreqMSG( connected_socket, jptstream, msgqueue, cachelist);
      break;
    
    case XMLREQ:
      handle_XMLreqMSG( connected_socket, jptstream, cachelist);
      break;
						
    case CIDREQ:
      handle_CIDreqMSG( connected_socket, cachelist);
      break;

    case CIDDST:
      handle_dstCIDreqMSG( connected_socket, cachelist);
      break;

    case JP2SAVE:
      handle_JP2saveMSG( connected_socket, cachelist, msgqueue, jptstream);
      break;

    case QUIT:
      quit = true;
      break;
    case MSGERROR:
      break;
    }
        
    printf("cut the connection. listening to port\n");
    if( closesocket(connected_socket) != 0){
      perror("close");
      return -1;
    }
    if( quit)
      break;
  }
  if( closesocket(listening_socket) != 0){
    perror("close");
    return -1;
  }

  delete_cachelist( &cachelist);

  if( msgqueue)
    delete_msgqueue( &msgqueue);
  
  save_codestream( jptstream, jptlen, "jpt");
  free( jptstream);

#ifdef _WIN32
  if( WSACleanup() != 0){
    printf("\nError in WSACleanup : %d %d",erreur,WSAGetLastError());
  }else{
    printf("\nWSACleanup OK\n");
  }
#endif

  return 0;
}

void handle_JPTstreamMSG( SOCKET connected_socket, cachelist_param_t *cachelist,
			  Byte_t **jptstream, int *jptlen, msgqueue_param_t *msgqueue)
{
  Byte_t *newjptstream;
  int newjptlen = 0;
  cache_param_t *cache;
  char target[MAX_LENOFTARGET], cid[MAX_LENOFCID];
  metadatalist_param_t *metadatalist;
      
  newjptstream = receive_JPTstream( connected_socket, target, cid, &newjptlen);
  
  parse_stream( newjptstream, newjptlen, *jptlen, msgqueue);
  
  *jptstream = update_JPTstream( newjptstream, newjptlen, *jptstream, jptlen);
  free( newjptstream);

  metadatalist = gene_metadatalist();
  parse_metamsg( msgqueue, *jptstream, *jptlen, metadatalist);
  
  // cid registration
  if( target[0] != 0 && cid[0] != 0){
    if((cache = search_cache( target, cachelist)))
      add_cachecid( cid, cache);
    else{
      cache = gene_cache( target, msgqueue->last->csn, cid);
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

void handle_PNMreqMSG( SOCKET connected_socket, Byte_t *jptstream, msgqueue_param_t *msgqueue, cachelist_param_t *cachelist)
{
  Byte_t *pnmstream;
  ihdrbox_param_t *ihdrbox;
  char cid[MAX_LENOFCID], tmp[10];
  cache_param_t *cache;
  int fw, fh;
  
  receive_line( connected_socket, cid);
  if(!(cache = search_cacheBycid( cid, cachelist)))
    return;

  receive_line( connected_socket, tmp);
  fw = atoi( tmp);
  
  receive_line( connected_socket, tmp);
  fh = atoi( tmp);

  pnmstream = jpt_to_pnm( jptstream, msgqueue, cache->csn, fw, fh, &cache->ihdrbox);
  ihdrbox = cache->ihdrbox;
  send_PNMstream( connected_socket, pnmstream, ihdrbox->width, ihdrbox->height, ihdrbox->nc, ihdrbox->bpc > 8 ? 255 : (1 << ihdrbox->bpc) - 1);

  free( pnmstream);
}

void handle_XMLreqMSG( SOCKET connected_socket, Byte_t *jptstream, cachelist_param_t *cachelist)
{
  char cid[MAX_LENOFCID];
  cache_param_t *cache;

  receive_line( connected_socket, cid);
  if(!(cache = search_cacheBycid( cid, cachelist)))
    return;
  
  boxcontents_param_t *boxcontents = cache->metadatalist->last->boxcontents;
  Byte_t *xmlstream = (Byte_t *)malloc( boxcontents->length);
  memcpy( xmlstream, jptstream+boxcontents->offset, boxcontents->length);
  send_XMLstream( connected_socket, xmlstream, boxcontents->length);
  free( xmlstream);
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

void handle_JP2saveMSG( SOCKET connected_socket, cachelist_param_t *cachelist, msgqueue_param_t *msgqueue, Byte_t *jptstream)
{
  char cid[MAX_LENOFCID];
  cache_param_t *cache;
  Byte_t *jp2stream;
  Byte8_t jp2len;

  receive_line( connected_socket, cid);
  if(!(cache = search_cacheBycid( cid, cachelist)))
    return;
  
  jp2stream = recons_jp2( msgqueue, jptstream, cache->csn, &jp2len);

  if( jp2stream){
    save_codestream( jp2stream, jp2len, "jp2");
    free( jp2stream);
  }
}
