/*
* Copyright (c) 2003-2004, François-Olivier Devaux
* Copyright (c) 2003-2004,  Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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
#include <string.h>
#include <time.h>
#include <math.h>

#include "mj2.h"
#include <cio.h>

#define MJ2_JP    0x6a502020
#define MJ2_FTYP  0x66747970
#define MJ2_MJ2   0x6d6a7032
#define MJ2_MJ2S  0x6d6a3273
#define MJ2_MDAT  0x6d646174
#define MJ2_MOOV  0x6d6f6f76
#define MJ2_MVHD  0x6d766864
#define MJ2_TRAK  0x7472616b
#define MJ2_TKHD  0x746b6864
#define MJ2_MDIA  0x6d646961
#define MJ2_MDHD  0x6d646864
#define MJ2_MHDR  0x6d686472
#define MJ2_HDLR  0x68646C72
#define MJ2_MINF  0x6d696e66
#define MJ2_VMHD  0x766d6864
#define MJ2_SMHD  0x736d6864
#define MJ2_HMHD  0x686d6864
#define MJ2_DINF  0x64696e66
#define MJ2_DREF  0x64726566
#define MJ2_URL   0x75726c20
#define MJ2_URN   0x75726e20
#define MJ2_STBL  0x7374626c
#define MJ2_STSD  0x73747364
#define MJ2_STTS  0x73747473
#define MJ2_STSC  0x73747363
#define MJ2_STSZ  0x7374737A
#define MJ2_STCO  0x7374636f
#define MJ2_MOOF  0x6d6f6f66
#define MJ2_FREE  0x66726565
#define MJ2_SKIP  0x736b6970
#define MJ2_JP2C  0x6a703263
#define MJ2_FIEL  0x6669656c
#define MJ2_JP2P  0x6a703270
#define MJ2_JP2X  0x6a703278
#define MJ2_JSUB  0x6a737562
#define MJ2_ORFO  0x6f72666f
#define MJ2_MVEX  0x6d766578
#define MJ2_JP2   0x6a703220
#define MJ2_J2P0  0x4a325030

/*
* 
* Free movie structure memory
*
*/
void mj2_memory_free(mj2_movie_t * movie)
{
  int i;
  mj2_tk_t *tk=NULL;

  if (movie->num_cl != 0)
    free(movie->cl);

  for (i = 0; i < movie->num_vtk + movie->num_stk + movie->num_htk; i++) {
    tk = &movie->tk[i];
    if (tk->name_size != 0)
      free(tk->name);
    if (tk->jp2_struct.comps != 0)
      free(tk->jp2_struct.comps);
    if (tk->jp2_struct.cl != 0)
      free(tk->jp2_struct.cl);
    if (tk->num_url != 0)
      free(tk->url);
    if (tk->num_urn != 0)
      free(tk->urn);
    if (tk->num_br != 0)
      free(tk->br);
    if (tk->num_jp2x != 0)
      free(tk->jp2xdata);
    if (tk->num_tts != 0)
      free(tk->tts);
    if (tk->num_chunks != 0)
      free(tk->chunk);
    if (tk->num_samplestochunk != 0)
      free(tk->sampletochunk);
    if (tk->num_samples != 0)
      free(tk->sample);
  }

  free(movie->tk);
}

/*
* 
* Read box headers
*
*/

int mj2_read_boxhdr(mj2_box_t * box)
{
  box->init_pos = cio_tell();
  box->length = cio_read(4);
  box->type = cio_read(4);
  if (box->length == 1) {
    if (cio_read(4) != 0) {
      fprintf(stderr, "Error: Cannot handle box sizes higher than 2^32\n");
      return 1;
    };
    box->length = cio_read(4);
    if (box->length == 0) 
      box->length = cio_numbytesleft() + 12;
  }
  else if (box->length == 0) {
    box->length = cio_numbytesleft() + 8;
  }
  return 0;
}

/*
* 
* Initialisation of a Standard Video Track
* with one sample per chunk
*/

int mj2_init_stdmovie(mj2_movie_t * movie)
{
  int i;
  unsigned int j;
  time_t ltime;
  movie->brand = MJ2_MJ2;
  movie->minversion = 0;
  movie->num_cl = 2;
  movie->cl =
    (unsigned int *) malloc(movie->num_cl * sizeof(unsigned int));

  movie->cl[0] = MJ2_MJ2;
  movie->cl[1] = MJ2_MJ2S;
  time(&ltime);			/* Time since 1/1/70 */
  movie->creation_time = (unsigned int) ltime + 2082844800;	/* Seconds between 1/1/04 and 1/1/70 */
  movie->timescale = 1000;

  movie->rate = 1 << 16;		/* Rate to play presentation  (default = 0x00010000)          */
  movie->volume = 1 << 8;		/* Movie volume (default = 0x0100)                            */
  movie->trans_matrix[0] = 0x00010000;	/* Transformation matrix for video                            */
  movie->trans_matrix[1] = 0;	/* Unity is { 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 }  */
  movie->trans_matrix[2] = 0;
  movie->trans_matrix[3] = 0;
  movie->trans_matrix[4] = 0x00010000;
  movie->trans_matrix[5] = 0;
  movie->trans_matrix[6] = 0;
  movie->trans_matrix[7] = 0;
  movie->trans_matrix[8] = 0x40000000;
  movie->next_tk_id = 1;

  for (i = 0; i < movie->num_htk + movie->num_stk + movie->num_vtk; i++) {
    mj2_tk_t *tk = &movie->tk[i];
    movie->next_tk_id++;
    if (tk->track_type == 0) {
      if (tk->num_samples == 0)
	return 1;

      tk->Dim[0] = 0;
      tk->Dim[1] = 0;

      tk->timescale = 1000;	/* Timescale = 1 ms                                          */

      tk->chunk[0].num_samples = 1;
      tk->chunk[0].sample_descr_idx = 1;

      tk->same_sample_size = 0;

      tk->num_samplestochunk = 1;	/* One sample per chunk                                      */
      tk->sampletochunk =
	(mj2_sampletochunk_t *) malloc(tk->num_samplestochunk *
				       sizeof(mj2_sampletochunk_t));
      tk->sampletochunk[0].first_chunk = 1;
      tk->sampletochunk[0].samples_per_chunk = 1;
      tk->sampletochunk[0].sample_descr_idx = 1;

      for (j = 0; j < tk->num_samples; j++)
	tk->sample[j].sample_delta = tk->timescale / tk->sample_rate;

      tk->num_tts = 1;
      tk->tts = (mj2_tts_t *) malloc(tk->num_tts * sizeof(mj2_tts_t));
      tk->tts[0].sample_count = tk->num_samples;
      tk->tts[0].sample_delta = tk->timescale / tk->sample_rate;

      tk->horizresolution = 0x00480000;	/* Horizontal resolution (typically 72)                       */
      tk->vertresolution = 0x00480000;	/* Vertical resolution (typically 72)                         */
      tk->compressorname[0] = 0x0f4d6f74;	/* Compressor Name[]: Motion JPEG2000                         */
      tk->compressorname[1] = 0x696f6e20;
      tk->compressorname[2] = 0x4a504547;
      tk->compressorname[3] = 0x32303030;
      tk->compressorname[4] = 0x00120000;
      tk->compressorname[5] = 0;
      tk->compressorname[6] = 0x00000042;
      tk->compressorname[7] = 0x000000DC;
      tk->num_url = 0;		/* Number of URL                                              */
      tk->num_urn = 0;		/* Number of URN                                              */
      tk->graphicsmode = 0;	/* Graphicsmode                                               */
      tk->opcolor[0] = 0;	/* OpColor                                                    */
      tk->opcolor[1] = 0;	/* OpColor                                                    */
      tk->opcolor[2] = 0;	/* OpColor                                                    */
      tk->creation_time = movie->creation_time;	/* Seconds between 1/1/04 and 1/1/70          */
      tk->language = 0;		/* Language (undefined)					      */
      tk->layer = 0;
      tk->volume = 1 << 8;		/* Movie volume (default = 0x0100) */
      tk->trans_matrix[0] = 0x00010000;	/* Transformation matrix for track */
      tk->trans_matrix[1] = 0;	/* Unity is { 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 }  */
      tk->trans_matrix[2] = 0;
      tk->trans_matrix[3] = 0;
      tk->trans_matrix[4] = 0x00010000;
      tk->trans_matrix[5] = 0;
      tk->trans_matrix[6] = 0;
      tk->trans_matrix[7] = 0;
      tk->trans_matrix[8] = 0x40000000;
      tk->fieldcount = 1;
      tk->fieldorder = 0;
      tk->or_fieldcount = 1;
      tk->or_fieldorder = 0;
      tk->num_br = 2;
      tk->br = (unsigned int *) malloc(tk->num_br * sizeof(unsigned int));
      tk->br[0] = MJ2_JP2;
      tk->br[1] = MJ2_J2P0;
      tk->num_jp2x = 0;
      tk->hsub = 2;		/* 4:2:0                                                      */
      tk->vsub = 2;		/* 4:2:0                                                      */
      tk->hoff = 0;
      tk->voff = 0;
      tk->visual_w = tk->w << 16;
      tk->visual_h = tk->h << 16;
    }
  }
  return 0;
}

/*
* Time To Sample box Decompact
*
*/
void mj2_tts_decompact(mj2_tk_t * tk)
{
  int i, j;
  tk->num_samples = 0;
  for (i = 0; i < tk->num_tts; i++) {
    tk->num_samples += tk->tts[i].sample_count;
  }

  tk->sample =
    (mj2_sample_t *) malloc(tk->num_samples * sizeof(mj2_sample_t));

  for (i = 0; i < tk->num_tts; i++) {
    for (j = 0; j < tk->tts[i].sample_count; j++) {
      tk->sample[j].sample_delta = tk->tts[i].sample_delta;
    }
  }
}

/*
* Sample To Chunk box Decompact
*
*/
void mj2_stsc_decompact(mj2_tk_t * tk)
{
  int j, i;
  unsigned int k;
  int sampleno=0;
  
  if (tk->num_samplestochunk == 1) {
    tk->num_chunks =
      (unsigned int) ceil((double) tk->num_samples /
      (double) tk->sampletochunk[0].samples_per_chunk);
    tk->chunk =
      (mj2_chunk_t *) malloc(tk->num_chunks * sizeof(mj2_chunk_t));
    for (k = 0; k < tk->num_chunks; k++) {
      tk->chunk[k].num_samples = tk->sampletochunk[0].samples_per_chunk;
    }
    
  } else {
    tk->chunk =
      (mj2_chunk_t *) malloc(tk->num_samples * sizeof(mj2_chunk_t));
    tk->num_chunks = 0;
    for (i = 0; i < tk->num_samplestochunk -1 ; i++) {
      for (j = tk->sampletochunk[i].first_chunk - 1;
      j < tk->sampletochunk[i + 1].first_chunk - 1; j++) {
	tk->chunk[j].num_samples = tk->sampletochunk[i].samples_per_chunk;
	tk->num_chunks++;
	sampleno += tk->chunk[j].num_samples;
      }
    }
    tk->num_chunks += (int)(tk->num_samples  - sampleno) / tk->sampletochunk[tk->num_samplestochunk - 1].samples_per_chunk;
    for (k = tk->sampletochunk[tk->num_samplestochunk - 1].first_chunk - 1;
    k < tk->num_chunks; k++) {
      tk->chunk[k].num_samples =
	tk->sampletochunk[tk->num_samplestochunk - 1].samples_per_chunk;
    }
    tk->chunk = realloc(tk->chunk, tk->num_chunks * sizeof(mj2_chunk_t));
  }
  
}


/*
* Chunk offset box Decompact
*
*/
void mj2_stco_decompact(mj2_tk_t * tk)
{
  int j;
  unsigned int i;
  int k = 0;
  int intra_chunk_offset;

  for (i = 0; i < tk->num_chunks; i++) {
    intra_chunk_offset = 0;
    for (j = 0; j < tk->chunk[i].num_samples; j++) {
      tk->sample[k].offset = intra_chunk_offset + tk->chunk[i].offset;
      intra_chunk_offset += tk->sample[k].sample_size;
      k++;
    }
  }
}

/*
* Write the JP box
*
* JP Signature box
*
*/
void mj2_write_jp()
{
  mj2_box_t box;
  box.init_pos = cio_tell();
  cio_skip(4);

  cio_write(MJ2_JP, 4);		/* JP */
  cio_write(0x0d0a870a, 4);	/* 0x0d0a870a required in a JP box */

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);
  cio_seek(box.init_pos + box.length);
}

/*
* Read the JP box
*
* JPEG 2000 signature
*
*/
int mj2_read_jp()
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_JP != box.type) {	/* Check Marker */
    fprintf(stderr, "Error: Expected JP Marker\n");
    return 1;
  }
  if (0x0d0a870a != cio_read(4)) {	/* read the 0x0d0a870a required in a JP box */
    fprintf(stderr, "Error with JP Marker\n");
    return 1;
  }
  if (cio_tell() - box.init_pos != box.length) {	/* Check box length */
    fprintf(stderr, "Error with JP Box size \n");
    return 1;
  }
  return 0;

}

/*
* Write the FTYP box
*
* File type box
*
*/
void mj2_write_ftyp(mj2_movie_t * movie)
{
  int i;
  mj2_box_t box;
  box.init_pos = cio_tell();
  cio_skip(4);

  cio_write(MJ2_FTYP, 4);	/* FTYP       */
  cio_write(movie->brand, 4);	/* BR         */
  cio_write(movie->minversion, 4);	/* MinV       */

  for (i = 0; i < movie->num_cl; i++)
    cio_write(movie->cl[i], 4);	/* CL         */

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* Length     */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the FTYP box
*
* File type box
*
*/
int mj2_read_ftyp(mj2_movie_t * movie)
{
  int i;
  mj2_box_t box;

  mj2_read_boxhdr(&box);	/* Box Size */
  if (MJ2_FTYP != box.type) {
    fprintf(stderr, "Error: Expected FTYP Marker\n");
    return 1;
  }

  movie->brand = cio_read(4);	/* BR              */
  movie->minversion = cio_read(4);	/* MinV            */
  movie->num_cl = (box.length - 16) / 4;
  movie->cl =
    (unsigned int *) malloc(movie->num_cl * sizeof(unsigned int));

  for (i = movie->num_cl - 1; i > -1; i--)
    movie->cl[i] = cio_read(4);	/* CLi */

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with FTYP Box\n");
    return 1;
  }
  return 0;
}


/*
* Write the STCO box
*
* Chunk Offset Box
*
*/
void mj2_write_stco(mj2_tk_t * tk)
{
  mj2_box_t box;
  unsigned int i;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_STCO, 4);	/* STCO       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  cio_write(tk->num_chunks, 4);	/* Entry Count */

  for (i = 0; i < tk->num_chunks; i++) {
    cio_write(tk->chunk[i].offset, 4);	/* Entry offset */
  }

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the STCO box
*
* Chunk Offset Box
*
*/
int mj2_read_stco(mj2_tk_t * tk)
{
  unsigned int i;
  mj2_box_t box;

  mj2_read_boxhdr(&box);	/* Box Size */
  if (MJ2_STCO != box.type) {
    fprintf(stderr, "Error: Expected STCO Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in STCO box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0  */
    fprintf(stderr, "Error with flag in STCO box. Expected flag 0\n");
    return 1;
  }


  if (cio_read(4) != tk->num_chunks) {
    fprintf(stderr,
	    "Error in STCO box: expecting same amount of entry-count as chunks \n");
  } else {
    for (i = 0; i < tk->num_chunks; i++) {
      tk->chunk[i].offset = cio_read(4);	/* Entry offset */
    }
  }

  mj2_stco_decompact(tk);


  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with STCO Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the STSZ box
*
* Sample size box
*
*/
void mj2_write_stsz(mj2_tk_t * tk)
{
  mj2_box_t box;
  unsigned int i;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_STSZ, 4);	/* STSZ       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  if (tk->same_sample_size == 1) {	/* If they all have the same size */
    cio_write(tk->sample[0].sample_size, 4);	/* Size */

    cio_write(1, 4);		/* Entry count = 1 */
  }

  else {
    cio_write(0, 4);		/* Sample Size = 0 becase they all have different sizes */

    cio_write(tk->num_samples, 4);	/* Sample Count */

    for (i = 0; i < tk->num_samples; i++) {
      cio_write(tk->sample[i].sample_size, 4);
    }
  }

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the STSZ box
*
* Sample size box
*
*/
int mj2_read_stsz(mj2_tk_t * tk)
{
  int sample_size;
  unsigned int i;
  mj2_box_t box;

  mj2_read_boxhdr(&box);	/* Box Size */
  if (MJ2_STSZ != box.type) {
    fprintf(stderr, "Error: Expected STSZ Marker\n");
    return 1;
  }


  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in STSZ box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0  */
    fprintf(stderr, "Error with flag in STSZ box. Expected flag 0\n");
    return 1;
  }

  sample_size = cio_read(4);

  if (sample_size != 0) {	/* Samples do have the same size */
    tk->same_sample_size = 1;
    for (i = 0; i < tk->num_samples; i++) {
      tk->sample[i].sample_size = sample_size;
    }
    cio_skip(4);		/* Sample count = 1 */
  } else {
    tk->same_sample_size = 0;
    if (tk->num_samples != cio_read(4)) {	/* Sample count */
      fprintf(stderr,
	      "Error in STSZ box. Expected that sample-count is number of samples in track\n");
      return 1;
    }
    for (i = 0; i < tk->num_samples; i++) {
      tk->sample[i].sample_size = cio_read(4);	/* Sample Size */
    }

    if (cio_tell() - box.init_pos != box.length) {
      fprintf(stderr, "Error with STSZ Box size\n");
      return 1;
    }
  }
  return 0;

}

/*
* Write the STSC box
*
* Sample to Chunk
*
*/
void mj2_write_stsc(mj2_tk_t * tk)
{
  int i;
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_STSC, 4);	/* STSC       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  cio_write(tk->num_samplestochunk, 4);	/* Entry Count */

  for (i = 0; i < tk->num_samplestochunk; i++) {
    cio_write(tk->sampletochunk[i].first_chunk, 4);	/* First Chunk */
    cio_write(tk->sampletochunk[i].samples_per_chunk, 4);	/* Samples per chunk */
    cio_write(tk->sampletochunk[i].sample_descr_idx, 4);	/* Samples description index */
  }


  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the STSC box
*
* Sample to Chunk
*
*/
int mj2_read_stsc(mj2_tk_t * tk)
{
  int i;
  mj2_box_t box;

  mj2_read_boxhdr(&box);	/* Box Size */
  if (MJ2_STSC != box.type) {
    fprintf(stderr, "Error: Expected STSC Marker\n");
    return 1;
  }


  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in STSC box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0  */
    fprintf(stderr, "Error with flag in STSC box. Expected flag 0\n");
    return 1;
  }

  tk->num_samplestochunk = cio_read(4);

  tk->sampletochunk =
    (mj2_sampletochunk_t *) malloc(tk->num_samplestochunk *
				   sizeof(mj2_sampletochunk_t));


  for (i = 0; i < tk->num_samplestochunk; i++) {
    tk->sampletochunk[i].first_chunk = cio_read(4);
    tk->sampletochunk[i].samples_per_chunk = cio_read(4);
    tk->sampletochunk[i].sample_descr_idx = cio_read(4);
  }

  mj2_stsc_decompact(tk);	/* decompact sample to chunk box */


  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with STSC Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the STTS box
*
* Time to Sample Box
*
*/
void mj2_write_stts(mj2_tk_t * tk)
{

  int i;
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_STTS, 4);	/* STTS       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  cio_write(tk->num_tts, 4);	/* entry_count */
  for (i = 0; i < tk->num_tts; i++) {
    cio_write(tk->tts[i].sample_count, 4);	/* Sample-count */
    cio_write(tk->tts[i].sample_delta, 4);	/* Sample-Delta */
  }

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the STTS box
*
* 
*
*/
int mj2_read_stts(mj2_tk_t * tk)
{
  int i;

  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_STTS != box.type) {
    fprintf(stderr, "Error: Expected STTS Marker\n");
    return 1;
  }


  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in STTS box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0  */
    fprintf(stderr, "Error with flag in STTS box. Expected flag 0\n");
    return 1;
  }

  tk->num_tts = cio_read(4);

  tk->tts = (mj2_tts_t *) malloc(tk->num_tts * sizeof(mj2_tts_t));

  for (i = 0; i < tk->num_tts; i++) {
    tk->tts[i].sample_count = cio_read(4);
    tk->tts[i].sample_delta = cio_read(4);
  }

  mj2_tts_decompact(tk);

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with STTS Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the FIEL box
*
* Field coding Box
*
*/
void mj2_write_fiel(mj2_tk_t * tk)
{

  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_FIEL, 4);	/* STTS       */

  cio_write(tk->fieldcount, 1);	/* Field count */
  cio_write(tk->fieldorder, 1);	/* Field order */


  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the FIEL box
*
* Field coding Box
*
*/
int mj2_read_fiel(mj2_tk_t * tk)
{

  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_FIEL != box.type) {
    fprintf(stderr, "Error: Expected FIEL Marker\n");
    return 1;
  }


  tk->fieldcount = cio_read(1);
  tk->fieldorder = cio_read(1);

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with FIEL Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the ORFO box
*
* Original Format Box
*
*/
void mj2_write_orfo(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_ORFO, 4);

  cio_write(tk->or_fieldcount, 1);	/* Original Field count */
  cio_write(tk->or_fieldorder, 1);	/* Original Field order */


  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the ORFO box
*
* Original Format Box
*
*/
int mj2_read_orfo(mj2_tk_t * tk)
{

  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_ORFO != box.type) {
    fprintf(stderr, "Error: Expected ORFO Marker\n");
    return 1;
  }


  tk->or_fieldcount = cio_read(1);
  tk->or_fieldorder = cio_read(1);

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with ORFO Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the JP2P box
*
* MJP2 Profile Box
*
*/
void mj2_write_jp2p(mj2_tk_t * tk)
{

  int i;
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_JP2P, 4);

  cio_write(0, 4);		/* Version 0, flags =0 */

  for (i = 0; i < tk->num_br; i++) {
    cio_write(tk->br[i], 4);
  }

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the JP2P box
*
* MJP2 Profile Box
*
*/
int mj2_read_jp2p(mj2_tk_t * tk)
{
  int i;

  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_JP2P != box.type) {
    fprintf(stderr, "Error: Expected JP2P Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in JP2P box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0  */
    fprintf(stderr, "Error with flag in JP2P box. Expected flag 0\n");
    return 1;
  }


  tk->num_br = (box.length - 12) / 4;
  tk->br = (unsigned int *) malloc(tk->num_br * sizeof(unsigned int));

  for (i = 0; i < tk->num_br; i++) {
    tk->br[i] = cio_read(4);
  }

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with JP2P Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the JP2X box
*
* MJP2 Prefix Box
*
*/
void mj2_write_jp2x(mj2_tk_t * tk)
{

  int i;
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_JP2X, 4);

  for (i = 0; i < tk->num_jp2x; i++) {
    cio_write(tk->jp2xdata[i], 1);
  }

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the JP2X box
*
* MJP2 Prefix Box
*
*/
int mj2_read_jp2x(mj2_tk_t * tk)
{
  unsigned int i;

  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_JP2X != box.type) {
    fprintf(stderr, "Error: Expected JP2X Marker\n");
    return 1;
  }


  tk->num_jp2x = (box.length - 8);
  tk->jp2xdata =
    (unsigned char *) malloc(tk->num_jp2x * sizeof(unsigned char));

  for (i = 0; i < tk->num_jp2x; i++) {
    tk->jp2xdata[i] = cio_read(1);
  }

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with JP2X Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the JSUB box
*
* MJP2 Subsampling Box
*
*/
void mj2_write_jsub(mj2_tk_t * tk)
{

  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_JSUB, 4);

  cio_write(tk->hsub, 1);
  cio_write(tk->vsub, 1);
  cio_write(tk->hoff, 1);
  cio_write(tk->voff, 1);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the JSUB box
*
* MJP2 Subsampling Box
*
*/
int mj2_read_jsub(mj2_tk_t * tk)
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_JSUB != box.type) {
    fprintf(stderr, "Error: Expected JSUB Marker\n");
    return 1;
  }

  tk->hsub = cio_read(1);
  tk->vsub = cio_read(1);
  tk->hoff = cio_read(1);;
  tk->voff = cio_read(1);

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with JSUB Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the SMJ2 box
*
* Visual Sample Entry Description
*
*/
void mj2_write_smj2(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_MJ2, 4);	/* MJ2       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  cio_write(1, 4);

  cio_write(0, 2);		/* Pre-defined */

  cio_write(0, 2);		/* Reserved */

  cio_write(0, 4);		/* Pre-defined */
  cio_write(0, 4);		/* Pre-defined */
  cio_write(0, 4);		/* Pre-defined */

  cio_write(tk->w, 2);		/* Width  */
  cio_write(tk->h, 2);		/* Height */

  cio_write(tk->horizresolution, 4);	/* Horizontal resolution */
  cio_write(tk->vertresolution, 4);	/* Vertical resolution   */

  cio_write(0, 4);		/* Reserved */

  cio_write(1, 2);		/* Pre-defined = 1 */

  cio_write(tk->compressorname[0], 4);	/* Compressor Name */
  cio_write(tk->compressorname[1], 4);
  cio_write(tk->compressorname[2], 4);
  cio_write(tk->compressorname[3], 4);
  cio_write(tk->compressorname[4], 4);
  cio_write(tk->compressorname[5], 4);
  cio_write(tk->compressorname[6], 4);
  cio_write(tk->compressorname[7], 4);

  cio_write(tk->depth, 2);	/* Depth */

  cio_write(0xffff, 2);		/* Pre-defined = -1 */

  jp2_write_jp2h(&tk->jp2_struct);

  mj2_write_fiel(tk);

  if (tk->num_br != 0)
    mj2_write_jp2p(tk);
  if (tk->num_jp2x != 0)
    mj2_write_jp2x(tk);

  mj2_write_jsub(tk);
  mj2_write_orfo(tk);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the SMJ2 box
*
* Visual Sample Entry Description
*
*/
int mj2_read_smj2(j2k_image_t * img, mj2_tk_t * tk)
{
  mj2_box_t box;
  mj2_box_t box2;
  int i;

  mj2_read_boxhdr(&box);

  if (MJ2_MJ2 != box.type) {
    fprintf(stderr, "Error in SMJ2 box: Expected MJ2 Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in MJP2 box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0  */
    fprintf(stderr, "Error with flag in MJP2 box. Expected flag 0\n");
    return 1;
  }

  cio_skip(4);

  cio_skip(2);			/* Pre-defined */

  cio_skip(2);			/* Reserved */

  cio_skip(4);			/* Pre-defined */
  cio_skip(4);			/* Pre-defined */
  cio_skip(4);			/* Pre-defined */

  tk->w = cio_read(2);		/* Width  */
  tk->h = cio_read(2);		/* Height */

  tk->horizresolution = cio_read(4);	/* Horizontal resolution */
  tk->vertresolution = cio_read(4);	/* Vertical resolution   */

  cio_skip(4);			/* Reserved */

  cio_skip(2);			/* Pre-defined = 1 */

  tk->compressorname[0] = cio_read(4);	/* Compressor Name */
  tk->compressorname[1] = cio_read(4);
  tk->compressorname[2] = cio_read(4);
  tk->compressorname[3] = cio_read(4);
  tk->compressorname[4] = cio_read(4);
  tk->compressorname[5] = cio_read(4);
  tk->compressorname[6] = cio_read(4);
  tk->compressorname[7] = cio_read(4);

  tk->depth = cio_read(2);	/* Depth */

  /* Init std value */
  tk->num_jp2x = 0;
  tk->fieldcount = 1;
  tk->fieldorder = 0;
  tk->or_fieldcount = 1;
  tk->or_fieldorder = 0;

  cio_skip(2);			/* Pre-defined = -1 */

  if (jp2_read_jp2h(&tk->jp2_struct)) {
    fprintf(stderr, "Error with JP2H Box\n");
    return 1;
  }
  
  tk->jp2_struct.comps = (jp2_comps_t *) malloc(tk->jp2_struct.numcomps * sizeof(jp2_comps_t));
  tk->jp2_struct.cl = (int *) malloc(sizeof(int));

  tk->num_br = 0;
  tk->num_jp2x = 0;

  for (i = 0; cio_tell() - box.init_pos < box.length; i++) {
    mj2_read_boxhdr(&box2);
    cio_seek(box2.init_pos);
    switch (box2.type) {
    case MJ2_FIEL:
      if (mj2_read_fiel(tk))
	return 1;
      break;

    case MJ2_JP2P:
      if (mj2_read_jp2p(tk))
	return 1;
      break;

    case MJ2_JP2X:
      if (mj2_read_jp2x(tk))
	return 1;
      break;

    case MJ2_JSUB:
      if (mj2_read_jsub(tk))
	return 1;
      break;

    case MJ2_ORFO:
      if (mj2_read_orfo(tk))
	return 1;
      break;

    default:
      fprintf(stderr, "Error with MJP2 Box size\n");
      return 1;
      break;

    }
  }
  return 0;
}


/*
* Write the STSD box
*
* Sample Description
*
*/
void mj2_write_stsd(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_STSD, 4);	/* STSD       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  cio_write(1, 4);		/* entry_count = 1 (considering same JP2 headerboxes) */

  if (tk->track_type == 0) {
    mj2_write_smj2(tk);
  } else if (tk->track_type == 1) {
    // Not implemented
  }
  if (tk->track_type == 2) {
    // Not implemented
  }


  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the STSD box
*
* Sample Description
*
*/
int mj2_read_stsd(mj2_tk_t * tk, j2k_image_t * img)
{
  int i;
  int entry_count, len_2skip;

  mj2_box_t box;

  mj2_read_boxhdr(&box);

  if (MJ2_STSD != box.type) {
    fprintf(stderr, "Error: Expected STSD Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in STSD box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0  */
    fprintf(stderr, "Error with flag in STSD box. Expected flag 0\n");
    return 1;
  }

  entry_count = cio_read(4);

  if (tk->track_type == 0) {
    for (i = 0; i < entry_count; i++) {
      if (mj2_read_smj2(img, tk))
	return 1;
    }
  } else if (tk->track_type == 1) {
    len_2skip = cio_read(4);	// Not implemented -> skipping box
    cio_skip(len_2skip - 4);
  } else if (tk->track_type == 2) {
    len_2skip = cio_read(4);	// Not implemented -> skipping box
    cio_skip(len_2skip - 4);
  }


  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with STSD Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the STBL box
*
* Sample table box box
*
*/
void mj2_write_stbl(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_STBL, 4);	/* STBL       */

  mj2_write_stsd(tk);
  mj2_write_stts(tk);
  mj2_write_stsc(tk);
  mj2_write_stsz(tk);
  mj2_write_stco(tk);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the STBL box
*
* Sample table box box
*
*/
int mj2_read_stbl(mj2_tk_t * tk, j2k_image_t * img)
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_STBL != box.type) {
    fprintf(stderr, "Error: Expected STBL Marker\n");
    return 1;
  }

  if (mj2_read_stsd(tk, img))
    return 1;
  if (mj2_read_stts(tk))
    return 1;
  if (mj2_read_stsc(tk))
    return 1;
  if (mj2_read_stsz(tk))
    return 1;
  if (mj2_read_stco(tk))
    return 1;

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with STBL Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the URL box
*
* URL box
*
*/
void mj2_write_url(mj2_tk_t * tk, int url_num)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_URL, 4);	/* URL       */

  if (url_num == 0)
    cio_write(1, 4);		/* Version = 0, flags = 1 because stored in same file */
  else {
    cio_write(0, 4);		/* Version = 0, flags =  0 */
    cio_write(tk->url[url_num - 1].location[0], 4);
    cio_write(tk->url[url_num - 1].location[1], 4);
    cio_write(tk->url[url_num - 1].location[2], 4);
    cio_write(tk->url[url_num - 1].location[3], 4);
  }

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the URL box
*
* URL box
*
*/
int mj2_read_url(mj2_tk_t * tk, int urn_num)
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_URL != box.type) {
    fprintf(stderr, "Error: Expected URL Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in URL box\n");
    return 1;
  }

  if (1 != cio_read(3)) {	/* If flags = 1 --> media data in file */
    tk->url[urn_num].location[0] = cio_read(4);
    tk->url[urn_num].location[1] = cio_read(4);
    tk->url[urn_num].location[2] = cio_read(4);
    tk->url[urn_num].location[3] = cio_read(4);
  } else {
    tk->num_url--;
  }


  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with URL Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the URN box
*
* URN box
*
*/
void mj2_write_urn(mj2_tk_t * tk, int urn_num)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_URN, 4);	/* URN       */

  cio_write(0, 4);		/* Version = 0, flags =  0 */

  cio_write(tk->urn[urn_num].name[0], 4);
  cio_write(tk->urn[urn_num].name[1], 4);
  cio_write(tk->urn[urn_num].name[2], 4);
  cio_write(tk->urn[urn_num].name[3], 4);
  cio_write(tk->urn[urn_num].location[0], 4);
  cio_write(tk->urn[urn_num].location[1], 4);
  cio_write(tk->urn[urn_num].location[2], 4);
  cio_write(tk->urn[urn_num].location[3], 4);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the URN box
*
* URN box
*
*/
int mj2_read_urn(mj2_tk_t * tk, int urn_num)
{

  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_URN != box.type) {
    fprintf(stderr, "Error: Expected URN Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in URN box\n");
    return 1;
  }

  if (1 != cio_read(3)) {	/* If flags = 1 --> media data in file */
    tk->urn[urn_num].name[0] = cio_read(4);
    tk->urn[urn_num].name[1] = cio_read(4);
    tk->urn[urn_num].name[2] = cio_read(4);
    tk->urn[urn_num].name[3] = cio_read(4);
    tk->urn[urn_num].location[0] = cio_read(4);
    tk->urn[urn_num].location[1] = cio_read(4);
    tk->urn[urn_num].location[2] = cio_read(4);
    tk->urn[urn_num].location[3] = cio_read(4);
  }


  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with URN Box size\n");
    return 1;
  }
  return 0;
}


/*
* Write the DREF box
*
* Data reference box
*
*/
void mj2_write_dref(mj2_tk_t * tk)
{
  int i;
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_DREF, 4);	/* DREF       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  if (tk->num_url + tk->num_urn == 0) {	/* Media data in same file */
    cio_write(1, 4);		/* entry_count = 1 */
    mj2_write_url(tk, 0);
  } else {
    cio_write(tk->num_url + tk->num_urn, 4);	/* entry_count */

    for (i = 0; i < tk->num_url; i++)
      mj2_write_url(tk, i + 1);

    for (i = 0; i < tk->num_urn; i++)
      mj2_write_urn(tk, i);
  }

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the DREF box
*
* Data reference box
*
*/
int mj2_read_dref(mj2_tk_t * tk)
{

  int i;
  int entry_count, marker;
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_DREF != box.type) {
    fprintf(stderr, "Error: Expected DREF Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in DREF box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0  */
    fprintf(stderr, "Error with flag in DREF box. Expected flag 0\n");
    return 1;
  }

  entry_count = cio_read(4);
  tk->num_url = 0;
  tk->num_urn = 0;

  for (i = 0; i < entry_count; i++) {
    cio_skip(4);
    marker = cio_read(4);
    if (marker == MJ2_URL) {
      cio_skip(-8);
      tk->num_url++;
      if (mj2_read_url(tk, tk->num_url))
	return 1;
    } else if (marker == MJ2_URN) {
      cio_skip(-8);
      tk->num_urn++;
      if (mj2_read_urn(tk, tk->num_urn))
	return 1;
    } else {
      fprintf(stderr, "Error with in DREF box. Expected URN or URL box\n");
      return 1;
    }

  }


  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with DREF Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the DINF box
*
* Data information box
*
*/
void mj2_write_dinf(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_DINF, 4);	/* DINF       */

  mj2_write_dref(tk);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the DINF box
*
* Data information box
*
*/
int mj2_read_dinf(mj2_tk_t * tk)
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_DINF != box.type) {
    fprintf(stderr, "Error: Expected DINF Marker\n");
    return 1;
  }

  if (mj2_read_dref(tk))
    return 1;

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with DINF Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the VMHD box
*
* Video Media information box
*
*/
void mj2_write_vmhd(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_VMHD, 4);	/* VMHD       */

  cio_write(1, 4);		/* Version = 0, flags = 1 */

  cio_write(tk->graphicsmode, 2);
  cio_write(tk->opcolor[0], 2);
  cio_write(tk->opcolor[1], 2);
  cio_write(tk->opcolor[2], 2);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the VMHD box
*
* Video Media information box
*
*/
int mj2_read_vmhd(mj2_tk_t * tk)
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_VMHD != box.type) {
    fprintf(stderr, "Error: Expected VMHD Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in VMHD box\n");
    return 1;
  }

  if (1 != cio_read(3)) {	/* Flags = 1  */
    fprintf(stderr, "Error with flag in VMHD box. Expected flag 1\n");
    return 1;
  }

  tk->track_type = 0;
  tk->graphicsmode = cio_read(2);
  tk->opcolor[0] = cio_read(2);
  tk->opcolor[1] = cio_read(2);
  tk->opcolor[2] = cio_read(2);

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with VMHD Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the SMHD box
*
* Sound Media information box
*
*/
void mj2_write_smhd(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_SMHD, 4);	/* SMHD       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  cio_write(tk->balance, 2);

  cio_write(0, 2);		/* Reserved */

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the SMHD box
*
* Sound Media information box
*
*/
int mj2_read_smhd(mj2_tk_t * tk)
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_SMHD != box.type) {
    fprintf(stderr, "Error: Expected SMHD Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in SMHD box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0  */
    fprintf(stderr, "Error with flag in SMHD box. Expected flag 0\n");
    return 1;
  }

  tk->track_type = 1;
  tk->balance = cio_read(2);

  cio_skip(2);			/* Reserved */

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with SMHD Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the HMHD box
*
* Hint Media information box
*
*/
void mj2_write_hmhd(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_HMHD, 4);	/* HMHD       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  cio_write(tk->maxPDUsize, 2);
  cio_write(tk->avgPDUsize, 2);
  cio_write(tk->maxbitrate, 4);
  cio_write(tk->avgbitrate, 4);
  cio_write(tk->slidingavgbitrate, 4);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the HMHD box
*
* Hint Media information box
*
*/
int mj2_read_hmhd(mj2_tk_t * tk)
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_HMHD != box.type) {
    fprintf(stderr, "Error: Expected HMHD Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in HMHD box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0  */
    fprintf(stderr, "Error with flag in HMHD box. Expected flag 0\n");
    return 1;
  }

  tk->track_type = 2;
  tk->maxPDUsize = cio_read(2);
  tk->avgPDUsize = cio_read(2);
  tk->maxbitrate = cio_read(4);
  tk->avgbitrate = cio_read(4);
  tk->slidingavgbitrate = cio_read(4);

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with HMHD Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the MINF box
*
* Media information box
*
*/
void mj2_write_minf(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_MINF, 4);	/* MINF       */

  if (tk->track_type == 0) {
    mj2_write_vmhd(tk);
  } else if (tk->track_type == 1) {
    mj2_write_smhd(tk);
  } else if (tk->track_type == 2) {
    mj2_write_hmhd(tk);
  }

  mj2_write_dinf(tk);
  mj2_write_stbl(tk);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the MINF box
*
* Media information box
*
*/
int mj2_read_minf(mj2_tk_t * tk, j2k_image_t * img)
{

  unsigned int box_type;
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_MINF != box.type) {
    fprintf(stderr, "Error: Expected MINF Marker\n");
    return 1;
  }

  cio_skip(4);
  box_type = cio_read(4);
  cio_skip(-8);

  if (box_type == MJ2_VMHD) {
    if (mj2_read_vmhd(tk))
      return 1;
  } else if (box_type == MJ2_SMHD) {
    if (mj2_read_smhd(tk))
      return 1;
  } else if (box_type == MJ2_HMHD) {
    if (mj2_read_hmhd(tk))
      return 1;
  } else {
    fprintf(stderr, "Error in MINF box expected vmhd, smhd or hmhd\n");
    return 1;
  }

  if (mj2_read_dinf(tk))
    return 1;

  if (mj2_read_stbl(tk, img))
    return 1;

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with MINF Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the HDLR box
*
* Handler reference box
*
*/
void mj2_write_hdlr(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_HDLR, 4);	/* HDLR       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  cio_write(0, 4);		/* Predefine */

  tk->name = 0;			/* The track name is immediately determined by the track type */

  if (tk->track_type == 0) {
    tk->handler_type = 0x76696465;	/* Handler type: vide */
    cio_write(tk->handler_type, 4);

    cio_write(0, 4);
    cio_write(0, 4);
    cio_write(0, 4);		/* Reserved */

    cio_write(0x76696465, 4);
    cio_write(0x6F206d65, 4);
    cio_write(0x64696120, 4);
    cio_write(0x74726163, 4);
    cio_write(0x6b00, 2);	/* String: video media track */
  } else if (tk->track_type == 1) {
    tk->handler_type = 0x736F756E;	/* Handler type: soun */
    cio_write(tk->handler_type, 4);

    cio_write(0, 4);
    cio_write(0, 4);
    cio_write(0, 4);		/* Reserved */

    cio_write(0x536F756E, 4);
    cio_write(0x6400, 2);	/* String: Sound */
  } else if (tk->track_type == 2) {
    tk->handler_type = 0x68696E74;	/* Handler type: hint */
    cio_write(tk->handler_type, 4);

    cio_write(0, 4);
    cio_write(0, 4);
    cio_write(0, 4);		/* Reserved */

    cio_write(0x48696E74, 4);
    cio_write(0, 2);		/* String: Hint */
  }

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the HDLR box
*
* Handler reference box
*
*/
int mj2_read_hdlr(mj2_tk_t * tk)
{
  int i;
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_HDLR != box.type) {
    fprintf(stderr, "Error: Expected HDLR Marker\n");
    return 1;
  }


  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in HDLR box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0  */
    fprintf(stderr, "Error with flag in HDLR box. Expected flag 0\n");
    return 1;
  }

  cio_skip(4);			/* Reserved */

  tk->handler_type = cio_read(4);
  cio_skip(12);			/* Reserved */

  tk->name_size = box.length - 32;

  tk->name = (char *) malloc(tk->name_size * sizeof(char));
  for (i = 0; i < tk->name_size; i++) {
    tk->name[i] = cio_read(1);	/* Name */
  }

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with HDLR Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the MDHD box
*
* Media Header Box
*
*/
void mj2_write_mdhd(mj2_tk_t * tk)
{
  mj2_box_t box;
  unsigned int i;
  time_t ltime;
  unsigned int modification_time;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_MDHD, 4);	/* MDHD       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  cio_write(tk->creation_time, 4);	/* Creation Time */

  time(&ltime);			/* Time since 1/1/70 */
  modification_time = ltime + 2082844800;	/* Seoonds between 1/1/04 and 1/1/70 */

  cio_write(modification_time, 4);	/* Modification Time */

  cio_write(tk->timescale, 4);	/* Timescale */

  tk->duration = 0;

  for (i = 0; i < tk->num_samples; i++)
    tk->duration += tk->sample[i].sample_delta;

  cio_write(tk->duration, 4);	/* Duration */

  cio_write(tk->language, 2);	/* Language */

  cio_write(0, 2);		/* Predefined */

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the MDHD box
*
* Media Header Box
*
*/
int mj2_read_mdhd(mj2_tk_t * tk)
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (!(MJ2_MHDR == box.type || MJ2_MDHD == box.type)) {	// Kakadu writes MHDR instead of MDHD
    fprintf(stderr, "Error: Expected MDHD Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in MDHD box\n");
    return 1;
  }

  if (0 != cio_read(3)) {	/* Flags = 0 */
    fprintf(stderr, "Error with flag in MDHD box. Expected flag 0\n");
    return 1;
  }


  tk->creation_time = cio_read(4);	/* Creation Time */

  tk->modification_time = cio_read(4);	/* Modification Time */

  tk->timescale = cio_read(4);	/* Timescale */

  tk->duration = cio_read(4);	/* Duration */

  tk->language = cio_read(2);	/* Language */

  cio_skip(2);			/* Predefined */

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with MDHD Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the MDIA box
*
* Media box
*
*/
void mj2_write_mdia(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_MDIA, 4);	/* MDIA       */

  mj2_write_mdhd(tk);
  mj2_write_hdlr(tk);
  mj2_write_minf(tk);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the MDIA box
*
* Media box
*
*/
int mj2_read_mdia(mj2_tk_t * tk, j2k_image_t * img)
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_MDIA != box.type) {
    fprintf(stderr, "Error: Expected MDIA Marker\n");
    return 1;
  }

  if (mj2_read_mdhd(tk))
    return 1;
  if (mj2_read_hdlr(tk))
    return 1;
  if (mj2_read_minf(tk, img))
    return 1;

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with MDIA Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the TKHD box
*
* Track Header box
*
*/
void mj2_write_tkhd(mj2_tk_t * tk)
{
  mj2_box_t box;
  unsigned int i;
  time_t ltime;

  box.init_pos = cio_tell();
  cio_skip(4);

  cio_write(MJ2_TKHD, 4);	/* TKHD       */

  cio_write(3, 4);		/* Version=0, flags=3 */

  time(&ltime);			/* Time since 1/1/70 */
  tk->modification_time = ltime + 2082844800;	/* Seoonds between 1/1/04 and 1/1/70 */

  cio_write(tk->creation_time, 4);	/* Creation Time */

  cio_write(tk->modification_time, 4);	/* Modification Time */

  cio_write(tk->track_ID, 4);	/* Track ID */

  cio_write(0, 4);		/* Reserved */

  tk->duration = 0;

  for (i = 0; i < tk->num_samples; i++)
    tk->duration += tk->sample[i].sample_delta;

  cio_write(tk->duration, 4);	/* Duration */

  cio_write(0, 4);		/* Reserved */
  cio_write(0, 4);		/* Reserved */

  cio_write(tk->layer, 2);	/* Layer    */

  cio_write(0, 2);		/* Predefined */

  cio_write(tk->volume, 2);	/* Volume       */

  cio_write(0, 2);		/* Reserved */

  cio_write(tk->trans_matrix[0], 4);	/* Transformation matrix for track */
  cio_write(tk->trans_matrix[1], 4);
  cio_write(tk->trans_matrix[2], 4);
  cio_write(tk->trans_matrix[3], 4);
  cio_write(tk->trans_matrix[4], 4);
  cio_write(tk->trans_matrix[5], 4);
  cio_write(tk->trans_matrix[6], 4);
  cio_write(tk->trans_matrix[7], 4);
  cio_write(tk->trans_matrix[8], 4);

  cio_write(tk->visual_w, 4);	/* Video Visual Width  */

  cio_write(tk->visual_h, 4);	/* Video Visual Height */

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the TKHD box
*
* Track Header box
*
*/
int mj2_read_tkhd(mj2_tk_t * tk)
{
  int flag;

  mj2_box_t box;

  mj2_read_boxhdr(&box);

  if (MJ2_TKHD != box.type) {
    fprintf(stderr, "Error: Expected TKHD Marker\n");
    return 1;
  }

  if (0 != cio_read(1)) {	/* Version = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in TKHD box\n");
    return 1;
  }

  flag = cio_read(3);

  if (!(flag == 1 || flag == 2 || flag == 3 || flag == 4)) {	/* Flags = 1,2,3 or 4 */
    fprintf(stderr,
	    "Error with flag in TKHD box: Expected flag 1,2,3 or 4\n");
    return 1;
  }

  tk->creation_time = cio_read(4);	/* Creation Time */

  tk->modification_time = cio_read(4);	/* Modification Time */

  tk->track_ID = cio_read(4);	/* Track ID */

  cio_skip(4);			/* Reserved */

  tk->duration = cio_read(4);	/* Duration */

  cio_skip(8);			/* Reserved */

  tk->layer = cio_read(2);	/* Layer    */

  cio_read(2);			/* Predefined */

  tk->volume = cio_read(2);	/* Volume       */

  cio_skip(2);			/* Reserved */

  tk->trans_matrix[0] = cio_read(4);	/* Transformation matrix for track */
  tk->trans_matrix[1] = cio_read(4);
  tk->trans_matrix[2] = cio_read(4);
  tk->trans_matrix[3] = cio_read(4);
  tk->trans_matrix[4] = cio_read(4);
  tk->trans_matrix[5] = cio_read(4);
  tk->trans_matrix[6] = cio_read(4);
  tk->trans_matrix[7] = cio_read(4);
  tk->trans_matrix[8] = cio_read(4);

  tk->visual_w = cio_read(4);	/* Video Visual Width  */

  tk->visual_h = cio_read(4);	/* Video Visual Height */

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with TKHD Box size\n");
    return 1;
  }
  return 0;
}

/*
* Write the TRAK box
*
* Track box
*
*/
void mj2_write_trak(mj2_tk_t * tk)
{
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);

  cio_write(MJ2_TRAK, 4);	/* TRAK       */

  mj2_write_tkhd(tk);
  mj2_write_mdia(tk);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the TRAK box
*
* Track box
*
*/
int mj2_read_trak(mj2_tk_t * tk, j2k_image_t * img)
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_TRAK != box.type) {
    fprintf(stderr, "Error: Expected TRAK Marker\n");
    return 1;
  }
  if (mj2_read_tkhd(tk))
    return 1;
  if (mj2_read_mdia(tk, img))
    return 1;
  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with TRAK Box\n");
    return 1;
  }
  return 0;
}

/*
* Write the MVHD box
*
* Movie header Box
*
*/
void mj2_write_mvhd(mj2_movie_t * movie)
{
  int i;
  mj2_box_t box;
  unsigned j;
  time_t ltime;
  int max_tk_num = 0;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_MVHD, 4);	/* MVHD       */

  cio_write(0, 4);		/* Version = 0, flags = 0 */

  time(&ltime);			/* Time since 1/1/70 */
  movie->modification_time = ltime + 2082844800;	/* Seoonds between 1/1/04 and 1/1/70 */

  cio_write(movie->creation_time, 4);	/* Creation Time */

  cio_write(movie->modification_time, 4);	/* Modification Time */

  cio_write(movie->timescale, 4);	/* Timescale */

  movie->duration = 0;

  for (i = 0; i < (movie->num_stk + movie->num_htk + movie->num_vtk); i++) {
    mj2_tk_t *tk = &movie->tk[i];

    for (j = 0; j < tk->num_samples; j++) {
      movie->duration += tk->sample[j].sample_delta;
    }
  }

  cio_write(movie->duration, 4);

  cio_write(movie->rate, 4);	/* Rate to play presentation    */

  cio_write(movie->volume, 2);	/* Volume       */

  cio_write(0, 2);		/* Reserved */
  cio_write(0, 4);		/* Reserved */
  cio_write(0, 4);		/* Reserved */

  cio_write(movie->trans_matrix[0], 4);	/* Transformation matrix for video */
  cio_write(movie->trans_matrix[1], 4);
  cio_write(movie->trans_matrix[2], 4);
  cio_write(movie->trans_matrix[3], 4);
  cio_write(movie->trans_matrix[4], 4);
  cio_write(movie->trans_matrix[5], 4);
  cio_write(movie->trans_matrix[6], 4);
  cio_write(movie->trans_matrix[7], 4);
  cio_write(movie->trans_matrix[8], 4);

  cio_write(0, 4);		/* Pre-defined */
  cio_write(0, 4);		/* Pre-defined */
  cio_write(0, 4);		/* Pre-defined */
  cio_write(0, 4);		/* Pre-defined */
  cio_write(0, 4);		/* Pre-defined */
  cio_write(0, 4);		/* Pre-defined */


  for (i = 0; i < movie->num_htk + movie->num_stk + movie->num_vtk; i++) {
    if (max_tk_num < movie->tk[i].track_ID)
      max_tk_num = movie->tk[i].track_ID;
  }

  movie->next_tk_id = max_tk_num + 1;

  cio_write(movie->next_tk_id, 4);	/* ID of Next track to be added */

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the MVHD box
*
* Movie header Box
*
*/
int mj2_read_mvhd(mj2_movie_t * movie)
{
  mj2_box_t box;

  mj2_read_boxhdr(&box);
  if (MJ2_MVHD != box.type) {
    fprintf(stderr, "Error: Expected MVHD Marker\n");
    return 1;
  }


  if (0 != cio_read(4)) {	/* Version = 0, flags = 0 */
    fprintf(stderr, "Error: Only Version 0 handled in MVHD box\n");
  }

  movie->creation_time = cio_read(4);	/* Creation Time */

  movie->modification_time = cio_read(4);	/* Modification Time */

  movie->timescale = cio_read(4);	/* Timescale */

  movie->duration = cio_read(4);	/* Duration */

  movie->rate = cio_read(4);		/* Rate to play presentation    */

  movie->volume = cio_read(2);		/* Volume       */

  cio_skip(10);				/* Reserved */

  movie->trans_matrix[0] = cio_read(4);	/* Transformation matrix for video */
  movie->trans_matrix[1] = cio_read(4);
  movie->trans_matrix[2] = cio_read(4);
  movie->trans_matrix[3] = cio_read(4);
  movie->trans_matrix[4] = cio_read(4);
  movie->trans_matrix[5] = cio_read(4);
  movie->trans_matrix[6] = cio_read(4);
  movie->trans_matrix[7] = cio_read(4);
  movie->trans_matrix[8] = cio_read(4);

  cio_skip(24);			/* Pre-defined */

  movie->next_tk_id = cio_read(4);	/* ID of Next track to be added */

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with MVHD Box Size\n");
    return 1;
  }
  return 0;
}


/*
* Write the MOOV box
*
* Movie Box
*
*/
void mj2_write_moov(mj2_movie_t * movie)
{
  int i;
  mj2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_MOOV, 4);	/* MOOV       */

  mj2_write_mvhd(movie);

  for (i = 0; i < (movie->num_stk + movie->num_htk + movie->num_vtk); i++) {
    mj2_write_trak(&movie->tk[i]);
  }

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/* L          */
  cio_seek(box.init_pos + box.length);

}

/*
* Read the MOOV box
*
* Movie Box
*
*/
int mj2_read_moov(mj2_movie_t * movie, j2k_image_t * img)
{
  unsigned int i;
  mj2_box_t box;
  mj2_box_t box2;

  mj2_read_boxhdr(&box);
  if (MJ2_MOOV != box.type) {
    fprintf(stderr, "Error: Expected MOOV Marker\n");
    return 1;
  }



  if (mj2_read_mvhd(movie))
    return 1;


  movie->tk =
    (mj2_tk_t *) malloc((movie->next_tk_id - 1) * sizeof(mj2_tk_t));

  for (i = 0; cio_tell() - box.init_pos < box.length; i++) {
    mj2_read_boxhdr(&box2);
    if (box2.type == MJ2_TRAK) {
      cio_seek(box2.init_pos);
      if (mj2_read_trak(&movie->tk[i], img))
	return 1;

      if (movie->tk[i].track_type == 0) {
	movie->num_vtk++;
      } else if (movie->tk[i].track_type == 1) {
	movie->num_stk++;
      } else if (movie->tk[i].track_type == 2) {
	movie->num_htk++;
      }
    } else if (box2.type == MJ2_MVEX) {
      cio_seek(box2.init_pos);
      cio_skip(box2.length);
      i--;
    } else {
      fprintf(stderr, "Error with MOOV Box: Expected TRAK or MVEX box\n");
      return 1;
    }
  }
  return 0;
}

int mj2_read_struct(FILE *file, mj2_movie_t * movie) {
  mj2_box_t box;
  j2k_image_t img;
  char * src;
  int fsresult;
  int foffset;

  movie->num_vtk=0;
  movie->num_stk=0;
  movie->num_htk=0;
    
  src = (char*) malloc (300 * sizeof(char));
  fread(src,300,1, file);  // Assuming that jp and ftyp markers size do
  // not exceed 300 bytes
  
  cio_init(src, 300);
  
  if (mj2_read_jp())
    return 1;
  if (mj2_read_ftyp(movie))
    return 1;

  fsresult = fseek(file,cio_tell(),SEEK_SET);
  if( fsresult ) {
    fprintf(stderr, "End of file reached while trying to read data after FTYP box\n" );
    return 1;
  }

  foffset = cio_tell();
  
  box.type = 0;
  
  fread(src,30,1,file);
  cio_init(src, 30);
  mj2_read_boxhdr(&box);
  
  while(box.type != MJ2_MOOV) {
    
    switch(box.type)
    {
    case MJ2_MDAT:
      fsresult = fseek(file,foffset+box.length,SEEK_SET);
      if( fsresult ) {
	fprintf(stderr, "End of file reached while trying to read MDAT box\n" );
	return 1;
      }
      foffset += box.length;
      break;
      
    case MJ2_MOOF:
      fsresult = fseek(file,foffset+box.length,SEEK_SET);
      if( fsresult ) {
	fprintf(stderr, "End of file reached while trying to read MOOF box\n" );
	return 1;
      }
      foffset += box.length;
      break;      
    case MJ2_FREE:
      fsresult = fseek(file,foffset+box.length,SEEK_SET);
      if( fsresult ) {
	fprintf(stderr, "End of file reached while trying to read FREE box\n" );
	return 1;
      }
      foffset += box.length;
      break;      
    case MJ2_SKIP:
      fsresult = fseek(file,foffset+box.length,SEEK_SET);
      if( fsresult ) {
	fprintf(stderr, "End of file reached while trying to read SKIP box\n" );
	return 1;
      }
      foffset += box.length;
      break;      
    default:
      fprintf(stderr, "Unknown box in MJ2 stream\n");
      fsresult = fseek(file,foffset+box.length,SEEK_SET);
      if( fsresult ) {
	fprintf(stderr, "End of file reached while trying to read end of unknown box\n"); 
	return 1;
      }      
      foffset += box.length;
      break;
    }
    fsresult = fread(src,8,1,file);
    if (fsresult != 1) {
      fprintf(stderr, "MOOV box not found in file\n"); 
      return 1;
    }
    cio_init(src, 8);
    mj2_read_boxhdr(&box);
  }

  fseek(file,foffset,SEEK_SET);
  src = realloc(src,box.length);
  fsresult = fread(src,box.length,1,file);
  if (fsresult != 1) {
    fprintf(stderr, "End of file reached while trying to read MOOV box\n"); 
    return 1;
  }

  cio_init(src, box.length);
  
  if (mj2_read_moov(movie, &img))
    return 1;
  
  free(src);
  return 0;
}
