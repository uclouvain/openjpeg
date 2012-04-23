/*
* Copyright (c) 2003-2004, François-Olivier Devaux
* Copyright (c) 2002-2004,  Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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

#include "opj_config.h"
#include "openjpeg.h"
#include "j2k_lib.h"
#include "j2k.h"
#include "jp2.h"
#include "mj2.h"
#include "mj2_convert.h"

#ifdef HAVE_LIBLCMS2
#include <lcms2.h>
#endif
#ifdef HAVE_LIBLCMS1
#include <lcms.h>
#endif
#include "color.h"
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
	mj2_dparameters_t mj2_parameters;			/* decompression parameters */
	opj_dinfo_t* dinfo; 
	opj_event_mgr_t event_mgr;		/* event manager */	
	opj_cio_t *cio = NULL;
  unsigned int tnum, snum, failed;
  opj_mj2_t *movie;
  mj2_tk_t *track;
  mj2_sample_t *sample;
  unsigned char* frame_codestream;
  FILE *infile, *outfile;
  opj_image_t *img = NULL;
	unsigned int max_codstrm_size = 0;
	double total_time = 0;
	unsigned int numframes = 0;
			
  if (argc != 3) {
    printf("\nUsage: %s inputfile.mj2 outputfile.yuv\n\n",argv[0]); 
    return 1;
  }
  
  infile = fopen(argv[1], "rb");
  
  if (!infile) {
    fprintf(stderr, "failed to open %s for reading\n", argv[1]);
    return 1;
  }
   {
	unsigned char buf[28];
	size_t n;

	memset(buf, 0, 28);
	n = fread(buf, 1, 24, infile);

	if(memcmp(buf, JP2_RFC3745_MAGIC, 12) == 0
	&& memcmp(buf+20, "\x6d\x6a\x70\x32", 4) == 0)
  {
	rewind(infile);
  }
	else
  {
	fclose(infile);
	fprintf(stderr,"%s:%d: %s\n\tThis file is not an MJ2 file."
	"Quitting\n",__FILE__,__LINE__,argv[0]);
	return 1;
  }
   }
/* Checking output file: */
  outfile = fopen(argv[2], "w");

  if (!outfile) {
    fprintf(stderr, "failed to open %s for writing\n", argv[2]);
    fclose(infile);
	return 1;
  }
  fclose(outfile); remove(argv[2]);

	frame_codestream = NULL;
	failed = 1;
	/*
	configure the event callbacks (not required)
	setting of each callback is optionnal
	*/
	memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
	event_mgr.error_handler = error_callback;
	event_mgr.warning_handler = warning_callback;
	event_mgr.info_handler = NULL;
	
	/* get a MJ2 decompressor handle */
	dinfo = mj2_create_decompress();
	if(dinfo == NULL) goto fin;

	movie = (opj_mj2_t*)dinfo->mj2_handle;
	
	/* catch events using our callbacks and give a local context */
	opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);		

	memset(&mj2_parameters, 0, sizeof(mj2_dparameters_t));
	/* set J2K decoding parameters to default values */
	opj_set_default_decoder_parameters(&mj2_parameters.j2k_parameters);
	
	/* setup the decoder decoding parameters using user parameters */
	mj2_setup_decoder(movie, &mj2_parameters);

/* Create the movie structure: */			
  if (mj2_read_struct(infile, movie))
    goto fin;
	
/* Decode first video track */
	for (tnum=0; tnum < (unsigned int)(movie->num_htk + movie->num_stk + movie->num_vtk); tnum++) {
		if (movie->tk[tnum].track_type == 0) 
			break;
	}
	
	if (movie->tk[tnum].track_type != 0) {
		printf("Error. Movie does not contain any video track\n");
		goto fin;
	}
	
  track = &movie->tk[tnum];

	if(track->jp2_struct.enumcs != ENUMCS_SYCC)
   {
	fprintf(stderr,"%s:%d: %s\n"
	 "\tERROR: this MJ2 file does not contain YUV frames.\n"
	 "\tPlease try extract_j2k_from_mj2 for this file.\n",
	 __FILE__,__LINE__,argv[0]);
	goto fin;
   }
/* Output info on first video track: */
  fprintf(stdout,"The first video track contains %d frames.\nWidth: %d, Height: %d \n\n",
    track->num_samples, track->w, track->h);
	
	max_codstrm_size = track->sample[0].sample_size-8;
	frame_codestream = (unsigned char*) 
	 malloc(max_codstrm_size * sizeof(unsigned char)); 
	if(frame_codestream == NULL) goto fin;

	numframes = track->num_samples;
	
  for (snum=0; snum < numframes; snum++)
  {
	double init_time = opj_clock();
	double elapsed_time;

    sample = &track->sample[snum];
		if (sample->sample_size-8 > max_codstrm_size) {
			max_codstrm_size =  sample->sample_size-8;
			if ((frame_codestream = (unsigned char*)
				realloc(frame_codestream, max_codstrm_size)) == NULL) {
				printf("Error reallocation memory\n");
				goto fin;
			}; 		
		}
    fseek(infile,sample->offset+8,SEEK_SET);
/* Assuming that jp and ftyp markers size do: */
    fread(frame_codestream, sample->sample_size-8, 1, infile);
		
		/* open a byte stream */
	cio = opj_cio_open((opj_common_ptr)dinfo, frame_codestream, sample->sample_size-8);

	if(cio == NULL) goto fin;

	img = opj_decode(dinfo, cio);

	if(img == NULL) goto fin;

/* Convert frame to YUV: */
	if (!imagetoyuv(img, argv[2]))
				goto fin;

	opj_cio_close(cio);	

	opj_image_destroy(img);

	elapsed_time = opj_clock()-init_time;
	fprintf(stderr, "Frame number %d/%d decoded in %.2f mseconds\n", 
	 snum + 1, numframes, elapsed_time*1000);
	total_time += elapsed_time;

   }/* for (snum */

	fprintf(stdout, "%d frame(s) correctly decompressed\n", snum);
	fprintf(stdout,"Total decoding time: %.2f seconds (%.1f fps)\n", 
	 total_time, (float)numframes/total_time);
		
	failed = 0;

fin:
	fclose(infile);	

	if(frame_codestream) free(frame_codestream);	

	/* free remaining structures */
	if(dinfo) 
   {
	mj2_destroy_decompress(movie);
	free(dinfo);
   }
	
  return failed;
}/* main() */
