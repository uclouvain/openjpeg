/*
 * Copyright (c) 2007, Digital Signal Processing Laboratory, Università degli studi di Perugia (UPG), Italy
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
/////////////////////////////////////////////////////////////////////////////
// Name:        imagmj2.cpp
// Purpose:     wxImage Motion JPEG 2000 file format handler
// Author:      Giuseppe Baruffa - based on imagjpeg.cpp, Vaclav Slavik
// RCS-ID:      $Id: imagmj2.cpp,v 0.00 2007/02/18 23:59:00 MW Exp $
// Copyright:   (c) Giuseppe Baruffa
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_IMAGE && wxUSE_LIBOPENJPEG

#include "imagmj2.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/intl.h"
    #include "wx/bitmap.h"
    #include "wx/module.h"
#endif


#include "libopenjpeg/openjpeg.h"


#include "wx/filefn.h"
#include "wx/wfstream.h"

// ----------------------------------------------------------------------------
// types
// ----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// wxMJ2Handler
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxMJ2Handler,wxImageHandler)

#if wxUSE_STREAMS

//------------- JPEG 2000 Data Source Manager

#define J2K_CFMT 0
#define JP2_CFMT 1
#define JPT_CFMT 2
#define MJ2_CFMT 3
#define PXM_DFMT 0
#define PGX_DFMT 1
#define BMP_DFMT 2
#define YUV_DFMT 3

#define MAX_MESSAGE_LEN 200

/* sample error callback expecting a FILE* client object */
void mj2_error_callback(const char *msg, void *client_data) {
	int message_len = strlen(msg) - 1;
	if (msg[message_len] != '\n')
		message_len = MAX_MESSAGE_LEN;
#ifndef __WXGTK__ 
		wxMutexGuiEnter();
#endif /* __WXGTK__ */
	wxLogMessage(wxT("[ERROR] %.*s"), message_len, msg);
#ifndef __WXGTK__ 
    wxMutexGuiLeave();
#endif /* __WXGTK__ */
}

/* sample warning callback expecting a FILE* client object */
void mj2_warning_callback(const char *msg, void *client_data) {
	int message_len = strlen(msg) - 1;
	if (msg[message_len] != '\n')
		message_len = MAX_MESSAGE_LEN;
#ifndef __WXGTK__ 
		wxMutexGuiEnter();
#endif /* __WXGTK__ */
	wxLogMessage(wxT("[WARNING] %.*s"), message_len, msg);
#ifndef __WXGTK__ 
    wxMutexGuiLeave();
#endif /* __WXGTK__ */
}

/* sample debug callback expecting no client object */
void mj2_info_callback(const char *msg, void *client_data) {
	int message_len = strlen(msg) - 1;
	if (msg[message_len] != '\n')
		message_len = MAX_MESSAGE_LEN;
#ifndef __WXGTK__ 
		wxMutexGuiEnter();
#endif /* __WXGTK__ */
	wxLogMessage(wxT("[INFO] %.*s"), message_len, msg);
#ifndef __WXGTK__ 
    wxMutexGuiLeave();
#endif /* __WXGTK__ */
}

/* macro functions */
/* From little endian to big endian, 2 and 4 bytes */
#define	BYTE_SWAP2(X)	((X & 0x00FF) << 8) | ((X & 0xFF00) >> 8)
#define	BYTE_SWAP4(X)	((X & 0x000000FF) << 24) | ((X & 0x0000FF00) << 8) | ((X & 0x00FF0000) >> 8) | ((X & 0xFF000000) >> 24)

#ifdef __WXGTK__
#define	BYTE_SWAP8(X)	((X & 0x00000000000000FFULL) << 56) | ((X & 0x000000000000FF00ULL) << 40) | \
                        ((X & 0x0000000000FF0000ULL) << 24) | ((X & 0x00000000FF000000ULL) << 8) | \
						((X & 0x000000FF00000000ULL) >> 8)  | ((X & 0x0000FF0000000000ULL) >> 24) | \
						((X & 0x00FF000000000000ULL) >> 40) | ((X & 0xFF00000000000000ULL) >> 56)
#else
#define	BYTE_SWAP8(X)	((X & 0x00000000000000FF) << 56) | ((X & 0x000000000000FF00) << 40) | \
                        ((X & 0x0000000000FF0000) << 24) | ((X & 0x00000000FF000000) << 8) | \
						((X & 0x000000FF00000000) >> 8)  | ((X & 0x0000FF0000000000) >> 24) | \
						((X & 0x00FF000000000000) >> 40) | ((X & 0xFF00000000000000) >> 56)
#endif

/* From codestream to int values */
#define STREAM_TO_UINT32(C, P)	(((unsigned long int) (C)[(P) + 0] << 24) + \
								((unsigned long int) (C)[(P) + 1] << 16) + \
								((unsigned long int) (C)[(P) + 2] << 8) + \
								((unsigned long int) (C)[(P) + 3] << 0))

#define STREAM_TO_UINT16(C, P)	(((unsigned long int) (C)[(P) + 0] << 8) + \
								((unsigned long int) (C)[(P) + 1] << 0))

/* defines */
#define SHORT_DESCR_LEN        32
#define LONG_DESCR_LEN         256

/* enumeration for file formats */
#define J2FILENUM              4
typedef enum {

        JP2_FILE,
        J2K_FILE,
		MJ2_FILE,
		UNK_FILE

} my_j2filetype;

/* enumeration for the box types */
#define J2BOXNUM                23
typedef enum {

			FILE_BOX,
			JP_BOX,
			FTYP_BOX,
			JP2H_BOX,
			IHDR_BOX,
			COLR_BOX,
			JP2C_BOX,
			JP2I_BOX,
			XML_BOX,
			UUID_BOX,
			UINF_BOX,
			MOOV_BOX,
			MVHD_BOX,
			TRAK_BOX,
			TKHD_BOX,
			MDIA_BOX,
			MINF_BOX,
			STBL_BOX,
			STSD_BOX,
			MJP2_BOX,
			MDAT_BOX,
			ANY_BOX,
			UNK_BOX

} my_j2boxtype;

/* jp2 family box signatures */
#define FILE_SIGN           ""
#define JP_SIGN             "jP\040\040"
#define FTYP_SIGN           "ftyp"
#define JP2H_SIGN           "jp2h"
#define IHDR_SIGN           "ihdr"
#define COLR_SIGN           "colr"
#define JP2C_SIGN           "jp2c"
#define JP2I_SIGN           "jp2i"
#define XML_SIGN            "xml\040"
#define UUID_SIGN           "uuid"
#define UINF_SIGN           "uinf"
#define MOOV_SIGN           "moov"
#define MVHD_SIGN           "mvhd"
#define TRAK_SIGN           "trak"
#define TKHD_SIGN           "tkhd"
#define MDIA_SIGN           "mdia"
#define MINF_SIGN           "minf"
#define VMHD_SIGN           "vmhd"
#define STBL_SIGN           "stbl"
#define STSD_SIGN           "stsd"
#define MJP2_SIGN           "mjp2"
#define MDAT_SIGN           "mdat"
#define ANY_SIGN 			""
#define UNK_SIGN            ""

/* the box structure itself */
struct my_boxdef {

        char                  value[5];                 /* hexadecimal value/string*/
		char                  name[SHORT_DESCR_LEN];    /* short description       */
		char                  descr[LONG_DESCR_LEN];    /* long  description       */
		int                   sbox;                     /* is it a superbox?       */
		int                   req[J2FILENUM];           /* mandatory box           */
		my_j2boxtype             ins;                      /* contained in box...     */

};

/* the possible boxes */
struct my_boxdef j2box[] =
{
/* sign */	{FILE_SIGN,
/* short */	"placeholder for nothing",
/* long */	"Nothing to say",
/* sbox */	0,
/* req */	{1, 1, 1},
/* ins */	FILE_BOX},

/* sign */	{JP_SIGN,
/* short */	"JPEG 2000 Signature box",
/* long */	"This box uniquely identifies the file as being part of the JPEG 2000 family of files",
/* sbox */	0,
/* req */	{1, 1, 1},
/* ins */	FILE_BOX},

/* sign */	{FTYP_SIGN,
/* short */	"File Type box",
/* long */	"This box specifies file type, version and compatibility information, including specifying if this file "
			"is a conforming JP2 file or if it can be read by a conforming JP2 reader",
/* sbox */	0,
/* req */	{1, 1, 1},
/* ins */	FILE_BOX},

/* sign */	{JP2H_SIGN,
/* short */	"JP2 Header box",
/* long */	"This box contains a series of boxes that contain header-type information about the file",
/* sbox */	1,
/* req */	{1, 1, 1},
/* ins */	FILE_BOX},

/* sign */	{IHDR_SIGN,
/* short */	"Image Header box",
/* long */	"This box specifies the size of the image and other related fields",
/* sbox */	0,
/* req */	{1, 1, 1},
/* ins */	JP2H_BOX},

/* sign */	{COLR_SIGN,
/* short */	"Colour Specification box",
/* long */	"This box specifies the colourspace of the image",
/* sbox */	0,
/* req */	{1, 1, 1},
/* ins */	JP2H_BOX},

/* sign */	{JP2C_SIGN,
/* short */	"Contiguous Codestream box",
/* long */	"This box contains the codestream as defined by Annex A",
/* sbox */	0,
/* req */	{1, 1, 1},
/* ins */	FILE_BOX},

/* sign */	{JP2I_SIGN,
/* short */	"Intellectual Property box",
/* long */	"This box contains intellectual property information about the image",
/* sbox */	0,
/* req */	{0, 0, 0},
/* ins */	FILE_BOX},

/* sign */	{XML_SIGN,
/* short */	"XML box",
/* long */	"This box provides a tool by which vendors can add XML formatted information to a JP2 file",
/* sbox */	0,
/* req */	{0, 0, 0},
/* ins */	FILE_BOX},

/* sign */	{UUID_SIGN,
/* short */	"UUID box",
/* long */	"This box provides a tool by which vendors can add additional information to a file "
			"without risking conflict with other vendors",
/* sbox */	0,
/* req */	{0, 0, 0},
/* ins */	FILE_BOX},

/* sign */	{UINF_SIGN,
/* short */	"UUID Info box",
/* long */	"This box provides a tool by which a vendor may provide access to additional information associated with a UUID",
/* sbox */	0,
/* req */	{0, 0, 0},
/* ins */	FILE_BOX},

/* sign */	{MOOV_SIGN,
/* short */	"Movie box",
/* long */	"This box contains the media data. In video tracks, this box would contain JPEG2000 video frames",
/* sbox */	1,
/* req */	{1, 1, 1},
/* ins */	FILE_BOX},

/* sign */	{MVHD_SIGN,
/* short */	"Movie Header box",
/* long */	"This box defines overall information which is media-independent, and relevant to the entire presentation "
			"considered as a whole",
/* sbox */	0,
/* req */	{1, 1, 1},
/* ins */	MOOV_BOX},

/* sign */	{TRAK_SIGN,
/* short */	"Track box",
/* long */	"This is a container box for a single track of a presentation. A presentation may consist of one or more tracks",
/* sbox */	1,
/* req */	{1, 1, 1},
/* ins */	MOOV_BOX},

/* sign */	{TKHD_SIGN,
/* short */	"Track Header box",
/* long */	"This box specifies the characteristics of a single track. Exactly one Track Header Box is contained in a track",
/* sbox */	0,
/* req */	{1, 1, 1},
/* ins */	TRAK_BOX},

/* sign */	{MDIA_SIGN,
/* short */	"Media box",
/* long */	"The media declaration container contains all the objects which declare information about the media data "
			"within a track",
/* sbox */	1,
/* req */	{1, 1, 1},
/* ins */	TRAK_BOX},

/* sign */	{MINF_SIGN,
/* short */	"Media Information box",
/* long */	"This box contains all the objects which declare characteristic information of the media in the track",
/* sbox */	1,
/* req */	{1, 1, 1},
/* ins */	MDIA_BOX},

/* sign */	{STBL_SIGN,
/* short */	"Sample Table box",
/* long */	"The sample table contains all the time and data indexing of the media samples in a track",
/* sbox */	1,
/* req */	{1, 1, 1},
/* ins */	MINF_BOX},

/* sign */	{STSD_SIGN,
/* short */	"Sample Description box",
/* long */	"The sample description table gives detailed information about the coding type used, and any initialization "
			"information needed for that coding",
/* sbox */	0,
/* req */	{1, 1, 1},
/* ins */	MINF_BOX},

/* sign */	{MJP2_SIGN,
/* short */	"MJP2 Sample Description box",
/* long */	"The MJP2 sample description table gives detailed information about the coding type used, and any initialization "
			"information needed for that coding",
/* sbox */	0,
/* req */	{1, 1, 1},
/* ins */	MINF_BOX},

/* sign */	{MDAT_SIGN,
/* short */	"Media Data box",
/* long */	"The meta-data for a presentation is stored in the single Movie Box which occurs at the top-level of a file",
/* sbox */	1,
/* req */	{1, 1, 1},
/* ins */	FILE_BOX},

/* sign */	{ANY_SIGN,
/* short */	"Any box",
/* long */	"All the existing boxes",
/* sbox */	0,
/* req */	{0, 0, 0},
/* ins */	FILE_BOX},

/* sign */	{UNK_SIGN,
/* short */	"Unknown Type box",
/* long */	"The signature is not recognised to be that of an existing box",
/* sbox */	0,
/* req */	{0, 0, 0},
/* ins */	ANY_BOX}

};

/* declaration */
int
my_box_handler_function(my_j2boxtype boxtype, wxInputStream& stream, unsigned long int filepoint, unsigned long int filelimit, int level,
					 char *scansign, unsigned long int *scanpoint);

#ifdef __WXMSW__
typedef unsigned __int64 int8byte;
#endif // __WXMSW__

#ifdef __WXGTK__
typedef unsigned long long int8byte;
#endif // __WXGTK__

/* internal mini-search for a box signature */
int
my_jpeg2000parse(wxInputStream& stream, unsigned long int filepoint, unsigned long int filelimit, int level,
			  char *scansign, unsigned long int *scanpoint)
{
	unsigned long int       LBox = 0x00000000;
	//int                     LBox_read;
	char                    TBox[5] = "\0\0\0\0";
	//int                     TBox_read;
	int8byte                 XLBox = 0x0000000000000000;
	//int                     XLBox_read;
	unsigned long int       box_length = 0;
	int                     last_box = 0, box_num = 0;
	int                     box_type = ANY_BOX;
	unsigned char           /*onebyte[1], twobytes[2],*/ fourbytes[4];
	int                     box_number = 0;

	/* cycle all over the file */
	box_num = 0;
	last_box = 0;
	while (!last_box) {

		/* do not exceed file limit */
		if (filepoint >= filelimit)
			return (0);

		/* seek on file */
		if (stream.SeekI(filepoint, wxFromStart) == wxInvalidOffset)
			return (-1);

		/* read the mandatory LBox, 4 bytes */
		if (!stream.Read(fourbytes, 4)) {
			(wxT("Problem reading LBox from the file (file ended?)"));
			return -1;
		};
		LBox = STREAM_TO_UINT32(fourbytes, 0);

		/* read the mandatory TBox, 4 bytes */
		if (!stream.Read(TBox, 4)) {
			wxLogError(wxT("Problem reading TBox from the file (file ended?)"));
			return -1;
		};

		/* look if scansign is got */
		if ((scansign != NULL) && (memcmp(TBox, scansign, 4) == 0)) {
			/* hack/exploit */
			// stop as soon as you find the level-th codebox
			if (box_number == level) {
				memcpy(scansign, "    ", 4);
				*scanpoint = filepoint;
				return (0);
			} else
				box_number++;

		};


		/* determine the box type */
		for (box_type = JP_BOX; box_type < UNK_BOX; box_type++)
			if (memcmp(TBox, j2box[box_type].value, 4) == 0)
				break;	

		/* read the optional XLBox, 8 bytes */
		if (LBox == 1) {

			if (!stream.Read(&XLBox, 8)) {
				wxLogError(wxT("Problem reading XLBox from the file (file ended?)"));
				return -1;
			};
			box_length = (unsigned long int) BYTE_SWAP8(XLBox);

		} else if (LBox == 0x00000000) {

			/* last box in file */
			last_box = 1; 
			box_length = filelimit - filepoint;

		} else

			box_length = LBox;


		/* go deep in the box */
		my_box_handler_function((my_j2boxtype) box_type, stream, (LBox == 1) ? (filepoint + 16) : (filepoint + 8), filepoint + box_length, level,
			scansign, scanpoint);

		/* if it's a superbox go inside it */
		if (j2box[box_type].sbox)
			my_jpeg2000parse(stream, (LBox == 1) ? (filepoint + 16) : (filepoint + 8), filepoint + box_length,
				level, scansign, scanpoint);

		/* increment box number and filepoint*/
		box_num++;
		filepoint += box_length;

	};

	/* all good */
	return (0);
}

// search first contiguos codestream box in an mj2 file
unsigned long int
searchjp2c(wxInputStream& stream, unsigned long int fsize, int number)
{
	char scansign[] = "jp2c";
	unsigned long int scanpoint = 0L;

	wxLogMessage(wxT("MJ2: searching jp2c box... "));

	/* do the parsing */
	if (my_jpeg2000parse(stream, 0, fsize, number, scansign, &scanpoint) < 0)		
		wxLogMessage(wxT("MJ2: Unrecoverable error during file parsing: stopping"));

	if (strcmp(scansign, "    "))
		wxLogMessage(wxT("MJ2: not found"));
	else {

		wxLogMessage(wxString::Format(wxT("MJ2: found at byte %d"), scanpoint));

	};


	return (scanpoint);
}

// search the jp2h box in the file
unsigned long int
searchjpegheaderbox(wxInputStream& stream, unsigned long int fsize)
{
	char scansign[] = "jp2h";
	unsigned long int scanpoint = 0L;

	wxLogMessage(wxT("MJ2: searching jp2h box... "));

	/* do the parsing */
	if (my_jpeg2000parse(stream, 0, fsize, 0, scansign, &scanpoint) < 0)		
		wxLogMessage(wxT("Unrecoverable error during file parsing: stopping"));

	if (strcmp(scansign, "    "))
		wxLogMessage(wxT("MJ2: not found"));
	else
		wxLogMessage(wxString::Format(wxT("MJ2: found at byte %d"), scanpoint));

	return (scanpoint);
}

/* handling functions */
#define ITEM_PER_ROW	10

/* Box handler function */
int
my_box_handler_function(my_j2boxtype boxtype, wxInputStream& stream, unsigned long int filepoint, unsigned long int filelimit, int level,
					 char *scansign, unsigned long int *scanpoint)
{
	switch (boxtype) {

			/* Sample Description box */
	case (STSD_BOX):
		my_jpeg2000parse(stream, filepoint + 8, filelimit, level, scansign, scanpoint);
		break;

			/* MJP2 Sample Description box */
	case (MJP2_BOX):
		my_jpeg2000parse(stream, filepoint + 78, filelimit, level, scansign, scanpoint);
		break;
		
	/* not yet implemented */
	default:
		break;

	};

	return (0);
}

// the jP and ftyp parts of the header
#define my_jPheadSIZE	32
unsigned char my_jPhead[my_jPheadSIZE] = {
		0x00, 0x00, 0x00, 0x0C,  'j',  'P',  ' ',  ' ',
		0x0D, 0x0A, 0x87, 0x0A, 0x00, 0x00, 0x00, 0x14,
		 'f',  't',  'y',  'p',  'j',  'p',  '2',  ' ',
		0x00, 0x00, 0x00, 0x00,  'j',  'p',  '2',  ' '			
};

/////////////////////////////////////////////////
/////////////////////////////////////////////////

// load the mj2 file format
bool wxMJ2Handler::LoadFile(wxImage *image, wxInputStream& stream, bool verbose, int index)
{
	opj_dparameters_t parameters;	/* decompression parameters */
	opj_event_mgr_t event_mgr;		/* event manager */
	opj_image_t *opjimage = NULL;
	unsigned char *src = NULL;
    unsigned char *ptr;
	int file_length, jp2c_point, jp2h_point;
	unsigned long int jp2hboxlen, jp2cboxlen;
	opj_codestream_info_t cstr_info;  /* Codestream information structure */

	// destroy the image
    image->Destroy();

	/* handle to a decompressor */
	opj_dinfo_t* dinfo = NULL;	
	opj_cio_t *cio = NULL;

	/* configure the event callbacks (not required) */
	memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
	event_mgr.error_handler = mj2_error_callback;
	event_mgr.warning_handler = mj2_warning_callback;
	event_mgr.info_handler = mj2_info_callback;

	/* set decoding parameters to default values */
	opj_set_default_decoder_parameters(&parameters);

	/* prepare parameters */
	strncpy(parameters.infile, "", sizeof(parameters.infile)-1);
	strncpy(parameters.outfile, "", sizeof(parameters.outfile)-1);
	parameters.decod_format = JP2_CFMT;
	parameters.cod_format = BMP_DFMT;
	if (m_reducefactor)
		parameters.cp_reduce = m_reducefactor;
	if (m_qualitylayers)
		parameters.cp_layer = m_qualitylayers;
	/*if (n_components)
		parameters. = n_components;*/

	/* JPWL only */
#ifdef USE_JPWL
	parameters.jpwl_exp_comps = m_expcomps;
	parameters.jpwl_max_tiles = m_maxtiles;
	parameters.jpwl_correct = m_enablejpwl;
#endif /* USE_JPWL */

	/* get a decoder handle */
	dinfo = opj_create_decompress(CODEC_JP2);

	/* find length of the stream */
	stream.SeekI(0, wxFromEnd);
	file_length = (int) stream.TellI();

	/* search for the first codestream box and the movie header box  */
	jp2c_point = searchjp2c(stream, file_length, m_framenum);
	jp2h_point = searchjpegheaderbox(stream, file_length);

	// read the jp2h box and store it
	stream.SeekI(jp2h_point, wxFromStart);
	stream.Read(&jp2hboxlen, sizeof(unsigned long int));
	jp2hboxlen = BYTE_SWAP4(jp2hboxlen);

	// read the jp2c box and store it
	stream.SeekI(jp2c_point, wxFromStart);
	stream.Read(&jp2cboxlen, sizeof(unsigned long int));
	jp2cboxlen = BYTE_SWAP4(jp2cboxlen);

	// malloc memory source
    src = (unsigned char *) malloc(my_jPheadSIZE + jp2hboxlen + jp2cboxlen);

	// copy the jP and ftyp
	memcpy(src, my_jPhead, my_jPheadSIZE);

	// copy the jp2h
	stream.SeekI(jp2h_point, wxFromStart);
	stream.Read(&src[my_jPheadSIZE], jp2hboxlen);

	// copy the jp2c
	stream.SeekI(jp2c_point, wxFromStart);
	stream.Read(&src[my_jPheadSIZE + jp2hboxlen], jp2cboxlen);

	/* catch events using our callbacks and give a local context */
	opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);

	/* setup the decoder decoding parameters using user parameters */
	opj_setup_decoder(dinfo, &parameters);

	/* open a byte stream */
	cio = opj_cio_open((opj_common_ptr)dinfo, src, my_jPheadSIZE + jp2hboxlen + jp2cboxlen);

	/* decode the stream and fill the image structure */
	opjimage = opj_decode_with_info(dinfo, cio, &cstr_info);
	if (!opjimage) {
		wxMutexGuiEnter();
		wxLogError(wxT("MJ2: failed to decode image!"));
		wxMutexGuiLeave();
		opj_destroy_decompress(dinfo);
		opj_cio_close(cio);
		free(src);
		return false;
	}

	/* close the byte stream */
	opj_cio_close(cio);

	/* common rendering method */
#include "imagjpeg2000.cpp"

    wxMutexGuiEnter();
    wxLogMessage(wxT("MJ2: image loaded."));
    wxMutexGuiLeave();

	/* close openjpeg structs */
	opj_destroy_decompress(dinfo);
	opj_image_destroy(opjimage);
	free(src);

	if (!image->Ok())
		return false;
	else
		return true;

}

// save the mj2 file format
bool wxMJ2Handler::SaveFile( wxImage *image, wxOutputStream& stream, bool verbose )
{
    wxLogError(wxT("MJ2: Couldn't save movie -> not implemented."));
    return false;
}

#ifdef __VISUALC__
    #pragma warning(default:4611)
#endif /* VC++ */

// recognize the Motion JPEG 2000 starting box
bool wxMJ2Handler::DoCanRead( wxInputStream& stream )
{
    unsigned char hdr[24];

    if ( !stream.Read(hdr, WXSIZEOF(hdr)) )
        return false;

    return (hdr[0] == 0x00 &&
			hdr[1] == 0x00 &&
			hdr[2] == 0x00 &&
			hdr[3] == 0x0C &&
			hdr[4] == 0x6A &&
			hdr[5] == 0x50 &&
			hdr[6] == 0x20 &&
			hdr[7] == 0x20 &&
			hdr[20] == 0x6D &&
			hdr[21] == 0x6A &&
			hdr[22] == 0x70 &&
			hdr[23] == 0x32);
}

#endif   // wxUSE_STREAMS

#endif   // wxUSE_LIBOPENJPEG
