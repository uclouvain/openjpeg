/*
 * Copyright (c) 2002-2007, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2007, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux and Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
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
#ifndef __TCD_H
#define __TCD_H
/**
@file tcd.h
@brief Implementation of a tile coder/decoder (TCD)

The functions in TCD.C have for goal to encode or decode each tile independently from
each other. The functions in TCD.C are used by some function in J2K.C.
*/

/** @defgroup TCD TCD - Implementation of a tile coder/decoder */
/*@{*/

/**
FIXME: documentation
*/
//typedef struct opj_tcd_seg {
//  unsigned char** data;
//  int dataindex;
//  int numpasses;
//  int len;
//  int maxpasses;
//  int numnewpasses;
//  int newlen;
//} opj_tcd_seg_t;

/**
FIXME: documentation
*/
typedef struct opj_tcd_seg {
	OPJ_BYTE ** data;
	OPJ_UINT32 dataindex;
	OPJ_UINT32 numpasses;
	OPJ_UINT32 real_num_passes;
	OPJ_UINT32 len;
	OPJ_UINT32 maxpasses;
	OPJ_UINT32 numnewpasses;
	OPJ_UINT32 newlen;
} opj_tcd_seg_t;

/**
FIXME: documentation
*/
typedef struct opj_tcd_pass {
  int rate;
  double distortiondec;
  int term, len;
} opj_tcd_pass_t;

typedef struct opj_tcd_pass_v2 {
	OPJ_UINT32 rate;
	OPJ_FLOAT64 distortiondec;
	OPJ_UINT32 len;
	OPJ_UINT32 term : 1;
} opj_tcd_pass_v2_t;

/**
FIXME: documentation
*/
typedef struct opj_tcd_layer {
	OPJ_UINT32 numpasses;		/* Number of passes in the layer */
	OPJ_UINT32 len;				/* len of information */
	OPJ_FLOAT64 disto;			/* add for index (Cfr. Marcela) */
	OPJ_BYTE *data;				/* data */
} opj_tcd_layer_t;

/**
FIXME: documentation
*/
typedef struct opj_tcd_cblk_enc {
  unsigned char* data;	/* Data */
  opj_tcd_layer_t* layers;	/* layer information */
  opj_tcd_pass_t* passes;	/* information about the passes */
  int x0, y0, x1, y1;		/* dimension of the code-blocks : left upper corner (x0, y0) right low corner (x1,y1) */
  int numbps;
  int numlenbits;
  int numpasses;		/* number of pass already done for the code-blocks */
  int numpassesinlayers;	/* number of passes in the layer */
  int totalpasses;		/* total number of passes */
} opj_tcd_cblk_enc_t;

/**
FIXME: documentation
*/
typedef struct opj_tcd_cblk_enc_v2 {
	OPJ_BYTE* data;					/* Data */
	opj_tcd_layer_t* layers;		/* layer information */
	opj_tcd_pass_v2_t* passes;		/* information about the passes */
	OPJ_INT32 x0, y0, x1, y1;		/* dimension of the code-blocks : left upper corner (x0, y0) right low corner (x1,y1) */
	OPJ_UINT32 numbps;
	OPJ_UINT32 numlenbits;
	OPJ_UINT32 numpasses;			/* number of pass already done for the code-blocks */
	OPJ_UINT32 numpassesinlayers;	/* number of passes in the layer */
	OPJ_UINT32 totalpasses;			/* total number of passes */
} opj_tcd_cblk_enc_v2_t;

typedef struct opj_tcd_cblk_dec {
  unsigned char* data;	/* Data */
  opj_tcd_seg_t* segs;		/* segments informations */
	int x0, y0, x1, y1;		/* dimension of the code-blocks : left upper corner (x0, y0) right low corner (x1,y1) */
  int numbps;
  int numlenbits;
  int len;			/* length */
  int numnewpasses;		/* number of pass added to the code-blocks */
  int numsegs;			/* number of segments */
} opj_tcd_cblk_dec_t;


typedef struct opj_tcd_cblk_dec_v2 {
	OPJ_BYTE * data;				/* Data */
	opj_tcd_seg_t* segs;			/* segments informations */
	OPJ_INT32 x0, y0, x1, y1;		/* dimension of the code-blocks : left upper corner (x0, y0) right low corner (x1,y1) */
	OPJ_UINT32 numbps;
	OPJ_UINT32 numlenbits;
	OPJ_UINT32 len;					/* length */
	OPJ_UINT32 numnewpasses;		/* number of pass added to the code-blocks */
	OPJ_UINT32 numsegs;				/* number of segments */
	OPJ_UINT32 real_num_segs;
	OPJ_UINT32 m_current_max_segs;
} opj_tcd_cblk_dec_v2_t;

/**
FIXME: documentation
*/
typedef struct opj_tcd_precinct {
  int x0, y0, x1, y1;		/* dimension of the precinct : left upper corner (x0, y0) right low corner (x1,y1) */
  int cw, ch;			/* number of precinct in width and heigth */
  union{		/* code-blocks informations */
	  opj_tcd_cblk_enc_t* enc;
	  opj_tcd_cblk_dec_t* dec;
  } cblks;
  opj_tgt_tree_t *incltree;		/* inclusion tree */
  opj_tgt_tree_t *imsbtree;		/* IMSB tree */
} opj_tcd_precinct_t;


typedef struct opj_tcd_precinct_v2 {
	OPJ_INT32 x0, y0, x1, y1;		/* dimension of the precinct : left upper corner (x0, y0) right low corner (x1,y1) */
	OPJ_UINT32 cw, ch;				/* number of precinct in width and heigth */
	union{							/* code-blocks informations */
		opj_tcd_cblk_enc_v2_t* enc;
		opj_tcd_cblk_dec_v2_t* dec;
	} cblks;
	OPJ_UINT32 block_size;			/* size taken by cblks (in bytes) */
	struct opj_tgt_tree *incltree;	/* inclusion tree */
	struct opj_tgt_tree *imsbtree;	/* IMSB tree */
} opj_tcd_precinct_v2_t;

/**
FIXME: documentation
*/
typedef struct opj_tcd_band {
  int x0, y0, x1, y1;		/* dimension of the subband : left upper corner (x0, y0) right low corner (x1,y1) */
  int bandno;
  opj_tcd_precinct_t *precincts;	/* precinct information */
  int numbps;
  float stepsize;
} opj_tcd_band_t;

typedef struct opj_tcd_band_v2 {
	OPJ_INT32 x0, y0, x1, y1;		/* dimension of the subband : left upper corner (x0, y0) right low corner (x1,y1) */
	OPJ_UINT32 bandno;
	opj_tcd_precinct_v2_t *precincts;	/* precinct information */
	OPJ_UINT32 precincts_data_size;	/* size of data taken by precincts */
	OPJ_INT32 numbps;
	OPJ_FLOAT32 stepsize;
} opj_tcd_band_v2_t;

/**
FIXME: documentation
*/
typedef struct opj_tcd_resolution {
  int x0, y0, x1, y1;		/* dimension of the resolution level : left upper corner (x0, y0) right low corner (x1,y1) */
  int pw, ph;
  int numbands;			/* number sub-band for the resolution level */
  opj_tcd_band_t bands[3];		/* subband information */
} opj_tcd_resolution_t;

typedef struct opj_tcd_resolution_v2 {
	OPJ_INT32 x0, y0, x1, y1;		/* dimension of the resolution level : left upper corner (x0, y0) right low corner (x1,y1) */
	OPJ_UINT32 pw, ph;
	OPJ_UINT32 numbands;			/* number sub-band for the resolution level */
	opj_tcd_band_v2_t bands[3];		/* subband information */
} opj_tcd_resolution_v2_t;

/**
FIXME: documentation
*/
typedef struct opj_tcd_tilecomp {
  int x0, y0, x1, y1;		/* dimension of component : left upper corner (x0, y0) right low corner (x1,y1) */
  int numresolutions;		/* number of resolutions level */
  opj_tcd_resolution_t *resolutions;	/* resolutions information */
  int *data;			/* data of the component */
  int numpix;			/* add fixed_quality */
} opj_tcd_tilecomp_t;

typedef struct opj_tcd_tilecomp_v2
{
	OPJ_INT32 x0, y0, x1, y1;				/* dimension of component : left upper corner (x0, y0) right low corner (x1,y1) */
	OPJ_UINT32 numresolutions;				/* number of resolutions level */
	OPJ_UINT32 minimum_num_resolutions;		/* number of resolutions level to decode (at max)*/
	opj_tcd_resolution_v2_t *resolutions;	/* resolutions information */
	OPJ_UINT32 resolutions_size;			/* size of data for resolutions (in bytes) */
	OPJ_INT32 *data;						/* data of the component */
	OPJ_UINT32 data_size;					/* size of the data of the component */
	OPJ_INT32 numpix;						/* add fixed_quality */
} opj_tcd_tilecomp_v2_t;


/**
FIXME: documentation
*/
typedef struct opj_tcd_tile {
  int x0, y0, x1, y1;		/* dimension of the tile : left upper corner (x0, y0) right low corner (x1,y1) */
  int numcomps;			/* number of components in tile */
  opj_tcd_tilecomp_t *comps;	/* Components information */
  int numpix;			/* add fixed_quality */
  double distotile;		/* add fixed_quality */
  double distolayer[100];	/* add fixed_quality */
  /** packet number */
  int packno;
} opj_tcd_tile_t;

typedef struct opj_tcd_tile_v2 {
	OPJ_INT32 x0, y0, x1, y1;		/* dimension of the tile : left upper corner (x0, y0) right low corner (x1,y1) */
	OPJ_UINT32 numcomps;			/* number of components in tile */
	opj_tcd_tilecomp_v2_t *comps;	/* Components information */
	OPJ_INT32 numpix;				/* add fixed_quality */
	OPJ_FLOAT64 distotile;			/* add fixed_quality */
	OPJ_FLOAT64 distolayer[100];	/* add fixed_quality */
	/** packet number */
	OPJ_UINT32 packno;
} opj_tcd_tile_v2_t;

/**
FIXME: documentation
*/
typedef struct opj_tcd_image {
  int tw, th;			/* number of tiles in width and heigth */
  opj_tcd_tile_t *tiles;		/* Tiles information */
} opj_tcd_image_t;

typedef struct opj_tcd_image_v2
{
	opj_tcd_tile_v2_t *tiles;		/* Tiles information */
}
opj_tcd_image_v2_t;

/**
Tile coder/decoder
*/
typedef struct opj_tcd {
	/** Position of the tilepart flag in Progression order*/
	int tp_pos;
	/** Tile part number*/
	int tp_num;
	/** Current tile part number*/
	int cur_tp_num;
	/** Total number of tileparts of the current tile*/
	int cur_totnum_tp;
	/** Current Packet iterator number */
	int cur_pino;
	/** codec context */
	opj_common_ptr cinfo;

	/** info on each image tile */
	opj_tcd_image_t *tcd_image;
	/** image */
	opj_image_t *image;
	/** coding parameters */
	opj_cp_t *cp;
	/** pointer to the current encoded/decoded tile */
	opj_tcd_tile_t *tcd_tile;
	/** coding/decoding parameters common to all tiles */
	opj_tcp_t *tcp;
	/** current encoded/decoded tile */
	int tcd_tileno;
	/** Time taken to encode a tile*/
	double encoding_time;
} opj_tcd_t;


struct opj_image;
struct opj_cp_v2;
struct opj_tcp_v2;
/**
Tile coder/decoder
*/
typedef struct opj_tcd_v2
{
	/** Position of the tilepart flag in Progression order*/
	OPJ_INT32 tp_pos;
	/** Tile part number*/
	OPJ_UINT32 tp_num;
	/** Current tile part number*/
	OPJ_UINT32 cur_tp_num;
	/** Total number of tileparts of the current tile*/
	OPJ_UINT32 cur_totnum_tp;
	/** Current Packet iterator number */
	OPJ_UINT32 cur_pino;
	/** info on each image tile */
	opj_tcd_image_v2_t *tcd_image;
	/** image header */
	struct opj_image_header *image_header;
	/** coding parameters */
	struct opj_cp_v2 *cp;
	/** coding/decoding parameters common to all tiles */
	struct opj_tcp_v2 *tcp;
	/** current encoded/decoded tile */
	OPJ_UINT32 tcd_tileno;
	/** tell if the tcd is a decoder. */
	OPJ_UINT32 m_is_decoder : 1;
} opj_tcd_v2_t;

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */

/**
Dump the content of a tcd structure
*/
void tcd_dump(FILE *fd, opj_tcd_t *tcd, opj_tcd_image_t *img);
/**
Create a new TCD handle
@param cinfo Codec context info
@return Returns a new TCD handle if successful returns NULL otherwise
*/
opj_tcd_t* tcd_create(opj_common_ptr cinfo);

/**
Create a new TCD handle
@param FIXME
@return Returns a new TCD handle if successful returns NULL otherwise
*/
opj_tcd_v2_t* tcd_create_v2(opj_bool p_is_decoder);

/**
Destroy a previously created TCD handle
@param tcd TCD handle to destroy
*/
void tcd_destroy_v2(opj_tcd_v2_t *tcd);

/**
 * Initialize the tile coder and may reuse some meory.
 * @param	p_tcd		TCD handle.
 * @param	p_image		raw image.
 * @param	p_cp		coding parameters.
 * @param	p_tile_no	current tile index to encode.
 *
 * @return true if the encoding values could be set (false otherwise).
*/
opj_bool tcd_init_v2(
						opj_tcd_v2_t *p_tcd,
						//struct opj_image * p_image,
						opj_image_header_t * p_image_header,
						//struct opj_cp * p_cp
						opj_cp_v2_t * p_cp
					);

/**
 * Allocates memory for decoding a specific tile.
 *
 * @param	p_tcd		the tile decoder.
 * @param	p_image		the image to decode.
 * @param	p_cp		the decoding parameters.
 * @param	p_tile_no	the index of the tile received in sequence. This not necesseraly lead to the
 * tile at index p_tile_no.
 * @param	p_cstr_info	codestream info (if any).
 *
 * @return	true if the remaining data is sufficient.s
 */
opj_bool tcd_init_decode_tile(
							opj_tcd_v2_t *p_tcd,
							OPJ_UINT32 p_tile_no
							);

/**
Destroy a previously created TCD handle
@param tcd TCD handle to destroy
*/
void tcd_destroy(opj_tcd_t *tcd);
/**
Initialize the tile coder (allocate the memory)
@param tcd TCD handle
@param image Raw image
@param cp Coding parameters
@param curtileno Number that identifies the tile that will be encoded
*/
void tcd_malloc_encode(opj_tcd_t *tcd, opj_image_t * image, opj_cp_t * cp, int curtileno);
/**
Free the memory allocated for encoding
@param tcd TCD handle
*/
void tcd_free_encode(opj_tcd_t *tcd);
/**
Initialize the tile coder (reuses the memory allocated by tcd_malloc_encode)
@param tcd TCD handle
@param image Raw image
@param cp Coding parameters
@param curtileno Number that identifies the tile that will be encoded
*/
void tcd_init_encode(opj_tcd_t *tcd, opj_image_t * image, opj_cp_t * cp, int curtileno);
/**
Initialize the tile decoder
@param tcd TCD handle
@param image Raw image
@param cp Coding parameters
*/
void tcd_malloc_decode(opj_tcd_t *tcd, opj_image_t * image, opj_cp_t * cp);
void tcd_malloc_decode_tile(opj_tcd_t *tcd, opj_image_t * image, opj_cp_t * cp, int tileno, opj_codestream_info_t *cstr_info);
void tcd_makelayer_fixed(opj_tcd_t *tcd, int layno, int final);
void tcd_rateallocate_fixed(opj_tcd_t *tcd);
void tcd_makelayer(opj_tcd_t *tcd, int layno, double thresh, int final);
opj_bool tcd_rateallocate(opj_tcd_t *tcd, unsigned char *dest, int len, opj_codestream_info_t *cstr_info);
/**
Encode a tile from the raw image into a buffer
@param tcd TCD handle
@param tileno Number that identifies one of the tiles to be encoded
@param dest Destination buffer
@param len Length of destination buffer
@param cstr_info Codestream information structure 
@return 
*/
int tcd_encode_tile(opj_tcd_t *tcd, int tileno, unsigned char *dest, int len, opj_codestream_info_t *cstr_info);
/**
Decode a tile from a buffer into a raw image
@param tcd TCD handle
@param src Source buffer
@param len Length of source buffer
@param tileno Number that identifies one of the tiles to be decoded
@param cstr_info Codestream information structure
*/
opj_bool tcd_decode_tile(opj_tcd_t *tcd, unsigned char *src, int len, int tileno, opj_codestream_info_t *cstr_info);
/**
Free the memory allocated for decoding
@param tcd TCD handle
*/
void tcd_free_decode(opj_tcd_t *tcd);
void tcd_free_decode_tile(opj_tcd_t *tcd, int tileno);


/**
 * Gets the maximum tile size that will be taken by the tile once decoded.
 */
OPJ_UINT32 tcd_get_decoded_tile_size (
						 opj_tcd_v2_t *p_tcd
						 );

/**
Decode a tile from a buffer into a raw image
@param tcd TCD handle
@param src Source buffer
@param len Length of source buffer
@param tileno Number that identifies one of the tiles to be decoded
*/
opj_bool tcd_decode_tile_v2(opj_tcd_v2_t *tcd,
							OPJ_BYTE *src,
							OPJ_UINT32 len,
							OPJ_UINT32 tileno,
							opj_codestream_index_t *cstr_info);


/**
 * Copies tile data from the system onto the given memory block.
 */
opj_bool tcd_update_tile_data (
						 opj_tcd_v2_t *p_tcd,
						 OPJ_BYTE * p_dest,
						 OPJ_UINT32 p_dest_length
						 );

/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __TCD_H */
