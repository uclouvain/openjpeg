/*
 * $Id: imgsock_manager.c 54 2011-05-10 13:22:47Z kaori $
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

#ifdef _WIN32
#include <windows.h>
#define strcasecmp  _stricmp
#else
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "imgsock_manager.h"

#define BUF_LEN 256

SOCKET open_listeningsocket()
{
  SOCKET listening_socket;
  struct sockaddr_in sin;
  int sock_optval = 1;
  int port = 5000;

  listening_socket = socket(AF_INET, SOCK_STREAM, 0);
  if ( listening_socket == -1 ){
    perror("socket");
    exit(1);
  }
  
  if ( setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR,
		  &sock_optval, sizeof(sock_optval)) == -1 ){
    perror("setsockopt");
    exit(1);
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = htonl(INADDR_ANY);

  if ( bind(listening_socket, (struct sockaddr *)&sin, sizeof(sin)) < 0 ){
    perror("bind");
    closesocket(listening_socket);
    exit(1);
  }

  if( listen(listening_socket, SOMAXCONN) == -1){
    perror("listen");
    closesocket(listening_socket);
    exit(1);
  }
  printf("port %d is listened\n", port);

  return listening_socket;
}

msgtype_t identify_clientmsg( SOCKET connected_socket)
{
  int receive_size;
  char buf[BUF_LEN];
  char *magicid[] = { "JPT-stream", "PNM request", "XML request", "CID request", "CID destroy", "JP2 save", "QUIT"};
  int i;
  
  receive_size = receive_line( connected_socket, buf);

  if( receive_size == 0){ 
    fprintf( stderr, "Error to receive the header of client message\n");
    return MSGERROR;
  }

  for( i=0; i<NUM_OF_MSGTYPES; i++){
    if( strncasecmp( magicid[i], buf, strlen(magicid[i])) == 0){
      printf("Client message: %s\n", magicid[i]);
      return i;
    }
  }
  
  fprintf( stderr, "Cannot identify client message type\n");
  return MSGERROR;
}

Byte_t * receive_JPTstream( SOCKET connected_socket, char *target, char *cid, int *streamlen)
{
  Byte_t *jptstream=NULL, *ptr;
  char buf[BUF_LEN], versionstring[] = "version 1.0";
  int linelen, redlen, remlen;
  
  target[0] = 0;
  cid[0] = 0;
  
  if((linelen = receive_line( connected_socket, buf)) == 0)
    return NULL;
  if( strncmp( versionstring, buf, strlen(versionstring))!=0){
    fprintf( stderr, "Wrong format\n");
    return NULL;
  }
  
  if((linelen = receive_line( connected_socket, buf)) == 0)
    return NULL;

  if( strstr( buf, "jp2")){ 
    // register cid option
    strcpy( target, buf);

    if((linelen = receive_line( connected_socket, buf)) == 0)
      return NULL;
    strcpy( cid, buf);
    
    if((linelen = receive_line( connected_socket, buf)) == 0)
      return NULL;
  }

  *streamlen = atoi( buf);
  fprintf( stderr, "Receiveing Data length: %d\n", *streamlen);
  
      
  jptstream = (unsigned  char *)malloc( (*streamlen));
  ptr = jptstream;
  remlen = (*streamlen);
  while( remlen > 0){
    redlen = recv( connected_socket, ptr, remlen, 0);
    if( redlen == -1){
      fprintf( stderr, "receive jptstream error\n");
      break;
    }
    remlen -= redlen;
    ptr = ptr + redlen;
  }
  fprintf( stderr, "    done\n");
    
  return jptstream;
}

void send_stream( SOCKET connected_socket, void *stream, int length);

void send_XMLstream( SOCKET connected_socket, Byte_t *xmlstream, int length)
{
  Byte_t header[5];
  
  header[0] = 'X';
  header[1] = 'M';
  header[2] = 'L';
  header[3] = (length >> 8) & 0xff;
  header[4] = length & 0xff;

  send_stream( connected_socket, header, 5);
  send_stream( connected_socket, xmlstream, length);
}

void send_CIDstream( SOCKET connected_socket, char *cid, int cidlen)
{
  Byte_t header[4];

  header[0] = 'C';
  header[1] = 'I';
  header[2] = 'D';
  header[3] = cidlen & 0xff;

  send_stream( connected_socket, header, 4);
  send_stream( connected_socket, cid, cidlen);
}

void send_PNMstream( SOCKET connected_socket, Byte_t *pnmstream, unsigned int width, unsigned int height, unsigned int numofcomp, Byte_t maxval)
{ 
  int pnmlen = 0;
  Byte_t header[7];

  pnmlen = width*height*numofcomp;
  
  header[0] = 'P';
  header[1] = numofcomp==3 ? 6:5;
  header[2] = (width >> 8) & 0xff;
  header[3] = width & 0xff;
  header[4] = (height >> 8) & 0xff;
  header[5] = height & 0xff;
  header[6] = maxval;

  send_stream( connected_socket, header, 7);
  send_stream( connected_socket, pnmstream, pnmlen);
}

void send_stream( SOCKET connected_socket, void *stream, int length)
{
  void *ptr = stream;
  int remlen = length;

  while( remlen > 0){
    int sentlen = send( connected_socket, ptr, remlen, 0);
    if( sentlen == -1){
      fprintf( stderr, "sending stream error\n");
      break;
    }
    remlen = remlen - sentlen;
    ptr = ptr + sentlen;
  }
}

int receive_line(SOCKET connected_socket, char *p)
{
  int len = 0;
  while (1){
    int ret;
    ret = recv( connected_socket, p, 1, 0);
    if ( ret == -1 ){
      perror("receive");
      exit(1);
    } else if ( ret == 0 ){
      break;
    }
    if ( *p == '\n' )
      break;
    p++;
    len++;
  }
  *p = '\0';

  if( len == 0)
    fprintf( stderr, "Header receive error\n");

  return len;
}

void response_signal( SOCKET connected_socket, bool succeed)
{
  Byte_t code;

  if( succeed)
    code = 1;
  else
    code = 0;

  if( send( connected_socket, &code, 1, 0) != 1)
    fprintf( stderr, "Response signalling error\n");
}
