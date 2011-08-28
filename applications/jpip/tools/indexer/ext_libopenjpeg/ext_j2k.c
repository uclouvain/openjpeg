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
#include "ext_openjpeg.h"
#include "cio.h"
#include "j2k.h"

/* 
 * Get main headr marker size
 *
 * @param[in] type      marker type
 * @param[in] cstr_info codestream information
 * @param[in] cio       file input handle
 * @return              marker size
 */
unsigned short get_mh_markersize( unsigned short type, opj_codestream_info_t cstr_info, opj_cio_t *cio);

void add_mainheader_marker_info( opj_cio_t *cio, opj_codestream_info_t *cstr_info)
{
  opj_marker_info_t marker;
  int pos;

  cstr_info->marker = (opj_marker_info_t *)malloc( 100*sizeof(opj_marker_info_t)); // max 100
  
  pos = cstr_info->main_head_start;
  cio_seek( cio, pos);
  
  while( pos <= cstr_info->main_head_end){
    marker.type = cio_read( cio, 2);
    marker.pos  = cio_tell( cio);
    marker.len  = get_mh_markersize( marker.type, *cstr_info, cio);
    cio_skip( cio, marker.len);
    
    cstr_info->marker[ cstr_info->marknum] = marker;

    cstr_info->marknum++;
    pos = cio_tell( cio);
  }
}

unsigned short get_mh_markersize( unsigned short type, opj_codestream_info_t cstr_info, opj_cio_t *cio)
{
  unsigned short siz;
  int pos;
  
  siz = 0;

  switch( type){
  case J2K_MS_SOC:
    siz = 0;
    break;
  case J2K_MS_SIZ:
    siz = 38+3*cstr_info.numcomps;
    break;
  case J2K_MS_COD:
  case J2K_MS_QCD:
  case J2K_MS_COM:
    pos = cio_tell( cio);
    siz = cio_read( cio, 2);
    cio_seek( cio, pos);
    break;
  default:
    fprintf( stderr, "marker %x length not defined yet!\n", type);
  }
  return siz;
}
