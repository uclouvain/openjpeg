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
 *  \brief jpip_to_j2k is a program to convert JPT- JPP- stream to J2K file
 *
 *  \section impinst Implementing instructions
 *  This program takes two arguments. \n
 *   -# Input  JPT or JPP file
 *   -# Output J2K file\n
 *   % ./jpip_to_j2k input.jpt output.j2k
 *   or
 *   % ./jpip_to_j2k input.jpp output.j2k
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "msgqueue_manager.h"


int main(int argc,char *argv[])
{
  msgqueue_param_t *msgqueue;
  int infd, outfd;
  Byte8_t jpiplen, j2klen;
  struct stat sb;
  Byte_t *jpipstream, *j2kstream;
  
  if( argc < 3){
    fprintf( stderr, "Too few arguments:\n");
    fprintf( stderr, " - input  jpt or jpp file\n");
    fprintf( stderr, " - output j2k file\n");
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
  jpiplen = (Byte8_t)sb.st_size;

  jpipstream = (Byte_t *)malloc( jpiplen);

  if( read( infd, jpipstream, jpiplen) != jpiplen){
    fprintf( stderr, "file reading error\n");
    free( jpipstream);
    return -1;
  }
  close(infd);

  msgqueue = gene_msgqueue( true, NULL);
  parse_JPIPstream( jpipstream, jpiplen, 0, msgqueue);
  
  //print_msgqueue( msgqueue);

  j2kstream = recons_j2k( msgqueue, jpipstream, msgqueue->first->csn, 0, &j2klen);
  
  delete_msgqueue( &msgqueue);
  free( jpipstream);

#ifdef _WIN32
  if(( outfd = open( argv[2], O_WRONLY|O_CREAT, _S_IREAD | _S_IWRITE)) == -1){
#else
  if(( outfd = open( argv[2], O_WRONLY|O_CREAT, S_IRWXU|S_IRWXG)) == -1){
#endif
    fprintf( stderr, "file %s open error\n", argv[2]);
    return -1;
  }
  
  if( write( outfd, j2kstream, j2klen) != j2klen)
    fprintf( stderr, "j2k file write error\n");

  free( j2kstream);
  close(outfd);

  return 0;
}
