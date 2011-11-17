/*
 * Copyright (c) 2002-2007, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2007, Professor Benoit Macq
 * Copyright (c) 2002-2003, Yannick Verschueren
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
#ifndef __JP2_H
#define __JP2_H
/**
@file jp2.h
@brief The JPEG-2000 file format Reader/Writer (JP2)

*/

/** @defgroup JP2 JP2 - JPEG-2000 file format reader/writer */
/*@{*/

#define JPIP_JPIP 0x6a706970

#define JP2_JP   0x6a502020		/**< JPEG 2000 signature box */
#define JP2_FTYP 0x66747970		/**< File type box */

#define JP2_JP2H 0x6a703268		/**< JP2 header box (super-box) */
#define 	JP2_IHDR 0x69686472		/**< Image header box */
#define 	JP2_BPCC 0x62706363		/**< Bits per component box */
#define 	JP2_COLR 0x636f6c72		/**< Colour specification box */
#define 	JP2_PCLR 0x70636c72		/**< Palette box */
#define 	JP2_CMAP 0x636d6170		/**< Component Mapping box */
#define 	JP2_CDEF 0x63646566		/**< Channel Definition box */
// For the future MSD
// #define JP2_RES 0x72657320	/**< Resolution box (super-box) */

#define JP2_JP2C 0x6a703263		/**< Contiguous codestream box */

// For the future MSD
// #define JP2_JP2I 0x6a703269	/**< Intellectual property box */
// #define JP2_XML 0x786d6c20	/**< XML box */
// #define JP2_UUID 0x75756994	/**< UUID box */
// #define JP2_UINF 0x75696e66	/**< UUID info box (super-box) */
// #define		JP2_ULST 0x756c7374	/**< UUID list box */
#define 	JP2_URL  0x75726c20		/**< Data entry URL box */

#define JP2_DTBL 0x6474626c		/**< Data Reference box */

#define JP2_JP2  0x6a703220		/**< File type fields */


/* ----------------------------------------------------------------------- */

typedef enum
{
	JP2_STATE_NONE			= 0x0,
	JP2_STATE_SIGNATURE		= 0x1,
	JP2_STATE_FILE_TYPE		= 0x2,
	JP2_STATE_HEADER		= 0x4,
	JP2_STATE_CODESTREAM	= 0x8,
	JP2_STATE_END_CODESTREAM	= 0x10,
	JP2_STATE_UNKNOWN		= 0x80000000
}
JP2_STATE;

typedef enum
{
	JP2_IMG_STATE_NONE			= 0x0,
	JP2_IMG_STATE_UNKNOWN		= 0x80000000
}
JP2_IMG_STATE;

/** 
Channel description: channel index, type, assocation
*/
typedef struct opj_jp2_cdef_info
{
    unsigned short cn, typ, asoc;
} opj_jp2_cdef_info_t;

/** 
Channel descriptions and number of descriptions
*/
typedef struct opj_jp2_cdef
{
    opj_jp2_cdef_info_t *info;
    unsigned short n;
} opj_jp2_cdef_t;

/** 
Component mappings: channel index, mapping type, palette index
*/
typedef struct opj_jp2_cmap_comp
{
    unsigned short cmp;
    unsigned char mtyp, pcol;
} opj_jp2_cmap_comp_t;

/** 
Palette data: table entries, palette columns
*/
typedef struct opj_jp2_pclr
{
    unsigned int *entries;
    unsigned char *channel_sign;
    unsigned char *channel_size;
    opj_jp2_cmap_comp_t *cmap;
    unsigned short nr_entries, nr_channels;
} opj_jp2_pclr_t;

/** 
Collector for ICC profile, palette, component mapping, channel description 
*/
typedef struct opj_jp2_color
{
    unsigned char *icc_profile_buf;
    int icc_profile_len;

    opj_jp2_cdef_t *jp2_cdef;
    opj_jp2_pclr_t *jp2_pclr;
    unsigned char jp2_has_colr;
} opj_jp2_color_t;

/** 
JP2 component
*/
typedef struct opj_jp2_comps {
  int depth;		  
  int sgnd;		   
  OPJ_UINT32 bpcc;
} opj_jp2_comps_t;

/**
JPEG-2000 file format reader/writer
*/
typedef struct opj_jp2 {
	/** codec context */
	opj_common_ptr cinfo;
	/** handle to the J2K codec  */
	opj_j2k_t *j2k;
	unsigned int w;
	unsigned int h;
	unsigned int numcomps;
	unsigned int bpc;
	unsigned int C;
	unsigned int UnkC;
	unsigned int IPR;
	unsigned int meth;
	unsigned int approx;
	unsigned int enumcs;
	unsigned int precedence;
	unsigned int brand;
	unsigned int minversion;
	unsigned int numcl;
	unsigned int *cl;
	opj_jp2_comps_t *comps;
	unsigned int j2k_codestream_offset;
	unsigned int j2k_codestream_length;
	opj_bool jpip_on;
} opj_jp2_t;

/**
JPEG-2000 file format reader/writer
*/
typedef struct opj_jp2_v2
{
	/** handle to the J2K codec  */
	struct opj_j2k_v2 *j2k;
	/** list of validation procedures */
	struct opj_procedure_list * m_validation_list;
	/** list of execution procedures */
	struct opj_procedure_list * m_procedure_list;

	/* width of image */
	OPJ_UINT32 w;
	/* height of image */
	OPJ_UINT32 h;
	/* number of components in the image */
	OPJ_UINT32 numcomps;
	OPJ_UINT32 bpc;
	OPJ_UINT32 C;
	OPJ_UINT32 UnkC;
	OPJ_UINT32 IPR;
	OPJ_UINT32 meth;
	OPJ_UINT32 approx;
	OPJ_UINT32 enumcs;
	OPJ_UINT32 precedence;
	OPJ_UINT32 brand;
	OPJ_UINT32 minversion;
	OPJ_UINT32 numcl;
	OPJ_UINT32 *cl;
	opj_jp2_comps_t *comps;
	OPJ_UINT32 j2k_codestream_offset;
	OPJ_UINT32 jp2_state;
	OPJ_UINT32 jp2_img_state;

	opj_jp2_color_t color;

}
opj_jp2_v2_t;

/**
JP2 Box
*/
typedef struct opj_jp2_box {
  OPJ_UINT32 length;
  OPJ_UINT32 type;
  OPJ_INT32 init_pos;
} opj_jp2_box_t;

typedef struct opj_jp2_header_handler
{
	/* marker value */
	int id;
	/* action linked to the marker */
	opj_bool (*handler) (opj_jp2_v2_t *jp2, unsigned char * p_header_data, OPJ_UINT32 p_header_size, struct opj_event_mgr * p_manager);
}
opj_jp2_header_handler_t;

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */
/**
Write the JP2H box - JP2 Header box (used in MJ2)
@param jp2 JP2 handle
@param cio Output buffer stream
*/
void jp2_write_jp2h(opj_jp2_t *jp2, opj_cio_t *cio);
/**
Read the JP2H box - JP2 Header box (used in MJ2)
@param jp2 JP2 handle
@param cio Input buffer stream
@param ext Collector for profile, cdef and pclr data
@return Returns true if successful, returns false otherwise
*/
opj_bool jp2_read_jp2h(opj_jp2_t *jp2, opj_cio_t *cio, opj_jp2_color_t *color);
/**
Creates a JP2 decompression structure
@param cinfo Codec context info
@return Returns a handle to a JP2 decompressor if successful, returns NULL otherwise
*/
opj_jp2_t* jp2_create_decompress(opj_common_ptr cinfo);
/**
Destroy a JP2 decompressor handle
@param jp2 JP2 decompressor handle to destroy
*/
void jp2_destroy_decompress(opj_jp2_t *jp2);
/**
Setup the decoder decoding parameters using user parameters.
Decoding parameters are returned in jp2->j2k->cp. 
@param jp2 JP2 decompressor handle
@param parameters decompression parameters
*/
void jp2_setup_decoder(opj_jp2_t *jp2, opj_dparameters_t *parameters);
/**
Setup the decoder decoding parameters using user parameters.
Decoding parameters are returned in jp2->j2k->cp.
@param jp2 JP2 decompressor handle
@param parameters decompression parameters
*/
void jp2_setup_decoder_v2(opj_jp2_v2_t *jp2, opj_dparameters_t *parameters);
/**
Decode an image from a JPEG-2000 file stream
@param jp2 JP2 decompressor handle
@param cio Input buffer stream
@param cstr_info Codestream information structure if required, NULL otherwise
@return Returns a decoded image if successful, returns NULL otherwise
*/
opj_image_t* opj_jp2_decode(opj_jp2_t *jp2, opj_cio_t *cio, opj_codestream_info_t *cstr_info);

/**
 * Decode an image from a JPEG-2000 file stream
 * @param jp2 JP2 decompressor handle
 * @param cio Input buffer stream
 * @param cstr_info Codestream information structure if required, NULL otherwise
 * @return Returns a decoded image if successful, returns NULL otherwise
*/
opj_bool jp2_decode_v2(	opj_jp2_v2_t *jp2,
						struct opj_stream_private *cio,
						opj_image_t* p_image,
						struct opj_event_mgr * p_manager);


/**
Creates a JP2 compression structure
@param cinfo Codec context info
@return Returns a handle to a JP2 compressor if successful, returns NULL otherwise
*/
opj_jp2_t* jp2_create_compress(opj_common_ptr cinfo);
/**
Destroy a JP2 compressor handle
@param jp2 JP2 compressor handle to destroy
*/
void jp2_destroy_compress(opj_jp2_t *jp2);
/**
Setup the encoder parameters using the current image and using user parameters. 
Coding parameters are returned in jp2->j2k->cp. 
@param jp2 JP2 compressor handle
@param parameters compression parameters
@param image input filled image
*/
void jp2_setup_encoder(opj_jp2_t *jp2, opj_cparameters_t *parameters, opj_image_t *image);
/**
Encode an image into a JPEG-2000 file stream
@param jp2 JP2 compressor handle
@param cio Output buffer stream
@param image Image to encode
@param cstr_info Codestream information structure if required, NULL otherwise
@return Returns true if successful, returns false otherwise
*/
opj_bool opj_jp2_encode(opj_jp2_t *jp2, opj_cio_t *cio, opj_image_t *image, opj_codestream_info_t *cstr_info);


/* ----------------------------------------------------------------------- */

/**
 * Ends the decompression procedures and possibiliy add data to be read after the
 * codestream.
 */
opj_bool jp2_end_decompress(opj_jp2_v2_t *jp2, struct opj_stream_private *cio, struct opj_event_mgr * p_manager);

/**
 * Reads a jpeg2000 file header structure.
 *
 * @param cio the stream to read data from.
 * @param jp2 the jpeg2000 file header structure.
 * @param p_manager the user event manager.
 *
 * @return true if the box is valid.
 */
opj_bool jp2_read_header(	struct opj_stream_private *p_stream,
							opj_jp2_v2_t *jp2,
							opj_image_t ** p_img_header,
							struct opj_event_mgr * p_manager
							);

/**
 * Reads a tile header.
 * @param	p_j2k		the jpeg2000 codec.
 * @param	p_stream			the stream to write data to.
 * @param	p_manager	the user event manager.
 */
opj_bool jp2_read_tile_header (
					 opj_jp2_v2_t * p_j2k,
					 OPJ_UINT32 * p_tile_index,
					 OPJ_UINT32 * p_data_size,
					 OPJ_INT32 * p_tile_x0,
					 OPJ_INT32 * p_tile_y0,
					 OPJ_INT32 * p_tile_x1,
					 OPJ_INT32 * p_tile_y1,
					 OPJ_UINT32 * p_nb_comps,
					 opj_bool * p_go_on,
					 struct opj_stream_private *p_stream,
					 struct opj_event_mgr * p_manager
					);

/**
 * Decode tile data.
 * @param	p_j2k		the jpeg2000 codec.
 * @param	p_stream			the stream to write data to.
 * @param	p_manager	the user event manager.
 */
opj_bool jp2_decode_tile (
					opj_jp2_v2_t * p_jp2,
					OPJ_UINT32 p_tile_index,
					OPJ_BYTE * p_data,
					OPJ_UINT32 p_data_size,
					struct opj_stream_private *p_stream,
					struct opj_event_mgr * p_manager
					);

/**
 * Creates a jpeg2000 file decompressor.
 *
 * @return	an empty jpeg2000 file codec.
 */
opj_jp2_v2_t* jp2_create (opj_bool p_is_decoder);

/**
Destroy a JP2 decompressor handle
@param jp2 JP2 decompressor handle to destroy
*/
void jp2_destroy(opj_jp2_v2_t *jp2);


/**
 * Sets the given area to be decoded. This function should be called right after opj_read_header and before any tile header reading.
 *
 * @param	p_jp2			the jpeg2000 codec.
 * @param	p_end_x			the right position of the rectangle to decode (in image coordinates).
 * @param	p_start_y		the up position of the rectangle to decode (in image coordinates).
 * @param	p_end_y			the bottom position of the rectangle to decode (in image coordinates).
 * @param	p_manager		the user event manager
 *
 * @return	true			if the area could be set.
 */
opj_bool jp2_set_decode_area(	opj_jp2_v2_t *p_jp2,
								opj_image_t* p_image,
								OPJ_INT32 p_start_x, OPJ_INT32 p_start_y,
								OPJ_INT32 p_end_x, OPJ_INT32 p_end_y,
								struct opj_event_mgr * p_manager );

opj_bool jp2_get_tile(	opj_jp2_v2_t *p_jp2,
						opj_stream_private_t *p_stream,
						opj_image_t* p_image,
						struct opj_event_mgr * p_manager,
						OPJ_UINT32 tile_index );


/**
 * Dump some elements from the JP2 decompression structure .
 *
 *@param p_jp2				the jp2 codec.
 *@param flag				flag to describe what elments are dump.
 *@param out_stream			output stream where dump the elements.
 *
*/
void jp2_dump (opj_jp2_v2_t* p_jp2, OPJ_INT32 flag, FILE* out_stream);

/**
 * Get the codestream info from a JPEG2000 codec.
 *
 *@param	p_jp2				jp2 codec.
 *
 *@return	the codestream information extract from the jpg2000 codec
 */
opj_codestream_info_v2_t* jp2_get_cstr_info(opj_jp2_v2_t* p_jp2);

/**
 * Get the codestream index from a JPEG2000 codec.
 *
 *@param	p_jp2				jp2 codec.
 *
 *@return	the codestream index extract from the jpg2000 codec
 */
opj_codestream_index_t* jp2_get_cstr_index(opj_jp2_v2_t* p_jp2);

opj_bool jp2_set_decoded_resolution_factor(opj_jp2_v2_t *p_jp2, OPJ_UINT32 res_factor, opj_event_mgr_t * p_manager);

/*@}*/

/*@}*/

#endif /* __JP2_H */

