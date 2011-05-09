/*
 * $Id: cache_manager.c 53 2011-05-09 16:55:39Z kaori $
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
#include "cache_manager.h"

//! maximum length of channel identifier
#define MAX_LENOFCID 30

cachelist_param_t * gene_cachelist()
{
  cachelist_param_t *cachelist;
  
  cachelist = (cachelist_param_t *)malloc( sizeof(cachelist_param_t));
  
  cachelist->first = NULL;
  cachelist->last  = NULL;

  return cachelist;
}

void delete_cachelist(cachelist_param_t **cachelist)
{
  cache_param_t *cachePtr, *cacheNext;
  
  cachePtr = (*cachelist)->first;
  while( cachePtr != NULL){
    cacheNext=cachePtr->next;
    delete_cache( &cachePtr);
    cachePtr=cacheNext;
  }
  free( *cachelist);
}

cache_param_t * gene_cache( char *targetname, int csn, char *cid)
{
  cache_param_t *cache;
  
  cache = (cache_param_t *)malloc( sizeof(cache_param_t));
  strcpy( cache->filename, targetname);
  cache->csn = csn;
  cache->cid = (char **)malloc( sizeof(char *));
  *cache->cid = (char *)malloc( MAX_LENOFCID);
  strcpy( *cache->cid, cid);
  cache->numOfcid = 1;
#if 1
  cache->metadatalist = NULL;
#else
  cache->metadatalist = gene_metadatalist();
#endif
  cache->ihdrbox = NULL;
  cache->next = NULL;

  return cache;
}

void delete_cache( cache_param_t **cache)
{
  int i;

  delete_metadatalist( &(*cache)->metadatalist);

  if((*cache)->ihdrbox)
    free((*cache)->ihdrbox);
  for( i=0; i<(*cache)->numOfcid; i++)
    free( (*cache)->cid[i]);
  free( (*cache)->cid);
  free( *cache);
}

void insert_cache_into_list( cache_param_t *cache, cachelist_param_t *cachelist)
{
  if( cachelist->first)
    cachelist->last->next = cache;
  else
    cachelist->first = cache;
  cachelist->last = cache;
}

cache_param_t * search_cache( char targetname[], cachelist_param_t *cachelist)
{
  cache_param_t *foundcache;

  foundcache = cachelist->first;
  
  while( foundcache != NULL){
    
    if( strcmp( targetname, foundcache->filename) == 0)
      return foundcache;
      
    foundcache = foundcache->next;
  }
  return NULL;
}

cache_param_t * search_cacheBycsn( int csn, cachelist_param_t *cachelist)
{
  cache_param_t *foundcache;

  foundcache = cachelist->first;
  
  while( foundcache != NULL){
    
    if(  csn == foundcache->csn)
      return foundcache;
    foundcache = foundcache->next;
  }
  return NULL;
}

cache_param_t * search_cacheBycid( char cid[], cachelist_param_t *cachelist)
{
  cache_param_t *foundcache;
  int i;

  foundcache = cachelist->first;
  
  while( foundcache != NULL){
    for( i=0; i<foundcache->numOfcid; i++)
      if( strcmp( cid, foundcache->cid[i]) == 0)
	return foundcache;
    foundcache = foundcache->next;
  }
  return NULL;
}

void add_cachecid( char *cid, cache_param_t *cache)
{
  char **tmp;
  int i;

  tmp = cache->cid;
  
  cache->cid = (char **)malloc( (cache->numOfcid+1)*sizeof(char *));

  for( i=0; i<cache->numOfcid; i++){
    cache->cid[i] = (char *)malloc( MAX_LENOFCID);
    strcpy( cache->cid[i], tmp[i]);
    free( tmp[i]);
  }
  free( tmp);

  cache->cid[ cache->numOfcid] = (char *)malloc( MAX_LENOFCID);
  strcpy( cache->cid[ cache->numOfcid], cid);

  cache->numOfcid ++;
}

void remove_cidInCache( char *cid, cache_param_t *cache);

void remove_cachecid( char *cid, cachelist_param_t *cachelist)
{
  cache_param_t *cache;

  cache = search_cacheBycid( cid, cachelist);
  remove_cidInCache( cid, cache);
}

void remove_cidInCache( char *cid, cache_param_t *cache)
{
  int idx = -1;
  char **tmp;
  int i, j;

  for( i=0; i<cache->numOfcid; i++)
    if( strcmp( cid, cache->cid[i]) == 0){
      idx = i;
      break;
    }

  if( idx == -1){
    fprintf( stderr, "cid: %s not found\n", cid);
    return;   
  }
  
  tmp = cache->cid;

  cache->cid = (char **)malloc( (cache->numOfcid-1)*sizeof(char *));
  
  for( i=0, j=0; i<cache->numOfcid; i++){
    if( i != idx){
      cache->cid[j] = (char *)malloc( MAX_LENOFCID);
      strcpy( cache->cid[j], tmp[i]);
      j++;
    }
    free( tmp[i]);
  }
  free( tmp);

  cache->numOfcid --;
}

void print_cache( cache_param_t *cache)
{
  int i;
  
  fprintf( stdout,"cache\n");
  fprintf( stdout,"\t filename: %s\n", cache->filename);
  fprintf( stdout,"\t csn: %d\n", cache->csn);
  fprintf( stdout,"\t cid:");

  for( i=0; i<cache->numOfcid; i++)
    fprintf( stdout," %s", cache->cid[i]);
  fprintf( stdout,"\n");
}

void print_allcache( cachelist_param_t *cachelist)
{
  cache_param_t *ptr;

  fprintf( stdout,"cache list\n");
  
  ptr = cachelist->first;
  while( ptr != NULL){
    print_cache( ptr);
    ptr=ptr->next;
  }
}
