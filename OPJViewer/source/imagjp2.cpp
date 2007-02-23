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
// Name:        imagjp2.cpp
// Purpose:     wxImage JPEG 2000 file format handler
// Author:      Giuseppe Baruffa - based on imagjpeg.cpp, Vaclav Slavik
// RCS-ID:      $Id: imagjp2.cpp,v 0.00 2007/02/08 23:59:00 MW Exp $
// Copyright:   (c) Giuseppe Baruffa
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_IMAGE && wxUSE_LIBOPENJPEG

#include "imagjp2.h"

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
// wxJP2Handler
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxJP2Handler,wxImageHandler)

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
void jp2_error_callback(const char *msg, void *client_data) {
	char m_msg[MAX_MESSAGE_LEN];
	int message_len = strlen(msg) - 1;
	if (msg[message_len] != '\n')
		message_len = MAX_MESSAGE_LEN;
	sprintf(m_msg, "[ERROR] %.*s", message_len, msg);
    wxMutexGuiEnter();
	wxLogMessage(m_msg);
    wxMutexGuiLeave();
}
/* sample warning callback expecting a FILE* client object */
void jp2_warning_callback(const char *msg, void *client_data) {
	char m_msg[MAX_MESSAGE_LEN];
	int message_len = strlen(msg) - 1;
	if (msg[message_len] != '\n')
		message_len = MAX_MESSAGE_LEN;
	sprintf(m_msg, "[WARNING] %.*s", message_len, msg);
    wxMutexGuiEnter();
	wxLogMessage(m_msg);
    wxMutexGuiLeave();
}
/* sample debug callback expecting no client object */
void jp2_info_callback(const char *msg, void *client_data) {
	char m_msg[MAX_MESSAGE_LEN];
	int message_len = strlen(msg) - 1;
	if (msg[message_len] != '\n')
		message_len = MAX_MESSAGE_LEN;
	sprintf(m_msg, "[INFO] %.*s", message_len, msg);
    wxMutexGuiEnter();
	wxLogMessage(m_msg);
    wxMutexGuiLeave();
}

// load the jp2 file format
bool wxJP2Handler::LoadFile(wxImage *image, wxInputStream& stream, bool verbose, int index)
{
	opj_dparameters_t parameters;	/* decompression parameters */
	opj_event_mgr_t event_mgr;		/* event manager */
	opj_image_t *opjimage = NULL;
	unsigned char *src = NULL;
    unsigned char *ptr;
	int file_length;

	// destroy the image
    image->Destroy();

	/* handle to a decompressor */
	opj_dinfo_t* dinfo = NULL;	
	opj_cio_t *cio = NULL;

	/* configure the event callbacks (not required) */
	memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
	event_mgr.error_handler = jp2_error_callback;
	event_mgr.warning_handler = jp2_warning_callback;
	event_mgr.info_handler = jp2_info_callback;

	/* set decoding parameters to default values */
	opj_set_default_decoder_parameters(&parameters);

	/* prepare parameters */
	strncpy(parameters.infile, "", sizeof(parameters.infile)-1);
	strncpy(parameters.outfile, "", sizeof(parameters.outfile)-1);
	parameters.decod_format = JP2_CFMT;
	parameters.cod_format = BMP_DFMT;

	/* get a decoder handle */
	dinfo = opj_create_decompress(CODEC_JP2);

	/* find length of the stream */
	stream.SeekI(0, wxFromEnd);
	file_length = (int) stream.TellI();

	/* get data */
	stream.SeekI(0, wxFromStart);
    src = (unsigned char *) malloc(file_length);
	stream.Read(src, file_length);

	/* catch events using our callbacks and give a local context */
	opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);

	/* setup the decoder decoding parameters using user parameters */
	opj_setup_decoder(dinfo, &parameters);

	/* open a byte stream */
	cio = opj_cio_open((opj_common_ptr)dinfo, src, file_length);

	/* decode the stream and fill the image structure */
	opjimage = opj_decode(dinfo, cio);
	if (!opjimage) {
		wxMutexGuiEnter();
		wxLogError("JP2: failed to decode image!");
		wxMutexGuiLeave();
		opj_destroy_decompress(dinfo);
		opj_cio_close(cio);
		free(src);
		return false;
	}

	/* close the byte stream */
	opj_cio_close(cio);

	// check image size
	if ((opjimage->numcomps != 1) && (opjimage->numcomps != 3)) {
		wxMutexGuiEnter();
		wxLogError("JP2: weird number of components");
		wxMutexGuiLeave();
		opj_destroy_decompress(dinfo);
		free(src);
		return false;
	}


	// prepare image size
    image->Create(opjimage->comps[0].w, opjimage->comps[0].h, true );

	// access image raw data
    image->SetMask( false );
    ptr = image->GetData();

	// RGB color picture
	if (opjimage->numcomps == 3) {
		int row, col;
		int *r = opjimage->comps[0].data;
		int *g = opjimage->comps[1].data;
		int *b = opjimage->comps[2].data;
		for (row = 0; row < opjimage->comps[0].h; row++) {
			for (col = 0; col < opjimage->comps[0].w; col++) {
				
				*(ptr++) = *(r++);
				*(ptr++) = *(g++);
				*(ptr++) = *(b++);

			}
		}
	}

	// B/W picture
	if (opjimage->numcomps == 1) {
		int row, col;
		int *y = opjimage->comps[0].data;
		for (row = 0; row < opjimage->comps[0].h; row++) {
			for (col = 0; col < opjimage->comps[0].w; col++) {
				
				*(ptr++) = *(y);
				*(ptr++) = *(y);
				*(ptr++) = *(y++);

			}
		}
	}

    wxMutexGuiEnter();
    wxLogMessage(wxT("JP2: image loaded."));
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

// save the jp2 file format
bool wxJP2Handler::SaveFile( wxImage *image, wxOutputStream& stream, bool verbose )
{
    wxLogError(wxT("JP2: Couldn't save image -> not implemented."));
    return false;
}

#ifdef __VISUALC__
    #pragma warning(default:4611)
#endif /* VC++ */

// recognize the JPEG 2000 starting box
bool wxJP2Handler::DoCanRead( wxInputStream& stream )
{
    unsigned char hdr[23];

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
			hdr[20] == 0x6A &&
			hdr[21] == 0x70 &&
			hdr[22] == 0x32);
}

#endif   // wxUSE_STREAMS

#endif   // wxUSE_LIBOPENJPEG
