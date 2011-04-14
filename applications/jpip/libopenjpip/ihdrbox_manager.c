/*
 * $Id: ihdrbox_manager.c 44 2011-02-15 12:32:29Z kaori $
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
#include "ihdrbox_manager.h"

ihdrbox_param_t * get_ihdrbox( metadatalist_param_t *metadatalist, Byte_t *jptstream)
{
  ihdrbox_param_t *ihdrbox;
  metadata_param_t *meta;
  box_param_t *jp2h, *ihdr;
  
  jp2h = NULL;
  meta = metadatalist->first;
  while( meta){
    if( meta->boxlist){
      jp2h = search_box( "jp2h", meta->boxlist);
      if( jp2h)
	break;
    }
    meta = meta->next;
  }
  if( !jp2h){
    fprintf( stderr, "jp2h box not found\n");
    return NULL;
  }
  
  ihdr = gene_boxbyTypeinStream( jptstream, get_DBoxoff( jp2h), get_DBoxlen( jp2h), "ihdr");

  if( !ihdr){
    fprintf( stderr, "ihdr box not found\n");
    return NULL;
  }
  
  ihdrbox = (ihdrbox_param_t *)malloc( sizeof(ihdrbox_param_t));
  
  ihdrbox->height = big4( jptstream+get_DBoxoff(ihdr));
  ihdrbox->width  = big4( jptstream+get_DBoxoff(ihdr)+4);
  ihdrbox->nc     = big2( jptstream+get_DBoxoff(ihdr)+8);
  ihdrbox->bpc    = *(jptstream+get_DBoxoff(ihdr)+10)+1;

  free( ihdr);

  return ihdrbox;
}

