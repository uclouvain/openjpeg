/*
 * Copyright (c) 2010, Mathieu Malaterre, GDCM
 * Copyright (c) 2011, Mickael Savinaud, Communications & Systemes <mickael.savinaud@c-s.fr>
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
#include "opj_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#include "windirent.h"
#else
#include <dirent.h>
#endif /* _WIN32 */

#ifdef _WIN32
#include <windows.h>
#else
#include <strings.h>
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#endif /* _WIN32 */

#include "openjpeg.h"
#include "opj_getopt.h"
#include "convert.h"
#include "index.h"

#include "format_defs.h"

typedef struct dircnt{
	/** Buffer for holding images read from Directory*/
	char *filename_buf;
	/** Pointer to the buffer*/
	char **filename;
}dircnt_t;


typedef struct img_folder{
	/** The directory path of the folder containing input images*/
	char *imgdirpath;
	/** Output format*/
	const char *out_format;
	/** Enable option*/
	char set_imgdir;
	/** Enable Cod Format for output*/
	char set_out_format;

}img_fol_t;

/* -------------------------------------------------------------------------- */
/* Declarations                                                               */
int get_num_images(char *imgdirpath);
int load_images(dircnt_t *dirptr, char *imgdirpath);
int get_file_format(char *filename);
char get_next_file(int imageno,dircnt_t *dirptr,img_fol_t *img_fol, opj_dparameters_t *parameters);

int parse_cmdline_decoder(int argc, char **argv, opj_dparameters_t *parameters,img_fol_t *img_fol);
int parse_DA_values( char* inArg, unsigned int *DA_x0, unsigned int *DA_y0, unsigned int *DA_x1, unsigned int *DA_y1);

/* -------------------------------------------------------------------------- */
void decode_help_display(void) {
	fprintf(stdout,"HELP for j2k_dump\n----\n\n");
	fprintf(stdout,"- the -h option displays this help information on screen\n\n");

/* UniPG>> */
	fprintf(stdout,"List of parameters for the JPEG 2000 "
#ifdef USE_JPWL
		"+ JPWL "
#endif /* USE_JPWL */
		"decoder:\n");
/* <<UniPG */
	fprintf(stdout,"\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"  -ImgDir \n");
	fprintf(stdout,"	Image file Directory path \n");
	fprintf(stdout,"  -i <compressed file>\n");
	fprintf(stdout,"    REQUIRED only if an Input image directory not specified\n");
	fprintf(stdout,"    Currently accepts J2K-files, JP2-files and JPT-files. The file type\n");
	fprintf(stdout,"    is identified based on its suffix.\n");
	fprintf(stdout,"  -o <output file>\n");
	fprintf(stdout,"    OPTIONAL\n");
	fprintf(stdout,"    Output file where file info will be dump.\n");
	fprintf(stdout,"    By default it will be in the stdout.\n");
	fprintf(stdout,"  -d <x0,x1,y0,y1>\n"); /* FIXME WIP_MSD */
	fprintf(stdout,"    OPTIONAL\n");
	fprintf(stdout,"    Decoding area\n");
	fprintf(stdout,"    By default all tiles header are read.\n");
	fprintf(stdout,"  -v "); /* FIXME WIP_MSD */
	fprintf(stdout,"    OPTIONAL\n");
	fprintf(stdout,"    Activate or not the verbose mode (display info and warning message)\n");
	fprintf(stdout,"    By default verbose mode is off.\n");
	fprintf(stdout,"\n");
}

/* -------------------------------------------------------------------------- */
int get_num_images(char *imgdirpath){
	DIR *dir;
	struct dirent* content;	
	int num_images = 0;

	/*Reading the input images from given input directory*/

	dir= opendir(imgdirpath);
	if(!dir){
		fprintf(stderr,"Could not open Folder %s\n",imgdirpath);
		return 0;
	}
	
	while((content=readdir(dir))!=NULL){
		if(strcmp(".",content->d_name)==0 || strcmp("..",content->d_name)==0 )
			continue;
		num_images++;
	}
	return num_images;
}

/* -------------------------------------------------------------------------- */
int load_images(dircnt_t *dirptr, char *imgdirpath){
	DIR *dir;
	struct dirent* content;	
	int i = 0;

	/*Reading the input images from given input directory*/

	dir= opendir(imgdirpath);
	if(!dir){
		fprintf(stderr,"Could not open Folder %s\n",imgdirpath);
		return 1;
	}else	{
		fprintf(stderr,"Folder opened successfully\n");
	}
	
	while((content=readdir(dir))!=NULL){
		if(strcmp(".",content->d_name)==0 || strcmp("..",content->d_name)==0 )
			continue;

		strcpy(dirptr->filename[i],content->d_name);
		i++;
	}
	return 0;	
}

/* -------------------------------------------------------------------------- */
int get_file_format(char *filename) {
	unsigned int i;
	static const char *extension[] = {"pgx", "pnm", "pgm", "ppm", "bmp","tif", "raw", "tga", "png", "j2k", "jp2", "jpt", "j2c", "jpc"  };
	static const int format[] = { PGX_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, BMP_DFMT, TIF_DFMT, RAW_DFMT, TGA_DFMT, PNG_DFMT, J2K_CFMT, JP2_CFMT, JPT_CFMT, J2K_CFMT, J2K_CFMT };
	char * ext = strrchr(filename, '.');
	if (ext == NULL)
		return -1;
	ext++;
	if(ext) {
		for(i = 0; i < sizeof(format)/sizeof(*format); i++) {
			if(_strnicmp(ext, extension[i], 3) == 0) {
				return format[i];
			}
		}
	}

	return -1;
}

/* -------------------------------------------------------------------------- */
char get_next_file(int imageno,dircnt_t *dirptr,img_fol_t *img_fol, opj_dparameters_t *parameters){
	char image_filename[OPJ_PATH_LEN], infilename[OPJ_PATH_LEN],outfilename[OPJ_PATH_LEN],temp_ofname[OPJ_PATH_LEN];
	char *temp_p, temp1[OPJ_PATH_LEN]="";

	strcpy(image_filename,dirptr->filename[imageno]);
	fprintf(stderr,"File Number %d \"%s\"\n",imageno,image_filename);
	parameters->decod_format = get_file_format(image_filename);
	if (parameters->decod_format == -1)
		return 1;
	sprintf(infilename,"%s/%s",img_fol->imgdirpath,image_filename);
	strncpy(parameters->infile, infilename, sizeof(infilename));

	/* Set output file */
	strcpy(temp_ofname,strtok(image_filename,"."));
	while((temp_p = strtok(NULL,".")) != NULL){
		strcat(temp_ofname,temp1);
		sprintf(temp1,".%s",temp_p);
	}
	if(img_fol->set_out_format==1){
		sprintf(outfilename,"%s/%s.%s",img_fol->imgdirpath,temp_ofname,img_fol->out_format);
		strncpy(parameters->outfile, outfilename, sizeof(outfilename));
	}
	return 0;
}

/* -------------------------------------------------------------------------- */
/**
 * Parse the command line
 */
/* -------------------------------------------------------------------------- */
int parse_cmdline_decoder(int argc, char **argv, opj_dparameters_t *parameters,img_fol_t *img_fol) {
	int totlen, c;
	opj_option_t long_option[]={
		{"ImgDir",REQ_ARG, NULL ,'y'},
	};
	const char optlist[] = "i:o:d:hv";

	totlen=sizeof(long_option);
	img_fol->set_out_format = 0;
	do {
		c = opj_getopt_long(argc, argv,optlist,long_option,totlen);
		if (c == -1)
			break;
		switch (c) {
			case 'i':			/* input file */
			{
				char *infile = opj_optarg;
				parameters->decod_format = get_file_format(infile);
				switch(parameters->decod_format) {
					case J2K_CFMT:
						break;
					case JP2_CFMT:
						break;
					case JPT_CFMT:
						break;
					default:
						fprintf(stderr, 
							"!! Unrecognized format for infile : %s [accept only *.j2k, *.jp2, *.jpc or *.jpt] !!\n\n", 
							infile);
						return 1;
				}
				strncpy(parameters->infile, infile, sizeof(parameters->infile)-1);
			}
			break;

				/* ------------------------------------------------------ */

			case 'o':     /* output file */
			{
			  char *outfile = opj_optarg;
			  strncpy(parameters->outfile, outfile, sizeof(parameters->outfile)-1);
			}
			break;
				
				/* ----------------------------------------------------- */

			case 'h': 			/* display an help description */
				decode_help_display();
				return 1;				

				/* ------------------------------------------------------ */

			case 'y':			/* Image Directory path */
			{
				img_fol->imgdirpath = (char*)malloc(strlen(opj_optarg) + 1);
				strcpy(img_fol->imgdirpath,opj_optarg);
				img_fol->set_imgdir=1;
			}
			break;

				/* ----------------------------------------------------- */

			case 'd':     		/* Input decode ROI */
			{
				int size_optarg = (int)strlen(opj_optarg) + 1;
				char *ROI_values = (char*) malloc(size_optarg);
				ROI_values[0] = '\0';
				strncpy(ROI_values, opj_optarg, strlen(opj_optarg));
				ROI_values[strlen(opj_optarg)] = '\0';
				/*printf("ROI_values = %s [%d / %d]\n", ROI_values, strlen(ROI_values), size_optarg ); */
				parse_DA_values( ROI_values, &parameters->DA_x0, &parameters->DA_y0, &parameters->DA_x1, &parameters->DA_y1);
			}
			break;
			/* ----------------------------------------------------- */

			case 'v':     		/* Verbose mode */
			{
				parameters->m_verbose = 1;
			}
			break;
			
				/* ----------------------------------------------------- */
			default:
				fprintf(stderr,"WARNING -> this option is not valid \"-%c %s\"\n",c, opj_optarg);
				break;
		}
	}while(c != -1);

	/* check for possible errors */
	if(img_fol->set_imgdir==1){
		if(!(parameters->infile[0]==0)){
			fprintf(stderr, "Error: options -ImgDir and -i cannot be used together !!\n");
			return 1;
		}
		if(img_fol->set_out_format == 0){
			fprintf(stderr, "Error: When -ImgDir is used, -OutFor <FORMAT> must be used !!\n");
			fprintf(stderr, "Only one format allowed! Valid format PGM, PPM, PNM, PGX, BMP, TIF, RAW and TGA!!\n");
			return 1;
		}
		if(!((parameters->outfile[0] == 0))){
			fprintf(stderr, "Error: options -ImgDir and -o cannot be used together !!\n");
			return 1;
		}
	}else{
		if((parameters->infile[0] == 0) ) {
			fprintf(stderr, "Example: %s -i image.j2k\n",argv[0]);
			fprintf(stderr, "    Try: %s -h\n",argv[0]);
			return 1;
		}
	}

	return 0;
}

/* -------------------------------------------------------------------------- */
/**
 * Parse decoding area input values
 * separator = ","
 */
/* -------------------------------------------------------------------------- */
int parse_DA_values( char* inArg, unsigned int *DA_x0, unsigned int *DA_y0, unsigned int *DA_x1, unsigned int *DA_y1)
{
	int it = 0;
	int values[4];
	char delims[] = ",";
	char *result = NULL;
	result = strtok( inArg, delims );

	while( (result != NULL) && (it < 4 ) ) {
		values[it] = atoi(result);
		result = strtok( NULL, delims );
		it++;
	}

	if (it != 4) {
		return EXIT_FAILURE;
	}
	else{
		*DA_x0 = values[0]; *DA_y0 = values[1];
		*DA_x1 = values[2]; *DA_y1 = values[3];
		return EXIT_SUCCESS;
	}
}


/* -------------------------------------------------------------------------- */
/**
 * J2K_DUMP MAIN
 */
/* -------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
	FILE *fsrc = NULL, *fout = NULL;

	opj_dparameters_t parameters;			/* Decompression parameters */
	opj_event_mgr_t event_mgr;				/* Event manager */
	opj_image_t image;					/* Image structure */
	opj_codec_t* dinfo = NULL;				/* Handle to a decompressor */
	opj_stream_t *cio = NULL;				/* Stream */
	opj_codestream_info_v2_t* cstr_info;
	opj_codestream_index_t* cstr_index;

	OPJ_INT32 num_images, imageno;
	img_fol_t img_fol;
	dircnt_t *dirptr = NULL;

	opj_bool l_go_on = OPJ_TRUE;
	OPJ_UINT32 l_max_data_size = 1000;
	OPJ_BYTE * l_data = (OPJ_BYTE *) malloc(1000);

	/* Set decoding parameters to default values */
	opj_set_default_decoder_parameters(&parameters);

	/* Initialize img_fol */
	memset(&img_fol,0,sizeof(img_fol_t));

	/* Parse input and get user encoding parameters */
	if(parse_cmdline_decoder(argc, argv, &parameters,&img_fol) == 1) {
		return EXIT_FAILURE;
	}

	/* Set default event mgr */
	opj_initialize_default_event_handler(&event_mgr, parameters.m_verbose);

	/* Initialize reading of directory */
	if(img_fol.set_imgdir==1){	
		int it_image;
		num_images=get_num_images(img_fol.imgdirpath);

		dirptr=(dircnt_t*)malloc(sizeof(dircnt_t));
		if(dirptr){
			dirptr->filename_buf = (char*)malloc(num_images*OPJ_PATH_LEN*sizeof(char));	/* Stores at max 10 image file names */
			dirptr->filename = (char**) malloc(num_images*sizeof(char*));

			if(!dirptr->filename_buf){
				return EXIT_FAILURE;
			}

			for(it_image=0;it_image<num_images;it_image++){
				dirptr->filename[it_image] = dirptr->filename_buf + it_image*OPJ_PATH_LEN;
			}
		}
		if(load_images(dirptr,img_fol.imgdirpath)==1){
			return EXIT_FAILURE;
		}

		if (num_images==0){
			fprintf(stdout,"Folder is empty\n");
			return EXIT_FAILURE;
		}
	}else{
		num_images=1;
	}

	/* Try to open for writing the output file if necessary */
	if (parameters.outfile[0] != 0){
		fout = fopen(parameters.outfile,"w");
		if (!fout){
			fprintf(stderr, "ERROR -> failed to open %s for writing\n", parameters.outfile);
			return EXIT_FAILURE;
		}
	}
	else
		fout = stdout;

	/* Read the header of each image one by one */
	for(imageno = 0; imageno < num_images ; imageno++){

		fprintf(stderr,"\n");

		if(img_fol.set_imgdir==1){
			if (get_next_file(imageno, dirptr,&img_fol, &parameters)) {
				fprintf(stderr,"skipping file...\n");
				continue;
			}
		}

		/* Read the input file and put it in memory */
		/* ---------------------------------------- */
		fsrc = fopen(parameters.infile, "rb");
		if (!fsrc) {
			fprintf(stderr, "ERROR -> failed to open %s for reading\n", parameters.infile);
			return EXIT_FAILURE;
		}

		cio = opj_stream_create_default_file_stream(fsrc,1);
		if (!cio){
			fclose(fsrc);
			fprintf(stderr, "ERROR -> failed to create the stream from the file\n");
			return EXIT_FAILURE;
		}

		/* Read the JPEG2000 stream */
		/* ------------------------ */

		switch(parameters.decod_format) {
			case J2K_CFMT:	/* JPEG-2000 codestream */
			{
				/* Get a decoder handle */
				dinfo = opj_create_decompress_v2(CODEC_J2K);
				break;
			}
			case JP2_CFMT:	/* JPEG 2000 compressed image data */
			{
				/* Get a decoder handle */
				dinfo = opj_create_decompress_v2(CODEC_JP2);
				break;
			}
			case JPT_CFMT:	/* JPEG 2000, JPIP */
			{
				/* Get a decoder handle */
				dinfo = opj_create_decompress_v2(CODEC_JPT);
				break;
			}
			default:
				fprintf(stderr, "skipping file..\n");
				opj_stream_destroy(cio);
				continue;
		}

		/* Setup the decoder decoding parameters using user parameters */
		if ( !opj_setup_decoder_v2(dinfo, &parameters, &event_mgr) ){
			fprintf(stderr, "ERROR -> j2k_dump: failed to setup the decoder\n");
			opj_stream_destroy(cio);
			fclose(fsrc);
			opj_destroy_codec(dinfo);
			fclose(fout);
			return EXIT_FAILURE;
		}

		/* Read the main header of the codestream and if necessary the JP2 boxes*/
		if(! opj_read_header(cio, dinfo, &image)){
			fprintf(stderr, "ERROR -> j2k_dump: failed to read the header\n");
			opj_stream_destroy(cio);
			fclose(fsrc);
			opj_destroy_codec(dinfo);
			fclose(fout);
			return EXIT_FAILURE;
		}

		opj_dump_codec(dinfo, OPJ_IMG_INFO | OPJ_J2K_MH_INFO | OPJ_J2K_MH_IND, fout );

		cstr_info = opj_get_cstr_info(dinfo);

		cstr_index = opj_get_cstr_index(dinfo);

#ifdef MSD
		fprintf(stdout,"Setting decoding area to %d,%d,%d,%d\n",
				parameters.DA_x0, parameters.DA_y0, parameters.DA_x1, parameters.DA_y1);


		/* FIXME WIP_MSD <*/
		if (! opj_set_decode_area(	dinfo,
									parameters.DA_x0, parameters.DA_y0,
									parameters.DA_x1, parameters.DA_y1)){
			fprintf(stderr, "ERROR -> j2k_dump: failed to set the decoded area\n");
			opj_stream_destroy(cio);
			opj_destroy_codec(dinfo);
			fclose(fsrc);
			fclose(fout);
			return EXIT_FAILURE;
		}

		while (l_go_on) {
			OPJ_INT32 l_current_tile_x0,l_current_tile_y0,l_current_tile_x1,l_current_tile_y1;
			OPJ_UINT32 l_nb_comps, l_tile_index, l_data_size;


			if (! opj_read_tile_header(	dinfo,
										cio,
										&l_tile_index,
										&l_data_size,
										&l_current_tile_x0,
										&l_current_tile_y0,
										&l_current_tile_x1,
										&l_current_tile_y1,
										&l_nb_comps,
										&l_go_on
										)) {
				fprintf(stderr, "ERROR -> j2k_dump: failed read the tile header\n");
				opj_stream_destroy(cio);
				fclose(fsrc);
				opj_destroy_codec(dinfo);
				return EXIT_FAILURE;
			}

			if (l_go_on) {

				if (l_data_size > l_max_data_size) {

					l_data = (OPJ_BYTE *) realloc(l_data,l_data_size);
					if (! l_data) {
						opj_stream_destroy(cio);
						opj_destroy_codec(dinfo);
						fclose(fsrc);
						fclose(fout);
						return EXIT_FAILURE;
					}

					l_max_data_size = l_data_size;
				}

				if (! opj_decode_tile_data(dinfo,l_tile_index,l_data,l_data_size,cio))
				{
					free(l_data);
					opj_stream_destroy(cio);
					opj_destroy_codec(dinfo);
					fclose(fsrc);
					fclose(fout);
					return EXIT_FAILURE;
				}
				/** now should inspect image to know the reduction factor and then how to behave with data */
			}
		}
		/* FIXME WIP_MSD >*/
#endif

		/* close the byte stream */
		opj_stream_destroy(cio);
		fclose(fsrc);

		/* free remaining structures */
		if (dinfo) {
			opj_destroy_codec(dinfo);
		}

		/* destroy the image header */
		opj_image_destroy(&image);

	}

	/* Close the output file */
	fclose(fout);

  return EXIT_SUCCESS;
}
