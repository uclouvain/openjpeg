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

/*! \file
 *  \brief jpip_to_jp2 is a program to convert JPT- JPP- stream to JP2 file
 *
 *  \section impinst Implementing instructions
 *  This program takes two arguments. \n
 *   -# Input JPT or JPP file
 *   -# Output JP2 file\n
 *   % ./jpip_to_jp2 input.jpt output.jp2
 *   or
 *   % ./jpip_to_jp2 input.jpp output.jp2
 */

#include <stdio.h>
#include "openjpip.h"

int main(int argc,char *argv[])
{
  jpip_dec_param_t *dec;
    
  if( argc < 3){
    fprintf( stderr, "Too few arguments:\n");
    fprintf( stderr, " - input  jpt or jpp file\n");
    fprintf( stderr, " - output jp2 file\n");
    return -1;
  }
  
  dec = init_jpipdecoder( true);
  
  if(!( fread_jpip( argv[1], dec)))
    return -1;
  
  decode_jpip( dec);
  
  if(!(fwrite_jp2k( argv[2], dec)))
    return -1;

  output_log( true, false, true, dec);

  destroy_jpipdecoder( &dec);

  return 0;
}
