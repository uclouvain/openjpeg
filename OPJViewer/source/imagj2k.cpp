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
// Name:        imagj2k.cpp
// Purpose:     wxImage JPEG 2000 codestream handler
// Author:      Giuseppe Baruffa - based on imagjpeg.cpp, Vaclav Slavik
// RCS-ID:      $Id: imagj2k.cpp,v 0.00 2007/02/08 23:59:00 MW Exp $
// Copyright:   (c) Giuseppe Baruffa
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_IMAGE && wxUSE_LIBOPENJPEG

#include "imagj2k.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/intl.h"
    #include "wx/bitmap.h"
    #include "wx/module.h"
#endif

#include "wx/filefn.h"
#include "wx/wfstream.h"

// ----------------------------------------------------------------------------
// types
// ----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// wxJ2KHandler
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxJ2KHandler,wxImageHandler)

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
void j2k_error_callback(const char *msg, void *client_data) {
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
void j2k_warning_callback(const char *msg, void *client_data) {
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
void j2k_info_callback(const char *msg, void *client_data) {
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

// load the j2k codestream
bool wxJ2KHandler::LoadFile(wxImage *image, wxInputStream& stream, bool verbose, int index)
{
	opj_dparameters_t parameters;	/* decompression parameters */
	opj_event_mgr_t event_mgr;		/* event manager */
	opj_image_t *opjimage = NULL;
	unsigned char *src = NULL;
    unsigned char *ptr;
	int file_length;
	opj_codestream_info_t cstr_info;  /* Codestream information structure */

	// destroy the image
    image->Destroy();

	/* handle to a decompressor */
	opj_dinfo_t* dinfo = NULL;	
	opj_cio_t *cio = NULL;


	/* configure the event callbacks (not required) */
	memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
	event_mgr.error_handler = j2k_error_callback;
	event_mgr.warning_handler = j2k_warning_callback;
	event_mgr.info_handler = j2k_info_callback;

	/* set decoding parameters to default values */
	opj_set_default_decoder_parameters(&parameters);

	/* prepare parameters */
	strncpy(parameters.infile, "", sizeof(parameters.infile)-1);
	strncpy(parameters.outfile, "", sizeof(parameters.outfile)-1);
	parameters.decod_format = J2K_CFMT;
	parameters.cod_format = BMP_DFMT;
	if (m_reducefactor)
		parameters.cp_reduce = m_reducefactor;
	if (m_qualitylayers)
		parameters.cp_layer = m_qualitylayers;

	/* JPWL only */
#ifdef USE_JPWL
	parameters.jpwl_exp_comps = m_expcomps;
	parameters.jpwl_max_tiles = m_maxtiles;
	parameters.jpwl_correct = m_enablejpwl;
#endif /* USE_JPWL */

	/* get a decoder handle */
	dinfo = opj_create_decompress(CODEC_J2K);

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
	opjimage = opj_decode(dinfo, cio, &cstr_info);
	if (!opjimage) {
#ifndef __WXGTK__ 
		wxMutexGuiEnter();
#endif /* __WXGTK__ */
		wxLogError(wxT("J2K: failed to decode image!"));
#ifndef __WXGTK__ 
		wxMutexGuiLeave();
#endif /* __WXGTK__ */
		opj_destroy_decompress(dinfo);
		opj_cio_close(cio);
		opj_image_destroy(opjimage);
		free(src);
		return false;
	}

	/* close the byte stream */
	opj_cio_close(cio);

	/* common rendering method */
#include "imagjpeg2000.cpp"

#ifndef __WXGTK__ 
    wxMutexGuiEnter();
#endif /* __WXGTK__ */
    wxLogMessage(wxT("J2K: image loaded."));
#ifndef __WXGTK__ 
    wxMutexGuiLeave();
#endif /* __WXGTK__ */

	/* close openjpeg structs */
	opj_destroy_decompress(dinfo);
	opj_image_destroy(opjimage);
	free(src);

	if (!image->Ok())
		return false;
	else
		return true;

}

// save the j2k codestream
bool wxJ2KHandler::SaveFile( wxImage *wimage, wxOutputStream& stream, bool verbose )
{
	opj_cparameters_t parameters;	/* compression parameters */
	opj_event_mgr_t event_mgr;		/* event manager */
	opj_image_t *oimage = NULL;
	opj_image_cmptparm_t *cmptparm;	
	opj_cio_t *cio = NULL;
	opj_codestream_info_t cstr_info;
	int codestream_length;
	bool bSuccess;
	int i;
	char indexfilename[OPJ_PATH_LEN] = "";	/* index file name */

	/*
	configure the event callbacks (not required)
	setting of each callback is optionnal
	*/
	memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
	event_mgr.error_handler = j2k_error_callback;
	event_mgr.warning_handler = j2k_warning_callback;
	event_mgr.info_handler = j2k_info_callback;

	/* set encoding parameters to default values */
	opj_set_default_encoder_parameters(&parameters);

	/* load parameters */
	parameters.cp_cinema = OFF;

	/* subsampling */
	if (sscanf(m_subsampling.c_str(), wxT("%d,%d"), &(parameters.subsampling_dx), &(parameters.subsampling_dy)) != 2) {
		wxLogError(wxT("Wrong sub-sampling encoder setting: dx,dy"));
		return false;
	}

	/* compression rates */
	if (m_rates != wxT("")) {
		char *s1 = (char *) m_rates.c_str();
		wxLogMessage("rates %s", s1);
		while (sscanf(s1, "%f", &(parameters.tcp_rates[parameters.tcp_numlayers])) == 1) {
			parameters.tcp_numlayers++;
			while (*s1 && *s1 != ',') {
				s1++;
			}
			if (!*s1)
				break;
			s1++;
		}
		wxLogMessage("%d layers", parameters.tcp_numlayers);
		parameters.cp_disto_alloc = 1;
	}

	/* image quality, dB */
	if (m_rates == wxT("")) {
		char *s2 = (char *) m_quality.c_str();
		wxLogMessage("qualities %s", s2);
		while (sscanf(s2, "%f", &parameters.tcp_distoratio[parameters.tcp_numlayers]) == 1) {
			parameters.tcp_numlayers++;
			while (*s2 && *s2 != ',') {
				s2++;
			}
			if (!*s2)
				break;
			s2++;
		}
		wxLogMessage("%d layers", parameters.tcp_numlayers);
		parameters.cp_fixed_quality = 1;
	}

	/* image origin */
	if (sscanf(m_origin.c_str(), "%d,%d", &parameters.image_offset_x0, &parameters.image_offset_y0) != 2) {
		wxLogError(wxT("bad coordinate of the image origin: x0,y0"));
		return false;
	}
				
	/* Create comment for codestream */
	if(m_enablecomm) {
		parameters.cp_comment = (char *) malloc(strlen(m_comment.c_str()) + 1);
		if(parameters.cp_comment) {
			strcpy(parameters.cp_comment, m_comment.c_str());
		}
	} else {
		parameters.cp_comment = NULL;
	}

	/* indexing file */
	if (m_enableidx) {
		strncpy(indexfilename, m_index.c_str(), OPJ_PATH_LEN);
		wxLogMessage("index file is %s", indexfilename);
	}

	/* if no rate entered, lossless by default */
	if (parameters.tcp_numlayers == 0) {
		parameters.tcp_rates[0] = 0;	/* MOD antonin : losslessbug */
		parameters.tcp_numlayers++;
		parameters.cp_disto_alloc = 1;
	}

	/* irreversible transform */
	parameters.irreversible = (m_irreversible == true) ? 1 : 0;

	/* resolutions */
	parameters.numresolution = m_resolutions;

	/* codeblocks size */
	if (m_cbsize != wxT("")) {
		int cblockw_init = 0, cblockh_init = 0;
		sscanf(m_cbsize.c_str(), "%d,%d", &cblockw_init, &cblockh_init);
		if (cblockw_init * cblockh_init > 4096 || cblockw_init > 1024 || cblockw_init < 4 || cblockh_init > 1024 || cblockh_init < 4) {
			wxLogError("!! Size of code_block error !! Restrictions:\n  width*height<=4096\n  4<=width,height<= 1024");
			return false;
		}
		parameters.cblockw_init = cblockw_init;
		parameters.cblockh_init = cblockh_init;
	}

	/* precincts size */
	if (m_prsize != wxT("")) {
		char sep;
		int res_spec = 0;
		char *s = (char *) m_prsize.c_str();
		do {
			sep = 0;
			sscanf(s, "[%d,%d]%c", &parameters.prcw_init[res_spec], &parameters.prch_init[res_spec], &sep);
			parameters.csty |= 0x01;
			res_spec++;
			s = strpbrk(s, "]") + 2;
		} while (sep == ',');
		parameters.res_spec = res_spec;
	}

	/* tiles */
	if (m_tsize != wxT("")) {
		sscanf(m_tsize.c_str(), "%d,%d", &parameters.cp_tdx, &parameters.cp_tdy);
		parameters.tile_size_on = true;
	}

	/* tile origin */
	if (sscanf(m_torigin.c_str(), "%d,%d", &parameters.cp_tx0, &parameters.cp_ty0) != 2) {
		wxLogError("tile offset setting error: X0,Y0");
		return false;
	}

	/* use SOP */
	if (m_enablesop)
		parameters.csty |= 0x02;

	/* use EPH */
	if (m_enableeph)
		parameters.csty |= 0x04;



	/* compression settings */
	//parameters.tcp_numlayers = 1;
	//parameters.tcp_rates[0] = 10.0;
	//parameters.cp_disto_alloc = 1;
	//parameters.irreversible = 1;
	parameters.tcp_mct = 1;

	/* convert wx image into opj image */
	cmptparm = (opj_image_cmptparm_t*) malloc(3 * sizeof(opj_image_cmptparm_t));

	/* initialize opj image components */	
	memset(&cmptparm[0], 0, 3 * sizeof(opj_image_cmptparm_t));
	for(i = 0; i < 3; i++) {		
		cmptparm[i].prec = 8;
		cmptparm[i].bpp = 8;
		cmptparm[i].sgnd = false;
		cmptparm[i].dx = parameters.subsampling_dx;
		cmptparm[i].dy = parameters.subsampling_dy;
		cmptparm[i].w = wimage->GetWidth();
		cmptparm[i].h = wimage->GetHeight();
	}

	/* create the image */
	oimage = opj_image_create(3, &cmptparm[0], CLRSPC_SRGB);
	if(!oimage) {
		if (cmptparm)
			free(cmptparm);
		return false;
	}

	/* set image offset and reference grid */
	oimage->x0 = parameters.image_offset_x0;
	oimage->y0 = parameters.image_offset_y0;
	oimage->x1 = parameters.image_offset_x0 + (wimage->GetWidth() - 1) * 1 + 1;
	oimage->y1 = parameters.image_offset_y0 + (wimage->GetHeight() - 1) * 1 + 1;

	/* load image data */
	unsigned char *value = wimage->GetData(); 
	int area = wimage->GetWidth() * wimage->GetHeight();
	for (i = 0; i < area; i++) {
			oimage->comps[0].data[i] = *(value++);
			oimage->comps[1].data[i] = *(value++);
			oimage->comps[2].data[i] = *(value++);
	}

	/* get a J2K compressor handle */
	opj_cinfo_t* cinfo = opj_create_compress(CODEC_J2K);

	/* catch events using our callbacks and give a local context */
	opj_set_event_mgr((opj_common_ptr)cinfo, &event_mgr, stderr);

	/* setup the encoder parameters using the current image and user parameters */
	opj_setup_encoder(cinfo, &parameters, oimage);

	/* open a byte stream for writing */
	/* allocate memory for all tiles */
	cio = opj_cio_open((opj_common_ptr)cinfo, NULL, 0);

	/* encode the image */
	bSuccess = opj_encode(cinfo, cio, oimage, &cstr_info);
	if (!bSuccess) {

		opj_cio_close(cio);
		opj_destroy_compress(cinfo);
		opj_image_destroy(oimage);
		if (cmptparm)
			free(cmptparm);
		if(parameters.cp_comment)
			free(parameters.cp_comment);
		if(parameters.cp_matrice)
			free(parameters.cp_matrice);

#ifndef __WXGTK__ 
    wxMutexGuiEnter();
#endif /* __WXGTK__ */

		wxLogError(wxT("failed to encode image"));

#ifndef __WXGTK__ 
    wxMutexGuiLeave();
#endif /* __WXGTK__ */

		return false;
	}
	codestream_length = cio_tell(cio);
	wxLogMessage(wxT("Codestream: %d bytes"), codestream_length);

	/* write the buffer to stream */
	stream.Write(cio->buffer, codestream_length);

	/* close and free the byte stream */
	opj_cio_close(cio);

	/* Write the index to disk */
	if (*indexfilename) {
		bSuccess = write_index_file(&cstr_info, indexfilename);
		if (bSuccess) {
			wxLogError(wxT("Failed to output index file"));
		}
	}

	/* free remaining compression structures */
	opj_destroy_compress(cinfo);

	/* free image data */
	opj_image_destroy(oimage);

	if (cmptparm)
		free(cmptparm);
	if(parameters.cp_comment)
		free(parameters.cp_comment);
	if(parameters.cp_matrice)
		free(parameters.cp_matrice);

#ifndef __WXGTK__ 
    wxMutexGuiEnter();
#endif /* __WXGTK__ */

    wxLogMessage(wxT("J2K: Image encoded!"));

#ifndef __WXGTK__ 
    wxMutexGuiLeave();
#endif /* __WXGTK__ */

    return true;
}

#ifdef __VISUALC__
    #pragma warning(default:4611)
#endif /* VC++ */

// recognize the 0xFF4F JPEG 2000 SOC marker
bool wxJ2KHandler::DoCanRead( wxInputStream& stream )
{
    unsigned char hdr[2];

    if ( !stream.Read(hdr, WXSIZEOF(hdr)) )
        return false;

    return hdr[0] == 0xFF && hdr[1] == 0x4F;
}

/**
Create an index and write it to a file
@param cstr_info Codestream information 
@param index Index filename
@return Returns 0 if successful, returns 1 otherwise
*/
bool wxJ2KHandler::write_index_file(opj_codestream_info_t *cstr_info, char *index) {
	int tileno, compno, layno, resno, precno, pack_nb, x, y;
	FILE *stream = NULL;
	int tilepartno;

	if (!cstr_info)		
		return 1;

	stream = fopen(index, "w");
	if (!stream) {
		fprintf(stderr, "failed to open index file [%s] for writing\n", index);
		return 1;
	}
	
	fprintf(stream, "%d %d\n", cstr_info->image_w, cstr_info->image_h);
	fprintf(stream, "%d\n", cstr_info->prog);
	fprintf(stream, "%d %d\n", cstr_info->tile_x, cstr_info->tile_y);
	fprintf(stream, "%d %d\n", cstr_info->tw, cstr_info->th);
	fprintf(stream, "%d\n", cstr_info->numcomps);
	fprintf(stream, "%d\n", cstr_info->numlayers);
	fprintf(stream, "%d\n", cstr_info->numdecompos);
	
	for (resno = cstr_info->numdecompos; resno >= 0; resno--) {
		fprintf(stream, "[%d,%d] ", 
			(1 << cstr_info->tile[0].pdx[resno]), (1 << cstr_info->tile[0].pdx[resno]));	/* based on tile 0 */
	}
	fprintf(stream, "\n");
	fprintf(stream, "%d\n", cstr_info->main_head_start);
	fprintf(stream, "%d\n", cstr_info->main_head_end);
	fprintf(stream, "%d\n", cstr_info->codestream_size);
	
	fprintf(stream, "\nINFO ON TILES\n");
	fprintf(stream, "tileno start_pos  end_hd  end_tile   nbparts\n");
	for (tileno = 0; tileno < cstr_info->tw * cstr_info->th; tileno++) {
		fprintf(stream, "%4d %9d %9d %9d %9d\n",
			cstr_info->tile[tileno].tileno,
			cstr_info->tile[tileno].start_pos,
			cstr_info->tile[tileno].end_header,
			cstr_info->tile[tileno].end_pos,
			cstr_info->tile[tileno].num_tps);
	}
		
	for (tileno = 0; tileno < cstr_info->tw * cstr_info->th; tileno++) {
		int start_pos, end_ph_pos, end_pos;
		pack_nb = 0;

		fprintf(stream, "\nTILE %d DETAILS\n", tileno);	
		fprintf(stream, "part_nb tileno  num_packs  start_pos end_tph_pos   end_pos\n");
		for (tilepartno = 0; tilepartno < cstr_info->tile[tileno].num_tps; tilepartno++)
			fprintf(stream, "%4d %9d  %9d  %9d %11d %9d\n",
				tilepartno, tileno,
				cstr_info->tile[tileno].tp[tilepartno].tp_numpacks,
				cstr_info->tile[tileno].tp[tilepartno].tp_start_pos,
				cstr_info->tile[tileno].tp[tilepartno].tp_end_header,
				cstr_info->tile[tileno].tp[tilepartno].tp_end_pos
				);
		if (cstr_info->prog == LRCP) {	/* LRCP */
			fprintf(stream, "LRCP\npack_nb tileno layno resno compno precno start_pos end_ph_pos end_pos\n");

			for (layno = 0; layno < cstr_info->numlayers; layno++) {
				for (resno = 0; resno < cstr_info->numdecompos + 1; resno++) {
					for (compno = 0; compno < cstr_info->numcomps; compno++) {
						int prec_max = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
						for (precno = 0; precno < prec_max; precno++) {
							start_pos = cstr_info->tile[tileno].packet[pack_nb].start_pos;
							end_ph_pos = cstr_info->tile[tileno].packet[pack_nb].end_ph_pos;
							end_pos = cstr_info->tile[tileno].packet[pack_nb].end_pos;
							fprintf(stream, "%4d %6d %7d %5d %6d  %6d    %6d     %6d %7d\n",
								pack_nb, tileno, layno, resno, compno, precno, start_pos, end_ph_pos, end_pos);
							pack_nb++;
						}
					}
				}
			}
		} /* LRCP */
		else if (cstr_info->prog == RLCP) {	/* RLCP */

			fprintf(stream, "RLCP\npack_nb tileno resno layno compno precno start_pos end_ph_pos end_pos\n");

			for (resno = 0; resno < cstr_info->numdecompos + 1; resno++) {
				for (layno = 0; layno < cstr_info->numlayers; layno++) {
					for (compno = 0; compno < cstr_info->numcomps; compno++) {
						int prec_max = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
						for (precno = 0; precno < prec_max; precno++) {
							start_pos = cstr_info->tile[tileno].packet[pack_nb].start_pos;
							end_ph_pos = cstr_info->tile[tileno].packet[pack_nb].end_ph_pos;
							end_pos = cstr_info->tile[tileno].packet[pack_nb].end_pos;
							fprintf(stream, "%4d %6d %5d %7d %6d %6d %9d   %9d %7d\n",
								pack_nb, tileno, resno, layno, compno, precno, start_pos, end_ph_pos, end_pos);
							pack_nb++;
						}
					}
				}
			}
		} /* RLCP */
		else if (cstr_info->prog == RPCL) {	/* RPCL */

			fprintf(stream, "RPCL\npack_nb tileno resno precno compno layno start_pos end_ph_pos end_pos\n"); 

			for (resno = 0; resno < cstr_info->numdecompos + 1; resno++) {
				/* I suppose components have same XRsiz, YRsiz */
				int x0 = cstr_info->tile_Ox + tileno - (int)floor((float)tileno/(float)cstr_info->tw ) * cstr_info->tw * cstr_info->tile_x;
				int y0 = cstr_info->tile_Ox + (int)floor( (float)tileno/(float)cstr_info->tw ) * cstr_info->tile_y;
				int x1 = x0 + cstr_info->tile_x;
				int y1 = y0 + cstr_info->tile_y;
				for (compno = 0; compno < cstr_info->numcomps; compno++) {
					int prec_max = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
					for (precno = 0; precno < prec_max; precno++) {
						int pcnx = cstr_info->tile[tileno].pw[resno];
						int pcx = (int) pow( 2, cstr_info->tile[tileno].pdx[resno] + cstr_info->numdecompos - resno );
						int pcy = (int) pow( 2, cstr_info->tile[tileno].pdy[resno] + cstr_info->numdecompos - resno );
						int precno_x = precno - (int) floor( (float)precno/(float)pcnx ) * pcnx;
						int precno_y = (int) floor( (float)precno/(float)pcnx );
						for(y = y0; y < y1; y++) {							
							if (precno_y*pcy == y ) {
								for (x = x0; x < x1; x++) {									
									if (precno_x*pcx == x ) {
										for (layno = 0; layno < cstr_info->numlayers; layno++) {
											start_pos = cstr_info->tile[tileno].packet[pack_nb].start_pos;
											end_ph_pos = cstr_info->tile[tileno].packet[pack_nb].end_ph_pos;
											end_pos = cstr_info->tile[tileno].packet[pack_nb].end_pos;
											fprintf(stream, "%4d %6d %5d %6d %6d %7d %9d   %9d %7d\n",
												pack_nb, tileno, resno, precno, compno, layno, start_pos, end_ph_pos, end_pos); 
											pack_nb++; 
										}
									}
								}/* x = x0..x1 */
							} 
						}  /* y = y0..y1 */
					} /* precno */
				} /* compno */
			} /* resno */
		} /* RPCL */
		else if (cstr_info->prog == PCRL) {	/* PCRL */
			/* I suppose components have same XRsiz, YRsiz */
			int x0 = cstr_info->tile_Ox + tileno - (int)floor( (float)tileno/(float)cstr_info->tw ) * cstr_info->tw * cstr_info->tile_x;
			int y0 = cstr_info->tile_Ox + (int)floor( (float)tileno/(float)cstr_info->tw ) * cstr_info->tile_y;
			int x1 = x0 + cstr_info->tile_x;
			int y1 = y0 + cstr_info->tile_y;

			fprintf(stream, "PCRL\npack_nb tileno precno compno resno layno start_pos end_ph_pos end_pos\n"); 

			for (compno = 0; compno < cstr_info->numcomps; compno++) {
				for (resno = 0; resno < cstr_info->numdecompos + 1; resno++) {
					int prec_max = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
					for (precno = 0; precno < prec_max; precno++) {
						int pcnx = cstr_info->tile[tileno].pw[resno];
						int pcx = (int) pow( 2, cstr_info->tile[tileno].pdx[resno] + cstr_info->numdecompos - resno );
						int pcy = (int) pow( 2, cstr_info->tile[tileno].pdy[resno] + cstr_info->numdecompos - resno );
						int precno_x = precno - (int) floor( (float)precno/(float)pcnx ) * pcnx;
						int precno_y = (int) floor( (float)precno/(float)pcnx );
						for(y = y0; y < y1; y++) {							
							if (precno_y*pcy == y ) {
								for (x = x0; x < x1; x++) {									
									if (precno_x*pcx == x ) {
										for (layno = 0; layno < cstr_info->numlayers; layno++) {
											start_pos = cstr_info->tile[tileno].packet[pack_nb].start_pos;
											end_ph_pos = cstr_info->tile[tileno].packet[pack_nb].end_ph_pos;
											end_pos = cstr_info->tile[tileno].packet[pack_nb].end_pos;
											fprintf(stream, "%4d %6d %6d %6d %5d %7d %9d   %9d %7d\n",
												pack_nb, tileno, precno, compno, resno, layno, start_pos, end_ph_pos, end_pos); 
											pack_nb++; 
										}
									}
								}/* x = x0..x1 */
							} 
						}  /* y = y0..y1 */
					} /* precno */
				} /* resno */
			} /* compno */
		} /* PCRL */
		else {	/* CPRL */

			fprintf(stream, "CPRL\npack_nb tileno compno precno resno layno start_pos end_ph_pos end_pos\n"); 

			for (compno = 0; compno < cstr_info->numcomps; compno++) {
				/* I suppose components have same XRsiz, YRsiz */
				int x0 = cstr_info->tile_Ox + tileno - (int)floor( (float)tileno/(float)cstr_info->tw ) * cstr_info->tw * cstr_info->tile_x;
				int y0 = cstr_info->tile_Ox + (int)floor( (float)tileno/(float)cstr_info->tw ) * cstr_info->tile_y;
				int x1 = x0 + cstr_info->tile_x;
				int y1 = y0 + cstr_info->tile_y;
				
				for (resno = 0; resno < cstr_info->numdecompos + 1; resno++) {
					int prec_max = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
					for (precno = 0; precno < prec_max; precno++) {
						int pcnx = cstr_info->tile[tileno].pw[resno];
						int pcx = (int) pow( 2, cstr_info->tile[tileno].pdx[resno] + cstr_info->numdecompos - resno );
						int pcy = (int) pow( 2, cstr_info->tile[tileno].pdy[resno] + cstr_info->numdecompos - resno );
						int precno_x = precno - (int) floor( (float)precno/(float)pcnx ) * pcnx;
						int precno_y = (int) floor( (float)precno/(float)pcnx );
						for(y = y0; y < y1; y++) {
							if (precno_y*pcy == y ) {
								for (x = x0; x < x1; x++) {
									if (precno_x*pcx == x ) {
										for (layno = 0; layno < cstr_info->numlayers; layno++) {
											start_pos = cstr_info->tile[tileno].packet[pack_nb].start_pos;
											end_ph_pos = cstr_info->tile[tileno].packet[pack_nb].end_ph_pos;
											end_pos = cstr_info->tile[tileno].packet[pack_nb].end_pos;
											fprintf(stream, "%4d %6d %6d %6d %5d %7d %9d   %9d %7d\n",
												pack_nb, tileno, compno, precno, resno, layno, start_pos, end_ph_pos, end_pos); 
											pack_nb++; 
										}
									}
								}/* x = x0..x1 */
							}
						} /* y = y0..y1 */
					} /* precno */
				} /* resno */
			} /* compno */
		} /* CPRL */   
	} /* tileno */
	
	fclose(stream);

	fprintf(stderr,"Generated index file %s\n", index);

	return true;
}

#endif   // wxUSE_STREAMS

#endif   // wxUSE_LIBOPENJPEG
