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
#include "OPJViewer.h"

/* From little endian to big endian, 2 bytes */
#define	BYTE_SWAP2(X)	((X & 0x00FF) << 8) | ((X & 0xFF00) >> 8)
#define	BYTE_SWAP4(X)	((X & 0x000000FF) << 24) | ((X & 0x0000FF00) << 8) | ((X & 0x00FF0000) >> 8) | ((X & 0xFF000000) >> 24)

/* From codestream to int values */
#define STREAM_TO_UINT32(C, P)	(((unsigned long int) (C)[(P) + 0] << 24) + \
								((unsigned long int) (C)[(P) + 1] << 16) + \
								((unsigned long int) (C)[(P) + 2] << 8) + \
								((unsigned long int) (C)[(P) + 3] << 0))

#define STREAM_TO_UINT16(C, P)	(((unsigned long int) (C)[(P) + 0] << 8) + \
								((unsigned long int) (C)[(P) + 1] << 0))


/* Markers values */
enum {
	SOC_VAL = 0xFF4F,
	SOT_VAL	= 0xFF90,
	SOD_VAL = 0xFF93,
	EOC_VAL	= 0xFFD9,
	SIZ_VAL	= 0xFF51,
	COD_VAL	= 0xFF52,
	COC_VAL = 0xFF53,
	RGN_VAL = 0xFF5E,
	QCD_VAL	= 0xFF5C,
	QCC_VAL	= 0xFF5D,
	POD_VAL	= 0xFF5F,
	TLM_VAL	= 0xFF55,
	PLM_VAL	= 0xFF57,
	PLT_VAL	= 0xFF58,
	PPM_VAL	= 0xFF60,
	PPT_VAL	= 0xFF61,
	SOP_VAL	= 0xFF91,
	EPH_VAL	= 0xFF92,
	CME_VAL	= 0xFF64,
#ifndef USEOLDJPWL 
	EPB_VAL	= 0xFF66,
	ESD_VAL	= 0xFF67,
	EPC_VAL	= 0xFF68,
	RED_VAL	= 0xFF69
#else
	EPB_VAL = 0xFF96,
	ESD_VAL	= 0xFF98,
	EPC_VAL	= 0xFF97,
	RED_VAL	= 0xFF99
#endif
};

// All the markers in one vector
unsigned short int marker_val[] = {
	SOC_VAL, SOT_VAL, SOD_VAL, EOC_VAL,
	SIZ_VAL,
	COD_VAL, COC_VAL, RGN_VAL, QCD_VAL, QCC_VAL, POD_VAL,
	TLM_VAL, PLM_VAL, PLT_VAL, PPM_VAL, PPT_VAL,
	SOP_VAL, EPH_VAL,
	CME_VAL,
	EPB_VAL, ESD_VAL, EPC_VAL, RED_VAL
};

// Marker names
char *marker_name[] = {
	"SOC", "SOT", "SOD", "EOC",
	"SIZ",
	"COD", "COC", "RGN", "QCD", "QCC", "POD",
	"TLM", "PLM", "PLT", "PPM", "PPT",
	"SOP", "EPH",
	"CME",
	"EPB", "ESD", "EPC", "RED"
};

// Marker descriptions
char *marker_descr[] = {
	"Start of codestream", "Start of tile-part", "Start of data", "End of codestream",
	"Image and tile size",
	"Coding style default", "Coding style component", "Region-of-interest", "Quantization default",
	"Quantization component", "Progression order change, default",
	"Tile-part lengths, main header", "Packet length, main header", "Packets length, tile-part header",
	"Packed packet headers, main header", "Packed packet headers, tile-part header",
	"Start of packet", "End of packet header",
	"Comment and extension",
	"Error Protection Block", "Error Sensitivity Descriptor", "Error Protection Capability",
	"Residual Errors Descriptor"
};

void OPJParseThread::ParseJ2KFile(wxFile *m_file, wxFileOffset offset, wxFileOffset length, wxTreeItemId parentid)
{
	// check if the file is opened
	if (m_file->IsOpened())
		WriteText(wxT("File OK"));
	else
		return;

	// position at the beginning
	m_file->Seek(offset, wxFromStart);

	//WriteText(wxString::Format(wxT("from  to %d"), length));

	// navigate the file
	int m, inside_sod = 0, nmarks = 0, maxmarks = 10000, done = 0;
	unsigned char onebyte[1];
	unsigned char twobytes[2];
	unsigned char fourbytes[4];
	unsigned short int currmark;
	unsigned short int currlen;
	int lastPsot = 0, lastsotpos = 0;

	WriteText(wxT("Start search..."));
	while ((offset < length) && (!m_file->Eof())) {

		done = 0;

		// read da marka
		if (m_file->Read(twobytes, 2) != 2)
			break;
		currmark = (((unsigned short int) twobytes[0]) << 8) + (unsigned short int) twobytes[1];

		// Markers cycle
		for (m = 0; m < 23; m++) {

			// check the marker
			if (currmark == marker_val[m]) {

				if (currmark == SOD_VAL) {

					// we enter SOD
					currlen = 0;
					inside_sod = 1;

				} else if ((currmark == SOC_VAL) || (currmark == EOC_VAL) || (currmark == EPH_VAL))
					
					currlen = 0;

				else {

					// read length
					if (m_file->Read(twobytes, 2) != 2)
						break;
					currlen = (((unsigned short int) twobytes[0]) << 8) + (unsigned short int) twobytes[1];

				}

				// inside SOD, only some markers are allowed
				if (inside_sod && (currmark != SOD_VAL) && (currmark != SOT_VAL)
					&& (currmark != EOC_VAL) && (currmark != SOP_VAL) && (currmark != EPH_VAL))
					break; /*randomly marker coincident data */

				if (inside_sod && (currmark == SOT_VAL) && (lastPsot == 0))
					inside_sod = 0; /* random data coincident with SOT, but last SOT was the last one */

				if (inside_sod && (currmark == SOT_VAL))
					inside_sod = 0; /* new tile part */

				// here we pass to AppendItem() normal and selected item images (we
				// suppose that selected image follows the normal one in the enum)
				int image, imageSel;
				image = m_tree->TreeCtrlIcon_Folder;
				imageSel = image + 1;

				// append the marker
				wxTreeItemId currid = m_tree->AppendItem(parentid,
					wxString::Format(wxT("%03d: %s (0x%04X)"), nmarks, marker_name[m], marker_val[m]),
					image, imageSel,
					new OPJMarkerData(wxT("MARK"), m_tree->m_fname.GetFullPath(), offset, offset + currlen + 1)
					);

				// append some info
				image = m_tree->TreeCtrlIcon_File;
				imageSel = image + 1;

				// marker name
				wxTreeItemId subcurrid1 = m_tree->AppendItem(currid,
					wxT("*** ") + wxString(marker_descr[m]) + wxT(" ***"),
					image, imageSel,
					new OPJMarkerData(wxT("INFO"))
					);
				m_tree->SetItemFont(subcurrid1, *wxITALIC_FONT);

				// position and length
				wxTreeItemId subcurrid2 = m_tree->AppendItem(currid,
					wxLongLong(offset).ToString() + wxT(" > ") + wxLongLong(offset + currlen + 1).ToString() + 
					wxT(", ") + wxString::Format(wxT("%d + 2 (%d)"), currlen, currlen + 2),
					image, imageSel,
					new OPJMarkerData(wxT("INFO"))
					);

				// give additional info on markers
				switch (currmark) {
					
				case SOP_VAL:
					{
					// read packet number
					if (m_file->Read(twobytes, 2) != 2)
						break;
					int packnum = STREAM_TO_UINT16(twobytes, 0);;
					wxTreeItemId subcurrid3 = m_tree->AppendItem(currid,
						wxString::Format(wxT("Pack. no. %d"), packnum),
						image, imageSel,
						new OPJMarkerData(wxT("INFO"))
						);
					}
					break;

				case SIZ_VAL:
					{
					m_file->Seek(2, wxFromCurrent);
					if (m_file->Read(fourbytes, 4) != 4)
						break;
					unsigned long int xsiz = STREAM_TO_UINT32(fourbytes, 0);

					if (m_file->Read(fourbytes, 4) != 4)
						break;
					unsigned long int ysiz = STREAM_TO_UINT32(fourbytes, 0);

					m_file->Seek(24, wxFromCurrent);
					if (m_file->Read(twobytes, 2) != 2)
						break;
					unsigned short int csiz = STREAM_TO_UINT16(twobytes, 0);

					if (m_file->Read(onebyte, 1) != 1)
						break;
					unsigned char ssiz = onebyte[0];

					wxTreeItemId subcurrid3 = m_tree->AppendItem(currid,
						wxString::Format(wxT("%d x %d, %d comps. @ %d bpp"), xsiz, ysiz, csiz, (ssiz + 1) & 0xEF),
						image, imageSel,
						new OPJMarkerData(wxT("INFO"))
						);

					}
					break;

				case SOT_VAL:
					{
					if (m_file->Read(twobytes, 2) != 2)
						break;
					unsigned short int isot = STREAM_TO_UINT16(twobytes, 0);

					if (m_file->Read(fourbytes, 4) != 4)
						break;
					unsigned long int psot = STREAM_TO_UINT32(fourbytes, 0);

					if (m_file->Read(onebyte, 1) != 1)
						break;
					unsigned char tpsot = onebyte[0];

					if (m_file->Read(onebyte, 1) != 1)
						break;
					unsigned char tnsot = onebyte[0];

					wxTreeItemId subcurrid3 = m_tree->AppendItem(currid,
						wxString::Format(wxT("tile %d, psot = %d, part %d of %d"), isot, psot, tpsot, tnsot),
						image, imageSel,
						new OPJMarkerData(wxT("INFO"))
						);

					lastPsot = psot;
					lastsotpos = offset;
					};
					break;

				case CME_VAL:
					{
					#define showlen 25
					unsigned char comment[showlen];

					m_file->Seek(2, wxFromCurrent);
					if (m_file->Read(comment, showlen) != showlen)
						break;

					wxTreeItemId subcurrid3 = m_tree->AppendItem(currid,
						wxString::Format(wxT("%.*s%s"), wxMin(showlen, currlen - 4), comment,
						(((currlen - 4) > showlen) ? "..." : "")),
						image, imageSel,
						new OPJMarkerData(wxT("INFO"))
						);
					}
					break;

				default:
					break;
				}
				
				
				// increment number of markers
				nmarks++;
				if (nmarks >= maxmarks)
					break;

				// increment offset
				if (currmark == SOD_VAL)
					offset += lastPsot - (offset - lastsotpos);
				else
					offset += (2 + currlen);

				m_file->Seek(offset, wxFromStart);
				done = 1;

				break;
			}
		}

		if (done)
			continue;
		else {
			offset++;
			m_file->Seek(offset, wxFromStart);
		}
	}
	
}
