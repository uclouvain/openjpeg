/*
 * $Id: jptstream_manager.c 44 2011-02-15 12:32:29Z kaori $
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
#include <time.h>
#include "jptstream_manager.h"
#include "jp2k_decoder.h"
#include "imgreg_manager.h"

Byte_t * update_JPTstream( Byte_t *newjptstream, int newjptlen, Byte_t *cache_jptstream, int *jptlen)
{
  Byte_t *jptstream = (Byte_t *)malloc( (*jptlen)+newjptlen);
  if( *jptlen > 0)
    memcpy( jptstream, cache_jptstream, *jptlen);
  memcpy( jptstream+(*jptlen), newjptstream, newjptlen);
  *jptlen += newjptlen;

  if(cache_jptstream)
    free( cache_jptstream);
  
  return jptstream;
}

Byte_t * jpt_to_pnm( Byte_t *jptstream, msgqueue_param_t *msgqueue, Byte8_t csn, int fw, int fh, ihdrbox_param_t **ihdrbox)
{
  Byte_t *pnmstream;
  Byte_t *j2kstream; // j2k or jp2 codestream
  Byte8_t j2klen;
  int level = 0;
  
  if( *ihdrbox){
    // infinit value is set for maxmum level
    int fx = fw, fy = fh;
    int xmin = 0, ymin = 0;
    int xmax = (*ihdrbox)->width, ymax = (*ihdrbox)->height;
    find_level( 1000, &level, &fx, &fy, &xmin, &ymin, &xmax, &ymax);
  }
  
  j2kstream = recons_j2k( msgqueue, jptstream, csn, level+1, &j2klen);
  
  pnmstream = j2k_to_pnm( j2kstream, j2klen, ihdrbox);
  free( j2kstream);

  return pnmstream;
}

void save_codestream( Byte_t *codestream, Byte8_t streamlen, char *fmt)
{
  time_t timer;
  struct tm *t_st;
  char filename[20];
  FILE *fp;

  time(&timer);
  t_st = localtime( &timer);
  
  sprintf( filename, "%4d%02d%02d%02d%02d%02d.%.3s", t_st->tm_year+1900, t_st->tm_mon+1, t_st->tm_mday, t_st->tm_hour, t_st->tm_min, t_st->tm_sec, fmt);

  fp = fopen( filename, "wb");
  fwrite( codestream, streamlen, 1, fp);
  fclose( fp);
}
