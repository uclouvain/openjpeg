/*
 * $Id: msgqueue_manager.c 53 2011-05-09 16:55:39Z kaori $
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
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include "msgqueue_manager.h"
#include "metadata_manager.h"


#ifdef SERVER
#include "fcgi_stdio.h"
#define logstream FCGI_stdout
#else
#define FCGI_stdout stdout
#define FCGI_stderr stderr
#define logstream stderr
#endif //SERVER

#define TILE_MSG 4
#define EXT_TILE_MSG 5
#define MAINHEADER_MSG 6
#define METADATA_MSG 8

msgqueue_param_t * gene_msgqueue( bool stateless, target_param_t *target)
{
  msgqueue_param_t *msgqueue;

  msgqueue = (msgqueue_param_t *)malloc( sizeof(msgqueue_param_t));

  msgqueue->first = NULL;
  msgqueue->last  = NULL;

  msgqueue->stateless = stateless;
  msgqueue->target = target;
  
  return msgqueue;
}

void delete_msgqueue( msgqueue_param_t **msgqueue)
{
  message_param_t *ptr, *next;

  if( !(*msgqueue))
    return;
  
  ptr = (*msgqueue)->first;

  while( ptr){
    next = ptr->next;
    free( ptr);
    ptr = next;
  }
  if( (*msgqueue)->stateless && (*msgqueue)->target)
    delete_target( &((*msgqueue)->target));

  free(*msgqueue); 
}

void print_msgqueue( msgqueue_param_t *msgqueue)
{
  message_param_t *ptr;

  if( !msgqueue)
    return;

  fprintf( logstream, "message queue:\n");
  ptr = msgqueue->first;

  while( ptr){
    fprintf( logstream, "\t class_id: %lld\n", ptr->class_id );
    fprintf( logstream, "\t in_class_id: %lld\n", ptr->in_class_id );
    fprintf( logstream, "\t csn: %lld\n", ptr->csn );
    fprintf( logstream, "\t bin_offset: %#llx\n", ptr->bin_offset );
    fprintf( logstream, "\t length: %#llx\n", ptr->length );
    fprintf( logstream, "\t aux: %lld\n", ptr->aux );
    fprintf( logstream, "\t last_byte: %d\n", ptr->last_byte );
    if( ptr->phld)
      print_placeholder( ptr->phld);
    else
      fprintf( logstream, "\t res_offset: %#llx\n", ptr->res_offset );
    fprintf( logstream, "\n");

    ptr = ptr->next;
  }
}

void enqueue_message( message_param_t *msg, msgqueue_param_t *msgqueue);

void enqueue_mainheader( msgqueue_param_t *msgqueue)
{
  target_param_t *target;
  message_param_t *msg;

  target = msgqueue->target;

  msg = (message_param_t *)malloc( sizeof(message_param_t));

  msg->last_byte = true;
  msg->in_class_id = 0;
  msg->class_id = MAINHEADER_MSG;
  msg->csn = target->csn;
  msg->bin_offset = 0;
  msg->length = target->codeidx->mhead_length;
  msg->aux = 0; // non exist
  msg->res_offset = target->codeidx->offset;
  msg->phld = NULL;
  msg->next = NULL;

  enqueue_message( msg, msgqueue);

  target->codeidx->mhead_model = true;
}

void enqueue_tile( int tile_id, int level, msgqueue_param_t *msgqueue)
{
  target_param_t *target;
  bool *tp_model;
  Byte8_t numOftparts; // num of tile parts par tile
  Byte8_t numOftiles;
  index_param_t *codeidx;
  faixbox_param_t *tilepart;
  message_param_t *msg;
  Byte8_t binOffset, binLength;
  int i;

  target = msgqueue->target;
  codeidx  = target->codeidx;
  tilepart = codeidx->tilepart;
  
  numOftparts = get_nmax( tilepart);
  numOftiles  = get_m( tilepart);

  if( tile_id < 0 || numOftiles <= tile_id){
    fprintf( FCGI_stderr, "Error, Invalid tile-id %d\n", tile_id);
    return;
  }
  
  tp_model = &codeidx->tp_model[ tile_id*numOftparts];
  
  binOffset=0;
  for( i=0; i<numOftparts-level; i++){
    binLength = get_elemLen( tilepart, i, tile_id);
    
    if( !tp_model[i]){
      msg = (message_param_t *)malloc( sizeof(message_param_t));
      
      msg->last_byte = i==numOftparts-1? true : false;
      msg->in_class_id = tile_id;
#if 0
      msg->class_id = TILE_MSG;
#else
      msg->class_id = EXT_TILE_MSG;
#endif
      msg->csn = target->csn;
      msg->bin_offset = binOffset;
      msg->length = binLength;
      msg->aux = numOftparts-i;
      msg->res_offset = codeidx->offset+get_elemOff( tilepart, i, tile_id)/*-1*/;
      msg->phld = NULL;
      msg->next = NULL;

      enqueue_message( msg, msgqueue);

      tp_model[i] = true;
    }
    binOffset += binLength;
  }
}

void enqueue_box(  int meta_id, boxlist_param_t *boxlist, msgqueue_param_t *msgqueue, Byte8_t *binOffset);
void enqueue_phld( int meta_id, placeholderlist_param_t *phldlist, msgqueue_param_t *msgqueue, Byte8_t *binOffset);
void enqueue_boxcontents( int meta_id, boxcontents_param_t *boxcontents, msgqueue_param_t *msgqueue, Byte8_t *binOffset);

void enqueue_metadata( int meta_id, msgqueue_param_t *msgqueue)
{
  metadatalist_param_t *metadatalist;
  metadata_param_t *metadata;
  Byte8_t binOffset;

  metadatalist = msgqueue->target->codeidx->metadatalist;
  metadata = search_metadata( meta_id, metadatalist);
  
  if( !metadata){
    fprintf( FCGI_stderr, "Error: metadata-bin %d not found\n", meta_id);
    return;
  }
  binOffset = 0;
  
  if( metadata->boxlist)
    enqueue_box( meta_id, metadata->boxlist, msgqueue, &binOffset);

  if( metadata->placeholderlist)
    enqueue_phld( meta_id, metadata->placeholderlist, msgqueue, &binOffset);

  if( metadata->boxcontents)
    enqueue_boxcontents( meta_id, metadata->boxcontents, msgqueue, &binOffset);
  
  msgqueue->last->last_byte = true;
}

message_param_t * gene_metamsg( int meta_id, Byte8_t binoffset, Byte8_t length, Byte8_t res_offset, placeholder_param_t *phld, Byte8_t csn);

void enqueue_box( int meta_id, boxlist_param_t *boxlist, msgqueue_param_t *msgqueue, Byte8_t *binOffset)
{
  box_param_t *box;
  message_param_t *msg;
  
  box = boxlist->first;
  while( box){
    msg = gene_metamsg( meta_id, *binOffset, box->length, box->offset, NULL, msgqueue->target->csn);
    enqueue_message( msg, msgqueue);

    *binOffset += box->length;
    box = box->next;
  }
}

void enqueue_phld( int meta_id, placeholderlist_param_t *phldlist, msgqueue_param_t *msgqueue, Byte8_t *binOffset)
{
  placeholder_param_t *phld;
  message_param_t *msg;
  
  phld = phldlist->first;
  while( phld){
    msg = gene_metamsg( meta_id, *binOffset, phld->LBox, 0, phld, msgqueue->target->csn);
    enqueue_message( msg, msgqueue);

    *binOffset += phld->LBox;
    phld = phld->next;
  }
}

void enqueue_boxcontents( int meta_id, boxcontents_param_t *boxcontents, msgqueue_param_t *msgqueue, Byte8_t *binOffset)
{
  message_param_t *msg;

  msg = gene_metamsg( meta_id, *binOffset, boxcontents->length, boxcontents->offset, NULL, msgqueue->target->csn);
  enqueue_message( msg, msgqueue);
  
  *binOffset += boxcontents->length;
}

message_param_t * gene_metamsg( int meta_id, Byte8_t binOffset, Byte8_t length, Byte8_t res_offset, placeholder_param_t *phld, Byte8_t csn)
{
  message_param_t *msg;

  msg = (message_param_t *)malloc( sizeof(message_param_t));
    
  msg->last_byte = false;
  msg->in_class_id = meta_id;
  msg->class_id = METADATA_MSG;
  msg->csn = csn;
  msg->bin_offset = binOffset;
  msg->length = length;
  msg->aux = 0; // non exist
  msg->res_offset = res_offset;
  msg->phld = phld;
  msg->next = NULL;

  return msg;
}

void enqueue_message( message_param_t *msg, msgqueue_param_t *msgqueue)
{
  if( msgqueue->first)
    msgqueue->last->next = msg;
  else
    msgqueue->first = msg;
  
  msgqueue->last = msg;
}


void emit_bin_id_vbas( Byte_t bb, Byte_t c, Byte8_t in_class_id);
void emit_vbas( Byte8_t code);
void emit_body( message_param_t *msg, int fd);
void emit_placeholder( placeholder_param_t *phld);

void emit_stream_from_msgqueue( msgqueue_param_t *msgqueue)
{
  message_param_t *msg;
  Byte8_t class_id, csn;
  Byte_t bb, c;
  
  if( !(msgqueue))
    return;

  msg = msgqueue->first;
  class_id = 0;
  csn = 0;
  while( msg){
    if( msg->csn == csn){
      if( msg->class_id == class_id)
	bb = 1;
      else{
	bb = 2;
	class_id = msg->class_id;
      }
    }
    else{
      bb = 3;
      class_id = msg->class_id;
      csn = msg->csn;
    }

    c = msg->last_byte ? 1 : 0;
      
    emit_bin_id_vbas( bb, c, msg->in_class_id);
    
    if( bb >= 2)
      emit_vbas( class_id);
    if (bb == 3)
      emit_vbas( csn);
    
    emit_vbas( msg->bin_offset);
    emit_vbas (msg->length);
    
    if( msg->class_id%2) // Aux is present only if the id is odd
      emit_vbas( msg->aux);

    if( msg->phld)
      emit_placeholder( msg->phld);
    else
      emit_body( msg, msgqueue->target->fd);

    msg = msg->next;
  }
}

void emit_vbas_with_bytelen( Byte8_t code, int bytelength);
void print_binarycode( Byte8_t n, int segmentlen);

void emit_bin_id_vbas( Byte_t bb, Byte_t c, Byte8_t in_class_id)
{
  int bytelength;
  Byte8_t tmp;

  // A.2.3 In-class identifiers 
  // 7k-3bits, where k is the number of bytes in the VBAS
  bytelength = 1;
  tmp = in_class_id >> 4;
  while( tmp){
    bytelength ++;
    tmp >>= 7;
  }

  in_class_id |= (((bb & 3) << 5) | (c & 1) << 4) << ((bytelength-1)*7);
  
  emit_vbas_with_bytelen( in_class_id, bytelength);
}

void emit_vbas( Byte8_t code)
{
  int bytelength;
  Byte8_t tmp;

  bytelength = 1;
  tmp = code;
  while( tmp >>= 7)
    bytelength ++;

  emit_vbas_with_bytelen( code, bytelength);
}

void emit_vbas_with_bytelen( Byte8_t code, int bytelength)
{
  int n;
  Byte8_t seg;
  
  n = bytelength - 1;
  while( n >= 0) {
    seg = ( code >> (n*7)) & 0x7f;
    if( n)
      seg |= 0x80;
    fputc(( Byte4_t)seg, FCGI_stdout);
    n--;
  }
}

void emit_body( message_param_t *msg, int fd)
{
  Byte_t *data;

  if( lseek( fd, msg->res_offset, SEEK_SET)==-1){
    fprintf( FCGI_stderr, "Error: fseek in emit_body()\n");
    return;
  }
  
  data = (Byte_t *)malloc( msg->length);
  if( read( fd, data, msg->length) != msg->length){
    free( data);
    fprintf( FCGI_stderr, "Error: fread in emit_body()\n");
    return;
  }

  if( fwrite( data, msg->length, 1, FCGI_stdout) < 1){
    free( data);
    fprintf( FCGI_stderr, "Error: fwrite in emit_body()\n");
    return;
  }
  free(data);
}

void emit_bigendian_bytes( Byte8_t code, int bytelength);

void emit_placeholder( placeholder_param_t *phld)
{
  emit_bigendian_bytes( phld->LBox, 4);
  if( fwrite( phld->TBox, 4, 1, FCGI_stdout) < 1){
    fprintf( FCGI_stderr, "Error: fwrite in emit_placeholder()\n");
    return;
  }
  emit_bigendian_bytes( phld->Flags, 4);
  emit_bigendian_bytes( phld->OrigID, 8);

  if( fwrite( phld->OrigBH, phld->OrigBHlen, 1, FCGI_stdout) < 1){
    fprintf( FCGI_stderr, "Error: fwrite in emit_placeholder()\n");
    return;
  }
}

void emit_bigendian_bytes( Byte8_t code, int bytelength)
{
  int n;
  Byte8_t seg;
  
  n = bytelength - 1;
  while( n >= 0) {
    seg = ( code >> (n*8)) & 0xff;
    fputc(( Byte4_t)seg, FCGI_stdout);
    n--;
  }
}
void print_binarycode( Byte8_t n, int segmentlen)
{
  char buf[256];
  int i=0, j, k;

  do{
    buf[i++] = n%2 ? '1' : '0';
  }while((n=n/2));

  for( j=segmentlen-1; j>=i; j--)
    putchar('0');
  
  for( j=i-1, k=0; j>=0; j--, k++){
    putchar( buf[j]);
    if( !((k+1)%segmentlen))
      printf(" ");
  }
  printf("\n");
}

Byte_t * parse_bin_id_vbas( Byte_t *streamptr, Byte_t *bb, Byte_t *c, Byte8_t *in_class_id);
Byte_t * parse_vbas( Byte_t *streamptr, Byte8_t *elem);

void parse_stream( Byte_t *stream, Byte8_t jptlen, Byte8_t offset, msgqueue_param_t *msgqueue)
{
  Byte_t *ptr;  // stream pointer
  message_param_t *msg;
  Byte_t bb, c;
  Byte8_t class_id, csn;

  class_id = -1; // dummy
  csn = 0;
  ptr = stream;
  while( ptr-stream < jptlen){
    msg = (message_param_t *)malloc( sizeof(message_param_t));
    
    ptr = parse_bin_id_vbas( ptr, &bb, &c, &msg->in_class_id);
    
    msg->last_byte   = c == 1 ? true : false;
    
    if( bb >= 2){
      ptr = parse_vbas( ptr, &class_id);
      //      fprintf( stdout, "class_id: %lld\n", class_id);
    }
    msg->class_id = class_id;
    
    if (bb == 3)
      ptr = parse_vbas( ptr, &csn);
    msg->csn = csn;
    
    ptr = parse_vbas( ptr, &msg->bin_offset);
    ptr = parse_vbas( ptr, &msg->length);
    
    if( msg->class_id%2) // Aux is present only if the id is odd
      ptr = parse_vbas( ptr, &msg->aux);
    else
      msg->aux = 0;
    
    msg->res_offset = ptr-stream+offset;   
    msg->phld = NULL;
    msg->next = NULL;

    if(msgqueue->first)
      msgqueue->last->next = msg;
    else
      msgqueue->first = msg;
    msgqueue->last = msg;
    
    ptr += msg->length;
  }
}

void parse_metadata( metadata_param_t *metadata, message_param_t *msg, Byte_t *stream);

void parse_metamsg( msgqueue_param_t *msgqueue, Byte_t *stream, Byte8_t jptlen, metadatalist_param_t *metadatalist)
{
  message_param_t *msg;

  if( metadatalist == NULL)
    return;
  
  msg = msgqueue->first;
  while( msg){
    if( msg->class_id == METADATA_MSG){
      metadata_param_t *metadata = gene_metadata( msg->in_class_id, NULL, NULL, NULL);
      insert_metadata_into_list( metadata, metadatalist);
      parse_metadata( metadata, msg, stream+msg->res_offset);
    }
    msg = msg->next;
  }
}

placeholder_param_t * parse_phld( Byte_t *datastream, Byte8_t metalength);

void parse_metadata( metadata_param_t *metadata, message_param_t *msg, Byte_t *datastream)
{
  char *boxtype = (char *)(datastream+4);

  msg->phld = NULL;

  if( strncmp( boxtype, "phld", 4) == 0){
    if( !metadata->placeholderlist)
	metadata->placeholderlist = gene_placeholderlist();
    
    placeholder_param_t *phld = parse_phld( datastream, msg->length);
    msg->phld = phld;
    insert_placeholder_into_list( phld, metadata->placeholderlist);
  }
  else if( isalpha(boxtype[0]) && isalpha(boxtype[1]) &&
	   (isalnum(boxtype[2])||isblank(boxtype[2])) &&
	   (isalpha(boxtype[3])||isblank(boxtype[3]))){
    if( !metadata->boxlist)
      metadata->boxlist = gene_boxlist();
    
    box_param_t *box = gene_boxbyOffinStream( datastream, msg->res_offset);
    insert_box_into_list( box, metadata->boxlist);
  }
  else
    metadata->boxcontents = gene_boxcontents( msg->res_offset, msg->length);
}

placeholder_param_t * parse_phld( Byte_t *datastream, Byte8_t metalength)
{
  placeholder_param_t *phld;

  phld = (placeholder_param_t *)malloc( sizeof(placeholder_param_t));
  
  phld->LBox = big4( datastream);
  strcpy( phld->TBox, "phld");
  phld->Flags = big4( datastream+8);
  phld->OrigID = big8( datastream+12);
  phld->OrigBHlen = metalength - 20;
  phld->OrigBH = (Byte_t *)malloc(phld->OrigBHlen);
  memcpy( phld->OrigBH, datastream+20, phld->OrigBHlen);
  phld->next = NULL;

  return phld;
}

Byte_t * parse_bin_id_vbas( Byte_t *streamptr, Byte_t *bb, Byte_t *c, Byte8_t *in_class_id)
{
  Byte_t code;
  Byte_t *ptr;

  ptr = streamptr;
  code = *(ptr++);

  *bb = (code >> 5) & 3;
  *c  = (code >> 4) & 1;
  
  *in_class_id = code & 15;

  while(code >> 7){
    code = *(ptr++);
    *in_class_id = (*in_class_id << 7) | (code & 0x7f);
  }
  return ptr;
}

Byte_t * parse_vbas( Byte_t *streamptr, Byte8_t *elem)
{
  Byte_t code;
  Byte_t *ptr;
  
  *elem = 0;
  ptr = streamptr;
  do{
    code = *(ptr++);
    *elem = (*elem << 7) | (code & 0x7f);
  }while(code >> 7);
  
  return ptr;
}

/**
 * search a message by class_id
 *
 * @param[in] class_id    class identifiers 
 * @param[in] in_class_id in-class identifiers, -1 means any
 * @param[in] csn         codestream number
 * @param[in] msg         first message pointer of the searching list
 * @return                found message pointer
 */
message_param_t * search_message( Byte8_t class_id, Byte8_t in_class_id, Byte8_t csn, message_param_t *msg);


/**
 * delete a message in msgqueue
 *
 * @param[in] message  address of the deleting message pointer
 * @param[in] msgqueue message queue pointer
 */
void delete_message_in_msgqueue( message_param_t **message, msgqueue_param_t *msgqueue);

Byte_t * recons_codestream( msgqueue_param_t *msgqueue, Byte_t *stream, Byte8_t csn, int minlev, Byte8_t *codelen);

// usable only to JPT-stream messages
Byte_t * recons_j2k( msgqueue_param_t *msgqueue, Byte_t *stream, Byte8_t csn, int minlev, Byte8_t *j2klen)
{
  Byte_t *j2kstream = NULL;
  
  if( !msgqueue)
    return NULL;
  
  j2kstream = recons_codestream( msgqueue, stream, csn, minlev, j2klen);

  return j2kstream;
}

Byte_t * add_emptyboxstream( placeholder_param_t *phld, Byte_t *jp2stream, Byte8_t *jp2len);
Byte_t * add_msgstream( message_param_t *message, Byte_t *origstream, Byte_t *j2kstream, Byte8_t *j2klen);

Byte_t * recons_jp2( msgqueue_param_t *msgqueue, Byte_t *stream, Byte8_t csn, Byte8_t *jp2len)
{
  message_param_t *ptr;
  Byte_t *jp2stream = NULL;
  Byte_t *codestream = NULL;
  Byte8_t codelen;
  Byte8_t jp2cDBoxOffset = 0, jp2cDBoxlen = 0;
  
  *jp2len = 0;

  if( !msgqueue)
    return NULL;
    
  ptr = msgqueue->first;
  while(( ptr = search_message( METADATA_MSG, -1, csn, ptr))!=NULL){
    if( ptr->phld){
      if( strncmp( (char *)ptr->phld->OrigBH+4, "jp2c", 4) == 0){
	jp2cDBoxOffset = *jp2len + ptr->phld->OrigBHlen;
	jp2stream = add_emptyboxstream( ptr->phld, jp2stream, jp2len); // header only
	jp2cDBoxlen = *jp2len - jp2cDBoxOffset;
      }
      else
	jp2stream = add_emptyboxstream( ptr->phld, jp2stream, jp2len); // header only
    }
    jp2stream = add_msgstream( ptr, stream, jp2stream, jp2len);
    ptr = ptr->next;
  }
  
  codestream = recons_codestream( msgqueue, stream, csn, 0, &codelen);
  
  if( jp2cDBoxOffset != 0 && codelen <= jp2cDBoxlen)
    memcpy( jp2stream+jp2cDBoxOffset, codestream, codelen);

  free( codestream);
  
  return jp2stream;
}

int get_last_tileID( msgqueue_param_t *msgqueue, Byte8_t csn);
Byte_t * add_emptytilestream( const int tileID, Byte_t *j2kstream, Byte8_t *j2klen);
Byte_t * add_EOC( Byte_t *j2kstream, Byte8_t *j2klen);

Byte_t * recons_codestream( msgqueue_param_t *msgqueue, Byte_t *stream, Byte8_t csn, int minlev, Byte8_t *codelen)
{
  message_param_t *ptr;
  Byte_t *codestream = NULL;
  int last_tileID;
  int tileID;
  
  *codelen = 0;

  // main header first
  ptr = msgqueue->first;
  while(( ptr = search_message( MAINHEADER_MSG, -1, csn, ptr))!=NULL){
    codestream = add_msgstream( ptr, stream, codestream, codelen);
    ptr = ptr->next;
  }

  last_tileID = get_last_tileID( msgqueue, csn); 
  
  for( tileID=0; tileID <= last_tileID; tileID++){
    bool found = false;
    ptr = msgqueue->first;
    while(( ptr = search_message( TILE_MSG, tileID, csn, ptr))!=NULL){
      found = true;
      codestream = add_msgstream( ptr, stream, codestream, codelen);
      ptr = ptr->next;
    }
    ptr = msgqueue->first;
    while(( ptr = search_message( EXT_TILE_MSG, tileID, csn, ptr))!=NULL){
      if( ptr->aux >= minlev){
	found = true;
	codestream = add_msgstream( ptr, stream, codestream, codelen);
      }
      ptr = ptr->next;
    }
    if(!found)
      codestream = add_emptytilestream( tileID, codestream, codelen);
  }
  codestream = add_EOC( codestream, codelen);
  
  return codestream;
}

int get_last_tileID( msgqueue_param_t *msgqueue, Byte8_t csn)
{
  int last_tileID = 0;
  message_param_t *msg;
  
  msg = msgqueue->first;
  while( msg){
    if((msg->class_id == TILE_MSG || msg->class_id == EXT_TILE_MSG) && msg->csn == csn && last_tileID < msg->in_class_id)
      last_tileID = msg->in_class_id;
    msg = msg->next;
  }
  return last_tileID;
}

message_param_t * search_message( Byte8_t class_id, Byte8_t in_class_id, Byte8_t csn, message_param_t *msg)
{
  while( msg != NULL){
    if( in_class_id == -1){
      if( msg->class_id == class_id && msg->csn == csn)
	return msg;
    }
    else{
      if( msg->class_id == class_id && msg->in_class_id == in_class_id && msg->csn == csn)
	return msg;
    }
    msg = msg->next;
  }
  return NULL;
}

void delete_message_in_msgqueue( message_param_t **msg, msgqueue_param_t *msgqueue)
{
  message_param_t *ptr;

  if( !(*msg))
    return;

  if( *msg == msgqueue->first)
    msgqueue->first = (*msg)->next;
  else{
    ptr = msgqueue->first;
    while( ptr->next != *msg){
      ptr=ptr->next;
    }
    
    ptr->next = (*msg)->next;
    
    if( *msg == msgqueue->last)
      msgqueue->last = ptr;
  }
  free( *msg);
}

Byte_t * gene_msgstream( message_param_t *message, Byte_t *stream, Byte8_t *length);
Byte_t * gene_emptytilestream( const int tileID, Byte8_t *length);


Byte_t * add_msgstream( message_param_t *message, Byte_t *origstream, Byte_t *j2kstream, Byte8_t *j2klen)
{
  Byte_t *newstream;
  Byte8_t newlen;
  Byte_t *buf;

  if( !message)
    return NULL;

  newstream = gene_msgstream( message, origstream, &newlen);

  buf = (Byte_t *)malloc(( *j2klen)+newlen);

  memcpy( buf, j2kstream, *j2klen);
  memcpy( buf+(*j2klen), newstream, newlen);
  
  *j2klen += newlen;
  
  free( newstream);
  if(j2kstream) free(j2kstream);

  return buf;
}
 

Byte_t * add_emptyboxstream( placeholder_param_t *phld, Byte_t *jp2stream, Byte8_t *jp2len)
{
  Byte_t *newstream;
  Byte8_t newlen;
  Byte_t *buf;
  
  if( phld->OrigBHlen == 8)
    newlen = big4(phld->OrigBH);
  else
    newlen = big8(phld->OrigBH+8);

  newstream = (Byte_t *)malloc( newlen);
  memset( newstream, 0, newlen);
  memcpy( newstream, phld->OrigBH, phld->OrigBHlen);

  buf = (Byte_t *)malloc(( *jp2len)+newlen);

  memcpy( buf, jp2stream, *jp2len);
  memcpy( buf+(*jp2len), newstream, newlen);
  
  *jp2len += newlen;
  
  free( newstream);
  if(jp2stream) free(jp2stream);

  return buf;
}

Byte_t * add_emptytilestream( const int tileID, Byte_t *j2kstream, Byte8_t *j2klen)
{
  Byte_t *newstream;
  Byte8_t newlen;
  Byte_t *buf;

  newstream = gene_emptytilestream( tileID, &newlen);

  buf = (Byte_t *)malloc(( *j2klen)+newlen);

  memcpy( buf, j2kstream, *j2klen);
  memcpy( buf+(*j2klen), newstream, newlen);
  
  *j2klen += newlen;

  free( newstream);
  if(j2kstream) free(j2kstream);

  return buf;
}

Byte_t * add_EOC( Byte_t *j2kstream, Byte8_t *j2klen)
{
  Byte2_t EOC = 0xd9ff;

  Byte_t *buf;

  buf = (Byte_t *)malloc(( *j2klen)+2);

  memcpy( buf, j2kstream, *j2klen);
  memcpy( buf+(*j2klen), &EOC, 2);

  *j2klen += 2;

  if(j2kstream) free(j2kstream);

  return buf;
}

Byte_t * gene_msgstream( message_param_t *message, Byte_t *stream, Byte8_t *length)
{
  Byte_t *buf;

  if( !message)
    return NULL;

  *length = message->length;
  buf = (Byte_t *)malloc( *length);
  memcpy( buf, stream+message->res_offset,  *length);

  return buf;
}

Byte_t * gene_emptytilestream( const int tileID, Byte8_t *length)
{
  Byte_t *buf;
  const Byte2_t SOT = 0x90ff;
  const Byte2_t Lsot = 0xa << 8;
  Byte2_t Isot;
  const Byte4_t Psot = 0xe << 24;
  const Byte_t TPsot = 0, TNsot = 0;
  const Byte2_t SOD = 0x93ff;

  *length = 14;
  buf = (Byte_t *)malloc(*length);

  Isot = tileID << 8;
  
  memcpy( buf, &SOT, 2);
  memcpy( buf+2, &Lsot, 2);
  memcpy( buf+4, &Isot, 2);
  memcpy( buf+6, &Psot, 4);
  memcpy( buf+10, &TPsot, 1);
  memcpy( buf+11, &TNsot, 1);
  memcpy( buf+12, &SOD, 2);

  return buf;
}
