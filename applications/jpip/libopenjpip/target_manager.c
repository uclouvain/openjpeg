/*
 * $Id: target_manager.c 44 2011-02-15 12:32:29Z kaori $
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "target_manager.h"

#ifdef SERVER
#include "fcgi_stdio.h"
#define logstream FCGI_stdout
#else
#define FCGI_stdout stdout
#define FCGI_stderr stderr
#define logstream stderr
#endif //SERVER


targetlist_param_t * gene_targetlist()
{
  targetlist_param_t *targetlist;

  targetlist = (targetlist_param_t *)malloc( sizeof(targetlist_param_t));
  
  targetlist->first = NULL;
  targetlist->last  = NULL;

  return targetlist;
}


/**
 * open jp2 format image file
 *
 * @param[in] filename file name (.jp2)
 * @return             file descriptor
 */
int open_jp2file( char filename[]);

target_param_t * gene_target( char *targetname)
{
  target_param_t *target;
  int fd;
  index_param_t *jp2idx;
  static int last_csn = 0;


  if( targetname[0]=='\0'){
    fprintf( FCGI_stderr, "Error: exception, no targetname in gene_target()\n");
    return NULL;
  }

  if((fd = open_jp2file( targetname)) == -1){
    fprintf( FCGI_stdout, "Status: 404\r\n"); 
    return NULL;
  }
  
  if( !(jp2idx = parse_jp2file( fd))){
    fprintf( FCGI_stdout, "Status: 501\r\n");
    return NULL;
  }

  target = (target_param_t *)malloc( sizeof(target_param_t));
  strcpy( target->filename, targetname); 
  target->fd = fd;
  target->csn = last_csn++;
  target->codeidx = jp2idx;
  
  target->next=NULL;

  return target;
}

void delete_target( target_param_t **target)
{
  close( (*target)->fd);
  delete_index ( &(*target)->codeidx);

#ifndef SERVER
  fprintf( logstream, "local log: target: %s deleted\n", (*target)->filename);
#endif
  free(*target);
}

void delete_target_in_list( target_param_t **target, targetlist_param_t *targetlist)
{
  target_param_t *ptr;

  if( *target == targetlist->first)
    targetlist->first = (*target)->next;
  else{
    ptr = targetlist->first;
    while( ptr->next != *target){
      ptr=ptr->next;
    }
    
    ptr->next = (*target)->next;
    
    if( *target == targetlist->last)
      targetlist->last = ptr;
  }
  delete_target( target);
}

void delete_targetlist(targetlist_param_t **targetlist)
{
  target_param_t *targetPtr, *targetNext;
  
  targetPtr = (*targetlist)->first;
  while( targetPtr != NULL){
    targetNext=targetPtr->next;
    delete_target( &targetPtr);
    targetPtr=targetNext;
  }
  free( *targetlist);
}

void print_alltarget( targetlist_param_t *targetlist)
{
  target_param_t *ptr;

  ptr = targetlist->first;
  while( ptr != NULL){
    fprintf( logstream,"csn=%d\n", ptr->csn);
    fprintf( logstream,"target=%s\n", ptr->filename);
    ptr=ptr->next;
  }
}

target_param_t * search_target( char targetname[], targetlist_param_t *targetlist)
{
  target_param_t *foundtarget;

  foundtarget = targetlist->first;
  
  while( foundtarget != NULL){
    
    if( strcmp( targetname, foundtarget->filename) == 0)
      return foundtarget;
      
    foundtarget = foundtarget->next;
  }
  return NULL;
}

int open_jp2file( char filename[])
{
  int fd;
  char *data;

  if( (fd = open( filename, O_RDONLY)) == -1){
    fprintf( FCGI_stdout, "Reason: Target %s not found\r\n", filename);
    return -1;
  }
  // Check resource is a JP family file.
  if( lseek( fd, 0, SEEK_SET)==-1){
    close(fd);
    fprintf( FCGI_stdout, "Reason: Target %s broken (lseek error)\r\n", filename);
    return -1;
  }
  
  data = (char *)malloc( 12); // size of header
  if( read( fd, data, 12) != 12){
    free( data);
    close(fd);
    fprintf( FCGI_stdout, "Reason: Target %s broken (read error)\r\n", filename);
    return -1;
  }
    
  if( *data || *(data + 1) || *(data + 2) ||
      *(data + 3) != 12 || strncmp (data + 4, "jP  \r\n\x87\n", 8)){
    free( data);
    close(fd);
    fprintf( FCGI_stdout, "Reason: No JPEG 2000 Signature box in target %s\r\n", filename);
    return -1;
  } 
  free( data);
  return fd;
}
