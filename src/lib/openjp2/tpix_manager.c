/*
 * $Id: tpix_manager.c 897 2011-08-28 21:43:57Z Kaori.Hagihara@gmail.com $
 *
 * Copyright (c) 2002-2011, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2011, Professor Benoit Macq
 * Copyright (c) 2003-2004, Yannick Verschueren
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
 *  \brief Modification of jpip.c from 2KAN indexer
 */

#include "opj_includes.h"

#define MAX(a,b) ((a)>(b)?(a):(b))


/* 
 * Write faix box of tpix
 *
 * @param[in] coff offset of j2k codestream
 * @param[in] compno    component number
 * @param[in] cstr_info codestream information
 * @param[in] j2klen    length of j2k codestream
 * @param[in] cio       file output handle
 * @return              length of faix box
 */
int write_tpixfaix( int coff, int compno, opj_codestream_info_t cstr_info, int j2klen, opj_cio_t *cio);


int write_tpix( int coff, opj_codestream_info_t cstr_info, int j2klen, opj_cio_t *cio)
{
  int len, lenp;
  lenp = cio_tell( cio);
  cio_skip( cio, 4);              /* L [at the end] */
  cio_write( cio, JPIP_TPIX, 4);  /* TPIX           */
  
  write_tpixfaix( coff, 0, cstr_info, j2klen, cio);

  len = cio_tell( cio)-lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);        /* L              */
  cio_seek( cio, lenp+len);

  return len;
}

int write_tpix_v2( int coff, opj_codestream_info_t cstr_info, int j2klen, opj_stream_private_t *cio,
              opj_event_mgr_t * p_manager )
{
  OPJ_BYTE l_data_header [4];
  int len, lenp;
#if 0
  lenp = cio_tell( cio);
  cio_skip( cio, 4);              /* L [at the end] */
  cio_write( cio, JPIP_TPIX, 4);  /* TPIX           */
#else
  lenp = opj_stream_tell(cio);
  opj_stream_skip(cio, 4, p_manager);
  opj_write_bytes(l_data_header,JPIP_TPIX,4); /* TPIX */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
#endif
  
  write_tpixfaix_v2( coff, 0, cstr_info, j2klen, cio,p_manager);

#if 0
  len = cio_tell( cio)-lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);        /* L              */
  cio_seek( cio, lenp+len);
#else
  len = opj_stream_tell(cio)-lenp;
  opj_stream_skip(cio, lenp, p_manager);
  opj_write_bytes(l_data_header,len,4);/* L              */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
  opj_stream_seek(cio, lenp+len,p_manager);
#endif

  return len;
}

/* 
 * Get number of maximum tile parts per tile
 *
 * @param[in] cstr_info codestream information
 * @return              number of maximum tile parts per tile
 */
int get_num_max_tile_parts( opj_codestream_info_t cstr_info);

int write_tpixfaix( int coff, int compno, opj_codestream_info_t cstr_info, int j2klen, opj_cio_t *cio)
{
  int len, lenp;
  int i, j;
  int Aux;
  int num_max_tile_parts;
  int size_of_coding; /* 4 or 8 */
  opj_tp_info_t tp;
  int version;

  num_max_tile_parts = get_num_max_tile_parts( cstr_info);

  if( j2klen > pow( 2, 32)){
    size_of_coding =  8;
    version = num_max_tile_parts == 1 ? 1:3;
  }
  else{
    size_of_coding = 4;
    version = num_max_tile_parts == 1 ? 0:2;
  }

  lenp = cio_tell( cio);
  cio_skip( cio, 4);              /* L [at the end]      */
  cio_write( cio, JPIP_FAIX, 4);  /* FAIX                */ 
  cio_write( cio, version, 1);     /* Version 0 = 4 bytes */

  cio_write( cio, num_max_tile_parts, size_of_coding);                      /* NMAX           */
  cio_write( cio, cstr_info.tw*cstr_info.th, size_of_coding);                               /* M              */
  for (i = 0; i < cstr_info.tw*cstr_info.th; i++){
    for (j = 0; j < cstr_info.tile[i].num_tps; j++){
      tp = cstr_info.tile[i].tp[j];
      cio_write( cio, tp.tp_start_pos-coff, size_of_coding); /* start position */
      cio_write( cio, tp.tp_end_pos-tp.tp_start_pos+1, size_of_coding);    /* length         */
      if (version & 0x02){
  if( cstr_info.tile[i].num_tps == 1 && cstr_info.numdecompos[compno] > 1)
    Aux = cstr_info.numdecompos[compno] + 1;
  else
    Aux = j + 1;
      
  cio_write( cio, Aux,4);
  /*cio_write(img.tile[i].tile_parts[j].num_reso_AUX,4);*/ /* Aux_i,j : Auxiliary value */
  /* fprintf(stderr,"AUX value %d\n",Aux);*/
      }
      /*cio_write(0,4);*/
    }
    /* PADDING */
    while (j < num_max_tile_parts){
      cio_write( cio, 0, size_of_coding); /* start position            */
      cio_write( cio, 0, size_of_coding); /* length                    */
      if (version & 0x02)
  cio_write( cio, 0,4);                  /* Aux_i,j : Auxiliary value */
      j++;
    }
  }
  
  len = cio_tell( cio)-lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);        /* L  */
  cio_seek( cio, lenp+len);

  return len;

}

int write_tpixfaix_v2( int coff, int compno, opj_codestream_info_t cstr_info, int j2klen, opj_stream_private_t *cio,
              opj_event_mgr_t * p_manager )
{
  int len, lenp;
  int i, j;
  int Aux;
  int num_max_tile_parts;
  int size_of_coding; /* 4 or 8 */
  opj_tp_info_t tp;
  OPJ_BYTE l_data_header [8];
  int version;

  num_max_tile_parts = get_num_max_tile_parts( cstr_info);

  if( j2klen > pow( 2, 32)){
    size_of_coding =  8;
    version = num_max_tile_parts == 1 ? 1:3;
  }
  else{
    size_of_coding = 4;
    version = num_max_tile_parts == 1 ? 0:2;
  }

#if 0
  lenp = cio_tell( cio);
  cio_skip( cio, 4);              /* L [at the end]      */
  cio_write( cio, JPIP_FAIX, 4);  /* FAIX                */ 
  cio_write( cio, version, 1);     /* Version 0 = 4 bytes */
#else
  lenp = opj_stream_tell(cio);
  opj_stream_skip(cio, 4, p_manager);         /* L [at the end]      */
  opj_write_bytes(l_data_header,JPIP_FAIX,4); /* FAIX */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
  opj_write_bytes(l_data_header,version,1);   /* Version 0 = 4 bytes */
  opj_stream_write_data(cio,l_data_header,1,p_manager);
#endif

#if 0
  cio_write( cio, num_max_tile_parts, size_of_coding);                      /* NMAX           */
  cio_write( cio, cstr_info.tw*cstr_info.th, size_of_coding);               /* M              */
#else
  opj_write_bytes(l_data_header,num_max_tile_parts,size_of_coding);         /* NMAX           */
  opj_stream_write_data(cio,l_data_header,size_of_coding,p_manager);
  opj_write_bytes(l_data_header,cstr_info.tw*cstr_info.th,size_of_coding);  /* M              */
  opj_stream_write_data(cio,l_data_header,size_of_coding,p_manager);
#endif
  for (i = 0; i < cstr_info.tw*cstr_info.th; i++)
    {
    for (j = 0; j < cstr_info.tile[i].num_tps; j++)
      {
      tp = cstr_info.tile[i].tp[j];
#if 0
      cio_write( cio, tp.tp_start_pos-coff, size_of_coding); /* start position */
      cio_write( cio, tp.tp_end_pos-tp.tp_start_pos+1, size_of_coding);    /* length         */
#else
      opj_write_bytes(l_data_header,tp.tp_start_pos-coff,size_of_coding);            /* start position */
      opj_stream_write_data(cio,l_data_header,size_of_coding,p_manager);
      opj_write_bytes(l_data_header,tp.tp_end_pos-tp.tp_start_pos+1,size_of_coding); /* length         */
      opj_stream_write_data(cio,l_data_header,size_of_coding,p_manager);
#endif
      if (version & 0x02)
        {
        if( cstr_info.tile[i].num_tps == 1 && cstr_info.numdecompos[compno] > 1)
          Aux = cstr_info.numdecompos[compno] + 1;
        else
          Aux = j + 1;

#if 0
        cio_write( cio, Aux,4);
#else
        opj_write_bytes(l_data_header,Aux,4);
        opj_stream_write_data(cio,l_data_header,4,p_manager);
#endif
        /*cio_write(img.tile[i].tile_parts[j].num_reso_AUX,4);*/ /* Aux_i,j : Auxiliary value */
        /* fprintf(stderr,"AUX value %d\n",Aux);*/
        }
      /*cio_write(0,4);*/
      }
    /* PADDING */
    while (j < num_max_tile_parts)
      {
#if 0
      cio_write( cio, 0, size_of_coding); /* start position            */
      cio_write( cio, 0, size_of_coding); /* length                    */
#else
      opj_write_bytes(l_data_header,0,size_of_coding);/* start position            */
      opj_stream_write_data(cio,l_data_header,size_of_coding,p_manager);
      opj_write_bytes(l_data_header,0,size_of_coding);/* length                    */
      opj_stream_write_data(cio,l_data_header,size_of_coding,p_manager);
#endif
      if (version & 0x02)
#if 0
        cio_write( cio, 0,4);                  /* Aux_i,j : Auxiliary value */
#else
      opj_write_bytes(l_data_header,0,4);      /* Aux_i,j : Auxiliary value */
      opj_stream_write_data(cio,l_data_header,4,p_manager);
#endif
      j++;
      }
    }
  
#if 0
  len = cio_tell( cio)-lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);        /* L  */
  cio_seek( cio, lenp+len);
#else
  len = opj_stream_tell(cio)-lenp;
  opj_stream_seek(cio, lenp,p_manager);
  opj_write_bytes(l_data_header,len,4);/* L  */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
  opj_stream_seek(cio, lenp+len,p_manager);
#endif

  return len;
}

int get_num_max_tile_parts( opj_codestream_info_t cstr_info)
{
  int num_max_tp = 0, i;

  for( i=0; i<cstr_info.tw*cstr_info.th; i++)
    num_max_tp = MAX( cstr_info.tile[i].num_tps, num_max_tp);
  
  return num_max_tp;
}
