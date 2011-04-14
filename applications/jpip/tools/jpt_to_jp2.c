/*
 * $Id: jpt_to_jp2.c 46 2011-02-17 14:50:55Z kaori $
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
 *  \brief jpt_to_jp2 is a program to convert JPT-stream to JP2 file
 *
 *  \section impinst Implementing instructions
 *  This program takes two arguments. \n
 *   -# Input JPT file
 *   -# Output JP2 file\n
 *   % ./jpt_to_jp2 input.jpt output.jp2
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "msgqueue_manager.h"
#include "byte_manager.h"
#include "ihdrbox_manager.h"
#include "metadata_manager.h"

int main(int argc,char *argv[])
{
  msgqueue_param_t *msgqueue;
  int infd, outfd;
  Byte8_t jptlen, jp2len;
  struct stat sb;
  Byte_t *jptstream, *jp2stream;
  metadatalist_param_t *metadatalist;
  ihdrbox_param_t *ihdrbox;
    
  if( argc < 3){
    fprintf( stderr, "Too few arguments:\n");
    fprintf( stderr, " - input  jpt file\n");
    fprintf( stderr, " - output jp2 file\n");
    return -1;
  }

  if(( infd = open( argv[1], O_RDONLY)) == -1){
    fprintf( stderr, "file %s not exist\n", argv[1]);
    return -1;
  }
  
  if( fstat( infd, &sb) == -1){
    fprintf( stderr, "input file stream is broken\n");
    return -1;
  }
  jptlen = (Byte8_t)sb.st_size;

  jptstream = (Byte_t *)malloc( jptlen);

  if( read( infd, jptstream, jptlen) != jptlen){
    fprintf( stderr, "file reading error\n");
    free( jptstream);
    return -1;
  }
  close(infd);

  metadatalist = gene_metadatalist();
  msgqueue = gene_msgqueue( true, NULL);
  parse_stream( jptstream, jptlen, 0, msgqueue);
  parse_metamsg( msgqueue, jptstream, jptlen, metadatalist);
  print_msgqueue( msgqueue);
  //print_allmetadata( metadatalist);

  ihdrbox = get_ihdrbox( metadatalist, jptstream);

  printf("W*H: %d*%d\n", ihdrbox->height, ihdrbox->width);
  printf("NC: %d, bpc: %d\n", ihdrbox->nc, ihdrbox->bpc);
  
  jp2stream = recons_jp2( msgqueue, jptstream, msgqueue->first->csn, &jp2len);
    
  if(( outfd = open( argv[2], O_WRONLY|O_CREAT, S_IRWXU|S_IRWXG)) == -1){
    fprintf( stderr, "file %s open error\n", argv[2]);
    return -1;
  }
  
  if( write( outfd, jp2stream, jp2len) != jp2len)
    fprintf( stderr, "jp2 file write error\n");

  //print_msgqueue( msgqueue);

  free( ihdrbox);
  free( jp2stream);
  close(outfd);
  
  delete_msgqueue( &msgqueue);
  delete_metadatalist( &metadatalist);

  free( jptstream);

  return 0;
}
