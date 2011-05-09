/*
 * $Id: index_manager.c 53 2011-05-09 16:55:39Z kaori $
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
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "index_manager.h"
#include "box_manager.h"
#include "manfbox_manager.h"
#include "mhixbox_manager.h"
#include "codestream_manager.h"
#include "marker_manager.h"
#include "faixbox_manager.h"
#include "boxheader_manager.h"

#ifdef SERVER
#include "fcgi_stdio.h"
#define logstream FCGI_stdout
#else
#define FCGI_stdout stdout
#define FCGI_stderr stderr
#define logstream stderr
#endif //SERVER

/**
 * chekc JP2 box indexing
 *
 * @param[in] toplev_boxlist top level box list
 * @return                   if correct (true) or wrong (false)
 */
bool check_JP2boxidx( boxlist_param_t *toplev_boxlist);

/**
 * set code index parameters (parse cidx box)
 * Annex I
 *
 * @param[in]  cidx_box pointer to the reference cidx_box
 * @param[out] codeidx  pointer to index parameters
 * @return              if succeeded (true) or failed (false)
 */
bool set_cidxdata( box_param_t *cidx_box, index_param_t *codeidx);

index_param_t * parse_jp2file( int fd)
{
  index_param_t *jp2idx;
  box_param_t *cidx;
  metadatalist_param_t *metadatalist;
  boxlist_param_t *toplev_boxlist;
  struct stat sb;
  
  if( fstat( fd, &sb) == -1){
    fprintf( FCGI_stdout, "Reason: Target broken (fstat error)\r\n");
    return NULL;
  }

  if( !(toplev_boxlist = get_boxstructure( fd, 0, sb.st_size))){
    fprintf( FCGI_stderr, "Error: Not correctl JP2 format\n");
    return NULL;
  }
  
  if( !check_JP2boxidx( toplev_boxlist)){
    fprintf( FCGI_stderr, "Index format not supported\n");
    delete_boxlist( &toplev_boxlist);
    return NULL;
  }

  if( !(cidx = search_box( "cidx", toplev_boxlist))){
    fprintf( FCGI_stderr, "Box cidx not found\n");
    delete_boxlist( &toplev_boxlist);
    return NULL;
  }

  jp2idx = (index_param_t *)malloc( sizeof(index_param_t));
  
  if( !set_cidxdata( cidx, jp2idx)){
    fprintf( FCGI_stderr, "Error: Not correctl format in cidx box\n");
    free(jp2idx);
    delete_boxlist( &toplev_boxlist);
    return NULL;
  }
  delete_boxlist( &toplev_boxlist);
  
  metadatalist = const_metadatalist( fd);
  jp2idx->metadatalist = metadatalist;

#ifndef SERVER
    fprintf( logstream, "local log: code index created\n");
#endif
  
  return jp2idx;
}

void print_index( index_param_t index)
{
  int i;

  fprintf( logstream, "index info:\n");
  fprintf( logstream, "\tCodestream  Offset: %#llx\n", index.offset);
  fprintf( logstream, "\t            Length: %#llx\n", index.length);
  fprintf( logstream, "\tMain header Length: %#llx\n", index.mhead_length);
  fprintf( logstream, "\t              Rsiz: %#x\n", index.Rsiz);
  fprintf( logstream, "\t        Xsiz, Ysiz: (%d,%d) = (%#x, %#x)\n", index.Xsiz, index.Ysiz, index.Xsiz, index.Ysiz);
  fprintf( logstream, "\t      XOsiz, YOsiz: (%d,%d) = (%#x, %#x)\n", index.XOsiz, index.YOsiz, index.XOsiz, index.YOsiz);
  fprintf( logstream, "\t      XTsiz, YTsiz: (%d,%d) = (%#x, %#x)\n", index.XTsiz, index.YTsiz, index.XTsiz, index.YTsiz);
  fprintf( logstream, "\t    XTOsiz, YTOsiz: (%d,%d) = (%#x, %#x)\n", index.XTOsiz, index.YTOsiz, index.XTOsiz, index.YTOsiz);
  fprintf( logstream, "\t    XTnum, YTnum: (%d,%d)\n", index.XTnum, index.YTnum);
  fprintf( logstream, "\t Num of Components: %d\n", index.Csiz);
  
  for( i=0; i<index.Csiz; i++)
    fprintf( logstream, "\t[%d] (Ssiz, XRsiz, YRsiz): (%d, %d, %d) = (%#x, %#x, %#x)\n", i, index.Ssiz[i], index.XRsiz[i], index.YRsiz[i], index.Ssiz[i], index.XRsiz[i], index.YRsiz[i]);

  print_faixbox( index.tilepart);

  print_allmetadata( index.metadatalist);
}

void print_cachemodel( index_param_t index)
{
  Byte8_t TPnum; // num of tile parts in each tile
  int i, j, k, n;

  TPnum = get_nmax( index.tilepart);
  
  fprintf( logstream, "\t main header model: %d\n", index.mhead_model);

  fprintf( logstream, "\t tile part model:\n");
  for( i=0, n=0; i<index.YTnum; i++){
    for( j=0; j<index.XTnum; j++){
      for( k=0; k<TPnum; k++)
	fprintf( logstream, "%d", index.tp_model[n++]);
      fprintf( logstream, " ");
    }
    fprintf( logstream, "\n");
  }
}

void delete_index( index_param_t **index)
{
  delete_metadatalist( &((*index)->metadatalist));
  delete_faixbox( &((*index)->tilepart));
  free( (*index)->tp_model);
  free(*index);
}

bool check_JP2boxidx( boxlist_param_t *toplev_boxlist)
{
  box_param_t *iptr, *fidx, *prxy;
  box_param_t *cidx, *jp2c;

  iptr = search_box( "iptr", toplev_boxlist);
  fidx = search_box( "fidx", toplev_boxlist);
  cidx = search_box( "cidx", toplev_boxlist);
  jp2c = search_box( "jp2c", toplev_boxlist);
  prxy = gene_childboxbyType( fidx, 0, "prxy");

  Byte8_t off = fetch_DBox8bytebigendian( iptr, 0);
  if( off != fidx->offset)
    fprintf( FCGI_stderr, "Reference File Index box offset in Index Finder box not correct\n");

  Byte8_t len = fetch_DBox8bytebigendian( iptr, 8);
  if( len != fidx->length)
    fprintf( FCGI_stderr, "Reference File Index box length in Index Finder box not correct\n");

 
  int pos = 0;
  Byte8_t ooff = fetch_DBox8bytebigendian( prxy, pos);
  if( ooff != jp2c->offset)
    fprintf( FCGI_stderr, "Reference jp2c offset in prxy box not correct\n");
  pos += 8;

  boxheader_param_t *obh = gene_childboxheader( prxy, pos);
  if( obh->length != jp2c->length || strncmp( obh->type, "jp2c",4)!=0)
    fprintf( FCGI_stderr, "Reference jp2c header in prxy box not correct\n");
  pos += obh->headlen;
  free(obh);
  
  Byte_t ni = fetch_DBox1byte( prxy, pos);
  if( ni != 1){
    fprintf( FCGI_stderr, "Multiple indexes not supported\n");
    return false;
  }  
  pos += 1;
  
  Byte8_t ioff = fetch_DBox8bytebigendian( prxy, pos);
  if( ioff != cidx->offset)
    fprintf( FCGI_stderr, "Reference cidx offset in prxy box not correct\n");
  pos += 8;

  boxheader_param_t *ibh = gene_childboxheader( prxy, pos);
  if( ibh->length != cidx->length || strncmp( ibh->type, "cidx",4)!=0)
    fprintf( FCGI_stderr, "Reference cidx header in prxy box not correct\n");
  pos += ibh->headlen;
  free(ibh);
  
  free(prxy);

  return true;
}

/**
 * set code index parameters from cptr box
 * I.3.2.2 Codestream Finder box
 *
 * @param[in]  cidx_box pointer to the reference cidx_box
 * @param[out] jp2idx   pointer to index parameters
 * @return              if succeeded (true) or failed (false)
 */
bool set_cptrdata( box_param_t *cidx_box, index_param_t *jp2idx);

/**
 * set code index parameters from mhix box for main header
 * I.3.2.4.3 Header Index Table box
 *
 * @param[in]  cidx_box   pointer to the reference cidx_box
 * @param[in]  codestream codestream parameters
 * @param[out] jp2idx     pointer to index parameters
 * @return                if succeeded (true) or failed (false)
 */
bool set_mainmhixdata( box_param_t *cidx_box, codestream_param_t codestream, index_param_t *jp2idx);

/**
 * set code index parameters from tpix box
 * I.3.2.4.4 Tile-part Index Table box
 *
 * @param[in]  cidx_box   pointer to the reference cidx_box
 * @param[out] jp2idx     pointer to index parameters
 * @return                if succeeded (true) or failed (false)
 */
bool set_tpixdata( box_param_t *cidx_box, index_param_t *jp2idx);

/**
 * set code index parameters from thix box
 * I.3.2.4.5 Tile Header Index Table box
 *
 * @param[in]  cidx_box   pointer to the reference cidx_box
 * @param[out] jp2idx     pointer to index parameters
 * @return                if succeeded (true) or failed (false)
 */
bool set_thixdata( box_param_t *cidx_box, index_param_t *jp2idx);

bool set_cidxdata( box_param_t *cidx_box, index_param_t *jp2idx)
{
  box_param_t *manf_box;
  manfbox_param_t *manf;
  codestream_param_t codestream;

  set_cptrdata( cidx_box, jp2idx);

  codestream = set_codestream( cidx_box->fd, jp2idx->offset, jp2idx->length);

  manf_box = gene_boxbyType( cidx_box->fd, get_DBoxoff( cidx_box), get_DBoxlen( cidx_box), "manf");
  manf = gene_manfbox( manf_box);

  if( !search_boxheader( "mhix", manf)){
    fprintf( FCGI_stderr, "Error: mhix box not present in manfbox\n");
    free(jp2idx);
    return false;
  }
  set_mainmhixdata( cidx_box, codestream, jp2idx);

  if( !search_boxheader( "tpix", manf)){
    fprintf( FCGI_stderr, "Error: tpix box not present in manfbox\n");
    free(jp2idx);
    return false;
  }
  set_tpixdata( cidx_box, jp2idx);

#ifdef NO_NEED_YET
  if( !search_boxheader( "thix", manf)){
    fprintf( FCGI_stderr, "Error: thix box not present in manfbox\n");
    return false;
  }
  set_thixdata( cidx_box, jp2idx);
#endif

  delete_manfbox( &manf);
  free( manf_box);

  return true;
}

bool set_cptrdata( box_param_t *cidx_box, index_param_t *jp2idx)
{
  box_param_t *box;   //!< cptr box
  Byte2_t dr, cont;

  if( !(box = gene_boxbyType( cidx_box->fd, get_DBoxoff( cidx_box), get_DBoxlen( cidx_box), "cptr")))
    return false;
  
  // DR: Data Reference. 
  // If 0, the codestream or its Fragment Table box exists in the current file
  if(( dr = fetch_DBox2bytebigendian( box, 0))){
    fprintf( FCGI_stderr, "Error: Codestream not present in current file\n");
    free( box);
    return false;  
  }
  
  // CONT: Container Type
  // If 0, the entire codestream appears as a contiguous range of
  // bytes within its file or resource.
  if(( cont = fetch_DBox2bytebigendian( box, 2))){
    fprintf( FCGI_stderr, "Error: Can't cope with fragmented codestreams yet\n");
    free( box);
    return false;  
  }
    
  jp2idx->offset = fetch_DBox8bytebigendian( box, 4);
  jp2idx->length = fetch_DBox8bytebigendian( box, 12);

  free( box);

  return true;
}


/**
 * set code index parameters from SIZ marker in codestream
 * A.5 Fixed information marker segment
 * A.5.1 Image and tile size (SIZ)
 *
 * @param[in]  sizmkidx   pointer to SIZ marker index in mhix box
 * @param[in]  codestream codestream parameters
 * @param[out] jp2idx     pointer to index parameters
 * @return                if succeeded (true) or failed (false)
 */
bool set_SIZmkrdata( markeridx_param_t *sizmkidx, codestream_param_t codestream, index_param_t *jp2idx);

bool set_mainmhixdata( box_param_t *cidx_box, codestream_param_t codestream, index_param_t *jp2idx)
{
  box_param_t *mhix_box;
  mhixbox_param_t *mhix;
  markeridx_param_t *sizmkidx;
  
  if( !(mhix_box = gene_boxbyType( cidx_box->fd, get_DBoxoff( cidx_box), get_DBoxlen( cidx_box), "mhix")))
    return false;

  jp2idx->mhead_model = 0;
  jp2idx->mhead_length = fetch_DBox8bytebigendian( mhix_box, 0);

  mhix = gene_mhixbox( mhix_box);
  free( mhix_box);

  sizmkidx = search_markeridx( 0xff51, mhix);
  set_SIZmkrdata( sizmkidx, codestream, jp2idx);

  delete_mhixbox( &mhix);

  return true;
}

bool set_tpixdata( box_param_t *cidx_box, index_param_t *jp2idx)
{
  box_param_t *tpix_box;   //!< tpix box
  box_param_t *faix_box;   //!< faix box
  faixbox_param_t *faix;   //!< faix
  size_t numOfelem;
  
  if( !(tpix_box = gene_boxbyType( cidx_box->fd, get_DBoxoff( cidx_box), get_DBoxlen( cidx_box), "tpix")))
    return false;

  if( !(faix_box = gene_boxbyType( tpix_box->fd, get_DBoxoff( tpix_box), get_DBoxlen( tpix_box), "faix")))
    return false;

  faix = gene_faixbox( faix_box);
  jp2idx->tilepart = faix;
  numOfelem = get_nmax( faix)*get_m( faix);
  
  jp2idx->tp_model = (bool *)malloc( numOfelem*sizeof(bool));
  memset( jp2idx->tp_model, 0, numOfelem*sizeof(bool));

  //delete_faixbox( &faix); // currently the jp2idx element
  free( tpix_box);
  free( faix_box);

  return true;
}

bool set_thixdata( box_param_t *cidx_box, index_param_t *jp2idx)
{
  box_param_t *thix_box, *manf_box, *mhix_box;
  manfbox_param_t *manf;
  boxheader_param_t *ptr;
  mhixbox_param_t *mhix;
  Byte8_t pos, mhixseqoff;
  
  if( !(thix_box = gene_boxbyType( cidx_box->fd, get_DBoxoff( cidx_box), get_DBoxlen( cidx_box), "thix")))
    return false;
  
  if( !(manf_box = gene_boxbyType( thix_box->fd, get_DBoxoff( thix_box), get_DBoxlen( thix_box), "manf"))){
    free( thix_box);
    return false;
  }
  
  manf = gene_manfbox( manf_box);
  ptr = manf->first;
  mhixseqoff = manf_box->offset+manf_box->length;
  pos = 0;
    
  while( ptr){
    mhix_box = gene_boxbyType( thix_box->fd, mhixseqoff+pos, get_DBoxlen( thix_box)-manf_box->length-pos, "mhix");
    mhix = gene_mhixbox( mhix_box);

    pos += mhix_box->length;
    ptr = ptr->next;

    free( mhix_box);
    delete_mhixbox( &mhix);
  }

  delete_manfbox( &manf);
  free( manf_box);
  free( thix_box);

  return true;
}


bool set_SIZmkrdata( markeridx_param_t *sizmkidx, codestream_param_t codestream, index_param_t *jp2idx)
{
  marker_param_t sizmkr;
  int i;

  sizmkr = set_marker( codestream, sizmkidx->code, sizmkidx->offset, sizmkidx->length);

  if( sizmkidx->length != fetch_marker2bytebigendian( sizmkr, 0)){
    fprintf( FCGI_stderr, "Error: marker %#x index is not correct\n", sizmkidx->code);
    return false;
  }
  
  jp2idx->Rsiz   = fetch_marker2bytebigendian( sizmkr, 2);
  jp2idx->Xsiz   = fetch_marker4bytebigendian( sizmkr, 4);
  jp2idx->Ysiz   = fetch_marker4bytebigendian( sizmkr, 8);
  jp2idx->XOsiz  = fetch_marker4bytebigendian( sizmkr, 12);
  jp2idx->YOsiz  = fetch_marker4bytebigendian( sizmkr, 16);
  jp2idx->XTsiz  = fetch_marker4bytebigendian( sizmkr, 20);
  jp2idx->YTsiz  = fetch_marker4bytebigendian( sizmkr, 24);
  jp2idx->XTOsiz = fetch_marker4bytebigendian( sizmkr, 28);
  jp2idx->YTOsiz = fetch_marker4bytebigendian( sizmkr, 32);
  jp2idx->Csiz   = fetch_marker2bytebigendian( sizmkr, 36);

  jp2idx->XTnum  = ( jp2idx->Xsiz-jp2idx->XTOsiz+jp2idx->XTsiz-1)/jp2idx->XTsiz;
  jp2idx->YTnum  = ( jp2idx->Ysiz-jp2idx->YTOsiz+jp2idx->YTsiz-1)/jp2idx->YTsiz;
  
  for( i=0; i<(int)jp2idx->Csiz; i++){
    jp2idx->Ssiz[i]  = fetch_marker1byte( sizmkr, 38+i*3);
    jp2idx->XRsiz[i] = fetch_marker1byte( sizmkr, 39+i*3);
    jp2idx->YRsiz[i] = fetch_marker1byte( sizmkr, 40+i*3);
  }
  return true;
}


Byte4_t max( Byte4_t n1, Byte4_t n2);
Byte4_t min( Byte4_t n1, Byte4_t n2);

range_param_t get_tile_range( Byte4_t Osiz, Byte4_t siz, Byte4_t TOsiz, Byte4_t Tsiz, Byte4_t tile_id, int level);

range_param_t get_tile_Xrange( index_param_t index, Byte4_t tile_xid, int level)
{
  return get_tile_range( index.XOsiz, index.Xsiz, index.XTOsiz, index.XTsiz, tile_xid, level);
}

range_param_t get_tile_Yrange( index_param_t index, Byte4_t tile_yid, int level)
{
  return get_tile_range( index.YOsiz, index.Ysiz, index.YTOsiz, index.YTsiz, tile_yid, level);
}

range_param_t get_tile_range( Byte4_t Osiz, Byte4_t siz, Byte4_t TOsiz, Byte4_t Tsiz, Byte4_t tile_id, int level)
{
  range_param_t range;
  int n;

  range.minvalue = max( Osiz, TOsiz+tile_id*Tsiz);
  range.maxvalue = min( siz,  TOsiz+(tile_id+1)*Tsiz);

  for( n=0; n<level; n++){
    range.minvalue = ceil(range.minvalue/2.0);
    range.maxvalue = ceil(range.maxvalue/2.0);
  }
  return range;
}

Byte4_t max( Byte4_t n1, Byte4_t n2)
{
  if( n1 < n2)
    return n2;
  else
    return n1;
}

Byte4_t min( Byte4_t n1, Byte4_t n2)
{
  if( n1 < n2)
    return n1;
  else
    return n2;
}
