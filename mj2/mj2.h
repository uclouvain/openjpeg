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

#include <j2k.h>
#include <jp2.h>

#ifndef __MJ2_H
#define __MJ2_H

typedef struct {		/* Time To Sample       */
  int sample_count;
  int sample_delta;
} mj2_tts_t;

typedef struct {		/* Chunk       */
  int num_samples;
  int sample_descr_idx;
  int offset;
} mj2_chunk_t;

typedef struct {		/* Sample to chunk */
  int first_chunk;
  int samples_per_chunk;
  int sample_descr_idx;
} mj2_sampletochunk_t;

typedef struct {		/* Sample       */
  unsigned int sample_size;
  unsigned int offset;
  unsigned int sample_delta;
} mj2_sample_t;

typedef struct {		/* URL       */
  int location[4];
} mj2_url_t;

typedef struct {		/* URN       */
  int name[2];
  int location[4];
} mj2_urn_t;

typedef struct {		/* Video Track Parameters    */
  int track_ID;
  int track_type;
  unsigned int creation_time;
  unsigned int modification_time;
  int duration;
  int timescale;
  int layer;
  int volume;
  int language;
  int balance;
  int maxPDUsize;
  int avgPDUsize;
  int maxbitrate;
  int avgbitrate;
  int slidingavgbitrate;
  int graphicsmode;
  int opcolor[3];
  int num_url;
  mj2_url_t *url;
  int num_urn;
  mj2_urn_t *urn;
  int Dim[2];
  int w;
  int h;
  int visual_w;
  int visual_h;
  int CbCr_subsampling_dx;
  int CbCr_subsampling_dy;
 // int subsampling_dx;
 // int subsampling_dy;
  int sample_rate;
  int sample_description;
  int horizresolution;
  int vertresolution;
  int compressorname[8];
  int depth;
  unsigned char fieldcount;
  unsigned char fieldorder;
  unsigned char or_fieldcount;
  unsigned char or_fieldorder;
  int num_br;
  unsigned int *br;
  unsigned char num_jp2x;
  unsigned char *jp2xdata;
  unsigned char hsub;
  unsigned char vsub;
  unsigned char hoff;
  unsigned char voff;
  int trans_matrix[9];
  unsigned int num_samples;	/* Number of samples */
  int transorm;
  int handler_type;
  int name_size;
  unsigned char same_sample_size;
  int num_tts;
  mj2_tts_t *tts;		/* Time to sample    */
  unsigned int num_chunks;
  mj2_chunk_t *chunk;
  int num_samplestochunk;
  mj2_sampletochunk_t *sampletochunk;
  char *name;
  jp2_struct_t jp2_struct;
  mj2_sample_t *sample;		/* Sample parameters */
} mj2_tk_t;			/* Track Parameters  */

typedef struct {		/* Movie */
  unsigned int brand;
  unsigned int minversion;
  int num_cl;
  unsigned int *cl;
  unsigned int creation_time;
  unsigned int modification_time;
  int timescale;
  unsigned int duration;
  int rate;
  int num_vtk;
  int num_stk;
  int num_htk;
  int volume;
  int trans_matrix[9];
  int next_tk_id;
  mj2_tk_t *tk;			/* Track Parameters  */
} mj2_movie_t;

typedef struct {
  int length;
  int type;
  int init_pos;
} mj2_box_t;

void mj2_write_jp();

void mj2_write_ftyp(mj2_movie_t * movie);

/*
 * Use this function to initialize a standard movie with standard values
 * It has one sample per chunk
 */
int mj2_init_stdmovie(mj2_movie_t * movie);


/* int mj2_encode(mj2_movie_t * movie, j2k_cp_t * cp, char *index); 
 *
 * Encode a MJ2 movie from a yuv file
 * movie: an existing mj2_movie structure (to create a standard one, use mj2_init_stdmovie 
 * cp: coding parameters of j2k images
 * index: index file name
 */
int mj2_encode(mj2_movie_t * movie, j2k_cp_t * cp, char *index);


/* int mj2_decode(unsigned char *src, int len, mj2_movie_t * movie,
 *	       j2k_cp_t * cp, char *outfile); 
 *
 * Decode a MJ2 movie to a yuv file
 * src: pointer to memory where frames will be stored
 * movie: a mj2_movie structure 
 * cp: coding parameters of j2k images
 * outfile: yuv file name
 */
int mj2_decode(unsigned char *src, int len, mj2_movie_t * movie,
	       j2k_cp_t * cp, char *outfile);


/*
 * Free memory used to encode and decode mj2 files
 * 
 */
void mj2_memory_free(mj2_movie_t * movie);

int mj2_read_struct(FILE *file, mj2_movie_t * movie);

void mj2_write_moov(mj2_movie_t * movie);

#endif
