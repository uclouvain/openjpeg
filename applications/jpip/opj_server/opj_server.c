/*
 * $Id: opj_server.c 53 2011-05-09 16:55:39Z kaori $
 *
 * Copyright (c) 2002-2011, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2011, Professor Benoit Macq
 * Copyright (c) 2010-2011, Kaori Hagihara 
 * Copyright (c) 2011,      Lucian Corlaciu, GSoC
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
#include <math.h>

#include "query_parser.h"
#include "jpip_parser.h"
#include "session_manager.h"
#include "target_manager.h"
#include "msgqueue_manager.h"

#ifndef QUIT_SIGNAL
#define QUIT_SIGNAL "quitJPIP"
#endif

#ifdef SERVER
#include "fcgi_stdio.h"
#define logstream FCGI_stdout
#else
#define FCGI_stdout stdout
#define FCGI_stderr stderr
#define logstream stderr
#endif //SERVER

int main(void)
{ 
  sessionlist_param_t *sessionlist;
  targetlist_param_t *targetlist;
  bool parse_status;
  
  sessionlist = gene_sessionlist();
  targetlist  = gene_targetlist();
  
#ifdef SERVER

  char *query_string;
  while(FCGI_Accept() >= 0)
#else

  char query_string[128];
  while( fgets( query_string, 128, stdin) && query_string[0]!='\n')
#endif
    {

#ifdef SERVER     
      query_string = getenv("QUERY_STRING");    
#endif //SERVER

      if( strcmp( query_string, QUIT_SIGNAL) == 0)
	break;
           
      query_param_t query_param;
      msgqueue_param_t *msgqueue;

      parse_query( query_string, &query_param); 

#ifndef SERVER
      print_queryparam( query_param);
#endif
      
      msgqueue = NULL;
      if( !(parse_status = parse_JPIPrequest( query_param, sessionlist, targetlist, &msgqueue)))
	fprintf( FCGI_stderr, "Error: JPIP request failed\n");
            
      fprintf( FCGI_stdout, "\r\n");

#ifndef SERVER
      //      if( parse_status)
      // 	print_allsession( sessionlist);
      print_msgqueue( msgqueue);
#endif

      emit_stream_from_msgqueue( msgqueue);

      delete_msgqueue( &msgqueue);
    }
  
  fprintf( FCGI_stderr, "JPIP server terminated by a client request\n");

  delete_sessionlist( &sessionlist);
  delete_targetlist( &targetlist);

  return 0;
}
