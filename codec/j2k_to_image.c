/*
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2005, Francois Devaux and Antonin Descampe
 * Copyright (c) 2005, Hervé Drolon, FreeImage Team
 * Copyright (c) 2002-2005, Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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
#include <string.h>
#include <stdlib.h>

#include "openjpeg.h"
#include "compat/getopt.h"
#include "convert.h"

#ifndef WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

/* ----------------------------------------------------------------------- */

#define J2K_CFMT 0
#define JP2_CFMT 1
#define JPT_CFMT 2
#define MJ2_CFMT 3
#define PXM_DFMT 0
#define PGX_DFMT 1
#define BMP_DFMT 2
#define YUV_DFMT 3

/* ----------------------------------------------------------------------- */

void decode_help_display() {
	fprintf(stdout,"HELP\n----\n\n");
	fprintf(stdout,"- the -h option displays this help information on screen\n\n");

	fprintf(stdout,"List of parameters for the JPEG 2000 encoder:\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"  -i <compressed file>\n");
	fprintf(stdout,"    REQUIRED\n");
	fprintf(stdout,"    Currently accepts J2K-files, JP2-files and JPT-files. The file type\n");
	fprintf(stdout,"    is identified based on its suffix.\n");
	fprintf(stdout,"  -o <decompressed file>\n");
	fprintf(stdout,"    REQUIRED\n");
	fprintf(stdout,"    Currently accepts PGM-files, PPM-files, PNM-files, PGX-files and\n");
	fprintf(stdout,"    BMP-files. Binary data is written to the file (not ascii). If a PGX\n");
	fprintf(stdout,"    filename is given, there will be as many output files as there are\n");
	fprintf(stdout,"    components: an indice starting from 0 will then be appended to the\n");
	fprintf(stdout,"    output filename, just before the \"pgx\" extension. If a PGM filename\n");
	fprintf(stdout,"    is given and there are more than one component, only the first component\n");
	fprintf(stdout,"    will be written to the file.\n");
	fprintf(stdout,"  -r <reduce factor>\n");
	fprintf(stdout,"    Set the number of highest resolution levels to be discarded. The\n");
	fprintf(stdout,"    image resolution is effectively divided by 2 to the power of the\n");
	fprintf(stdout,"    number of discarded levels. The reduce factor is limited by the\n");
	fprintf(stdout,"    smallest total number of decomposition levels among tiles.\n");
	fprintf(stdout,"  -l <number of quality layers to decode>\n");
	fprintf(stdout,"    Set the maximum number of quality layers to decode. If there are\n");
	fprintf(stdout,"    less quality layers than the specified number, all the quality layers\n");
	fprintf(stdout,"    are decoded.\n");
	fprintf(stdout,"\n");
}

/* -------------------------------------------------------------------------- */

int get_file_format(char *filename) {
	unsigned int i;
	static const char *extension[] = {"pgx", "pnm", "pgm", "ppm", "bmp", "j2k", "jp2", "jpt" };
	static const int format[] = { PGX_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, BMP_DFMT, J2K_CFMT, JP2_CFMT, JPT_CFMT };
	char * ext = strrchr(filename, '.') + 1;
	if(ext) {
		for(i = 0; i < sizeof(format)/sizeof(*format); i++) {
			if(strnicmp(ext, extension[i], 3) == 0) {
				return format[i];
			}
		}
	}

	return -1;
}

/* -------------------------------------------------------------------------- */

int parse_cmdline_decoder(int argc, char **argv, opj_dparameters_t *parameters) {
	/* parse the command line */

	while (1) {
		int c = getopt(argc, argv, "i:o:r:q:f:t:n:c:b:x:p:s:d:hP:S:E:M:R:T:C:I");
		if (c == -1)
			break;
		switch (c) {
			case 'i':			/* input file */
			{
				char *infile = optarg;
				parameters->decod_format = get_file_format(infile);
				switch(parameters->decod_format) {
					case J2K_CFMT:
					case JP2_CFMT:
					case JPT_CFMT:
						break;
					default:
						fprintf(stderr, 
							"!! Unrecognized format for infile : %s [accept only *.j2k, *.jp2, *.jpc or *.jpt] !!\n\n", 
							infile);
						return 1;
						break;
				}
				strncpy(parameters->infile, infile, MAX_PATH);
			}
			break;
				
				/* ----------------------------------------------------- */

			case 'o':			/* output file */
			{
				char *outfile = optarg;
				parameters->cod_format = get_file_format(outfile);
				switch(parameters->cod_format) {
					case PGX_DFMT:
					case PXM_DFMT:
					case BMP_DFMT:
						break;
					default:
						fprintf(stderr, "Unknown output format image %s [only *.pnm, *.pgm, *.ppm, *.pgx or *.bmp]!! \n", outfile);
						return 1;
						break;
				}
				strncpy(parameters->outfile, outfile, MAX_PATH);
			}
			break;
			
				/* ----------------------------------------------------- */
			
    
			case 'r':		/* reduce option */
			{
				sscanf(optarg, "%d", &parameters->cp_reduce);
			}
			break;
			
				/* ----------------------------------------------------- */
      

			case 'l':		/* layering option */
			{
				sscanf(optarg, "%d", &parameters->cp_layer);
			}
			break;
			
				/* ----------------------------------------------------- */
			
			case 'h': 			/* display an help description */
			{
				decode_help_display();
				return 1;
			}
			break;
            
				/* ----------------------------------------------------- */
			
			default:
				fprintf(stderr,"WARNING -> this option is not valid \"-%c %s\"\n",c, optarg);
				break;
		}
	}

	/* check for possible errors */

	if((parameters->infile[0] == 0) || (parameters->outfile[0] == 0)) {
		fprintf(stderr,"ERROR -> At least one required argument is missing\nCheck j2k_to_image -h for usage information\n");
		return 1;
	}

	return 0;
}

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
sample debug callback expecting no client object
*/
void info_callback(const char *msg, void *client_data) {
	(void)client_data;
	fprintf(stdout, "[INFO] %s", msg);
}

/* -------------------------------------------------------------------------- */

int main(int argc, char **argv) {
	opj_dparameters_t parameters;	/* decompression parameters */
	opj_event_mgr_t event_mgr;		/* event manager */
	opj_image_t *image = NULL;
	FILE *fsrc = NULL;
	unsigned char *src = NULL; 
	int file_length;

	opj_dinfo_t* dinfo = NULL;	/* handle to a decompressor */
	opj_cio_t *cio = NULL;

	/* configure the event callbacks (not required) */
	memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
	event_mgr.error_handler = error_callback;
	event_mgr.warning_handler = warning_callback;
	event_mgr.info_handler = info_callback;

	/* set decoding parameters to default values */
	opj_set_default_decoder_parameters(&parameters);

	/* parse input and get user decoding parameters */
	if(parse_cmdline_decoder(argc, argv, &parameters) == 1) {
		return 0;
	}
	
	/* read the input file and put it in memory */
	/* ---------------------------------------- */
	fsrc = fopen(parameters.infile, "rb");
	if (!fsrc) {
		fprintf(stderr, "ERROR -> failed to open %s for reading\n", parameters.infile);
		return 1;
	}  
	fseek(fsrc, 0, SEEK_END);
	file_length = ftell(fsrc);
	fseek(fsrc, 0, SEEK_SET);
	src = (unsigned char *) malloc(file_length);
	fread(src, 1, file_length, fsrc);
	fclose(fsrc);
	
	/* decode the code-stream */
	/* ---------------------- */

    switch(parameters.decod_format) {
		case J2K_CFMT:
		{
			/* JPEG-2000 codestream */

			/* get a decoder handle */
			dinfo = opj_create_decompress(CODEC_J2K);
			
			/* catch events using our callbacks and give a local context */
			opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);			

			/* setup the decoder decoding parameters using user parameters */
			opj_setup_decoder(dinfo, &parameters);

			/* open a byte stream */
			cio = opj_cio_open((opj_common_ptr)dinfo, src, file_length);

			/* decode the stream and fill the image structure */
			image = opj_decode(dinfo, cio);
			if(!image) {
				fprintf(stderr, "ERROR -> j2k_to_image: failed to decode image!\n");
				opj_destroy_decompress(dinfo);
				opj_cio_close(cio);
				return 1;
			}
			
			/* close the byte stream */
			opj_cio_close(cio);
		}
		break;

		case JP2_CFMT:
		{
			/* JPEG 2000 compressed image data */

			/* get a decoder handle */
			dinfo = opj_create_decompress(CODEC_JP2);
			
			/* catch events using our callbacks and give a local context */
			opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);			

			/* setup the decoder decoding parameters using the current image and using user parameters */
			opj_setup_decoder(dinfo, &parameters);

			/* open a byte stream */
			cio = opj_cio_open((opj_common_ptr)dinfo, src, file_length);

			/* decode the stream and fill the image structure */
			image = opj_decode(dinfo, cio);
			if(!image) {
				fprintf(stderr, "ERROR -> j2k_to_image: failed to decode image!\n");
				opj_destroy_decompress(dinfo);
				opj_cio_close(cio);
				return 1;
			}

			/* close the byte stream */
			opj_cio_close(cio);

		}
		break;

		case JPT_CFMT:
		{
			/* JPEG 2000, JPIP */

			/* get a decoder handle */
			dinfo = opj_create_decompress(CODEC_JPT);
			
			/* catch events using our callbacks and give a local context */
			opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);			

			/* setup the decoder decoding parameters using user parameters */
			opj_setup_decoder(dinfo, &parameters);

			/* open a byte stream */
			cio = opj_cio_open((opj_common_ptr)dinfo, src, file_length);

			/* decode the stream and fill the image structure */
			image = opj_decode(dinfo, cio);
			if(!image) {
				fprintf(stderr, "ERROR -> j2k_to_image: failed to decode image!\n");				
				opj_destroy_decompress(dinfo);
				opj_cio_close(cio);
				return 1;
			}	

			/* close the byte stream */
			opj_cio_close(cio);
		}
		break;

		default:
			fprintf(stderr, "ERROR -> j2k_to_image : Unknown input image format\n");
			return 1;
			break;
	}
  
	/* free the memory containing the code-stream */
	free(src);
	src = NULL;

	/* create output image */
	/* ------------------- */

	switch (parameters.cod_format) {
		case PXM_DFMT:			/* PNM PGM PPM */
			imagetopnm(image, parameters.outfile);
			break;
            
		case PGX_DFMT:			/* PGX */
			imagetopgx(image, parameters.outfile);
			break;
		
		case BMP_DFMT:			/* BMP */
			imagetobmp(image, parameters.outfile);
			break;
	}

	/* free remaining structures */
	if(dinfo) {
		opj_destroy_decompress(dinfo);
	}

	/* free image data structure */
	opj_image_destroy(image);
   
	return 0;
}

