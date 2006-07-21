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

void encode_help_display() {
	fprintf(stdout,"HELP\n----\n\n");
	fprintf(stdout,"- the -h option displays this help information on screen\n\n");


	fprintf(stdout,"List of parameters for the JPEG 2000 encoder:\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"REMARKS:\n");
	fprintf(stdout,"---------\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"The markers written to the main_header are : SOC SIZ COD QCD COM.\n");
	fprintf(stdout,"COD and QCD never appear in the tile_header.\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"- This coder can encode a mega image, a test was made on a 24000x24000 pixels \n");
	fprintf(stdout,"color image.  You need enough disk space memory (twice the original) to encode \n");
	fprintf(stdout,"the image,i.e. for a 1.5 GB image you need a minimum of 3GB of disk memory)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"By default:\n");
	fprintf(stdout,"------------\n");
	fprintf(stdout,"\n");
	fprintf(stdout," * Lossless\n");
	fprintf(stdout," * 1 tile\n");
	fprintf(stdout," * Size of precinct : 2^15 x 2^15 (means 1 precinct)\n");
	fprintf(stdout," * Size of code-block : 64 x 64\n");
	fprintf(stdout," * Number of resolutions: 6\n");
	fprintf(stdout," * No SOP marker in the codestream\n");
	fprintf(stdout," * No EPH marker in the codestream\n");
	fprintf(stdout," * No sub-sampling in x or y direction\n");
	fprintf(stdout," * No mode switch activated\n");
	fprintf(stdout," * Progression order: LRCP\n");
	fprintf(stdout," * No index file\n");
	fprintf(stdout," * No ROI upshifted\n");
	fprintf(stdout," * No offset of the origin of the image\n");
	fprintf(stdout," * No offset of the origin of the tiles\n");
	fprintf(stdout," * Reversible DWT 5-3\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Parameters:\n");
	fprintf(stdout,"------------\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Required Parameters (except with -h):\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-i           : source file  (-i source.pnm also *.pgm, *.ppm) \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-o           : destination file (-o dest.j2k or .jp2) \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Optional Parameters:\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-h           : display the help information \n ");
	fprintf(stdout,"\n");
	fprintf(stdout,"-r           : different compression ratios for successive layers (-r 20,10,5)\n ");
	fprintf(stdout,"	         - The rate specified for each quality level is the desired \n");
	fprintf(stdout,"	           compression factor.\n");
	fprintf(stdout,"		   Example: -r 20,10,1 means quality 1: compress 20x, \n");
	fprintf(stdout,"		     quality 2: compress 10x and quality 3: compress lossless\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"               (options -r and -q cannot be used together)\n ");
	fprintf(stdout,"\n");

	fprintf(stdout,"-q           : different psnr for successive layers (-q 30,40,50) \n ");

	fprintf(stdout,"               (options -r and -q cannot be used together)\n ");

	fprintf(stdout,"\n");
	fprintf(stdout,"-n           : number of resolutions (-n 3) \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-b           : size of code block (-b 32,32) \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-c           : size of precinct (-c 128,128) \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-t           : size of tile (-t 512,512) \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-p           : progression order (-p LRCP) [LRCP, RLCP, RPCL, PCRL, CPRL] \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-s           : subsampling factor (-s 2,2) [-s X,Y] \n");
	fprintf(stdout,"	     Remark: subsampling bigger than 2 can produce error\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-SOP         : write SOP marker before each packet \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-EPH         : write EPH marker after each header packet \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-M           : mode switch (-M 3) [1=BYPASS(LAZY) 2=RESET 4=RESTART(TERMALL)\n");
	fprintf(stdout,"                 8=VSC 16=ERTERM(SEGTERM) 32=SEGMARK(SEGSYM)] \n");
	fprintf(stdout,"                 Indicate multiple modes by adding their values. \n");
	fprintf(stdout,"                 ex: RESTART(4) + RESET(2) + SEGMARK(32) = -M 38\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-x           : create an index file *.Idx (-x index_name.Idx) \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-ROI         : c=%%d,U=%%d : quantization indices upshifted \n");
	fprintf(stdout,"               for component c=%%d [%%d = 0,1,2]\n");
	fprintf(stdout,"               with a value of U=%%d [0 <= %%d <= 37] (i.e. -ROI:c=0,U=25) \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-d           : offset of the origin of the image (-d 150,300) \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-T           : offset of the origin of the tiles (-T 100,75) \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-I           : use the irreversible DWT 9-7 (-I) \n");
	fprintf(stdout,"\n");
	fprintf(stdout,"IMPORTANT:\n");
	fprintf(stdout,"-----------\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"The index file has the structure below:\n");
	fprintf(stdout,"---------------------------------------\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Image_height Image_width\n");
	fprintf(stdout,"progression order\n");
	fprintf(stdout,"Tiles_size_X Tiles_size_Y\n");
	fprintf(stdout,"Components_nb\n");
	fprintf(stdout,"Layers_nb\n");
	fprintf(stdout,"decomposition_levels\n");
	fprintf(stdout,"[Precincts_size_X_res_Nr Precincts_size_Y_res_Nr]...\n");
	fprintf(stdout,"   [Precincts_size_X_res_0 Precincts_size_Y_res_0]\n");
	fprintf(stdout,"Main_header_end_position\n");
	fprintf(stdout,"Codestream_size\n");
	fprintf(stdout,"Tile_0 start_pos end_Theader end_pos TotalDisto NumPix MaxMSE\n");
	fprintf(stdout,"Tile_1   ''           ''        ''        ''       ''    ''\n");
	fprintf(stdout,"...\n");
	fprintf(stdout,"Tile_Nt   ''           ''        ''        ''       ''    ''\n");
	fprintf(stdout,"Tpacket_0 Tile layer res. comp. prec. start_pos end_pos disto\n");
	fprintf(stdout,"...\n");
	fprintf(stdout,"Tpacket_Np ''   ''    ''   ''    ''       ''       ''     ''\n");

	fprintf(stdout,"MaxDisto\n");

	fprintf(stdout,"TotalDisto\n\n");
}

OPJ_PROG_ORDER give_progression(char progression[4]) {
	if(strncmp(progression, "LRCP", 4) == 0) {
		return LRCP;
	}
	if(strncmp(progression, "RLCP", 4) == 0) {
		return RLCP;
	}
	if(strncmp(progression, "RPCL", 4) == 0) {
		return RPCL;
	}
	if(strncmp(progression, "PCRL", 4) == 0) {
		return PCRL;
	}
	if(strncmp(progression, "CPRL", 4) == 0) {
		return CPRL;
	}

	return PROG_UNKNOWN;
}

int get_file_format(char *filename) {
	unsigned int i;
	static const char *extension[] = {
    "pgx", "pnm", "pgm", "ppm", "bmp", "j2k", "jp2"
    };
	static const int format[] = {
    PGX_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, BMP_DFMT, J2K_CFMT, JP2_CFMT
    };
	char * ext = strrchr(filename, '.') + 1;
	for(i = 0; i < sizeof(format)/sizeof(*format); i++) {
		if(strnicmp(ext, extension[i], 3) == 0) {
			return format[i];
		}
	}

	return -1;
}

/* -------------------------------------------------------------------------*/

int parse_cmdline_encoder(int argc, char **argv, opj_cparameters_t *parameters) {
	int i, j;

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
					case PGX_DFMT:
					case PXM_DFMT:
					case BMP_DFMT:
						break;
					default:
						fprintf(stderr,
							"!! Unrecognized format for infile : %s "
              "[accept only *.pnm, *.pgm, *.ppm, *.pgx or *.bmp] !!\n\n", 
							infile);
						return 1;
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
					case J2K_CFMT:
					case JP2_CFMT:
						break;
					default:
						fprintf(stderr, "Unknown output format image %s [only *.j2k, *.jp2]!! \n", outfile);
						return 1;
				}
				strncpy(parameters->outfile, outfile, MAX_PATH);
			}
			break;

				/* ----------------------------------------------------- */

			case 'r':			/* rates rates/distorsion */
			{
				char *s = optarg;
				while (sscanf(s, "%d", &parameters->tcp_rates[parameters->tcp_numlayers]) == 1) {
					parameters->tcp_numlayers++;
					while (*s && *s != ',') {
						s++;
					}
					if (!*s)
						break;
					s++;
				}
				parameters->cp_disto_alloc = 1;
			}
			break;

				/* ----------------------------------------------------- */

			case 'q':			/* add fixed_quality */
			{
				char *s = optarg;
				while (sscanf(s, "%f", &parameters->tcp_distoratio[parameters->tcp_numlayers]) == 1) {
					parameters->tcp_numlayers++;
					while (*s && *s != ',') {
						s++;
					}
					if (!*s)
						break;
					s++;
				}
				parameters->cp_fixed_quality = 1;
			}
			break;

				/* dda */
				/* ----------------------------------------------------- */

			case 'f':			/* mod fixed_quality (before : -q) */
			{
				int *row = NULL, *col = NULL;
				int numlayers = 0, numresolution = 0, matrix_width = 0;

				char *s = optarg;
				sscanf(s, "%d", &numlayers);
				s++;
				if (numlayers > 9)
					s++;

				parameters->tcp_numlayers = numlayers;
				numresolution = parameters->numresolution;
				matrix_width = numresolution * 3;
				parameters->cp_matrice = (int *) malloc(numlayers * matrix_width * sizeof(int));
				s = s + 2;

				for (i = 0; i < numlayers; i++) {
					row = &parameters->cp_matrice[i * matrix_width];
					col = row;
					parameters->tcp_rates[i] = 1;
					sscanf(s, "%d,", &col[0]);
					s += 2;
					if (col[0] > 9)
						s++;
					col[1] = 0;
					col[2] = 0;
					for (j = 1; j < numresolution; j++) {
						col += 3;
						sscanf(s, "%d,%d,%d", &col[0], &col[1], &col[2]);
						s += 6;
						if (col[0] > 9)
							s++;
						if (col[1] > 9)
							s++;
						if (col[2] > 9)
							s++;
					}
					if (i < numlayers - 1)
						s++;
				}
				parameters->cp_fixed_alloc = 1;
			}
			break;

				/* ----------------------------------------------------- */

			case 't':			/* tiles */
			{
				sscanf(optarg, "%d,%d", &parameters->cp_tdx, &parameters->cp_tdy);
				parameters->tile_size_on = true;
			}
			break;

				/* ----------------------------------------------------- */

			case 'n':			/* resolution */
			{
				sscanf(optarg, "%d", &parameters->numresolution);
			}
			break;

				/* ----------------------------------------------------- */
			case 'c':			/* precinct dimension */
			{
				char sep;
				int res_spec = 0;

				char *s = optarg;
				do {
					sep = 0;
					sscanf(s, "[%d,%d]%c", &parameters->prcw_init[res_spec],
                                 &parameters->prch_init[res_spec], &sep);
					parameters->csty |= 0x01;
					res_spec++;
					s = strpbrk(s, "]") + 2;
				}
				while (sep == ',');
				parameters->res_spec = res_spec;
			}
			break;

				/* ----------------------------------------------------- */

			case 'b':			/* code-block dimension */
			{
				int cblockw_init = 0, cblockh_init = 0;
				sscanf(optarg, "%d,%d", &cblockw_init, &cblockh_init);
				if (cblockw_init * cblockh_init > 4096 || cblockw_init > 1024
					|| cblockw_init < 4 || cblockh_init > 1024 || cblockh_init < 4) {
					fprintf(stderr,
						"!! Size of code_block error (option -b) !!\n\nRestriction :\n"
            "    * width*height<=4096\n    * 4<=width,height<= 1024\n\n");
					return 1;
				}
				parameters->cblockw_init = cblockw_init;
				parameters->cblockh_init = cblockh_init;
			}
			break;

				/* ----------------------------------------------------- */

			case 'x':			/* creation of index file */
			{
				char *index = optarg;
				strncpy(parameters->index, index, MAX_PATH);
				parameters->index_on = 1;
			}
			break;

				/* ----------------------------------------------------- */

			case 'p':			/* progression order */
			{
				char progression[4];

				strncpy(progression, optarg, 4);
				parameters->prog_order = give_progression(progression);
				if (parameters->prog_order == -1) {
					fprintf(stderr, "Unrecognized progression order "
            "[LRCP, RLCP, RPCL, PCRL, CPRL] !!\n");
					return 1;
				}
			}
			break;

				/* ----------------------------------------------------- */

			case 's':			/* subsampling factor */
			{
				if (sscanf(optarg, "%d,%d", &parameters->subsampling_dx,
                                    &parameters->subsampling_dy) != 2) {
					fprintf(stderr,	"'-s' sub-sampling argument error !  [-s dx,dy]\n");
					return 1;
				}
			}
			break;

				/* ----------------------------------------------------- */

			case 'd':			/* coordonnate of the reference grid */
			{
				if (sscanf(optarg, "%d,%d", &parameters->image_offset_x0,
                                    &parameters->image_offset_y0) != 2) {
					fprintf(stderr,	"-d 'coordonnate of the reference grid' argument "
            "error !! [-d x0,y0]\n");
					return 1;
				}
			}
			break;

				/* ----------------------------------------------------- */

			case 'h':			/* display an help description */
				encode_help_display();
				return 1;

				/* ----------------------------------------------------- */

			case 'P':			/* POC */
			{
				int numpocs = 0;		/* number of progression order change (POC) default 0 */
				opj_poc_t *POC = NULL;	/* POC : used in case of Progression order change */

				char *s = optarg;
				POC = parameters->POC;

				fprintf(stderr, "/----------------------------------\\\n");
				fprintf(stderr, "|  POC option not fully tested !!  |\n");
				fprintf(stderr, "\\----------------------------------/\n");

				while (sscanf(s, "T%d=%d,%d,%d,%d,%d,%s", &POC[numpocs].tile,
					&POC[numpocs].resno0, &POC[numpocs].compno0,
					&POC[numpocs].layno1, &POC[numpocs].resno1,
					&POC[numpocs].compno1, POC[numpocs].progorder) == 7) {
					POC[numpocs].prg = give_progression(POC[numpocs].progorder);
					/* POC[numpocs].tile; */
					numpocs++;
					while (*s && *s != '/') {
						s++;
					}
					if (!*s) {
						break;
					}
					s++;
				}
				parameters->numpocs = numpocs;
			}
			break;

				/* ------------------------------------------------------ */

			case 'S':			/* SOP marker */
			{
				parameters->csty |= 0x02;
			}
			break;

				/* ------------------------------------------------------ */

			case 'E':			/* EPH marker */
			{
				parameters->csty |= 0x04;
			}
			break;

				/* ------------------------------------------------------ */

			case 'M':			/* Mode switch pas tous au point !! */
			{
				int value = 0;
				if (sscanf(optarg, "%d", &value) == 1) {
					for (i = 0; i <= 5; i++) {
						int cache = value & (1 << i);
						if (cache)
							parameters->mode |= (1 << i);
					}
				}
			}
			break;

				/* ------------------------------------------------------ */

			case 'R':			/* ROI */
			{
				if (sscanf(optarg, "OI:c=%d,U=%d", &parameters->roi_compno,
                                           &parameters->roi_shift) != 2) {
					fprintf(stderr, "ROI error !! [-ROI:c='compno',U='shift']\n");
					return 1;
				}
			}
			break;

				/* ------------------------------------------------------ */

			case 'T':			/* Tile offset */
			{
				if (sscanf(optarg, "%d,%d", &parameters->cp_tx0, &parameters->cp_ty0) != 2) {
					fprintf(stderr, "-T 'tile offset' argument error !! [-T X0,Y0]");
					return 1;
				}
			}
			break;

				/* ------------------------------------------------------ */

			case 'C':			/* add a comment */
			{
				parameters->cp_comment = (char*)malloc(strlen(optarg) + 1);
				if(parameters->cp_comment) {
					strcpy(parameters->cp_comment, optarg);
				}
			}
			break;

				/* ------------------------------------------------------ */

			case 'I':			/* reversible or not */
			{
				parameters->irreversible = 1;
			}
			break;

				/* ------------------------------------------------------ */

			default:
				fprintf(stderr, "ERROR -> this option is not valid \"-%c %s\"\n", c, optarg);
				return 1;
		}
	}

	/* check for possible errors */

	if((parameters->infile[0] == 0) || (parameters->outfile[0] == 0)) {
		fprintf(stderr, "usage: image_to_j2k -i image-file -o j2k/jp2-file (+ options)\n");
		return 1;
	}

	if ((parameters->cp_disto_alloc || parameters->cp_fixed_alloc || parameters->cp_fixed_quality)
		&& (!(parameters->cp_disto_alloc ^ parameters->cp_fixed_alloc ^ parameters->cp_fixed_quality))) {
		fprintf(stderr, "Error: options -r -q and -f cannot be used together !!\n");
		return 1;
	}				/* mod fixed_quality */

	/* if no rate entered, lossless by default */
	if (parameters->tcp_numlayers == 0) {
		parameters->tcp_rates[0] = 0;	/* MOD antonin : losslessbug */
		parameters->tcp_numlayers++;
		parameters->cp_disto_alloc = 1;
	}

	if((parameters->cp_tx0 > parameters->image_offset_x0) || (parameters->cp_ty0 > parameters->image_offset_y0)) {
		fprintf(stderr,
			"Error: Tile offset dimension is unnappropriate --> TX0(%d)<=IMG_X0(%d) TYO(%d)<=IMG_Y0(%d) \n",
			parameters->cp_tx0, parameters->image_offset_x0, parameters->cp_ty0, parameters->image_offset_y0);
		return 1;
	}

	for (i = 0; i < parameters->numpocs; i++) {
		if (parameters->POC[i].prg == -1) {
			fprintf(stderr,
				"Unrecognized progression order in option -P (POC n %d) [LRCP, RLCP, RPCL, PCRL, CPRL] !!\n",
				i + 1);
		}
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
sample debug callback expecting a FILE* client object
*/
void info_callback(const char *msg, void *client_data) {
	FILE *stream = (FILE*)client_data;
	fprintf(stream, "[INFO] %s", msg);
}

/* -------------------------------------------------------------------------- */

int main(int argc, char **argv) {
	bool bSuccess;
	opj_cparameters_t parameters;	/* compression parameters */
	opj_event_mgr_t event_mgr;		/* event manager */
	opj_image_t *image = NULL;

	/*
	configure the event callbacks (not required)
	setting of each callback is optionnal
	*/
	memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
	event_mgr.error_handler = error_callback;
	event_mgr.warning_handler = warning_callback;
	event_mgr.info_handler = info_callback;

	/* set encoding parameters to default values */
	opj_set_default_encoder_parameters(&parameters);

	/* parse input and get user encoding parameters */
	if(parse_cmdline_encoder(argc, argv, &parameters) == 1) {
		return 0;
	}

	if(parameters.cp_comment == NULL) {
    const char comment[] = "Created by OpenJPEG version ";
    const size_t clen = strlen(comment);
    const char *version = opj_version();
		parameters.cp_comment = (char*)malloc(clen+strlen(version)+1);
    strncpy(parameters.cp_comment, comment, clen); /* clen bytes */
    strcpy(parameters.cp_comment+clen, version); /* strlen(version) + \0 */
	}

	/* decode the source image */
	/* ----------------------- */

	switch (parameters.decod_format) {
		case PGX_DFMT:
			image = pgxtoimage(parameters.infile, &parameters);
			if (!image) {
				fprintf(stderr, " unable to load pgx file\n");
				return 1;
			}
			break;

		case PXM_DFMT:
			image = pnmtoimage(parameters.infile, &parameters);
			if (!image) {
				fprintf(stderr, " not a pnm file\n");
				return 1;
			}
			break;

		case BMP_DFMT:
			image = bmptoimage(parameters.infile, &parameters);
			if (!image) {
				fprintf(stderr, " not a bmp file\n");
				return 1;
			}
			break;
	}

	/* encode the destination image */
	/* ---------------------------- */

	if (parameters.cod_format == J2K_CFMT) {	/* J2K format output */
		int codestream_length;
		opj_cio_t *cio = NULL;
		FILE *f = NULL;

		/* get a J2K compressor handle */
		opj_cinfo_t* cinfo = opj_create_compress(CODEC_J2K);

		/* catch events using our callbacks and give a local context */
		opj_set_event_mgr((opj_common_ptr)cinfo, &event_mgr, stderr);

		/* setup the encoder parameters using the current image and user parameters */
		opj_setup_encoder(cinfo, &parameters, image);

		/* open a byte stream for writing */
		/* allocate memory for all tiles */
		cio = opj_cio_open((opj_common_ptr)cinfo, NULL, 0);

		/* encode the image */
		bSuccess = opj_encode(cinfo, cio, image, parameters.index);
		if (!bSuccess) {
			opj_cio_close(cio);
			fprintf(stderr, "failed to encode image\n");
			return 1;
		}
		codestream_length = cio_tell(cio);

		/* write the buffer to disk */
		f = fopen(parameters.outfile, "wb");
		if (!f) {
			fprintf(stderr, "failed to open %s for writing\n", parameters.outfile);
			return 1;
		}
		fwrite(cio->buffer, 1, codestream_length, f);
		fclose(f);

		/* close and free the byte stream */
		opj_cio_close(cio);

		/* free remaining compression structures */
		opj_destroy_compress(cinfo);

	} else {			/* JP2 format output */
		int codestream_length;
		opj_cio_t *cio = NULL;
		FILE *f = NULL;

		/* get a JP2 compressor handle */
		opj_cinfo_t* cinfo = opj_create_compress(CODEC_JP2);

		/* catch events using our callbacks and give a local context */
		opj_set_event_mgr((opj_common_ptr)cinfo, &event_mgr, stderr);			

		/* setup the encoder parameters using the current image and using user parameters */
		opj_setup_encoder(cinfo, &parameters, image);

		/* open a byte stream for writing */
		/* allocate memory for all tiles */
		cio = opj_cio_open((opj_common_ptr)cinfo, NULL, 0);

		/* encode the image */
		bSuccess = opj_encode(cinfo, cio, image, parameters.index);
		if (!bSuccess) {
			opj_cio_close(cio);
			fprintf(stderr, "failed to encode image\n");
			return 1;
		}
		codestream_length = cio_tell(cio);

		/* write the buffer to disk */
		f = fopen(parameters.outfile, "wb");
		if (!f) {
			fprintf(stderr, "failed to open %s for writing\n", parameters.outfile);
			return 1;
		}
		fwrite(cio->buffer, 1, codestream_length, f);
		fclose(f);

		/* close and free the byte stream */
		opj_cio_close(cio);

		/* free remaining compression structures */
		opj_destroy_compress(cinfo);

	}

	/* free user parameters structure */
  if(parameters.cp_comment) free(parameters.cp_comment);
	if(parameters.cp_matrice) free(parameters.cp_matrice);

	/* free image data */
	opj_image_destroy(image);

	return 0;
}

