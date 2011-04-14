/*
 * $Id: test_index.c 46 2011-02-17 14:50:55Z kaori $
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
 *  \brief test_index is a program to test the index code format of a JP2 file
 *
 *  \section impinst Implementing instructions
 *  This program takes one argument, and print out text type index information to the terminal. \n
 *   -# Input  JP2 file\n
 *   % ./test_index input.jp2\n
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "index_manager.h"


#ifdef SERVER
#include "fcgi_stdio.h"
#define logstream FCGI_stdout
#else
#define FCGI_stdout stdout
#define FCGI_stderr stderr
#define logstream stderr
#endif //SERVER


/**
 * Open JP2 file with the check of JP2 header
 *
 * @param[in] filename file name string
 * @return             file descriptor
 */
int myopen_jp2file( char filename[]);


int
main(int argc, char *argv[])
{
  int fd;
  index_param_t *jp2idx;
  
  if((fd = myopen_jp2file( argv[1])) == -1){
    fprintf( stderr, "jp2 file open error\n"); 
    return -1;
  }

  if( !(jp2idx = parse_jp2file( fd))){
    fprintf( FCGI_stdout, "Status: 501\r\n");
    return -1;
  }
  
  print_index( *jp2idx);

  delete_index( &jp2idx);

  return 0;
} /* main */



int myopen_jp2file( char filename[])
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

