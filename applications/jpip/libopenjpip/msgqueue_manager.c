/*
 * $Id: msgqueue_manager.c 53 2011-05-09 16:55:39Z kaori $
 *
 * Copyright (c) 2002-2011, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2011, Professor Benoit Macq
 * Copyright (c) 2010-2011, Kaori Hagihara
 * Copyright (c) 2011,      Lucian Corlaciu, GSoC
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
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include "msgqueue_manager.h"
#include "metadata_manager.h"
#include "index_manager.h"

#ifdef SERVER
#include "fcgi_stdio.h"
#define logstream FCGI_stdout
#else
#define FCGI_stdout stdout
#define FCGI_stderr stderr
#define logstream stderr
#endif //SERVER

msgqueue_param_t * gene_msgqueue( bool stateless, cachemodel_param_t *cachemodel)
{
  msgqueue_param_t *msgqueue;

  msgqueue = (msgqueue_param_t *)malloc( sizeof(msgqueue_param_t));

  msgqueue->first = NULL;
  msgqueue->last  = NULL;

  msgqueue->stateless = stateless;
  msgqueue->cachemodel = cachemodel;
  
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
  if( (*msgqueue)->stateless && (*msgqueue)->cachemodel)
    delete_cachemodel( &((*msgqueue)->cachemodel));

  free(*msgqueue); 
}

void print_msgqueue( msgqueue_param_t *msgqueue)
{
  message_param_t *ptr;
  char *message_class[] = { "Precinct", "Ext-Prec", "TileHead", "non", "Tile", "Ext-Tile", "Main", "non", "Meta"};

  if( !msgqueue)
    return;

  fprintf( logstream, "message queue:\n");
  ptr = msgqueue->first;

  while( ptr){
    fprintf( logstream, "\t class_id: %lld %s\n", ptr->class_id, message_class[ptr->class_id]);
    fprintf( logstream, "\t in_class_id: %lld\n", ptr->in_class_id );
    fprintf( logstream, "\t csn: %lld\n", ptr->csn );
    fprintf( logstream, "\t bin_offset: %#llx\n", ptr->bin_offset );
    fprintf( logstream, "\t length: %#llx\n", ptr->length );
    if( ptr->class_id%2)
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
  cachemodel_param_t *cachemodel;
  target_param_t *target;
  index_param_t *codeidx;
  message_param_t *msg;

  cachemodel = msgqueue->cachemodel;
  target = cachemodel->target;
  codeidx = target->codeidx;
  
  msg = (message_param_t *)malloc( sizeof(message_param_t));

  msg->last_byte = true;
  msg->in_class_id = 0;
  msg->class_id = MAINHEADER_MSG;
  msg->csn = target->csn;
  msg->bin_offset = 0;
  msg->length = codeidx->mhead_length;
  msg->aux = 0; // non exist
  msg->res_offset = codeidx->offset;
  msg->phld = NULL;
  msg->next = NULL;

  enqueue_message( msg, msgqueue);

  cachemodel->mhead_model = true;
}

void enqueue_tileheader( int tile_id, msgqueue_param_t *msgqueue)
{
  cachemodel_param_t *cachemodel;
  target_param_t *target;
  index_param_t *codeidx;
  message_param_t *msg;

  cachemodel = msgqueue->cachemodel;
  target = cachemodel->target;
  codeidx = target->codeidx;

  if( !cachemodel->th_model[ tile_id]){
    msg = (message_param_t *)malloc( sizeof(message_param_t));
    msg->last_byte = true;
    msg->in_class_id = tile_id;
    msg->class_id = TILE_HEADER_MSG;
    msg->csn = target->csn;
    msg->bin_offset = 0;
    msg->length = codeidx->tileheader[tile_id]->tlen-2; // SOT marker segment is removed
    msg->aux = 0; // non exist
    msg->res_offset = codeidx->offset + get_elemOff( codeidx->tilepart, 0, tile_id) + 2; // skip SOT marker seg
    msg->phld = NULL;
    msg->next = NULL;
    
    enqueue_message( msg, msgqueue);
    cachemodel->th_model[ tile_id] = true;
  }
}

void enqueue_tile( int tile_id, int level, msgqueue_param_t *msgqueue)
{
  cachemodel_param_t *cachemodel;
  target_param_t *target;
  bool *tp_model;
  Byte8_t numOftparts; // num of tile parts par tile
  Byte8_t numOftiles;
  index_param_t *codeidx;
  faixbox_param_t *tilepart;
  message_param_t *msg;
  Byte8_t binOffset, binLength, class_id;
  int i;

  cachemodel = msgqueue->cachemodel;
  target = cachemodel->target;
  codeidx  = target->codeidx;
  tilepart = codeidx->tilepart;
  
  numOftparts = get_nmax( tilepart);
  numOftiles  = get_m( tilepart);

  class_id = (numOftparts==1) ? TILE_MSG : EXT_TILE_MSG;
  
  if( tile_id < 0 || numOftiles <= tile_id){
    fprintf( FCGI_stderr, "Error, Invalid tile-id %d\n", tile_id);
    return;
  }
  
  tp_model = &cachemodel->tp_model[ tile_id*numOftparts];

  binOffset=0;
  for( i=0; i<numOftparts-level; i++){
    binLength = get_elemLen( tilepart, i, tile_id);
    
    if( !tp_model[i]){
      msg = (message_param_t *)malloc( sizeof(message_param_t));
      
      msg->last_byte = (i==numOftparts-1);
      msg->in_class_id = tile_id;
      msg->class_id = class_id;
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

void enqueue_precinct( int seq_id, int tile_id, int comp_id, int layers, msgqueue_param_t *msgqueue)
{
  cachemodel_param_t *cachemodel;
  index_param_t *codeidx;
  faixbox_param_t *precpacket;
  message_param_t *msg;
  Byte8_t nmax, binOffset, binLength;
  int layer_id, numOflayers;
  
  cachemodel = msgqueue->cachemodel;
  codeidx = cachemodel->target->codeidx;
  precpacket = codeidx->precpacket[ comp_id];
  numOflayers = codeidx->COD.numOflayers;

  nmax = get_nmax(precpacket);
  if( layers < 0)
    layers = numOflayers;
    
  binOffset = 0;
  for( layer_id = 0; layer_id < layers; layer_id++){

    binLength = get_elemLen( precpacket, seq_id*numOflayers+layer_id, tile_id);
    
    if( !cachemodel->pp_model[comp_id][ tile_id*nmax+seq_id*numOflayers+layer_id]){
  
      msg = (message_param_t *)malloc( sizeof(message_param_t));
      msg->last_byte = (layer_id == (numOflayers-1));
      msg->in_class_id = comp_precinct_id( tile_id, comp_id, seq_id, codeidx->SIZ.Csiz, codeidx->SIZ.XTnum * codeidx->SIZ.YTnum);
      msg->class_id = PRECINCT_MSG;
      msg->csn = cachemodel->target->csn;
      msg->bin_offset = binOffset;
      msg->length = binLength;
      msg->aux = 0;
      msg->res_offset = codeidx->offset+get_elemOff( precpacket, seq_id*numOflayers+layer_id, tile_id);
      msg->phld = NULL;
      msg->next = NULL;

      enqueue_message( msg, msgqueue);
      
      cachemodel->pp_model[comp_id][ tile_id*nmax+seq_id*numOflayers+layer_id] = true;
    }
    binOffset += binLength;
  }
}

Byte8_t comp_precinct_id( int t, int c, int s, int num_components, int num_tiles)
{
  return t + (c + s * num_components ) * num_tiles;
}

void enqueue_box(  int meta_id, boxlist_param_t *boxlist, msgqueue_param_t *msgqueue, Byte8_t *binOffset);
void enqueue_phld( int meta_id, placeholderlist_param_t *phldlist, msgqueue_param_t *msgqueue, Byte8_t *binOffset);
void enqueue_boxcontents( int meta_id, boxcontents_param_t *boxcontents, msgqueue_param_t *msgqueue, Byte8_t *binOffset);

void enqueue_metadata( int meta_id, msgqueue_param_t *msgqueue)
{
  metadatalist_param_t *metadatalist;
  metadata_param_t *metadata;
  Byte8_t binOffset;

  metadatalist = msgqueue->cachemodel->target->codeidx->metadatalist;
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
    msg = gene_metamsg( meta_id, *binOffset, box->length, box->offset, NULL, msgqueue->cachemodel->target->csn);
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
    msg = gene_metamsg( meta_id, *binOffset, phld->LBox, 0, phld, msgqueue->cachemodel->target->csn);
    enqueue_message( msg, msgqueue);

    *binOffset += phld->LBox;
    phld = phld->next;
  }
}

void enqueue_boxcontents( int meta_id, boxcontents_param_t *boxcontents, msgqueue_param_t *msgqueue, Byte8_t *binOffset)
{
  message_param_t *msg;

  msg = gene_metamsg( meta_id, *binOffset, boxcontents->length, boxcontents->offset, NULL, msgqueue->cachemodel->target->csn);
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
  class_id = -1;
  csn = -1;
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
      emit_body( msg, msgqueue->cachemodel->target->fd);

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

void parse_JPIPstream( Byte_t *JPIPstream, Byte8_t streamlen, Byte8_t offset, msgqueue_param_t *msgqueue)
{
  Byte_t *ptr;  // stream pointer
  message_param_t *msg;
  Byte_t bb, c;
  Byte8_t class_id, csn;

  class_id = -1; // dummy
  csn = -1;
  ptr = JPIPstream;
  while( ptr-JPIPstream < streamlen){
    msg = (message_param_t *)malloc( sizeof(message_param_t));
    
    ptr = parse_bin_id_vbas( ptr, &bb, &c, &msg->in_class_id);
    
    msg->last_byte   = c == 1 ? true : false;
    
    if( bb >= 2)
      ptr = parse_vbas( ptr, &class_id);

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
    
    msg->res_offset = ptr-JPIPstream+offset;   
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

void parse_metamsg( msgqueue_param_t *msgqueue, Byte_t *stream, Byte8_t streamlen, metadatalist_param_t *metadatalist)
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
