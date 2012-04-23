/*
 * Copyright (c) 2002-2007, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2007, Professor Benoit Macq
 * Copyright (c) 2003-2007, Francois-Olivier Devaux 
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

#include "openjpeg.h"
#include "cio.h"
#include "j2k.h"
#include "jp2.h"
#include "mj2.h"

/* -------------------------------------------------------------------------- */

/**
sample error callback expecting a FILE* client object
*/
void error_callback(const char *msg, void *client_data) {
	FILE *stream = (FILE*)client_data;
	fprintf(stream, "[ERROR] %s", msg);
}
/**
sample warning callback expecting a FILE* client object
*/
void warning_callback(const char *msg, void *client_data) {
	FILE *stream = (FILE*)client_data;
	fprintf(stream, "[WARNING] %s", msg);
}
/**
sample debug callback expecting a FILE* client object
*/
void info_callback(const char *msg, void *client_data) {
	FILE *stream = (FILE*)client_data;
	fprintf(stream, "[INFO] %s", msg);
}

/* -------------------------------------------------------------------------- */
#define JP2_RFC3745_MAGIC "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"

int main(int argc, char *argv[]) {
	opj_dinfo_t* dinfo; 
	opj_event_mgr_t event_mgr;		/* event manager */
  int tnum, failed;
  unsigned int snum;
  opj_mj2_t *movie;
  mj2_tk_t *track;
  mj2_sample_t *sample;
  unsigned char* frame_codestream;
  FILE *mj2file, *outfile;
  char *outfilename;
	mj2_dparameters_t parameters;

  if (argc != 3) {
    printf("\nUsage: %s mj2filename output_location\n",argv[0]); 
    printf("Example: %s foreman.mj2 output/foreman\n\n",argv[0]);
    return 1;
  }
  
  mj2file = fopen(argv[1], "rb");
  
  if (!mj2file) {
    fprintf(stderr, "failed to open %s for reading\n", argv[1]);
    return 1;
  }
   {
    unsigned char buf[28];

    memset(buf, 0, 28);
    fread(buf, 1, 24, mj2file);

    if(memcmp(buf, JP2_RFC3745_MAGIC, 12) == 0
    && memcmp(buf+20, "\x6d\x6a\x70\x32", 4) == 0)
  {
    rewind(mj2file);
  }
    else
  {
    fclose(mj2file);
    fprintf(stderr,"%s:%d: %s\n\tThis file is not an MJ2 file."
    "Quitting\n",__FILE__,__LINE__, argv[0]);
    return 1;
  }
   }
	/*
	configure the event callbacks (not required)
	setting of each callback is optionnal
	*/
	memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
	event_mgr.error_handler = error_callback;
	event_mgr.warning_handler = warning_callback;
	event_mgr.info_handler = info_callback;

	failed = 1;
	outfilename = (char*)malloc(strlen(argv[2]) + 32);
	if(outfilename == NULL) goto fin;

	/* get a MJ2 decompressor handle */
	dinfo = mj2_create_decompress();

	if(dinfo == NULL) goto fin;

	/* catch events using our callbacks and give a local context */
	opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);		

	/* setup the decoder decoding parameters using user parameters */
	memset(&parameters, 0, sizeof(mj2_dparameters_t));
	movie = (opj_mj2_t*) dinfo->mj2_handle;
	mj2_setup_decoder(movie, &parameters);

/* Create the movie structure: */
  if (mj2_read_struct(mj2file, movie))
    goto fin;

/* Decode first video track : */
  tnum = 0;
  while (movie->tk[tnum].track_type != 0)
    tnum ++;

  track = &movie->tk[tnum];

	if(track->jp2_struct.enumcs != ENUMCS_SRGB
	&& track->jp2_struct.enumcs != ENUMCS_GRAY)
   {
	fprintf(stderr,"%s:%d: %s\n"
	 "\tERROR: this MJ2 file does not contain J2K frames.\n"
	 "\tPlease try mj2_to_frames for this file.\n",
	 __FILE__,__LINE__, argv[0]);
	goto fin;
   }

  fprintf(stdout,"Extracting %d frames from file...\n",track->num_samples);

  for (snum=0; snum < track->num_samples; snum++)
  {
    sample = &track->sample[snum];
    frame_codestream = (unsigned char*) 
	 malloc (sample->sample_size-8); /* Skipping JP2C marker */
    fseek(mj2file,sample->offset+8,SEEK_SET);
/* Assuming that jp and ftyp markers size do: */
    fread(frame_codestream,sample->sample_size-8,1, mj2file);  

    sprintf(outfilename,"%s_%05d.j2k",argv[2],snum);
    outfile = fopen(outfilename, "wb");
    if (!outfile) {
      fprintf(stderr, "failed to open %s for writing\n",outfilename);
      goto fin;
    }
    fwrite(frame_codestream,sample->sample_size-8,1,outfile);
    fclose(outfile);
    free(frame_codestream);
  }
  fprintf(stdout, "%d frames correctly extracted\n", snum);
  failed = 0;

fin:
  fclose(mj2file);

  free(outfilename);

	/* free remaining structures */
	if(dinfo) 
   {
	mj2_destroy_decompress(movie);
	free(dinfo);
   }
  
  return failed;
}/* main() */
