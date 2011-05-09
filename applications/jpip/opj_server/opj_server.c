/*
 * $Id: opj_server.c 53 2011-05-09 16:55:39Z kaori $
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
 *  \brief opj_server is a JPIP server program, which supports HTTP connection, JPT-stream, session, channels, and cache model managements.
 *
 *  \section req Requirements
 *    FastCGI development kit (http://www.fastcgi.com).
 *
 *  \section impinst Implementing instructions
 *  Launch opj_server from the server terminal:\n
 *   % spawn-fcgi -f ./opj_server -p 3000 -n
 *
 *  Note: JP2 files are stored in the working directory of opj_server\n
 *  Check README for the JP2 Encoding\n
 *  
 *  We tested this software with a virtual server running on the same Linux machine as the clients.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "query_parser.h"
#include "channel_manager.h"
#include "session_manager.h"
#include "target_manager.h"
#include "imgreg_manager.h"
#include "msgqueue_manager.h"


#ifdef SERVER
#include "fcgi_stdio.h"
#define logstream FCGI_stdout
#else
#define FCGI_stdout stdout
#define FCGI_stderr stderr
#define logstream stderr
#endif //SERVER

/**
 * parse JPIP request
 *
 * @param[in]     query_param   structured query
 * @param[in]     sessionlist   session list pointer
 * @param[in,out] msgqueue      address of the message queue pointer
 * @return                      if succeeded (true) or failed (false)
 */
bool parse_JPIPrequest( query_param_t query_param,
			sessionlist_param_t *sessionlist,
			msgqueue_param_t **msgqueue);


/**
 * REQUEST: channel association
 *          this must be processed before any process
 *
 * @param[in]     query_param   structured query
 * @param[in]     sessionlist   session list pointer
 * @param[out]    cursession    address of the associated session pointer
 * @param[out]    curchannel    address of the associated channel pointer
 * @return                      if succeeded (true) or failed (false)
 */
bool associate_channel( query_param_t    query_param, 
			sessionlist_param_t *sessionlist,
			session_param_t **cursession, 
			channel_param_t **curchannel);
/**
 * REQUEST: new channel (cnew) assignment
 *
 * @param[in]     query_param   structured query
 * @param[in]     sessionlist   session list pointer
 * @param[in,out] cursession    address of the associated/opened session pointer
 * @param[in,out] curchannel    address of the associated/opened channel pointer
 * @return                      if succeeded (true) or failed (false)
 */
bool open_channel( query_param_t query_param, 
		   sessionlist_param_t *sessionlist,
		   session_param_t **cursession, 
		   channel_param_t **curchannel);

/**
 * REQUEST: channel close (cclose)
 *
 * @param[in]     query_param   structured query
 * @param[in]     sessionlist   session list pointer
 * @param[in,out] cursession    address of the session pointer of deleting channel
 * @param[in,out] curchannel    address of the deleting channel pointer
 * @return                      if succeeded (true) or failed (false)
 */
bool close_channel( query_param_t query_param, 
		    sessionlist_param_t *sessionlist,
		    session_param_t **cursession, 
		    channel_param_t **curchannel);

/**
 * REQUEST: view-window (fsiz)
 *
 * @param[in]     query_param structured query
 * @param[in,out] cursession  associated session pointer
 * @param[in,out] curchannel  associated channel pointer
 * @param[in,out] msgqueue    address of the message queue pointer
 * @return                    if succeeded (true) or failed (false)
 */
bool gene_JPTstream( query_param_t query_param,
		     session_param_t *cursession, 
		     channel_param_t *curchannel,
		     msgqueue_param_t **msgqueue);

int main(void)
{ 
  sessionlist_param_t *sessionlist;
  bool parse_status;
  
  sessionlist = gene_sessionlist();

#ifdef SERVER

  char *query_string;
  while(FCGI_Accept() >= 0)
#else

  char query_string[128];
  while((fgets( query_string, 128, stdin))[0] != '\n' )
#endif
    {

#ifdef SERVER     
      query_string = getenv("QUERY_STRING");    
#endif //SERVER
      
      fprintf( FCGI_stdout, "Content-type: image/jpt-stream\r\n");
      
      query_param_t query_param;
      msgqueue_param_t *msgqueue;

      parse_query( query_string, &query_param); 
      
#ifndef SERVER
      print_queryparam( query_param);
#endif

      msgqueue = NULL;
      parse_status = parse_JPIPrequest( query_param, sessionlist, &msgqueue);

      
      fprintf( FCGI_stdout, "\r\n");

#ifndef SERVER
      //      if( parse_status)
      // 	print_allsession( sessionlist);
      print_msgqueue( msgqueue);
#endif
      emit_stream_from_msgqueue( msgqueue);

      delete_msgqueue( &msgqueue);
    }
  delete_sessionlist( &sessionlist);

  return 0;
}

bool parse_JPIPrequest( query_param_t query_param,
			sessionlist_param_t *sessionlist,
			msgqueue_param_t **msgqueue)
{ 
  session_param_t *cursession = NULL;
  channel_param_t *curchannel = NULL;

  if( query_param.cid[0] != '\0')
    if( !associate_channel( query_param, sessionlist, &cursession, &curchannel))
      return false;
  
  if( query_param.cnew){
    if( !open_channel( query_param, sessionlist, &cursession, &curchannel))
      return false;
  }
  if( query_param.cclose[0][0] != '\0')
    if( !close_channel( query_param, sessionlist, &cursession, &curchannel))
      return false;
  
  if( (query_param.fx > 0 && query_param.fx > 0) || query_param.box_type[0][0] != 0)
    if( !gene_JPTstream( query_param, cursession, curchannel, msgqueue))
      return false;
      
  return true;
}

bool associate_channel( query_param_t    query_param, 
			sessionlist_param_t *sessionlist,
			session_param_t **cursession, 
			channel_param_t **curchannel)
{
  if( search_session_and_channel( query_param.cid, sessionlist, cursession, curchannel)){
    
    if( !query_param.cnew)
      set_channel_variable_param( query_param, *curchannel);
  }
  else{
    fprintf( FCGI_stderr, "Error: process canceled\n");
    return false;
  }
  return true;
}

bool open_channel( query_param_t query_param, 
		   sessionlist_param_t *sessionlist,
		   session_param_t **cursession, 
		   channel_param_t **curchannel)
{
  target_param_t *target=NULL;

  if( query_param.target[0] !='\0'){   // target query specified
    if( *cursession){
      if( !( target = search_target( query_param.target, (*cursession)->targetlist))){	
	if((target = gene_target( query_param.target)))
	  insert_target_into_session( *cursession, target);
	else
	  return false;
      }
    }
    else{
      if((target = gene_target( query_param.target))){
	// new session
	*cursession = gene_session( sessionlist);
      	insert_target_into_session( *cursession, target);
      }
      else
	return false;
    }
  }
  else{
    if( *cursession)
      target = (*curchannel)->target;
  }
  
  *curchannel = gene_channel( query_param, target, (*cursession)->channellist);
  if( *curchannel == NULL)
    return false;

  return true;
}

bool close_channel( query_param_t query_param, 
		    sessionlist_param_t *sessionlist,
		    session_param_t **cursession, 
		    channel_param_t **curchannel)
{
  if( query_param.cclose[0][0] =='*'){
#ifndef SERVER
    fprintf( logstream, "local log: close all\n");
#endif
    // all channels associatd with the session will be closed
    if( !delete_session( cursession, sessionlist))
      return false;
  }
  else{
    // check if all entry belonging to the same session
    int i=0;
    while( query_param.cclose[i][0] !='\0'){
      
      // In case of the first entry of close cid
      if( *cursession == NULL){
	if( !search_session_and_channel( query_param.cclose[i], sessionlist, cursession, curchannel))
	  return false;
      }
      else // second or more entry of close cid
	if( !(*curchannel=search_channel( query_param.cclose[i], (*cursession)->channellist))){
	  fprintf( FCGI_stdout, "Reason: Cclose id %s is from another session\r\n", query_param.cclose[i]); 
	  return false;
	}
      i++;
    }
    // delete channels
    i=0;
    while( query_param.cclose[i][0] !='\0'){
      
      *curchannel = search_channel( query_param.cclose[i], (*cursession)->channellist);
      delete_channel( curchannel, (*cursession)->channellist);
      i++;
    }
    
    if( (*cursession)->channellist->first == NULL || (*cursession)->channellist->last == NULL)
      // In case of empty session
      delete_session( cursession, sessionlist);
  }
  return true;
}


/**
 * enqueue tiles into the message queue
 *
 * @param[in]     query_param structured query
 * @param[in]     codeidx     pointer to index parameters
 * @param[in,out] msgqueue    message queue pointer  
 */
void enqueue_tiles( query_param_t query_param, index_param_t *codeidx, msgqueue_param_t *msgqueue);

/**
 * enqueue metadata bins into the message queue
 *
 * @param[in]     query_param  structured query
 * @param[in]     metadatalist pointer to metadata bin list
 * @param[in,out] msgqueue     message queue pointer  
 */
void enqueue_metabins( query_param_t query_param, metadatalist_param_t *metadatalist, msgqueue_param_t *msgqueue);


bool gene_JPTstream( query_param_t query_param,
		     session_param_t *cursession, 
		     channel_param_t *curchannel,
		     msgqueue_param_t **msgqueue)
{
  target_param_t *target;
  index_param_t *codeidx;
  
  if( !cursession || !curchannel){ // stateless
    if((target = gene_target( query_param.target)))
      *msgqueue = gene_msgqueue( true, target);
    else
      return false;
  }
  else{ // session
    target  = curchannel->target;
    *msgqueue = gene_msgqueue( false, target);
  }
  
  codeidx = target->codeidx;

  //meta
  if( query_param.box_type[0][0] != 0)
    enqueue_metabins( query_param, codeidx->metadatalist, *msgqueue); 

  // image code
  if( query_param.fx > 0 && query_param.fx > 0){
    if( !codeidx->mhead_model)
      enqueue_mainheader( *msgqueue);
    enqueue_tiles( query_param, codeidx, *msgqueue);
  }
  
  return true;
}

void enqueue_tiles( query_param_t query_param, index_param_t *codeidx, msgqueue_param_t *msgqueue)
{
  imgreg_param_t imgreg;
  range_param_t tile_Xrange, tile_Yrange;
  int u, v, tile_id;

  imgreg  = map_viewin2imgreg( query_param.fx, query_param.fy, 
			       query_param.rx, query_param.ry, query_param.rw, query_param.rh,
			       codeidx->XOsiz, codeidx->YOsiz, codeidx->Xsiz, codeidx->Ysiz, 
			       get_nmax( codeidx->tilepart));

  
  for( u=0, tile_id=0; u<codeidx->YTnum; u++){
    tile_Yrange = get_tile_Yrange( *codeidx, u, imgreg.level);
    
    for( v=0; v<codeidx->XTnum; v++, tile_id++){
      tile_Xrange = get_tile_Xrange( *codeidx, v, imgreg.level);
	
      if( tile_Xrange.minvalue < tile_Xrange.maxvalue && tile_Yrange.minvalue < tile_Yrange.maxvalue){
	if( tile_Xrange.maxvalue <= imgreg.xosiz + imgreg.ox || 
	    tile_Xrange.minvalue >= imgreg.xosiz + imgreg.ox + imgreg.sx ||
	    tile_Yrange.maxvalue <= imgreg.yosiz + imgreg.oy || 
	    tile_Yrange.minvalue >= imgreg.yosiz + imgreg.oy + imgreg.sy) {
	  //printf("Tile completely excluded from view-window %d\n", tile_id);
	  // Tile completely excluded from view-window
	}
	else if( tile_Xrange.minvalue >= imgreg.xosiz + imgreg.ox && 
		 tile_Xrange.maxvalue <= imgreg.xosiz + imgreg.ox + imgreg.sx && 
		 tile_Yrange.minvalue >= imgreg.yosiz + imgreg.oy && 
		 tile_Yrange.maxvalue <= imgreg.yosiz + imgreg.oy + imgreg.sy) {
	  // Tile completely contained within view-window
	  // high priority
	  //printf("Tile completely contained within view-window %d\n", tile_id);
	  enqueue_tile( tile_id, imgreg.level, msgqueue);
	}
	else{
	  // Tile partially overlaps view-window
	  // low priority
	  //printf("Tile partially overlaps view-window %d\n", tile_id);
	  enqueue_tile( tile_id, imgreg.level, msgqueue);
	}
      }
    }
  }
}

void enqueue_metabins( query_param_t query_param, metadatalist_param_t *metadatalist, msgqueue_param_t *msgqueue)
{
  int i;
  for( i=0; query_param.box_type[i][0]!=0 && i<MAX_NUMOFBOX; i++){
    if( query_param.box_type[i][0] == '*'){
      // not implemented
    }
    else{
      int idx = search_metadataidx( query_param.box_type[i], metadatalist);

      if( idx != -1)
	enqueue_metadata( idx, msgqueue);
    }
  }
}
