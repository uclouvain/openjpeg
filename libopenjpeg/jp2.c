/*
 * Copyright (c) 2002-2007, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2007, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux and Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
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

/** @defgroup JP2 JP2 - JPEG-2000 file format reader/writer */
/*@{*/

#define BOX_SIZE	1024

/** @name Local static functions */
/*@{*/

/**
Read box headers
@param cinfo Codec context info
@param cio Input stream
@param box
@return Returns true if successful, returns false otherwise
*/
static opj_bool jp2_read_boxhdr(opj_common_ptr cinfo, opj_cio_t *cio, opj_jp2_box_t *box);
/*static void jp2_write_url(opj_cio_t *cio, char *Idx_file);*/
/**
Read the IHDR box - Image Header box
@param jp2 JP2 handle
@param cio Input buffer stream
@return Returns true if successful, returns false otherwise
*/
static opj_bool jp2_read_ihdr(opj_jp2_t *jp2, opj_cio_t *cio);

/**
 * Reads a IHDR box - Image Header box
 *
 * @param	p_image_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_image_header_size			the size of the image header
 * @param	p_manager					the user event manager.
 *
 * @return	true if the image header is valid, fale else.
 */
static opj_bool jp2_read_ihdr_v2(
							opj_jp2_v2_t *jp2,
							unsigned char * p_image_header_data,
							unsigned int p_image_header_size,
							struct opj_event_mgr * p_manager
						  );

static void jp2_write_ihdr(opj_jp2_t *jp2, opj_cio_t *cio);
static void jp2_write_bpcc(opj_jp2_t *jp2, opj_cio_t *cio);
static opj_bool jp2_read_bpcc(opj_jp2_t *jp2, opj_cio_t *cio);

/**
 * Reads a Bit per Component box.
 *
 * @param	p_bpc_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_bpc_header_size			the size of the bpc header
 * @param	p_manager					the user event manager.
 *
 * @return	true if the bpc header is valid, fale else.
 */
static opj_bool jp2_read_bpcc_v2(
							opj_jp2_v2_t *jp2,
							unsigned char * p_bpc_header_data,
							unsigned int p_bpc_header_size,
							struct opj_event_mgr * p_manager
						  );

static opj_bool jp2_read_cdef_v2(	opj_jp2_v2_t * jp2,
									unsigned char * p_cdef_header_data,
									OPJ_UINT32 p_cdef_header_size,
									opj_event_mgr_t * p_manager
									);

static void jp2_write_colr(opj_jp2_t *jp2, opj_cio_t *cio);
/**
Write the FTYP box - File type box
@param jp2 JP2 handle
@param cio Output buffer stream
*/
static void jp2_write_ftyp(opj_jp2_t *jp2, opj_cio_t *cio);
/**
Read the FTYP box - File type box
@param jp2 JP2 handle
@param cio Input buffer stream
@return Returns true if successful, returns false otherwise
*/
static opj_bool jp2_read_ftyp(opj_jp2_t *jp2, opj_cio_t *cio);

/**
 * Reads a a FTYP box - File type box
 *
 * @param	p_header_data	the data contained in the FTYP box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the FTYP box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the FTYP box is valid.
 */
static opj_bool jp2_read_ftyp_v2(
							opj_jp2_v2_t *jp2,
							unsigned char * p_header_data,
							unsigned int p_header_size,
							struct opj_event_mgr * p_manager
						);

/**
 * Reads the Jpeg2000 file Header box - JP2 Header box (warning, this is a super box).
 *
 * @param	p_header_data	the data contained in the file header box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the file header box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the JP2 Header box was successfully reconized.
*/
static opj_bool jp2_read_jp2h_v2(
						opj_jp2_v2_t *jp2,
						unsigned char * p_header_data,
						unsigned int p_header_size,
						struct opj_event_mgr * p_manager
					);

static int jp2_write_jp2c(opj_jp2_t *jp2, opj_cio_t *cio, opj_image_t *image, opj_codestream_info_t *cstr_info);
static opj_bool jp2_read_jp2c(opj_jp2_t *jp2, opj_cio_t *cio, unsigned int *j2k_codestream_length, unsigned int *j2k_codestream_offset);
static void jp2_write_jp(opj_cio_t *cio);
/**
Read the JP box - JPEG 2000 signature
@param jp2 JP2 handle
@param cio Input buffer stream
@return Returns true if successful, returns false otherwise
*/
static opj_bool jp2_read_jp(opj_jp2_t *jp2, opj_cio_t *cio);

/**
 * Reads a jpeg2000 file signature box.
 *
 * @param	p_header_data	the data contained in the signature box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the signature box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the file signature box is valid.
 */
static opj_bool jp2_read_jp_v2(
					opj_jp2_v2_t *jp2,
					unsigned char * p_header_data,
					unsigned int p_header_size,
					struct opj_event_mgr * p_manager
				 );

/**
Decode the structure of a JP2 file
@param jp2 JP2 handle
@param cio Input buffer stream
@param color Collector for profile, cdef and pclr data
@return Returns true if successful, returns false otherwise
*/
static opj_bool jp2_read_struct(opj_jp2_t *jp2, opj_cio_t *cio,
	opj_jp2_color_t *color);
/**
Apply collected palette data
@param color Collector for profile, cdef and pclr data
@param image 
*/
static void jp2_apply_pclr(opj_image_t *image, opj_jp2_color_t *color);
/**
Collect palette data
@param jp2 JP2 handle
@param cio Input buffer stream
@param box
@param color Collector for profile, cdef and pclr data
@return Returns true if successful, returns false otherwise
*/
static opj_bool jp2_read_pclr(opj_jp2_t *jp2, opj_cio_t *cio,
    opj_jp2_box_t *box, opj_jp2_color_t *color);

static opj_bool jp2_read_pclr_v2(	opj_jp2_v2_t *jp2,
							unsigned char * p_pclr_header_data,
							OPJ_UINT32 p_pclr_header_size,
							opj_event_mgr_t * p_manager
						  );

/**
Collect component mapping data
@param jp2 JP2 handle
@param cio Input buffer stream
@param box
@param color Collector for profile, cdef and pclr data
@return Returns true if successful, returns false otherwise
*/
static opj_bool jp2_read_cmap(opj_jp2_t *jp2, opj_cio_t *cio,
    opj_jp2_box_t *box, opj_jp2_color_t *color);


static opj_bool jp2_read_cmap_v2(	opj_jp2_v2_t * jp2,
							unsigned char * p_cmap_header_data,
							OPJ_UINT32 p_cmap_header_size,
							opj_event_mgr_t * p_manager
						  );

/**
Collect colour specification data
@param jp2 JP2 handle
@param cio Input buffer stream
@param box
@param color Collector for profile, cdef and pclr data
@return Returns true if successful, returns false otherwise
*/
static opj_bool jp2_read_colr(opj_jp2_t *jp2, opj_cio_t *cio,
    opj_jp2_box_t *box, opj_jp2_color_t *color);

/**
 * Reads the Color Specification box.
 *
 * @param	p_colr_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_colr_header_size			the size of the color header
 * @param	p_manager					the user event manager.
 *
 * @return	true if the bpc header is valid, fale else.
*/
static opj_bool jp2_read_colr_v2(
							opj_jp2_v2_t *jp2,
							unsigned char * p_colr_header_data,
							OPJ_UINT32 p_colr_header_size,
							struct opj_event_mgr * p_manager
						  );

/**
Write file Index (superbox)
@param[in] offset_jp2c offset of jp2c box
@param[in] length_jp2c length of jp2c box
@param[in] offset_idx  offset of cidx box
@param[in] length_idx  length of cidx box
@param[in] cio         file output handle
@return                length of fidx box
*/
static int write_fidx( int offset_jp2c, int length_jp2c, int offset_idx, int length_idx, opj_cio_t *cio);
/**
Write index Finder box
@param[in] offset offset of fidx box
@param[in] length length of fidx box
@param[in] cio         file output handle
*/
static void write_iptr( int offset, int length, opj_cio_t *cio);
/**

Write proxy box
@param[in] offset_jp2c offset of jp2c box
@param[in] length_jp2c length of jp2c box
@param[in] offset_idx  offset of cidx box
@param[in] length_idx  length of cidx box
@param[in] cio         file output handle
*/
static void write_prxy( int offset_jp2c, int length_jp2c, int offset_idx, int length_idx, opj_cio_t *cio);
/*@}*/

/*@}*/

/**
 * Sets up the procedures to do on reading header after the codestream.
 * Developpers wanting to extend the library can add their own writting procedures.
 */
static void jp2_setup_end_header_reading (opj_jp2_v2_t *jp2);

/**
 * Reads a jpeg2000 file header structure.
 *
 * @param cio the stream to read data from.
 * @param jp2 the jpeg2000 file header structure.
 * @param p_manager the user event manager.
 *
 * @return true if the box is valid.
 */
opj_bool jp2_read_header_procedure(
								opj_jp2_v2_t *jp2,
								struct opj_stream_private *cio,
								struct opj_event_mgr * p_manager
							);

/**
 * Excutes the given procedures on the given codec.
 *
 * @param	p_procedure_list	the list of procedures to execute
 * @param	jp2					the jpeg2000 file codec to execute the procedures on.
 * @param	cio					the stream to execute the procedures on.
 * @param	p_manager			the user manager.
 *
 * @return	true				if all the procedures were successfully executed.
 */
static opj_bool jp2_exec (
					opj_jp2_v2_t * jp2,
					struct opj_procedure_list * p_procedure_list,
					struct opj_stream_private *cio,
					struct opj_event_mgr * p_manager
				  );

/**
 * Reads a box header. The box is the way data is packed inside a jpeg2000 file structure.
 *
 * @param	cio						the input stream to read data from.
 * @param	box						the box structure to fill.
 * @param	p_number_bytes_read		pointer to an int that will store the number of bytes read from the stream (shoul usually be 2).
 * @param	p_manager				user event manager.
 *
 * @return	true if the box is reconized, false otherwise
*/
static opj_bool jp2_read_boxhdr_v2(
								opj_jp2_box_t *box,
								OPJ_UINT32 * p_number_bytes_read,
								struct opj_stream_private *cio,
								struct opj_event_mgr * p_manager
							);

/**
 * Finds the execution function related to the given box id.
 *
 * @param	p_id	the id of the handler to fetch.
 *
 * @return	the given handler or NULL if it could not be found.
 */
static const opj_jp2_header_handler_t * jp2_find_handler (int p_id );

/**
 * Finds the image execution function related to the given box id.
 *
 * @param	p_id	the id of the handler to fetch.
 *
 * @return	the given handler or NULL if it could not be found.
 */
static const opj_jp2_header_handler_t * jp2_img_find_handler (int p_id);

const opj_jp2_header_handler_t jp2_header [] =
{
	{JP2_JP,jp2_read_jp_v2},
	{JP2_FTYP,jp2_read_ftyp_v2},
	{JP2_JP2H,jp2_read_jp2h_v2}
};

const opj_jp2_header_handler_t jp2_img_header [] =
{
	{JP2_IHDR,jp2_read_ihdr_v2},
	{JP2_COLR,jp2_read_colr_v2},
	{JP2_BPCC,jp2_read_bpcc_v2},
	{JP2_PCLR,jp2_read_pclr_v2},
	{JP2_CMAP,jp2_read_cmap_v2},
	{JP2_CDEF,jp2_read_cdef_v2}

};

/**
 * Reads a box header. The box is the way data is packed inside a jpeg2000 file structure. Data is read from a character string
 *
 * @param	p_data					the character string to read data from.
 * @param	box						the box structure to fill.
 * @param	p_number_bytes_read		pointer to an int that will store the number of bytes read from the stream (shoul usually be 2).
 * @param	p_box_max_size			the maximum number of bytes in the box.
 *
 * @return	true if the box is reconized, false otherwise
*/
static opj_bool jp2_read_boxhdr_char(
								opj_jp2_box_t *box,
								OPJ_BYTE * p_data,
								OPJ_UINT32 * p_number_bytes_read,
								OPJ_UINT32 p_box_max_size,
								struct opj_event_mgr * p_manager
							);

/**
 * Sets up the validation ,i.e. adds the procedures to lauch to make sure the codec parameters
 * are valid. Developpers wanting to extend the library can add their own validation procedures.
 */
static void jp2_setup_decoding_validation (opj_jp2_v2_t *jp2);

/**
 * Sets up the procedures to do on reading header.
 * Developpers wanting to extend the library can add their own writting procedures.
 */
static void jp2_setup_header_reading (opj_jp2_v2_t *jp2);



/* ----------------------------------------------------------------------- */

static opj_bool jp2_read_boxhdr(opj_common_ptr cinfo, opj_cio_t *cio, opj_jp2_box_t *box) {
	box->init_pos = cio_tell(cio);
	box->length = cio_read(cio, 4);
	box->type = cio_read(cio, 4);
	if (box->length == 1) {
		if (cio_read(cio, 4) != 0) {
			opj_event_msg(cinfo, EVT_ERROR, "Cannot handle box sizes higher than 2^32\n");
			return OPJ_FALSE;
		}
		box->length = cio_read(cio, 4);
		if (box->length == 0) 
			box->length = cio_numbytesleft(cio) + 12;
	}
	else if (box->length == 0) {
		box->length = cio_numbytesleft(cio) + 8;
	}
	
	return OPJ_TRUE;
}

/**
 * Reads a box header. The box is the way data is packed inside a jpeg2000 file structure.
 *
 * @param	cio						the input stream to read data from.
 * @param	box						the box structure to fill.
 * @param	p_number_bytes_read		pointer to an int that will store the number of bytes read from the stream (should usually be 8).
 * @param	p_manager				user event manager.
 *
 * @return	true if the box is reconized, false otherwise
*/
opj_bool jp2_read_boxhdr_v2(opj_jp2_box_t *box, OPJ_UINT32 * p_number_bytes_read, opj_stream_private_t *cio, opj_event_mgr_t * p_manager)
{
	/* read header from file */
	unsigned char l_data_header [8];

	// preconditions
	assert(cio != 00);
	assert(box != 00);
	assert(p_number_bytes_read != 00);
	assert(p_manager != 00);

	*p_number_bytes_read = opj_stream_read_data(cio,l_data_header,8,p_manager);
	if (*p_number_bytes_read != 8) {
		return OPJ_FALSE;
	}

	/* process read data */
	opj_read_bytes(l_data_header,&(box->length), 4);
	opj_read_bytes(l_data_header+4,&(box->type), 4);

	// do we have a "special very large box ?"
	// read then the XLBox
	if (box->length == 1) {
		OPJ_UINT32 l_xl_part_size;

		OPJ_UINT32 l_nb_bytes_read = opj_stream_read_data(cio,l_data_header,8,p_manager);
		if (l_nb_bytes_read != 8) {
			if (l_nb_bytes_read > 0) {
				*p_number_bytes_read += l_nb_bytes_read;
			}

			return OPJ_FALSE;
		}

		opj_read_bytes(l_data_header,&l_xl_part_size, 4);
		if (l_xl_part_size != 0) {
			opj_event_msg_v2(p_manager, EVT_ERROR, "Cannot handle box sizes higher than 2^32\n");
			return OPJ_FALSE;
		}
		opj_read_bytes(l_data_header,&(box->length), 4);
	}
	return OPJ_TRUE;
}

#if 0
static void jp2_write_url(opj_cio_t *cio, char *Idx_file) {
	unsigned int i;
	opj_jp2_box_t box;

	box.init_pos = cio_tell(cio);
	cio_skip(cio, 4);
	cio_write(cio, JP2_URL, 4);	/* DBTL */
	cio_write(cio, 0, 1);		/* VERS */
	cio_write(cio, 0, 3);		/* FLAG */

	if(Idx_file) {
		for (i = 0; i < strlen(Idx_file); i++) {
			cio_write(cio, Idx_file[i], 1);
		}
	}

	box.length = cio_tell(cio) - box.init_pos;
	cio_seek(cio, box.init_pos);
	cio_write(cio, box.length, 4);	/* L */
	cio_seek(cio, box.init_pos + box.length);
}
#endif

static opj_bool jp2_read_ihdr(opj_jp2_t *jp2, opj_cio_t *cio) {
	opj_jp2_box_t box;

	opj_common_ptr cinfo = jp2->cinfo;

	jp2_read_boxhdr(cinfo, cio, &box);
	if (JP2_IHDR != box.type) {
		opj_event_msg(cinfo, EVT_ERROR, "Expected IHDR Marker\n");
		return OPJ_FALSE;
	}

	jp2->h = cio_read(cio, 4);			/* HEIGHT */
	jp2->w = cio_read(cio, 4);			/* WIDTH */
	jp2->numcomps = cio_read(cio, 2);	/* NC */
	jp2->comps = (opj_jp2_comps_t*) opj_malloc(jp2->numcomps * sizeof(opj_jp2_comps_t));

	jp2->bpc = cio_read(cio, 1);		/* BPC */

	jp2->C = cio_read(cio, 1);			/* C */
	jp2->UnkC = cio_read(cio, 1);		/* UnkC */
	jp2->IPR = cio_read(cio, 1);		/* IPR */

	if (cio_tell(cio) - box.init_pos != box.length) {
		opj_event_msg(cinfo, EVT_ERROR, "Error with IHDR Box\n");
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}

/**
 * Reads a IHDR box - Image Header box
 *
 * @param	p_image_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_image_header_size			the size of the image header
 * @param	p_image_header_max_size		maximum size of the header, any size bigger than this value should result the function to output false.
 * @param	p_manager					the user event manager.
 *
 * @return	true if the image header is valid, fale else.
 */
opj_bool jp2_read_ihdr_v2(
							opj_jp2_v2_t *jp2,
							unsigned char * p_image_header_data,
							unsigned int p_image_header_size,
							opj_event_mgr_t * p_manager
						  )
{
	// preconditions
	assert(p_image_header_data != 00);
	assert(jp2 != 00);
	assert(p_manager != 00);

	if (p_image_header_size != 14) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Bad image header box (bad size)\n");
		return OPJ_FALSE;
	}

	opj_read_bytes(p_image_header_data,&(jp2->h),4);			/* HEIGHT */
	p_image_header_data += 4;
	opj_read_bytes(p_image_header_data,&(jp2->w),4);			/* WIDTH */
	p_image_header_data += 4;
	opj_read_bytes(p_image_header_data,&(jp2->numcomps),2);			/* NC */
	p_image_header_data += 2;

	/* allocate memory for components */
	jp2->comps = (opj_jp2_comps_t*) opj_malloc(jp2->numcomps * sizeof(opj_jp2_comps_t));
	if (jp2->comps == 0) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Not enough memory to handle image header (ihdr)\n");
		return OPJ_FALSE;
	}
	memset(jp2->comps,0,jp2->numcomps * sizeof(opj_jp2_comps_t));

	opj_read_bytes(p_image_header_data,&(jp2->bpc),1);			/* BPC */
	++ p_image_header_data;

	// if equal to 0 then need a BPC box (cf. chapter about image header box of the norm)
	/*if (jp2->bpc == 0){
			// indicate with a flag that we will wait a BPC box
		}*/

	opj_read_bytes(p_image_header_data,&(jp2->C),1);			/* C */
	++ p_image_header_data;

	// Should be equal to 7 cf. chapter about image header box of the norm
	if (jp2->C != 7){
		opj_event_msg_v2(p_manager, EVT_INFO, "JP2 IHDR box: compression type indicate that the file is not a conforming JP2 file (%d) \n", jp2->C);
	}

	opj_read_bytes(p_image_header_data,&(jp2->UnkC),1);			/* UnkC */
	++ p_image_header_data;
	opj_read_bytes(p_image_header_data,&(jp2->IPR),1);			/* IPR */
	++ p_image_header_data;

	return OPJ_TRUE;
}

static void jp2_write_ihdr(opj_jp2_t *jp2, opj_cio_t *cio) {
	opj_jp2_box_t box;

	box.init_pos = cio_tell(cio);
	cio_skip(cio, 4);
	cio_write(cio, JP2_IHDR, 4);		/* IHDR */

	cio_write(cio, jp2->h, 4);			/* HEIGHT */
	cio_write(cio, jp2->w, 4);			/* WIDTH */
	cio_write(cio, jp2->numcomps, 2);	/* NC */

	cio_write(cio, jp2->bpc, 1);		/* BPC */

	cio_write(cio, jp2->C, 1);			/* C : Always 7 */
	cio_write(cio, jp2->UnkC, 1);		/* UnkC, colorspace unknown */
	cio_write(cio, jp2->IPR, 1);		/* IPR, no intellectual property */

	box.length = cio_tell(cio) - box.init_pos;
	cio_seek(cio, box.init_pos);
	cio_write(cio, box.length, 4);	/* L */
	cio_seek(cio, box.init_pos + box.length);
}

static void jp2_write_bpcc(opj_jp2_t *jp2, opj_cio_t *cio) {
	unsigned int i;
	opj_jp2_box_t box;

	box.init_pos = cio_tell(cio);
	cio_skip(cio, 4);
	cio_write(cio, JP2_BPCC, 4);	/* BPCC */

	for (i = 0; i < jp2->numcomps; i++) {
		cio_write(cio, jp2->comps[i].bpcc, 1);
	}

	box.length = cio_tell(cio) - box.init_pos;
	cio_seek(cio, box.init_pos);
	cio_write(cio, box.length, 4);	/* L */
	cio_seek(cio, box.init_pos + box.length);
}


static opj_bool jp2_read_bpcc(opj_jp2_t *jp2, opj_cio_t *cio) {
	unsigned int i;
	opj_jp2_box_t box;

	opj_common_ptr cinfo = jp2->cinfo;

	jp2_read_boxhdr(cinfo, cio, &box);
	if (JP2_BPCC != box.type) {
		opj_event_msg(cinfo, EVT_ERROR, "Expected BPCC Marker\n");
		return OPJ_FALSE;
	}

	for (i = 0; i < jp2->numcomps; i++) {
		jp2->comps[i].bpcc = cio_read(cio, 1);
	}

	if (cio_tell(cio) - box.init_pos != box.length) {
		opj_event_msg(cinfo, EVT_ERROR, "Error with BPCC Box\n");
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}

/**
 * Reads a Bit per Component box.
 *
 * @param	p_bpc_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_bpc_header_size			pointer that will hold the size of the bpc header
 * @param	p_bpc_header_max_size		maximum size of the header, any size bigger than this value should result the function to output false.
 * @param	p_manager					the user event manager.
 *
 * @return	true if the bpc header is valid, fale else.
 */
opj_bool jp2_read_bpcc_v2(	opj_jp2_v2_t *jp2,
							unsigned char * p_bpc_header_data,
							unsigned int p_bpc_header_size,
							opj_event_mgr_t * p_manager
						  )
{
	OPJ_UINT32 i;

	// preconditions
	assert(p_bpc_header_data != 00);
	assert(jp2 != 00);
	assert(p_manager != 00);

	// TODO MSD
	/*if (jp2->bpc != 0 ){
		opj_event_msg_v2(p_manager, EVT_WARNING, "A BPCC header box is available although BPC is different to zero (%d)\n",jp2->bpc);
	}*/

	// and length is relevant
	if (p_bpc_header_size != jp2->numcomps) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Bad BPCC header box (bad size)\n");
		return OPJ_FALSE;
	}

	// read info for each component
	for (i = 0; i < jp2->numcomps; ++i) {
		opj_read_bytes(p_bpc_header_data,&jp2->comps[i].bpcc ,1);	/* read each BPCC component */
		++p_bpc_header_data;
	}

	return OPJ_TRUE;
}

static void jp2_write_colr(opj_jp2_t *jp2, opj_cio_t *cio) {
	opj_jp2_box_t box;

	box.init_pos = cio_tell(cio);
	cio_skip(cio, 4);
	cio_write(cio, JP2_COLR, 4);		/* COLR */

	cio_write(cio, jp2->meth, 1);		/* METH */
	cio_write(cio, jp2->precedence, 1);	/* PRECEDENCE */
	cio_write(cio, jp2->approx, 1);		/* APPROX */

	if(jp2->meth == 2)
	 jp2->enumcs = 0;

	cio_write(cio, jp2->enumcs, 4);	/* EnumCS */

	box.length = cio_tell(cio) - box.init_pos;
	cio_seek(cio, box.init_pos);
	cio_write(cio, box.length, 4);	/* L */
	cio_seek(cio, box.init_pos + box.length);
}

static void jp2_free_pclr(opj_jp2_color_t *color)
{
    opj_free(color->jp2_pclr->channel_sign);
    opj_free(color->jp2_pclr->channel_size);
    opj_free(color->jp2_pclr->entries);

	if(color->jp2_pclr->cmap) opj_free(color->jp2_pclr->cmap);

    opj_free(color->jp2_pclr); color->jp2_pclr = NULL;
}

static void free_color_data(opj_jp2_color_t *color)
{
	if(color->jp2_pclr)
   {
	jp2_free_pclr(color);
   }
	if(color->jp2_cdef) 
   {
	if(color->jp2_cdef->info) opj_free(color->jp2_cdef->info);
	opj_free(color->jp2_cdef);
   }
	if(color->icc_profile_buf) opj_free(color->icc_profile_buf);
}


static void jp2_apply_pclr(opj_image_t *image, opj_jp2_color_t *color)
{
	opj_image_comp_t *old_comps, *new_comps;
	OPJ_BYTE *channel_size, *channel_sign;
	OPJ_UINT32 *entries;
	opj_jp2_cmap_comp_t *cmap;
	OPJ_INT32 *src, *dst;
	OPJ_UINT32 j, max;
	OPJ_UINT16 i, nr_channels, cmp, pcol;
	OPJ_INT32 k, top_k;

	channel_size = color->jp2_pclr->channel_size;
	channel_sign = color->jp2_pclr->channel_sign;
	entries = color->jp2_pclr->entries;
	cmap = color->jp2_pclr->cmap;
	nr_channels = color->jp2_pclr->nr_channels;

	old_comps = image->comps;
	new_comps = (opj_image_comp_t*)
			opj_malloc(nr_channels * sizeof(opj_image_comp_t));

	for(i = 0; i < nr_channels; ++i) {
		pcol = cmap[i].pcol; cmp = cmap[i].cmp;

		new_comps[pcol] = old_comps[cmp];

		/* Direct use */
		if(cmap[i].mtyp == 0){
			old_comps[cmp].data = NULL; continue;
		}

		/* Palette mapping: */
		new_comps[pcol].data = (int*)
				opj_malloc(old_comps[cmp].w * old_comps[cmp].h * sizeof(int));
		new_comps[pcol].prec = channel_size[i];
		new_comps[pcol].sgnd = channel_sign[i];
	}

	top_k = color->jp2_pclr->nr_entries - 1;

	for(i = 0; i < nr_channels; ++i) {
		/* Direct use: */
		if(cmap[i].mtyp == 0) continue;

		/* Palette mapping: */
		cmp = cmap[i].cmp; pcol = cmap[i].pcol;
		src = old_comps[cmp].data;
		dst = new_comps[pcol].data;
		max = new_comps[pcol].w * new_comps[pcol].h;

		for(j = 0; j < max; ++j)
		{
			/* The index */
			if((k = src[j]) < 0) k = 0; else if(k > top_k) k = top_k;

			/* The colour */
			dst[j] = entries[k * nr_channels + pcol];
		}
	}

	max = image->numcomps;
	for(i = 0; i < max; ++i) {
		if(old_comps[i].data) opj_free(old_comps[i].data);
	}

	opj_free(old_comps);
	image->comps = new_comps;
	image->numcomps = nr_channels;

	jp2_free_pclr(color);

}/* apply_pclr() */


static opj_bool jp2_read_pclr(opj_jp2_t *jp2, opj_cio_t *cio,
	opj_jp2_box_t *box, opj_jp2_color_t *color)
{
	opj_jp2_pclr_t *jp2_pclr;
	unsigned char *channel_size, *channel_sign;
	unsigned int *entries;
	unsigned short nr_entries, nr_channels;
	unsigned short i, j;
	unsigned char uc;

	OPJ_ARG_NOT_USED(box);
	OPJ_ARG_NOT_USED(jp2);

/* Part 1, I.5.3.4: 'There shall be at most one Palette box inside
 * a JP2 Header box' :
*/
	if(color->jp2_pclr) return OPJ_FALSE;

	nr_entries = (unsigned short)cio_read(cio, 2); /* NE */
	nr_channels = (unsigned short)cio_read(cio, 1);/* NPC */

	entries = (unsigned int*)
	 opj_malloc(nr_channels * nr_entries * sizeof(unsigned int));
	channel_size = (unsigned char*)opj_malloc(nr_channels);
	channel_sign = (unsigned char*)opj_malloc(nr_channels);

	jp2_pclr = (opj_jp2_pclr_t*)opj_malloc(sizeof(opj_jp2_pclr_t));
	jp2_pclr->channel_sign = channel_sign;
	jp2_pclr->channel_size = channel_size;
	jp2_pclr->entries = entries;
	jp2_pclr->nr_entries = nr_entries;
	jp2_pclr->nr_channels = nr_channels;
	jp2_pclr->cmap = NULL;

	color->jp2_pclr = jp2_pclr;

	for(i = 0; i < nr_channels; ++i)
   {
	uc = cio_read(cio, 1); /* Bi */
	channel_size[i] = (uc & 0x7f) + 1;
	channel_sign[i] = (uc & 0x80)?1:0;
   }

	for(j = 0; j < nr_entries; ++j)
   {
	for(i = 0; i < nr_channels; ++i)
  {
/* Cji */
	*entries++ = cio_read(cio, channel_size[i]>>3);
  }
   }

	return OPJ_TRUE;
}/* jp2_read_pclr() */

/**
 * Reads a palette box.
 *
 * @param	p_bpc_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_bpc_header_size			pointer that will hold the size of the bpc header
 * @param	p_bpc_header_max_size		maximum size of the header, any size bigger than this value should result the function to output false.
 * @param	p_manager					the user event manager.
 *
 * @return	true if the bpc header is valid, fale else.
 */
opj_bool jp2_read_pclr_v2(	opj_jp2_v2_t *jp2,
							unsigned char * p_pclr_header_data,
							OPJ_UINT32 p_pclr_header_size,
							opj_event_mgr_t * p_manager
						  ){
	opj_jp2_pclr_t *jp2_pclr;
	OPJ_BYTE *channel_size, *channel_sign;
	OPJ_UINT32 *entries;
	OPJ_UINT16 nr_entries,nr_channels;
	OPJ_UINT16 i, j;
	OPJ_UINT32 l_value;

	// preconditions
	assert(p_pclr_header_data != 00);
	assert(jp2 != 00);
	assert(p_manager != 00);

	if(jp2->color.jp2_pclr)
		return OPJ_FALSE;

	opj_read_bytes(p_pclr_header_data, &l_value , 2);	/* NE */
	p_pclr_header_data += 2;
	nr_entries = (OPJ_UINT16) l_value;

	opj_read_bytes(p_pclr_header_data, &l_value , 1);	/* NPC */
	++p_pclr_header_data;
	nr_channels = (OPJ_UINT16) l_value;

	entries = (OPJ_UINT32*) opj_malloc(nr_channels * nr_entries * sizeof(OPJ_UINT32));
	channel_size = (OPJ_BYTE*) opj_malloc(nr_channels);
	channel_sign = (OPJ_BYTE*) opj_malloc(nr_channels);

	jp2_pclr = (opj_jp2_pclr_t*)opj_malloc(sizeof(opj_jp2_pclr_t));
	jp2_pclr->channel_sign = channel_sign;
	jp2_pclr->channel_size = channel_size;
	jp2_pclr->entries = entries;
	jp2_pclr->nr_entries = nr_entries;
	jp2_pclr->nr_channels = nr_channels;
	jp2_pclr->cmap = NULL;

	jp2->color.jp2_pclr = jp2_pclr;

	for(i = 0; i < nr_channels; ++i) {
		opj_read_bytes(p_pclr_header_data, &l_value , 1);	/* Bi */
		++p_pclr_header_data;

		channel_size[i] = (l_value & 0x7f) + 1;
		channel_sign[i] = (l_value & 0x80)? 1 : 0;
	}

	for(j = 0; j < nr_entries; ++j) {
		for(i = 0; i < nr_channels; ++i) {
			//*entries++ = cio_read(cio, channel_size[i]>>3);
			opj_read_bytes(p_pclr_header_data, &l_value , channel_size[i]>>3);	/* Cji */
			p_pclr_header_data += channel_size[i]>>3;
			*entries = (OPJ_UINT32) l_value;
			entries++;
		}
	}

	return OPJ_TRUE;
}


static opj_bool jp2_read_cmap(opj_jp2_t *jp2, opj_cio_t *cio,
	opj_jp2_box_t *box, opj_jp2_color_t *color)
{
	opj_jp2_cmap_comp_t *cmap;
	unsigned short i, nr_channels;

	OPJ_ARG_NOT_USED(box);
	OPJ_ARG_NOT_USED(jp2);

/* Need nr_channels: */
	if(color->jp2_pclr == NULL) return OPJ_FALSE;

/* Part 1, I.5.3.5: 'There shall be at most one Component Mapping box
 * inside a JP2 Header box' :
*/
	if(color->jp2_pclr->cmap) return OPJ_FALSE;

	nr_channels = color->jp2_pclr->nr_channels;
	cmap = (opj_jp2_cmap_comp_t*)
	 opj_malloc(nr_channels * sizeof(opj_jp2_cmap_comp_t));

	for(i = 0; i < nr_channels; ++i)
   {
	cmap[i].cmp = (unsigned short)cio_read(cio, 2);
	cmap[i].mtyp = cio_read(cio, 1);
	cmap[i].pcol = cio_read(cio, 1);

   }
	color->jp2_pclr->cmap = cmap;

	return OPJ_TRUE;

}/* jp2_read_cmap() */

/**
 * Reads the Component Mapping box.
 *
 * @param	p_cmap_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_cmap_header_size			pointer that will hold the size of the color header
 * @param	p_manager					the user event manager.
 *
 * @return	true if the cdef header is valid, false else.
*/
static opj_bool jp2_read_cmap_v2(	opj_jp2_v2_t * jp2,
							unsigned char * p_cmap_header_data,
							OPJ_UINT32 p_cmap_header_size,
							opj_event_mgr_t * p_manager
						  )
{
	opj_jp2_cmap_comp_t *cmap;
	OPJ_BYTE i, nr_channels;
	OPJ_UINT32 l_value;

	// preconditions
	assert(jp2 != 00);
	assert(p_cmap_header_data != 00);
	assert(p_manager != 00);

	/* Need nr_channels: */
	if(jp2->color.jp2_pclr == NULL) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Need to read a PCLR box before the CMAP box.\n");
		return OPJ_FALSE;
	}

	/* Part 1, I.5.3.5: 'There shall be at most one Component Mapping box
	 * inside a JP2 Header box' :
	*/
	if(jp2->color.jp2_pclr->cmap) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Only one CMAP box is allowed.\n");
		return OPJ_FALSE;
	}

	nr_channels = jp2->color.jp2_pclr->nr_channels;
	cmap = (opj_jp2_cmap_comp_t*) opj_malloc(nr_channels * sizeof(opj_jp2_cmap_comp_t));

	for(i = 0; i < nr_channels; ++i) {
		opj_read_bytes(p_cmap_header_data, &l_value, 2);			/* CMP^i */
		p_cmap_header_data +=2;
		cmap[i].cmp = (OPJ_UINT16) l_value;

		opj_read_bytes(p_cmap_header_data, &l_value, 1);			/* MTYP^i */
		++p_cmap_header_data;
		cmap[i].mtyp = (OPJ_BYTE) l_value;

		opj_read_bytes(p_cmap_header_data, &l_value, 1);			/* PCOL^i */
		++p_cmap_header_data;
		cmap[i].pcol = (OPJ_BYTE) l_value;
	}

	jp2->color.jp2_pclr->cmap = cmap;

	return OPJ_TRUE;
}

static void jp2_apply_cdef(opj_image_t *image, opj_jp2_color_t *color)
{
	opj_jp2_cdef_info_t *info;
	OPJ_INT32 color_space;
	OPJ_UINT16 i, n, cn, typ, asoc, acn;

	color_space = image->color_space;
	info = color->jp2_cdef->info;
	n = color->jp2_cdef->n;

	for(i = 0; i < n; ++i)
	{
		/* WATCH: acn = asoc - 1 ! */
		if((asoc = info[i].asoc) == 0) continue;

		cn = info[i].cn; typ = info[i].typ; acn = asoc - 1;

		if(cn != acn)
		{
			opj_image_comp_t saved;

			memcpy(&saved, &image->comps[cn], sizeof(opj_image_comp_t));
			memcpy(&image->comps[cn], &image->comps[acn], sizeof(opj_image_comp_t));
			memcpy(&image->comps[acn], &saved, sizeof(opj_image_comp_t));

			info[i].asoc = cn + 1;
			info[acn].asoc = info[acn].cn + 1;
		}
	}

	if(color->jp2_cdef->info) opj_free(color->jp2_cdef->info);

	opj_free(color->jp2_cdef); color->jp2_cdef = NULL;

}/* jp2_apply_cdef() */

static opj_bool jp2_read_cdef(opj_jp2_t *jp2, opj_cio_t *cio,
	opj_jp2_box_t *box, opj_jp2_color_t *color)
{
	opj_jp2_cdef_info_t *info;
	unsigned short i, n;

	OPJ_ARG_NOT_USED(box);
	OPJ_ARG_NOT_USED(jp2);

/* Part 1, I.5.3.6: 'The shall be at most one Channel Definition box
 * inside a JP2 Header box.' 
*/
	if(color->jp2_cdef) return OPJ_FALSE;

	if((n = (unsigned short)cio_read(cio, 2)) == 0) return OPJ_FALSE; /* szukw000: FIXME */

	info = (opj_jp2_cdef_info_t*)
	 opj_malloc(n * sizeof(opj_jp2_cdef_info_t));

	color->jp2_cdef = (opj_jp2_cdef_t*)opj_malloc(sizeof(opj_jp2_cdef_t));
	color->jp2_cdef->info = info;
	color->jp2_cdef->n = n;

	for(i = 0; i < n; ++i)
   {
	info[i].cn = (unsigned short)cio_read(cio, 2);
	info[i].typ = (unsigned short)cio_read(cio, 2);
	info[i].asoc = (unsigned short)cio_read(cio, 2);

   }
	return OPJ_TRUE;
}/* jp2_read_cdef() */

/**
 * Reads the Component Definition box.
 *
 * @param	p_cdef_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_cdef_header_size			pointer that will hold the size of the color header
 * @param	p_manager					the user event manager.
 *
 * @return	true if the cdef header is valid, false else.
*/
static opj_bool jp2_read_cdef_v2(	opj_jp2_v2_t * jp2,
							unsigned char * p_cdef_header_data,
							OPJ_UINT32 p_cdef_header_size,
							opj_event_mgr_t * p_manager
						  )
{
	opj_jp2_cdef_info_t *cdef_info;
	unsigned short i;
	OPJ_UINT32 l_value;

	// preconditions
	assert(jp2 != 00);
	assert(p_cdef_header_data != 00);
	assert(p_manager != 00);

	/* Part 1, I.5.3.6: 'The shall be at most one Channel Definition box
	 * inside a JP2 Header box.'*/
	if(jp2->color.jp2_cdef) return OPJ_FALSE;

	opj_read_bytes(p_cdef_header_data,&l_value ,2);			/* N */
	p_cdef_header_data+= 2;

	if ( (OPJ_UINT16)l_value == 0){ /* szukw000: FIXME */
		opj_event_msg_v2(p_manager, EVT_ERROR, "Number of channel description is equal to zero in CDEF box.\n");
		return OPJ_FALSE;
	}

	cdef_info = (opj_jp2_cdef_info_t*) opj_malloc(l_value * sizeof(opj_jp2_cdef_info_t));

	jp2->color.jp2_cdef = (opj_jp2_cdef_t*)opj_malloc(sizeof(opj_jp2_cdef_t));
	jp2->color.jp2_cdef->info = cdef_info;
	jp2->color.jp2_cdef->n = (OPJ_UINT16) l_value;

	for(i = 0; i < jp2->color.jp2_cdef->n; ++i) {
		opj_read_bytes(p_cdef_header_data, &l_value, 2);			/* Cn^i */
		p_cdef_header_data +=2;
		cdef_info[i].cn = (OPJ_UINT16) l_value;

		opj_read_bytes(p_cdef_header_data, &l_value, 2);			/* Typ^i */
		p_cdef_header_data +=2;
		cdef_info[i].typ = (OPJ_UINT16) l_value;

		opj_read_bytes(p_cdef_header_data, &l_value, 2);			/* Asoc^i */
		p_cdef_header_data +=2;
		cdef_info[i].asoc = (OPJ_UINT16) l_value;
   }

	return OPJ_TRUE;
}


static opj_bool jp2_read_colr(opj_jp2_t *jp2, opj_cio_t *cio,
	opj_jp2_box_t *box, opj_jp2_color_t *color) 
{
	int skip_len;
    opj_common_ptr cinfo;

/* Part 1, I.5.3.3 : 'A conforming JP2 reader shall ignore all Colour
 * Specification boxes after the first.' 
*/
	if(color->jp2_has_colr) return OPJ_FALSE;

	cinfo = jp2->cinfo;

	jp2->meth = cio_read(cio, 1);		/* METH */
	jp2->precedence = cio_read(cio, 1);	/* PRECEDENCE */
	jp2->approx = cio_read(cio, 1);		/* APPROX */

	if (jp2->meth == 1) 
   {
	jp2->enumcs = cio_read(cio, 4);	/* EnumCS */
   } 
	else
	if (jp2->meth == 2) 
   {
/* skip PROFILE */
	skip_len = box->init_pos + box->length - cio_tell(cio);
	if (skip_len < 0) 
  {
	opj_event_msg(cinfo, EVT_ERROR, "Error with COLR box size\n");
	return OPJ_FALSE;
  }
	if(skip_len > 0)
  {
	unsigned char *start;

	start = cio_getbp(cio);
	color->icc_profile_buf = (unsigned char*)opj_malloc(skip_len);
	color->icc_profile_len = skip_len;

	cio_skip(cio, box->init_pos + box->length - cio_tell(cio));

	memcpy(color->icc_profile_buf, start, skip_len);
  }
   }

	if (cio_tell(cio) - box->init_pos != box->length) 
   {
	opj_event_msg(cinfo, EVT_ERROR, "Error with COLR Box\n");
	return OPJ_FALSE;
   }
	color->jp2_has_colr = 1;

	return OPJ_TRUE;
}/* jp2_read_colr() */

/**
 * Reads the Colour Specification box.
 *
 * @param	p_colr_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_colr_header_size			pointer that will hold the size of the color header
 * @param	p_colr_header_max_size		maximum size of the header, any size bigger than this value should result the function to output false.
 * @param	p_manager					the user event manager.
 *
 * @return	true if the bpc header is valid, fale else.
*/
static opj_bool jp2_read_colr_v2(	opj_jp2_v2_t * jp2,
							unsigned char * p_colr_header_data,
							OPJ_UINT32 p_colr_header_size,
							opj_event_mgr_t * p_manager
						  )
{
	OPJ_UINT32 l_value;

	// preconditions
	assert(jp2 != 00);
	assert(p_colr_header_data != 00);
	assert(p_manager != 00);

	if (p_colr_header_size < 3) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Bad COLR header box (bad size)\n");
		return OPJ_FALSE;
	}

	/* Part 1, I.5.3.3 : 'A conforming JP2 reader shall ignore all Colour
	 * Specification boxes after the first.'
	*/
	if(jp2->color.jp2_has_colr) {
		opj_event_msg_v2(p_manager, EVT_INFO, "A conforming JP2 reader shall ignore all Colour Specification boxes after the first, so we ignore this one.\n");
		p_colr_header_data += p_colr_header_size;
		return OPJ_TRUE;
	}

	opj_read_bytes(p_colr_header_data,&jp2->meth ,1);			/* METH */
	++p_colr_header_data;

	opj_read_bytes(p_colr_header_data,&jp2->precedence ,1);		/* PRECEDENCE */
	++p_colr_header_data;

	opj_read_bytes(p_colr_header_data,&jp2->approx ,1);			/* APPROX */
	++p_colr_header_data;

	if (jp2->meth == 1) {
		if (p_colr_header_size != 7) {
			opj_event_msg_v2(p_manager, EVT_ERROR, "Bad BPCC header box (bad size)\n");
			return OPJ_FALSE;
		}

		opj_read_bytes(p_colr_header_data,&jp2->enumcs ,4);			/* EnumCS */
	}
	else if (jp2->meth == 2) {
		// ICC profile
		int it_icc_value = 0;
		int icc_len = p_colr_header_size - 3;

		jp2->color.icc_profile_len = icc_len;
		jp2->color.icc_profile_buf = (unsigned char*) opj_malloc(icc_len);

		memset(jp2->color.icc_profile_buf, 0, icc_len * sizeof(unsigned char));

		for (it_icc_value = 0; it_icc_value < icc_len; ++it_icc_value)
		{
			opj_read_bytes(p_colr_header_data,&l_value,1);		/* icc values */
			++p_colr_header_data;
			jp2->color.icc_profile_buf[it_icc_value] = (OPJ_BYTE) l_value;
		}

	}
	else // TODO MSD
		opj_event_msg_v2(p_manager, EVT_INFO, "COLR BOX meth value is not a regular value (%d), so we will skip the fields following the approx field.\n", jp2->meth);

	jp2->color.jp2_has_colr = 1;

	return OPJ_TRUE;
}

opj_bool jp2_read_jp2h(opj_jp2_t *jp2, opj_cio_t *cio, opj_jp2_color_t *color) 
{
	opj_jp2_box_t box;
	int jp2h_end;

	opj_common_ptr cinfo = jp2->cinfo;

	jp2_read_boxhdr(cinfo, cio, &box);
	do 
   {
	if (JP2_JP2H != box.type) 
  {
	if (box.type == JP2_JP2C) 
 {
	opj_event_msg(cinfo, EVT_ERROR, "Expected JP2H Marker\n");
	return OPJ_FALSE;
 }
	cio_skip(cio, box.length - 8);

	if(cio->bp >= cio->end) return OPJ_FALSE;

	jp2_read_boxhdr(cinfo, cio, &box);
  }
   } while(JP2_JP2H != box.type);

	if (!jp2_read_ihdr(jp2, cio))
		return OPJ_FALSE;
	jp2h_end = box.init_pos + box.length;

	if (jp2->bpc == 255) 
   {
	if (!jp2_read_bpcc(jp2, cio))
		return OPJ_FALSE;
   }
	jp2_read_boxhdr(cinfo, cio, &box);

	while(cio_tell(cio) < jp2h_end)
   {
	if(box.type == JP2_COLR)
  {
	if( !jp2_read_colr(jp2, cio, &box, color))
 {
    cio_seek(cio, box.init_pos + 8);
    cio_skip(cio, box.length - 8);
 }
    jp2_read_boxhdr(cinfo, cio, &box);
    continue;
  }
    if(box.type == JP2_CDEF)
  {
    if( !jp2_read_cdef(jp2, cio, &box, color))
 {
    cio_seek(cio, box.init_pos + 8);
    cio_skip(cio, box.length - 8);
 }
    jp2_read_boxhdr(cinfo, cio, &box);
    continue;
  }
    if(box.type == JP2_PCLR)
  {
    if( !jp2_read_pclr(jp2, cio, &box, color))
 {
    cio_seek(cio, box.init_pos + 8);
    cio_skip(cio, box.length - 8);
 }
    jp2_read_boxhdr(cinfo, cio, &box);
    continue;
  }
    if(box.type == JP2_CMAP)
  {
    if( !jp2_read_cmap(jp2, cio, &box, color))
 {
    cio_seek(cio, box.init_pos + 8);
    cio_skip(cio, box.length - 8);
 }
    jp2_read_boxhdr(cinfo, cio, &box);
    continue;
  }
	cio_seek(cio, box.init_pos + 8);
	cio_skip(cio, box.length - 8);
	jp2_read_boxhdr(cinfo, cio, &box);

   }/* while(cio_tell(cio) < box_end) */

	cio_seek(cio, jp2h_end);

/* Part 1, I.5.3.3 : 'must contain at least one' */
	return (color->jp2_has_colr == 1);

}/* jp2_read_jp2h() */

opj_image_t* opj_jp2_decode(opj_jp2_t *jp2, opj_cio_t *cio, 
	opj_codestream_info_t *cstr_info) 
{
	opj_common_ptr cinfo;
	opj_image_t *image = NULL;
	opj_jp2_color_t color;

	if(!jp2 || !cio) 
   {
	return NULL;
   }
	memset(&color, 0, sizeof(opj_jp2_color_t));
	cinfo = jp2->cinfo;

/* JP2 decoding */
	if(!jp2_read_struct(jp2, cio, &color)) 
   {
	free_color_data(&color);
	opj_event_msg(cinfo, EVT_ERROR, "Failed to decode jp2 structure\n");
	return NULL;
   }

/* J2K decoding */
	image = j2k_decode(jp2->j2k, cio, cstr_info);

	if(!image) 
   {
	free_color_data(&color);
	opj_event_msg(cinfo, EVT_ERROR, "Failed to decode J2K image\n");
	return NULL;
   }

/* Set Image Color Space */
	if (jp2->enumcs == 16)
		image->color_space = CLRSPC_SRGB;
	else if (jp2->enumcs == 17)
		image->color_space = CLRSPC_GRAY;
	else if (jp2->enumcs == 18)
		image->color_space = CLRSPC_SYCC;
	else
		image->color_space = CLRSPC_UNKNOWN;

	if(color.jp2_cdef)
   {
	jp2_apply_cdef(image, &color);
   }
	if(color.jp2_pclr)
   {
/* Part 1, I.5.3.4: Either both or none : */
	if( !color.jp2_pclr->cmap) 
	 jp2_free_pclr(&color);
	else
	 jp2_apply_pclr(image, &color);
   }
	if(color.icc_profile_buf)
   {
	image->icc_profile_buf = color.icc_profile_buf;
	color.icc_profile_buf = NULL;
	image->icc_profile_len = color.icc_profile_len;
   }
	return image;

}/* opj_jp2_decode() */

opj_bool jp2_decode_v2(	opj_jp2_v2_t *jp2,
						struct opj_stream_private *cio,
						opj_image_t* p_image,
						struct opj_event_mgr * p_manager)
{
	if (!p_image)
		return OPJ_FALSE;

	/* J2K decoding */
	if( ! j2k_decode_v2(jp2->j2k, cio, p_image, p_manager) ) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to decode the codestream in the JP2 file\n");
		return OPJ_FALSE;
	}

	/* Set Image Color Space */
	if (jp2->enumcs == 16)
		p_image->color_space = CLRSPC_SRGB;
	else if (jp2->enumcs == 17)
		p_image->color_space = CLRSPC_GRAY;
	else if (jp2->enumcs == 18)
		p_image->color_space = CLRSPC_SYCC;
	else
		p_image->color_space = CLRSPC_UNKNOWN;

	/* Apply the color space if needed */
	if(jp2->color.jp2_cdef) {
		jp2_apply_cdef(p_image, &(jp2->color));
	}

	if(jp2->color.jp2_pclr) {
		/* Part 1, I.5.3.4: Either both or none : */
		if( !jp2->color.jp2_pclr->cmap)
			jp2_free_pclr(&(jp2->color));
		else
			jp2_apply_pclr(p_image, &(jp2->color));
	}

	if(jp2->color.icc_profile_buf) {
		p_image->icc_profile_buf = jp2->color.icc_profile_buf;
		p_image->icc_profile_len = jp2->color.icc_profile_len;
		jp2->color.icc_profile_buf = NULL;
	}

	return OPJ_TRUE;
}


void jp2_write_jp2h(opj_jp2_t *jp2, opj_cio_t *cio) {
	opj_jp2_box_t box;

	box.init_pos = cio_tell(cio);
	cio_skip(cio, 4);
	cio_write(cio, JP2_JP2H, 4);	/* JP2H */

	jp2_write_ihdr(jp2, cio);

	if (jp2->bpc == 255) {
		jp2_write_bpcc(jp2, cio);
	}
	jp2_write_colr(jp2, cio);

	box.length = cio_tell(cio) - box.init_pos;
	cio_seek(cio, box.init_pos);
	cio_write(cio, box.length, 4);	/* L */
	cio_seek(cio, box.init_pos + box.length);
}

static void jp2_write_ftyp(opj_jp2_t *jp2, opj_cio_t *cio) {
	unsigned int i;
	opj_jp2_box_t box;

	box.init_pos = cio_tell(cio);
	cio_skip(cio, 4);
	cio_write(cio, JP2_FTYP, 4);		/* FTYP */

	cio_write(cio, jp2->brand, 4);		/* BR */
	cio_write(cio, jp2->minversion, 4);	/* MinV */

	for (i = 0; i < jp2->numcl; i++) {
		cio_write(cio, jp2->cl[i], 4);	/* CL */
	}

	box.length = cio_tell(cio) - box.init_pos;
	cio_seek(cio, box.init_pos);
	cio_write(cio, box.length, 4);	/* L */
	cio_seek(cio, box.init_pos + box.length);
}

static opj_bool jp2_read_ftyp(opj_jp2_t *jp2, opj_cio_t *cio) {
	int i;
	opj_jp2_box_t box;

	opj_common_ptr cinfo = jp2->cinfo;

	jp2_read_boxhdr(cinfo, cio, &box);

	if (JP2_FTYP != box.type) {
		opj_event_msg(cinfo, EVT_ERROR, "Expected FTYP Marker\n");
		return OPJ_FALSE;
	}

	jp2->brand = cio_read(cio, 4);		/* BR */
	jp2->minversion = cio_read(cio, 4);	/* MinV */
	jp2->numcl = (box.length - 16) / 4;
	jp2->cl = (unsigned int *) opj_malloc(jp2->numcl * sizeof(unsigned int));

	for (i = 0; i < (int)jp2->numcl; i++) {
		jp2->cl[i] = cio_read(cio, 4);	/* CLi */
	}

	if (cio_tell(cio) - box.init_pos != box.length) {
		opj_event_msg(cinfo, EVT_ERROR, "Error with FTYP Box\n");
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}

static int jp2_write_jp2c(opj_jp2_t *jp2, opj_cio_t *cio, opj_image_t *image, opj_codestream_info_t *cstr_info) {
	unsigned int j2k_codestream_offset, j2k_codestream_length;
	opj_jp2_box_t box;

	opj_j2k_t *j2k = jp2->j2k;

	box.init_pos = cio_tell(cio);
	cio_skip(cio, 4);
	cio_write(cio, JP2_JP2C, 4);	/* JP2C */

	/* J2K encoding */
	j2k_codestream_offset = cio_tell(cio);
	if(!j2k_encode(j2k, cio, image, cstr_info)) {
		opj_event_msg(j2k->cinfo, EVT_ERROR, "Failed to encode image\n");
		return 0;
	}
	j2k_codestream_length = cio_tell(cio) - j2k_codestream_offset;

	jp2->j2k_codestream_offset = j2k_codestream_offset;
	jp2->j2k_codestream_length = j2k_codestream_length;

	box.length = 8 + jp2->j2k_codestream_length;
	cio_seek(cio, box.init_pos);
	cio_write(cio, box.length, 4);	/* L */
	cio_seek(cio, box.init_pos + box.length);

	return box.length;
}

static opj_bool jp2_read_jp2c(opj_jp2_t *jp2, opj_cio_t *cio, unsigned int *j2k_codestream_length, unsigned int *j2k_codestream_offset) {
	opj_jp2_box_t box;

	opj_common_ptr cinfo = jp2->cinfo;

	jp2_read_boxhdr(cinfo, cio, &box);
	do {
		if(JP2_JP2C != box.type) {
			cio_skip(cio, box.length - 8);
			jp2_read_boxhdr(cinfo, cio, &box);
		}
	} while(JP2_JP2C != box.type);

	*j2k_codestream_offset = cio_tell(cio);
	*j2k_codestream_length = box.length - 8;

	return OPJ_TRUE;
}

static void jp2_write_jp(opj_cio_t *cio) {
	opj_jp2_box_t box;

	box.init_pos = cio_tell(cio);
	cio_skip(cio, 4);
	cio_write(cio, JP2_JP, 4);		/* JP2 signature */
	cio_write(cio, 0x0d0a870a, 4);

	box.length = cio_tell(cio) - box.init_pos;
	cio_seek(cio, box.init_pos);
	cio_write(cio, box.length, 4);	/* L */
	cio_seek(cio, box.init_pos + box.length);
}

static opj_bool jp2_read_jp(opj_jp2_t *jp2, opj_cio_t *cio) {
	opj_jp2_box_t box;

	opj_common_ptr cinfo = jp2->cinfo;

	jp2_read_boxhdr(cinfo, cio, &box);
	if (JP2_JP != box.type) {
		opj_event_msg(cinfo, EVT_ERROR, "Expected JP Marker\n");
		return OPJ_FALSE;
	}
	if (0x0d0a870a != cio_read(cio, 4)) {
		opj_event_msg(cinfo, EVT_ERROR, "Error with JP Marker\n");
		return OPJ_FALSE;
	}
	if (cio_tell(cio) - box.init_pos != box.length) {
		opj_event_msg(cinfo, EVT_ERROR, "Error with JP Box size\n");
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}


static opj_bool jp2_read_struct(opj_jp2_t *jp2, opj_cio_t *cio,
	opj_jp2_color_t *color) {
	if (!jp2_read_jp(jp2, cio))
		return OPJ_FALSE;
	if (!jp2_read_ftyp(jp2, cio))
		return OPJ_FALSE;
	if (!jp2_read_jp2h(jp2, cio, color))
		return OPJ_FALSE;
	if (!jp2_read_jp2c(jp2, cio, &jp2->j2k_codestream_length, &jp2->j2k_codestream_offset))
		return OPJ_FALSE;
	
	return OPJ_TRUE;
}


static int write_fidx( int offset_jp2c, int length_jp2c, int offset_idx, int length_idx, opj_cio_t *cio)
{  
  int len, lenp;
  
  lenp = cio_tell( cio);
  cio_skip( cio, 4);              /* L [at the end] */
  cio_write( cio, JPIP_FIDX, 4);  /* IPTR           */
  
  write_prxy( offset_jp2c, length_jp2c, offset_idx, length_idx, cio);

  len = cio_tell( cio)-lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);        /* L              */
  cio_seek( cio, lenp+len);  

  return len;
}

static void write_prxy( int offset_jp2c, int length_jp2c, int offset_idx, int length_idx, opj_cio_t *cio)
{
  int len, lenp;

  lenp = cio_tell( cio);
  cio_skip( cio, 4);              /* L [at the end] */
  cio_write( cio, JPIP_PRXY, 4);  /* IPTR           */
  
  cio_write( cio, offset_jp2c, 8); /* OOFF           */
  cio_write( cio, length_jp2c, 4); /* OBH part 1     */
  cio_write( cio, JP2_JP2C, 4);        /* OBH part 2     */
  
  cio_write( cio, 1,1);           /* NI             */

  cio_write( cio, offset_idx, 8);  /* IOFF           */
  cio_write( cio, length_idx, 4);  /* IBH part 1     */
  cio_write( cio, JPIP_CIDX, 4);   /* IBH part 2     */

  len = cio_tell( cio)-lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);        /* L              */
  cio_seek( cio, lenp+len);
}

static void write_iptr( int offset, int length, opj_cio_t *cio)
{
  int len, lenp;
  
  lenp = cio_tell( cio);
  cio_skip( cio, 4);              /* L [at the end] */
  cio_write( cio, JPIP_IPTR, 4);  /* IPTR           */
  
  cio_write( cio, offset, 8);
  cio_write( cio, length, 8);

  len = cio_tell( cio)-lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);        /* L             */
  cio_seek( cio, lenp+len);
}


/* ----------------------------------------------------------------------- */
/* JP2 decoder interface                                             */
/* ----------------------------------------------------------------------- */

opj_jp2_t* jp2_create_decompress(opj_common_ptr cinfo) {
	opj_jp2_t *jp2 = (opj_jp2_t*) opj_calloc(1, sizeof(opj_jp2_t));
	if(jp2) {
		jp2->cinfo = cinfo;
		/* create the J2K codec */
		jp2->j2k = j2k_create_decompress(cinfo);
		if(jp2->j2k == NULL) {
			jp2_destroy_decompress(jp2);
			return NULL;
		}
	}
	return jp2;
}

void jp2_destroy_decompress(opj_jp2_t *jp2) {
	if(jp2) {
		/* destroy the J2K codec */
		j2k_destroy_decompress(jp2->j2k);

		if(jp2->comps) {
			opj_free(jp2->comps);
		}
		if(jp2->cl) {
			opj_free(jp2->cl);
		}
		opj_free(jp2);
	}
}

void jp2_setup_decoder(opj_jp2_t *jp2, opj_dparameters_t *parameters) {
	/* setup the J2K codec */
	j2k_setup_decoder(jp2->j2k, parameters);
	/* further JP2 initializations go here */
}

void jp2_setup_decoder_v2(opj_jp2_v2_t *jp2, opj_dparameters_t *parameters)
{
	/* setup the J2K codec */
	j2k_setup_decoder_v2(jp2->j2k, parameters);

	/* further JP2 initializations go here */
	jp2->color.jp2_has_colr = 0;
}


/* ----------------------------------------------------------------------- */
/* JP2 encoder interface                                             */
/* ----------------------------------------------------------------------- */

opj_jp2_t* jp2_create_compress(opj_common_ptr cinfo) {
	opj_jp2_t *jp2 = (opj_jp2_t*)opj_malloc(sizeof(opj_jp2_t));
	if(jp2) {
		jp2->cinfo = cinfo;
		/* create the J2K codec */
		jp2->j2k = j2k_create_compress(cinfo);
		if(jp2->j2k == NULL) {
			jp2_destroy_compress(jp2);
			return NULL;
		}
	}
	return jp2;
}

void jp2_destroy_compress(opj_jp2_t *jp2) {
	if(jp2) {
		/* destroy the J2K codec */
		j2k_destroy_compress(jp2->j2k);

		if(jp2->comps) {
			opj_free(jp2->comps);
		}
		if(jp2->cl) {
			opj_free(jp2->cl);
		}
		opj_free(jp2);
	}
}

void jp2_setup_encoder(opj_jp2_t *jp2, opj_cparameters_t *parameters, opj_image_t *image) {
	int i;
	int depth_0, sign;

	if(!jp2 || !parameters || !image)
		return;

	/* setup the J2K codec */
	/* ------------------- */

	/* Check if number of components respects standard */
	if (image->numcomps < 1 || image->numcomps > 16384) {
		opj_event_msg(jp2->cinfo, EVT_ERROR, "Invalid number of components specified while setting up JP2 encoder\n");
		return;
	}

	j2k_setup_encoder(jp2->j2k, parameters, image);

	/* setup the JP2 codec */
	/* ------------------- */
	
	/* Profile box */

	jp2->brand = JP2_JP2;	/* BR */
	jp2->minversion = 0;	/* MinV */
	jp2->numcl = 1;
	jp2->cl = (unsigned int*) opj_malloc(jp2->numcl * sizeof(unsigned int));
	jp2->cl[0] = JP2_JP2;	/* CL0 : JP2 */

	/* Image Header box */

	jp2->numcomps = image->numcomps;	/* NC */
	jp2->comps = (opj_jp2_comps_t*) opj_malloc(jp2->numcomps * sizeof(opj_jp2_comps_t));
	jp2->h = image->y1 - image->y0;		/* HEIGHT */
	jp2->w = image->x1 - image->x0;		/* WIDTH */
	/* BPC */
	depth_0 = image->comps[0].prec - 1;
	sign = image->comps[0].sgnd;
	jp2->bpc = depth_0 + (sign << 7);
	for (i = 1; i < image->numcomps; i++) {
		int depth = image->comps[i].prec - 1;
		sign = image->comps[i].sgnd;
		if (depth_0 != depth)
			jp2->bpc = 255;
	}
	jp2->C = 7;			/* C : Always 7 */
	jp2->UnkC = 0;		/* UnkC, colorspace specified in colr box */
	jp2->IPR = 0;		/* IPR, no intellectual property */
	
	/* BitsPerComponent box */

	for (i = 0; i < image->numcomps; i++) {
		jp2->comps[i].bpcc = image->comps[i].prec - 1 + (image->comps[i].sgnd << 7);
	}
	jp2->meth = 1;
	if (image->color_space == 1)
		jp2->enumcs = 16;	/* sRGB as defined by IEC 61966-2.1 */
	else if (image->color_space == 2)
		jp2->enumcs = 17;	/* greyscale */
	else if (image->color_space == 3)
		jp2->enumcs = 18;	/* YUV */
	jp2->precedence = 0;	/* PRECEDENCE */
	jp2->approx = 0;		/* APPROX */
	
	jp2->jpip_on = parameters->jpip_on;
}

opj_bool opj_jp2_encode(opj_jp2_t *jp2, opj_cio_t *cio, opj_image_t *image, opj_codestream_info_t *cstr_info) {

	int pos_iptr, pos_cidx, pos_jp2c, len_jp2c, len_cidx, end_pos, pos_fidx, len_fidx;

	/* JP2 encoding */

	/* JPEG 2000 Signature box */
	jp2_write_jp(cio);
	/* File Type box */
	jp2_write_ftyp(jp2, cio);
	/* JP2 Header box */
	jp2_write_jp2h(jp2, cio);

	if( jp2->jpip_on){
	  pos_iptr = cio_tell( cio);
	  cio_skip( cio, 24); /* IPTR further ! */
	  
	  pos_jp2c = cio_tell( cio);
	}

	/* J2K encoding */
	if(!(len_jp2c = jp2_write_jp2c( jp2, cio, image, cstr_info))){
	    opj_event_msg(jp2->cinfo, EVT_ERROR, "Failed to encode image\n");
	    return OPJ_FALSE;
	}

	if( jp2->jpip_on){
	  pos_cidx = cio_tell( cio);
	  
	  len_cidx = write_cidx( pos_jp2c+8, cio, image, *cstr_info, len_jp2c-8);
	  
	  pos_fidx = cio_tell( cio);
	  len_fidx = write_fidx( pos_jp2c, len_jp2c, pos_cidx, len_cidx, cio);
	  
	  end_pos = cio_tell( cio);
	  
	  cio_seek( cio, pos_iptr);
	  write_iptr( pos_fidx, len_fidx, cio);
	  	  cio_seek( cio, end_pos);
	}

	return OPJ_TRUE;
}

/**
 * Ends the decompression procedures and possibiliy add data to be read after the
 * codestream.
 */
opj_bool jp2_end_decompress(opj_jp2_v2_t *jp2, opj_stream_private_t *cio, opj_event_mgr_t * p_manager)
{
	// preconditions
	assert(jp2 != 00);
	assert(cio != 00);
	assert(p_manager != 00);

	/* customization of the end encoding */
	jp2_setup_end_header_reading(jp2);

	/* write header */
	if (! jp2_exec (jp2,jp2->m_procedure_list,cio,p_manager)) {
		return OPJ_FALSE;
	}

	return j2k_end_decompress(jp2->j2k, cio, p_manager);
}

/**
 * Sets up the procedures to do on reading header after the codestream.
 * Developpers wanting to extend the library can add their own writting procedures.
 */
void jp2_setup_end_header_reading (opj_jp2_v2_t *jp2)
{
	// preconditions
	assert(jp2 != 00);
	opj_procedure_list_add_procedure(jp2->m_procedure_list,(void*)jp2_read_header_procedure );
	/* DEVELOPER CORNER, add your custom procedures */
}


/**
 * Reads a jpeg2000 file header structure.
 *
 * @param cio the stream to read data from.
 * @param jp2 the jpeg2000 file header structure.
 * @param p_manager the user event manager.
 *
 * @return true if the box is valid.
 */
opj_bool jp2_read_header_procedure(
					 opj_jp2_v2_t *jp2,
					 opj_stream_private_t *cio,
					 opj_event_mgr_t * p_manager)
{
	opj_jp2_box_t box;
	OPJ_UINT32 l_nb_bytes_read;
	const opj_jp2_header_handler_t * l_current_handler;
	OPJ_UINT32 l_last_data_size = BOX_SIZE;
	OPJ_UINT32 l_current_data_size;
	unsigned char * l_current_data = 00;

	// preconditions
	assert(cio != 00);
	assert(jp2 != 00);
	assert(p_manager != 00);

	l_current_data = (unsigned char*)opj_malloc(l_last_data_size);

	if (l_current_data == 00) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Not enough memory to handle jpeg2000 file header\n");
		return OPJ_FALSE;
	}
	memset(l_current_data, 0 , l_last_data_size);

	while (jp2_read_boxhdr_v2(&box,&l_nb_bytes_read,cio,p_manager)) {
		// is it the codestream box ?
		if (box.type == JP2_JP2C) {
			if (jp2->jp2_state & JP2_STATE_HEADER) {
				jp2->jp2_state |= JP2_STATE_CODESTREAM;
                opj_free(l_current_data);
				return OPJ_TRUE;
			}
			else {
				opj_event_msg_v2(p_manager, EVT_ERROR, "bad placed jpeg codestream\n");
				opj_free(l_current_data);
				return OPJ_FALSE;
			}
		}
		else if	(box.length == 0) {
			opj_event_msg_v2(p_manager, EVT_ERROR, "Cannot handle box of undefined sizes\n");
			opj_free(l_current_data);
			return OPJ_FALSE;
		}

		l_current_handler = jp2_find_handler(box.type);
		l_current_data_size = box.length - l_nb_bytes_read;

		if (l_current_handler != 00) {
			if (l_current_data_size > l_last_data_size) {
				l_current_data = (unsigned char*)opj_realloc(l_current_data,l_current_data_size);
				l_last_data_size = l_current_data_size;
			}

			l_nb_bytes_read = opj_stream_read_data(cio,l_current_data,l_current_data_size,p_manager);
			if (l_nb_bytes_read != l_current_data_size) {
				opj_event_msg_v2(p_manager, EVT_ERROR, "Problem with reading JPEG2000 box, stream error\n");
				return OPJ_FALSE;
			}

			if (! l_current_handler->handler(jp2,l_current_data,l_current_data_size,p_manager)) {
				opj_free(l_current_data);
				return OPJ_FALSE;
			}
		}
		else {
			jp2->jp2_state |= JP2_STATE_UNKNOWN;
			if (opj_stream_skip(cio,l_current_data_size,p_manager) != l_current_data_size) {
				opj_event_msg_v2(p_manager, EVT_ERROR, "Problem with skipping JPEG2000 box, stream error\n");
				opj_free(l_current_data);
				return OPJ_FALSE;
			}
		}
	}

	opj_free(l_current_data);

	return OPJ_TRUE;
}

/**
 * Excutes the given procedures on the given codec.
 *
 * @param	p_procedure_list	the list of procedures to execute
 * @param	jp2					the jpeg2000 file codec to execute the procedures on.
 * @param	cio					the stream to execute the procedures on.
 * @param	p_manager			the user manager.
 *
 * @return	true				if all the procedures were successfully executed.
 */
opj_bool jp2_exec (
					opj_jp2_v2_t * jp2,
					opj_procedure_list_t * p_procedure_list,
					opj_stream_private_t *cio,
					opj_event_mgr_t * p_manager
				  )
{
	opj_bool (** l_procedure) (opj_jp2_v2_t * jp2, opj_stream_private_t *, opj_event_mgr_t *) = 00;
	opj_bool l_result = OPJ_TRUE;
	OPJ_UINT32 l_nb_proc, i;

	// preconditions
	assert(p_procedure_list != 00);
	assert(jp2 != 00);
	assert(cio != 00);
	assert(p_manager != 00);

	l_nb_proc = opj_procedure_list_get_nb_procedures(p_procedure_list);
	l_procedure = (opj_bool (**) (opj_jp2_v2_t * jp2, opj_stream_private_t *, opj_event_mgr_t *)) opj_procedure_list_get_first_procedure(p_procedure_list);

	for	(i=0;i<l_nb_proc;++i) {
		l_result = l_result && (*l_procedure) (jp2,cio,p_manager);
		++l_procedure;
	}

	// and clear the procedure list at the end.
	opj_procedure_list_clear(p_procedure_list);
	return l_result;
}


/**
 * Finds the execution function related to the given box id.
 *
 * @param	p_id	the id of the handler to fetch.
 *
 * @return	the given handler or 00 if it could not be found.
 */
const opj_jp2_header_handler_t * jp2_find_handler (int p_id)
{
	OPJ_UINT32 i, l_handler_size = sizeof(jp2_header) / sizeof(opj_jp2_header_handler_t);

	for (i=0;i<l_handler_size;++i) {
		if (jp2_header[i].id == p_id) {
			return &jp2_header[i];
		}
	}
	return NULL;
}

/**
 * Finds the image execution function related to the given box id.
 *
 * @param	p_id	the id of the handler to fetch.
 *
 * @return	the given handler or 00 if it could not be found.
 */
static const opj_jp2_header_handler_t * jp2_img_find_handler (
												int p_id
												)
{
	OPJ_UINT32 i, l_handler_size = sizeof(jp2_img_header) / sizeof(opj_jp2_header_handler_t);
	for (i=0;i<l_handler_size;++i)
	{
		if (jp2_img_header[i].id == p_id) {
			return &jp2_img_header[i];
		}
	}

	return NULL;
}


/**
 * Reads a jpeg2000 file signature box.
 *
 * @param	p_header_data	the data contained in the signature box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the signature box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the file signature box is valid.
 */
opj_bool jp2_read_jp_v2(
					opj_jp2_v2_t *jp2,
					unsigned char * p_header_data,
					unsigned int p_header_size,
					opj_event_mgr_t * p_manager
				 )
{
	unsigned int l_magic_number;

	// preconditions
	assert(p_header_data != 00);
	assert(jp2 != 00);
	assert(p_manager != 00);

	if (jp2->jp2_state != JP2_STATE_NONE) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "The signature box must be the first box in the file.\n");
		return OPJ_FALSE;
	}

	/* assure length of data is correct (4 -> magic number) */
	if (p_header_size != 4) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Error with JP signature Box size\n");
		return OPJ_FALSE;
	}

	// rearrange data
	opj_read_bytes(p_header_data,&l_magic_number,4);
	if (l_magic_number != 0x0d0a870a ) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Error with JP Signature : bad magic number\n");
		return OPJ_FALSE;
	}

	jp2->jp2_state |= JP2_STATE_SIGNATURE;

	return OPJ_TRUE;
}


/**
 * Reads a a FTYP box - File type box
 *
 * @param	p_header_data	the data contained in the FTYP box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the FTYP box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the FTYP box is valid.
 */
opj_bool jp2_read_ftyp_v2(
							opj_jp2_v2_t *jp2,
							unsigned char * p_header_data,
							unsigned int p_header_size,
							opj_event_mgr_t * p_manager
						)
{
	OPJ_UINT32 i, l_remaining_bytes;

	// preconditions
	assert(p_header_data != 00);
	assert(jp2 != 00);
	assert(p_manager != 00);

	if (jp2->jp2_state != JP2_STATE_SIGNATURE) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "The ftyp box must be the second box in the file.\n");
		return OPJ_FALSE;
	}

	/* assure length of data is correct */
	if (p_header_size < 8) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Error with FTYP signature Box size\n");
		return OPJ_FALSE;
	}

	opj_read_bytes(p_header_data,&jp2->brand,4);		/* BR */
	p_header_data += 4;

	opj_read_bytes(p_header_data,&jp2->minversion,4);		/* MinV */
	p_header_data += 4;

	l_remaining_bytes = p_header_size - 8;

	/* the number of remaining bytes should be a multiple of 4 */
	if ((l_remaining_bytes & 0x3) != 0) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Error with FTYP signature Box size\n");
		return OPJ_FALSE;
	}

	/* div by 4 */
	jp2->numcl = l_remaining_bytes >> 2;
	if (jp2->numcl) {
		jp2->cl = (unsigned int *) opj_malloc(jp2->numcl * sizeof(unsigned int));
		if (jp2->cl == 00) {
			opj_event_msg_v2(p_manager, EVT_ERROR, "Not enough memory with FTYP Box\n");
			return OPJ_FALSE;
		}
		memset(jp2->cl,0,jp2->numcl * sizeof(unsigned int));
	}

	for (i = 0; i < jp2->numcl; ++i)
	{
		opj_read_bytes(p_header_data,&jp2->cl[i],4);		/* CLi */
		p_header_data += 4;
	}

	jp2->jp2_state |= JP2_STATE_FILE_TYPE;

	return OPJ_TRUE;
}


/**
 * Reads the Jpeg2000 file Header box - JP2 Header box (warning, this is a super box).
 *
 * @param	p_header_data	the data contained in the file header box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the file header box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the JP2 Header box was successfully reconized.
*/
opj_bool jp2_read_jp2h_v2(
						opj_jp2_v2_t *jp2,
						unsigned char * p_header_data,
						unsigned int p_header_size,
						opj_event_mgr_t * p_manager
					)
{
	OPJ_UINT32 l_box_size=0, l_current_data_size = 0;
	opj_jp2_box_t box;
	const opj_jp2_header_handler_t * l_current_handler;

	// preconditions
	assert(p_header_data != 00);
	assert(jp2 != 00);
	assert(p_manager != 00);

	/* make sure the box is well placed */
	if ((jp2->jp2_state & JP2_STATE_FILE_TYPE) != JP2_STATE_FILE_TYPE ) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "The  box must be the first box in the file.\n");
		return OPJ_FALSE;
	}

	jp2->jp2_img_state = JP2_IMG_STATE_NONE;

	/* iterate while remaining data */
	while (p_header_size > 0) {

		if (! jp2_read_boxhdr_char(&box,p_header_data,&l_box_size,p_header_size, p_manager)) {
			opj_event_msg_v2(p_manager, EVT_ERROR, "Stream error while reading JP2 Header box\n");
			return OPJ_FALSE;
		}

		if (box.length > p_header_size) {
			opj_event_msg_v2(p_manager, EVT_ERROR, "Stream error while reading JP2 Header box: box length is inconsistent.\n");
			return OPJ_FALSE;
		}

		l_current_handler = jp2_img_find_handler(box.type);
		l_current_data_size = box.length - l_box_size;
		p_header_data += l_box_size;

		if (l_current_handler != 00) {
			if (! l_current_handler->handler(jp2,p_header_data,l_current_data_size,p_manager)) {
				return OPJ_FALSE;
			}
		}
		else {
			jp2->jp2_img_state |= JP2_IMG_STATE_UNKNOWN;
		}

		p_header_data += l_current_data_size;
		p_header_size -= box.length;
	}

	jp2->jp2_state |= JP2_STATE_HEADER;

	return OPJ_TRUE;
}

/**
 * Reads a box header. The box is the way data is packed inside a jpeg2000 file structure. Data is read from a character string
 *
 * @param	p_data					the character string to read data from.
 * @param	box						the box structure to fill.
 * @param	p_number_bytes_read		pointer to an int that will store the number of bytes read from the stream (shoul usually be 2).
 * @param	p_box_max_size			the maximum number of bytes in the box.
 *
 * @return	true if the box is reconized, false otherwise
*/
static opj_bool jp2_read_boxhdr_char(
								opj_jp2_box_t *box,
								OPJ_BYTE * p_data,
								OPJ_UINT32 * p_number_bytes_read,
								OPJ_UINT32 p_box_max_size,
								opj_event_mgr_t * p_manager
							)
{
	OPJ_UINT32 l_value;

	// preconditions
	assert(p_data != 00);
	assert(box != 00);
	assert(p_number_bytes_read != 00);
	assert(p_manager != 00);

	if (p_box_max_size < 8) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Cannot handle box of less than 8 bytes\n");
		return OPJ_FALSE;
	}

	/* process read data */
	opj_read_bytes(p_data, &l_value, 4);
	p_data += 4;
	box->length = (OPJ_INT32)(l_value);

	opj_read_bytes(p_data, &l_value, 4);
	p_data += 4;
	box->type = (OPJ_INT32)(l_value);

	*p_number_bytes_read = 8;

	// do we have a "special very large box ?"
	// read then the XLBox
	if (box->length == 1) {
		unsigned int l_xl_part_size;

		if (p_box_max_size < 16) {
			opj_event_msg_v2(p_manager, EVT_ERROR, "Cannot handle XL box of less than 16 bytes\n");
			return OPJ_FALSE;
		}

		opj_read_bytes(p_data,&l_xl_part_size, 4);
		p_data += 4;
		*p_number_bytes_read += 4;

		if (l_xl_part_size != 0) {
			opj_event_msg_v2(p_manager, EVT_ERROR, "Cannot handle box sizes higher than 2^32\n");
			return OPJ_FALSE;
		}

		opj_read_bytes(p_data, &l_value, 4);
		*p_number_bytes_read += 4;
		box->length = (OPJ_INT32)(l_value);

		if (box->length == 0) {
			opj_event_msg_v2(p_manager, EVT_ERROR, "Cannot handle box of undefined sizes\n");
			return OPJ_FALSE;
		}
	}
	else if (box->length == 0) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Cannot handle box of undefined sizes\n");
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}


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
							opj_image_t** p_image,
							struct opj_event_mgr * p_manager
							)
{
	/* preconditions */
	assert(jp2 != 00);
	assert(p_stream != 00);
	assert(p_manager != 00);

	/* customization of the validation */
	jp2_setup_decoding_validation (jp2);

	/* customization of the encoding */
	jp2_setup_header_reading(jp2);

	/* validation of the parameters codec */
	if (! jp2_exec(jp2,jp2->m_validation_list,p_stream,p_manager)) {
		return OPJ_FALSE;
	}

	/* read header */
	if (! jp2_exec (jp2,jp2->m_procedure_list,p_stream,p_manager)) {
		return OPJ_FALSE;
	}

	return j2k_read_header(	p_stream,
							jp2->j2k,
							p_image,
							p_manager);
}

/**
 * Sets up the validation ,i.e. adds the procedures to lauch to make sure the codec parameters
 * are valid. Developpers wanting to extend the library can add their own validation procedures.
 */
void jp2_setup_decoding_validation (opj_jp2_v2_t *jp2)
{
	// preconditions
	assert(jp2 != 00);
	/* DEVELOPER CORNER, add your custom validation procedure */
}

/**
 * Sets up the procedures to do on reading header.
 * Developpers wanting to extend the library can add their own writting procedures.
 */
void jp2_setup_header_reading (opj_jp2_v2_t *jp2)
{
	// preconditions
	assert(jp2 != 00);

	opj_procedure_list_add_procedure(jp2->m_procedure_list,(void*)jp2_read_header_procedure );
	/* DEVELOPER CORNER, add your custom procedures */
}


/**
 * Reads a tile header.
 * @param	p_j2k		the jpeg2000 codec.
 * @param	p_stream			the stream to write data to.
 * @param	p_manager	the user event manager.
 */
opj_bool jp2_read_tile_header(	opj_jp2_v2_t * p_jp2,
					 	 		OPJ_UINT32 * p_tile_index,
					 	 		OPJ_UINT32 * p_data_size,
					 	 		OPJ_INT32 * p_tile_x0, OPJ_INT32 * p_tile_y0,
					 	 		OPJ_INT32 * p_tile_x1, OPJ_INT32 * p_tile_y1,
					 	 		OPJ_UINT32 * p_nb_comps,
					 	 		opj_bool * p_go_on,
					 	 		opj_stream_private_t *p_stream,
					 	 		opj_event_mgr_t * p_manager )
{
	return j2k_read_tile_header(p_jp2->j2k,
								p_tile_index,
								p_data_size,
								p_tile_x0, p_tile_y0,
								p_tile_x1, p_tile_y1,
								p_nb_comps,
								p_go_on,
								p_stream,
								p_manager);
}

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
					opj_stream_private_t *p_stream,
					opj_event_mgr_t * p_manager
					)
{
	return j2k_decode_tile (p_jp2->j2k,p_tile_index,p_data,p_data_size,p_stream,p_manager);
}

/**
 * Destroys a jpeg2000 file decompressor.
 *
 * @param	jp2		a jpeg2000 file decompressor.
 */
void jp2_destroy(opj_jp2_v2_t *jp2)
{
	if (jp2) {
		/* destroy the J2K codec */
		j2k_destroy(jp2->j2k);
		jp2->j2k = 00;

		if (jp2->comps) {
			opj_free(jp2->comps);
			jp2->comps = 00;
		}

		if (jp2->cl) {
			opj_free(jp2->cl);
			jp2->cl = 00;
		}

		if (jp2->color.icc_profile_buf) {
			opj_free(jp2->color.icc_profile_buf);
			jp2->color.icc_profile_buf = 00;
		}

		if (jp2->color.jp2_cdef) {
			if (jp2->color.jp2_cdef->info) {
				opj_free(jp2->color.jp2_cdef->info);
				jp2->color.jp2_cdef->info = NULL;
			}

			opj_free(jp2->color.jp2_cdef);
			jp2->color.jp2_cdef = 00;
		}

		if (jp2->color.jp2_pclr) {
			if (jp2->color.jp2_pclr->cmap) {
				opj_free(jp2->color.jp2_pclr->cmap);
				jp2->color.jp2_pclr->cmap = NULL;
			}
			if (jp2->color.jp2_pclr->channel_sign) {
				opj_free(jp2->color.jp2_pclr->channel_sign);
				jp2->color.jp2_pclr->channel_sign = NULL;
			}
			if (jp2->color.jp2_pclr->channel_size) {
				opj_free(jp2->color.jp2_pclr->channel_size);
				jp2->color.jp2_pclr->channel_size = NULL;
			}
			if (jp2->color.jp2_pclr->entries) {
				opj_free(jp2->color.jp2_pclr->entries);
				jp2->color.jp2_pclr->entries = NULL;
			}

			opj_free(jp2->color.jp2_pclr);
			jp2->color.jp2_pclr = 00;
		}

		if (jp2->m_validation_list) {
			opj_procedure_list_destroy(jp2->m_validation_list);
			jp2->m_validation_list = 00;
		}

		if (jp2->m_procedure_list) {
			opj_procedure_list_destroy(jp2->m_procedure_list);
			jp2->m_procedure_list = 00;
		}

		opj_free(jp2);
	}
}

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
								OPJ_INT32 p_start_x, OPJ_INT32 p_start_y,
								OPJ_INT32 p_end_x, OPJ_INT32 p_end_y,
								struct opj_event_mgr * p_manager )
{
	return j2k_set_decode_area(p_jp2->j2k, p_start_x, p_start_y, p_end_x, p_end_y, p_manager);
}

/* ----------------------------------------------------------------------- */
/* JP2 encoder interface                                             */
/* ----------------------------------------------------------------------- */

opj_jp2_v2_t* jp2_create(opj_bool p_is_decoder)
{
	opj_jp2_v2_t *jp2 = (opj_jp2_v2_t*)opj_malloc(sizeof(opj_jp2_v2_t));
	if (jp2) {
		memset(jp2,0,sizeof(opj_jp2_t));

		/* create the J2K codec */
		if (! p_is_decoder) {
			jp2->j2k = j2k_create_compress_v2();
		}
		else {
			jp2->j2k = j2k_create_decompress_v2();
		}

		if (jp2->j2k == 00) {
			jp2_destroy(jp2);
			return 00;
		}

		/* Color structure */
		jp2->color.icc_profile_buf = NULL;
		jp2->color.icc_profile_len = 0;
		jp2->color.jp2_cdef = NULL;
		jp2->color.jp2_pclr = NULL;
		jp2->color.jp2_has_colr = 0;

		// validation list creation
		jp2->m_validation_list = opj_procedure_list_create();
		if (! jp2->m_validation_list) {
			jp2_destroy(jp2);
			return 00;
		}

		// execution list creation
		jp2->m_procedure_list = opj_procedure_list_create();
		if (! jp2->m_procedure_list) {
			jp2_destroy(jp2);
			return 00;
		}
	}

	return jp2;
}

void jp2_dump(opj_jp2_v2_t* p_jp2, OPJ_INT32 flag, FILE* out_stream)
{
	/* preconditions */
	assert(p_jp2 != 00);

	j2k_dump(p_jp2->j2k,
					flag,
					out_stream);
}

opj_codestream_index_t* jp2_get_cstr_index(opj_jp2_v2_t* p_jp2)
{
	return j2k_get_cstr_index(p_jp2->j2k);
}

opj_codestream_info_v2_t* jp2_get_cstr_info(opj_jp2_v2_t* p_jp2)
{
	return j2k_get_cstr_info(p_jp2->j2k);
}

