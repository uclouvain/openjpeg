/*
 * $Id: query_parser.c 44 2011-02-15 12:32:29Z kaori $
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

#include <strings.h>
#include <stdio.h>
#include <string.h>
#include "query_parser.h"

#ifdef SERVER
#include "fcgi_stdio.h"
#define logstream FCGI_stdout
#else
#define FCGI_stdout stdout
#define FCGI_stderr stderr
#define logstream stderr
#endif //SERVER


/**
 * initialize query parameters
 *
 * @param[in,out] query_param  query parameters
 */
void init_queryparam( query_param_t *query_param);

/**
 * parse string to string array
 *
 * @param[in]  src src string
 * @param[out] cclose parsed string array
 */
void str2cclose( char *src, char cclose[][MAX_LENOFCID]);

void parse_metareq( char *field, query_param_t *query_param);


void parse_query( char *query_string, query_param_t *query_param)
{
  char *pquery, *field, *fieldname;

  init_queryparam( query_param);
  
  pquery = query_string;

  while( pquery!=NULL) {
    
    field = strsep(&pquery, "&\n");
    //field = strtok_r( query, "&", pquery);
    
    fieldname = strsep(&field, "=");
    //fieldname = strtok( field, "=");

    if( field != NULL){
      if( strcasecmp( fieldname, "target") == 0)
	strcpy( query_param->target,field);
      
      else if( strcasecmp( fieldname, "fsiz") == 0)
	sscanf( field, "%d,%d", &query_param->fx, &query_param->fy);
      
      else if( strcasecmp( fieldname, "roff") == 0)
	sscanf( field, "%d,%d", &query_param->rx, &query_param->ry);

      else if( strcasecmp( fieldname, "rsiz") == 0)
	sscanf( field, "%d,%d", &query_param->rw, &query_param->rh);
      
      else if( strcasecmp( fieldname, "cid") == 0)
	strcpy( query_param->cid, field);

      else if( strcasecmp( fieldname, "cnew") == 0)
	query_param->cnew = true;
      
      else if( strcasecmp( fieldname, "cclose") == 0)
	str2cclose( field, query_param->cclose);
      
      else if( strcasecmp( fieldname, "metareq") == 0)
	parse_metareq( field, query_param);
    }
  }
}


void init_queryparam( query_param_t *query_param)
{
  query_param->target[0]='\0';
  query_param->fx=-1;
  query_param->fy=-1;
  query_param->rx=-1;
  query_param->ry=-1;
  query_param->rw=-1;
  query_param->rh=-1;
  query_param->cid[0]='\0';
  query_param->cnew=false;
  memset( query_param->cclose, 0, MAX_NUMOFCCLOSE*MAX_LENOFCID);
  memset( query_param->box_type, 0, MAX_NUMOFBOX*4);
  memset( query_param->limit, 0, MAX_NUMOFBOX*sizeof(int));
  for( int i=0; i<MAX_NUMOFBOX; i++){
    query_param->w[i] = false;
    query_param->s[i] = false;
    query_param->g[i] = false;
    query_param->a[i] = false;
    query_param->priority[i] = false;
  }
  query_param->root_bin = 0;
  query_param->max_depth = -1;
  query_param->metadata_only = false;
}


void print_queryparam( query_param_t query_param)
{
  fprintf( logstream, "query parameters:\n");
  fprintf( logstream, "\t target: %s\n", query_param.target);
  fprintf( logstream, "\t fx,fy: %d, %d\n", query_param.fx, query_param.fy);
  fprintf( logstream, "\t rx,ry: %d, %d \t rw,rh: %d, %d\n", query_param.rx, query_param.ry, query_param.rw, query_param.rh);
  fprintf( logstream, "\t cnew: %d\n", query_param.cnew);
  fprintf( logstream, "\t cid: %s\n", query_param.cid);
  
  fprintf( logstream, "\t cclose: ");
  for( int i=0; query_param.cclose[i][0]!=0 && i<MAX_NUMOFCCLOSE; i++)
    fprintf( logstream, "%s ", query_param.cclose[i]);
  fprintf(logstream, "\n");
  
  fprintf( logstream, "\t req-box-prop\n");
  for( int i=0; query_param.box_type[i][0]!=0 && i<MAX_NUMOFBOX; i++){
    fprintf( logstream, "\t\t box_type: %.4s limit: %d w:%d s:%d g:%d a:%d priority:%d\n", query_param.box_type[i], query_param.limit[i], query_param.w[i], query_param.s[i], query_param.g[i], query_param.a[i], query_param.priority[i]);
  }
  
  fprintf( logstream, "\t root-bin:  %d\n", query_param.root_bin);
  fprintf( logstream, "\t max-depth: %d\n", query_param.max_depth);
  fprintf( logstream, "\t metadata-only: %d\n", query_param.metadata_only);
}

void str2cclose( char *src, char cclose[][MAX_LENOFCID])
{
  size_t len = strlen( src);
  
  for( int i=0, u=0, v=0; i<len; i++){
    if( src[i]==','){
      u++;
      v=0;
    }
    else
      cclose[u][v++] = src[i];
  }
}

void parse_req_box_prop( char *req_box_prop, int idx, query_param_t *query_param);

void parse_metareq( char *field, query_param_t *query_param)
{
  char req_box_prop[20];
  char *ptr, *src;
  int numofboxreq = 0;
  
  bzero( req_box_prop, 20);

  // req-box-prop
  ptr = strchr( field, '[');
  ptr++;
  src = ptr;
  while( *ptr != ']'){
    if( *ptr == ';'){
      strncpy( req_box_prop, src, ptr-src);
      parse_req_box_prop( req_box_prop, numofboxreq++, query_param);
      ptr++;
      src = ptr;
      bzero( req_box_prop, 20);
    }
    ptr++;
  }
  strncpy( req_box_prop, src, ptr-src);

  parse_req_box_prop( req_box_prop, numofboxreq++, query_param);

  if(( ptr = strchr( field, 'R')))
    sscanf( ptr+1, "%d", &(query_param->root_bin));
  
  if(( ptr = strchr( field, 'D')))
    sscanf( ptr+1, "%d", &(query_param->max_depth));

  if(( ptr = strstr( field, "!!")))
    query_param->metadata_only = true;
}

void parse_req_box_prop( char *req_box_prop, int idx, query_param_t *query_param)
{
  char *ptr;
  
  if( *req_box_prop == '*')
    query_param->box_type[idx][0]='*';
  else
    strncpy( query_param->box_type[idx], req_box_prop, 4);

  if(( ptr = strchr( req_box_prop, ':'))){
    if( *(ptr+1)=='r')
      query_param->limit[idx] = -1;
    else
      sscanf( ptr+1, "%d", &(query_param->limit[idx]));
  }

  if(( ptr = strchr( req_box_prop, '/'))){
    ptr++;
    while( *ptr=='w' || *ptr=='s' || *ptr=='g' || *ptr=='a'){
      switch( *ptr){
      case 'w': query_param->w[idx] = true; break;
      case 's': query_param->s[idx] = true; break;
      case 'g': query_param->g[idx] = true; break;
      case 'a': query_param->a[idx] = true; break;
      }
      ptr++;
    }
  }
  else{
    query_param->g[idx] = true;
    query_param->s[idx] = true;
    query_param->w[idx] = true;
  }

  if((ptr = strchr( req_box_prop, '!')))
    query_param->priority[idx] = true;
  
  idx++;   
}
