/*
 * $Id: cidx_manager.c 897 2011-08-28 21:43:57Z Kaori.Hagihara@gmail.com $
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

#include "opj_includes.h"


/* 
 * Write CPTR Codestream finder box
 *
 * @param[in] coff offset of j2k codestream
 * @param[in] clen length of j2k codestream
 * @param[in] cio  file output handle
 */
void write_cptr(int coff, int clen, opj_cio_t *cio);

void write_cptr_v2(int coff, int clen, opj_stream_private_t *cio,
              opj_event_mgr_t * p_manager );

void write_manf_v2(int second, int v, opj_jp2_box_t *box, opj_stream_private_t *cio,
              opj_event_mgr_t * p_manager );

/* 
 * Write main header index table (box)
 *
 * @param[in] coff offset of j2k codestream
 * @param[in] cstr_info codestream information
 * @param[in] cio  file output handle
 * @return         length of mainmhix box
 */
int write_mainmhix( int coff, opj_codestream_info_t cstr_info, opj_cio_t *cio);


/* 
 * Check if EPH option is used
 *
 * @param[in] coff    offset of j2k codestream
 * @param[in] markers marker information
 * @param[in] marknum number of markers
 * @param[in] cio     file output handle
 * @return            true if EPH is used
 */
opj_bool check_EPHuse( int coff, opj_marker_info_t *markers, int marknum, opj_cio_t *cio);


int write_cidx( int offset, opj_cio_t *cio, opj_image_t *image, opj_codestream_info_t cstr_info, int j2klen)
{
  int len, i, lenp;
  opj_jp2_box_t *box;
  int num_box = 0;
  opj_bool  EPHused;
  (void)image; /* unused ? */

  lenp = -1;
  box = (opj_jp2_box_t *)opj_calloc( 32, sizeof(opj_jp2_box_t));

  for (i=0;i<2;i++){
  
    if(i)
      cio_seek( cio, lenp);

    lenp = cio_tell( cio);

    cio_skip( cio, 4);              /* L [at the end] */
    cio_write( cio, JPIP_CIDX, 4);  /* CIDX           */
    write_cptr( offset, cstr_info.codestream_size, cio);

    write_manf( i, num_box, box, cio);
    
    num_box = 0;
    box[num_box].length = write_mainmhix( offset, cstr_info, cio);
    box[num_box].type = JPIP_MHIX;
    num_box++;

    box[num_box].length = write_tpix( offset, cstr_info, j2klen, cio);
    box[num_box].type = JPIP_TPIX;
    num_box++;
      
    box[num_box].length = write_thix( offset, cstr_info, cio);
    box[num_box].type = JPIP_THIX;
    num_box++;

    EPHused = check_EPHuse( offset, cstr_info.marker, cstr_info.marknum, cio);
      
    box[num_box].length = write_ppix( offset, cstr_info, EPHused, j2klen, cio);
    box[num_box].type = JPIP_PPIX;
    num_box++;
    
    box[num_box].length = write_phix( offset, cstr_info, EPHused, j2klen, cio);
    box[num_box].type = JPIP_PHIX;
    num_box++;
      
    len = cio_tell( cio)-lenp;
    cio_seek( cio, lenp);
    cio_write( cio, len, 4);        /* L             */
    cio_seek( cio, lenp+len);
  }

  opj_free( box);
  
  return len;
}

int write_cidx_v2( int offset, opj_stream_private_t *cio, opj_codestream_info_t cstr_info, int j2klen,
              opj_event_mgr_t * p_manager )
{
  int len, i, lenp;
  opj_jp2_box_t *box;
  int num_box = 0;
  opj_bool  EPHused;
  OPJ_BYTE l_data_header [4];

  lenp = -1;
  box = (opj_jp2_box_t *)opj_calloc( 32, sizeof(opj_jp2_box_t));

  for (i=0;i<2;i++){
  
    if(i)
      opj_stream_seek(cio,lenp,p_manager);


    lenp = opj_stream_tell (cio);

    opj_stream_skip(cio, 4, p_manager); /* L [at the end] */
#if 0
    cio_write( cio, JPIP_CIDX, 4);  /* CIDX           */
#else
    opj_write_bytes(l_data_header,JPIP_CIDX,4); /* CIDX */
    opj_stream_write_data(cio,l_data_header,4,p_manager);
#endif
    write_cptr_v2( offset, cstr_info.codestream_size, cio,p_manager);

    write_manf_v2( i, num_box, box, cio,p_manager);
    
    num_box = 0;
    box[num_box].length = write_mainmhix_v2( offset, cstr_info, cio,p_manager);
    box[num_box].type = JPIP_MHIX;
    num_box++;

    box[num_box].length = write_tpix_v2( offset, cstr_info, j2klen, cio);
    box[num_box].type = JPIP_TPIX;
    num_box++;
      
    box[num_box].length = write_thix_v2( offset, cstr_info, cio);
    box[num_box].type = JPIP_THIX;
    num_box++;

    EPHused = check_EPHuse_v2( offset, cstr_info.marker, cstr_info.marknum, cio);
      
    box[num_box].length = write_ppix_v2( offset, cstr_info, EPHused, j2klen, cio);
    box[num_box].type = JPIP_PPIX;
    num_box++;
    
    box[num_box].length = write_phix_v2( offset, cstr_info, EPHused, j2klen, cio);
    box[num_box].type = JPIP_PHIX;
    num_box++;
      
#if 0
    len = cio_tell( cio)-lenp;
    cio_seek( cio, lenp);
    cio_write( cio, len, 4);        /* L             */
    cio_seek( cio, lenp+len);
#else
    len = opj_stream_tell(cio)-lenp;
    opj_stream_seek(cio, lenp,p_manager);
    opj_write_bytes(l_data_header,len,4);/* L  */
    opj_stream_write_data(cio,l_data_header,4,p_manager);
    opj_stream_seek(cio, lenp+len,p_manager);
#endif
  }

  opj_free( box);
  
  return len;
}

void write_cptr(int coff, int clen, opj_cio_t *cio)
{
  int len, lenp;

  lenp = cio_tell( cio);
  cio_skip( cio, 4);               /* L [at the end]     */
  cio_write( cio, JPIP_CPTR, 4);   /* T                  */
  cio_write( cio, 0, 2);           /* DR  A PRECISER !!  */
  cio_write( cio, 0, 2);           /* CONT               */
  cio_write( cio, coff, 8);    /* COFF A PRECISER !! */
  cio_write( cio, clen, 8);    /* CLEN               */
  len = cio_tell( cio) - lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);         /* L                  */
  cio_seek( cio, lenp+len);
}

void write_cptr_v2(int coff, int clen, opj_stream_private_t *cio,
              opj_event_mgr_t * p_manager )
{
  OPJ_BYTE l_data_header [3*8];
  int len, lenp;

#if 0
  lenp = cio_tell( cio);
  cio_skip( cio, 4);               /* L [at the end]     */
  cio_write( cio, JPIP_CPTR, 4);   /* T                  */
  cio_write( cio, 0, 2);           /* DR  A PRECISER !!  */
  cio_write( cio, 0, 2);           /* CONT               */
  cio_write( cio, coff, 8);    /* COFF A PRECISER !! */
  cio_write( cio, clen, 8);    /* CLEN               */
  len = cio_tell( cio) - lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);         /* L                  */
  cio_seek( cio, lenp+len);
#else
  lenp = opj_stream_tell(cio);
  opj_stream_skip( cio, 4, p_manager);               /* L [at the end]     */
  opj_write_bytes( l_data_header, JPIP_CPTR, 4);   /* T                  */
  opj_write_bytes( l_data_header+4, 0, 2);           /* DR  A PRECISER !!  */
  opj_write_bytes( l_data_header+6, 0, 2);           /* CONT               */
  opj_write_bytes( l_data_header+8, coff, 8);    /* COFF A PRECISER !! */
  opj_write_bytes( l_data_header+16, clen, 8);    /* CLEN               */
  opj_stream_write_data(cio,l_data_header,3*8,p_manager);

  len = opj_stream_tell(cio) - lenp;
  opj_stream_seek(cio,lenp,p_manager);
  opj_write_bytes(l_data_header, len, 4);         /* L                  */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
  opj_stream_seek(cio, lenp+len,p_manager);
#endif
}

void write_manf(int second, int v, opj_jp2_box_t *box, opj_cio_t *cio)
{
  int len, lenp, i;
  
  lenp = cio_tell( cio); 
  cio_skip( cio, 4);                         /* L [at the end]                    */
  cio_write( cio, JPIP_MANF,4);              /* T                                 */

  if (second){                          /* Write only during the second pass */
    for( i=0; i<v; i++){
      cio_write( cio, box[i].length, 4);  /* Box length                     */ 
      cio_write( cio, box[i].type, 4); /* Box type                       */
    }
  }

  len = cio_tell( cio) - lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);                   /* L                                 */
  cio_seek( cio, lenp+len);
}

void write_manf_v2(int second, int v, opj_jp2_box_t *box, opj_stream_private_t *cio,
              opj_event_mgr_t * p_manager )
{
  OPJ_BYTE l_data_header [4];
  int len, lenp, i;
  
#if 0
  lenp = cio_tell( cio); 
  cio_skip( cio, 4);                         /* L [at the end]                    */
  cio_write( cio, JPIP_MANF,4);              /* T                                 */
#else
  lenp = opj_stream_tell(cio);
  opj_stream_skip( cio, 4, p_manager);             /* L [at the end]     */
  opj_write_bytes( l_data_header, JPIP_MANF, 4);   /* T                  */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
#endif

  if (second){                          /* Write only during the second pass */
    for( i=0; i<v; i++){
#if 0
      cio_write( cio, box[i].length, 4);  /* Box length                     */ 
      cio_write( cio, box[i].type, 4); /* Box type                       */
#else
      opj_write_bytes( l_data_header, box[i].length, 4); /* Box length                     */
      opj_stream_write_data(cio,l_data_header,4,p_manager);
      opj_write_bytes( l_data_header, box[i].type, 4); /* Box type                       */
      opj_stream_write_data(cio,l_data_header,4,p_manager);
#endif
    }
  }

#if 0
  len = cio_tell( cio) - lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);                   /* L                                 */
  cio_seek( cio, lenp+len);
#else
  len = opj_stream_tell(cio) - lenp;
  opj_stream_seek(cio,lenp,p_manager);
  opj_write_bytes(l_data_header, len, 4);/* L                                 */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
  opj_stream_seek(cio,lenp+len,p_manager);
#endif
}

int write_mainmhix( int coff, opj_codestream_info_t cstr_info, opj_cio_t *cio)
{
  int i;
  int len, lenp;
  
  lenp = cio_tell( cio);
  cio_skip( cio, 4);                               /* L [at the end]                    */
  cio_write( cio, JPIP_MHIX, 4);                   /* MHIX                              */

  cio_write( cio, cstr_info.main_head_end-cstr_info.main_head_start+1, 8);        /* TLEN                              */

  for(i = 1; i < cstr_info.marknum; i++){    /* Marker restricted to 1 apparition, skip SOC marker */
    cio_write( cio, cstr_info.marker[i].type, 2);
    cio_write( cio, 0, 2);
    cio_write( cio, cstr_info.marker[i].pos-coff, 8);
    cio_write( cio, cstr_info.marker[i].len, 2);
  }

  len = cio_tell( cio) - lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);        /* L           */
  cio_seek( cio, lenp+len);
  
  return len;
}

int write_mainmhix_v2( int coff, opj_codestream_info_t cstr_info, opj_stream_private_t *cio,
              opj_event_mgr_t * p_manager )
{
  OPJ_BYTE l_data_header [8];
  int i;
  int len, lenp;
  
#if 0
  lenp = cio_tell( cio);
  cio_skip( cio, 4);                               /* L [at the end]                    */
  cio_write( cio, JPIP_MHIX, 4);                   /* MHIX                              */
#else
  lenp = opj_stream_tell (cio);
  opj_stream_skip(cio, 4, p_manager);               /* L [at the end]                    */
  opj_write_bytes(l_data_header,JPIP_MHIX,4);       /* MHIX                              */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
#endif

#if 0
  cio_write( cio, cstr_info.main_head_end-cstr_info.main_head_start+1, 8);        /* TLEN                              */
#else
  opj_write_bytes(l_data_header, cstr_info.main_head_end-cstr_info.main_head_start+1, 8);        /* TLEN                              */
  opj_stream_write_data(cio,l_data_header,8,p_manager);
#endif

  for(i = 1; i < cstr_info.marknum; i++){    /* Marker restricted to 1 apparition, skip SOC marker */
#if 0
    cio_write( cio, cstr_info.marker[i].type, 2);
    cio_write( cio, 0, 2);
    cio_write( cio, cstr_info.marker[i].pos-coff, 8);
    cio_write( cio, cstr_info.marker[i].len, 2);
#else
    opj_write_bytes( l_data_header, cstr_info.marker[i].type, 2);
    opj_write_bytes( l_data_header+2, 0, 2);
    opj_stream_write_data(cio,l_data_header,4,p_manager);
    opj_write_bytes( l_data_header, cstr_info.marker[i].pos-coff, 8);
    opj_stream_write_data(cio,l_data_header,8,p_manager);
    opj_write_bytes( l_data_header, cstr_info.marker[i].len, 2);
    opj_stream_write_data(cio,l_data_header,2,p_manager);
#endif
  }

#if 0
  len = cio_tell( cio) - lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);        /* L           */
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

opj_bool check_EPHuse( int coff, opj_marker_info_t *markers, int marknum, opj_cio_t *cio)
{
  opj_bool EPHused = OPJ_FALSE;
  int i=0;
  int org_pos;
  unsigned int Scod;

  for(i = 0; i < marknum; i++){
    if( markers[i].type == J2K_MS_COD){
      org_pos = cio_tell( cio);
      cio_seek( cio, coff+markers[i].pos+2);
      
      Scod = cio_read( cio, 1);
      if( ((Scod >> 2) & 1))
  EPHused = OPJ_TRUE;
      cio_seek( cio, org_pos);

      break;
    }
  }    
  return EPHused;
}

opj_bool check_EPHuse_v2( int coff, opj_marker_info_t *markers, int marknum, opj_stream_private_t *cio,
              opj_event_mgr_t * p_manager )
{
  OPJ_BYTE l_data_header [4];
  opj_bool EPHused = OPJ_FALSE;
  int i=0;
  int org_pos;
  unsigned int Scod;

  for(i = 0; i < marknum; i++)
    {
    if( markers[i].type == J2K_MS_COD)
      {
#if 0
      org_pos = cio_tell( cio);
      cio_seek( cio, coff+markers[i].pos+2);
#else
      org_pos = opj_stream_tell(cio);
      opj_stream_seek(cio, coff+markers[i].pos+2,p_manager);
#endif

#if 0
      Scod = cio_read( cio, 1);
#else
      opj_stream_read_data(cio,l_data_header,1,p_manager);
      opj_read_bytes(l_data_header,&Scod,1);
#endif
      if( ((Scod >> 2) & 1))
        EPHused = OPJ_TRUE;
#if 0
      cio_seek( cio, org_pos);
#else
      opj_stream_seek( cio, org_pos, p_manager);
#endif

      break;
      }
    }    
  return EPHused;
}
