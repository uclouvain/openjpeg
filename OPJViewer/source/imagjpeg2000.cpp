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
// Name:        imagjpeg2000.cpp
// Purpose:     wxImage JPEG 2000 imagage rendering common functions
// Author:      Giuseppe Baruffa
// RCS-ID:      $Id: imagjpeg2000.cpp,v 0.00 2007/04/27 22:11:00 MW Exp $
// Copyright:   (c) Giuseppe Baruffa
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

/*

- At this point, we have the structure "opjimage" that is filled with decompressed
  data, as processed by the OpenJPEG decompression engine

- We need to fill the class "image" with the proper pixel sample values

*/
{
	int shiftbpp;
	int c, tempcomps;

	// check components number
	if (m_components > opjimage->numcomps)
		m_components = opjimage->numcomps;

	// check image depth (only on the first one, for now)
	if (m_components)
		shiftbpp = opjimage->comps[m_components - 1].prec - 8;
	else
		shiftbpp = opjimage->comps[0].prec - 8;

	// prepare image size
	if (m_components)
		image->Create(opjimage->comps[m_components - 1].w, opjimage->comps[m_components - 1].h, true);
	else
		image->Create(opjimage->comps[0].w, opjimage->comps[0].h, true);

	// access image raw data
    image->SetMask(false);
    ptr = image->GetData();

	// workaround for components different from 1 or 3
	if ((opjimage->numcomps != 1) && (opjimage->numcomps != 3)) {
#ifndef __WXGTK__ 
		wxMutexGuiEnter();
#endif /* __WXGTK__ */
		wxLogMessage(wxT("JPEG2000: weird number of components"));
#ifndef __WXGTK__ 
		wxMutexGuiLeave();
#endif /* __WXGTK__ */
		tempcomps = 1;
	} else
		tempcomps = opjimage->numcomps;

	// workaround for subsampled components
	for (c = 1; c < tempcomps; c++) {
		if ((opjimage->comps[c].w != opjimage->comps[c - 1].w) || (opjimage->comps[c].h != opjimage->comps[c - 1].h)) {
			tempcomps = 1;
			break;
		}
	}

	// workaround for different precision components
	for (c = 1; c < tempcomps; c++) {
		if (opjimage->comps[c].bpp != opjimage->comps[c - 1].bpp) {
			tempcomps = 1;
			break;
		}
	}

	// only one component selected
	if (m_components)
		tempcomps = 1;

	// RGB color picture
	if (tempcomps == 3) {
		int row, col;
		int *r = opjimage->comps[0].data;
		int *g = opjimage->comps[1].data;
		int *b = opjimage->comps[2].data;
		if (shiftbpp > 0) {
			for (row = 0; row < opjimage->comps[0].h; row++) {
				for (col = 0; col < opjimage->comps[0].w; col++) {
					
					*(ptr++) = (*(r++)) >> shiftbpp;
					*(ptr++) = (*(g++)) >> shiftbpp;
					*(ptr++) = (*(b++)) >> shiftbpp;

				}
			}

		} else if (shiftbpp < 0) {
			for (row = 0; row < opjimage->comps[0].h; row++) {
				for (col = 0; col < opjimage->comps[0].w; col++) {
					
					*(ptr++) = (*(r++)) << -shiftbpp;
					*(ptr++) = (*(g++)) << -shiftbpp;
					*(ptr++) = (*(b++)) << -shiftbpp;

				}
			}
			
		} else {
			for (row = 0; row < opjimage->comps[0].h; row++) {
				for (col = 0; col < opjimage->comps[0].w; col++) {

					*(ptr++) = *(r++);
					*(ptr++) = *(g++);
					*(ptr++) = *(b++);
				
				}
			}
		}
	}

	// B/W picture
	if (tempcomps == 1) {
		int row, col;
		int selcomp;

		if (m_components)
			selcomp = m_components - 1;
		else
			selcomp = 0;

		int *y = opjimage->comps[selcomp].data;
		if (shiftbpp > 0) {
			for (row = 0; row < opjimage->comps[selcomp].h; row++) {
				for (col = 0; col < opjimage->comps[selcomp].w; col++) {
					
					*(ptr++) = (*(y)) >> shiftbpp;
					*(ptr++) = (*(y)) >> shiftbpp;
					*(ptr++) = (*(y++)) >> shiftbpp;

				}
			}
		} else if (shiftbpp < 0) {
			for (row = 0; row < opjimage->comps[selcomp].h; row++) {
				for (col = 0; col < opjimage->comps[selcomp].w; col++) {
					
					*(ptr++) = (*(y)) << -shiftbpp;
					*(ptr++) = (*(y)) << -shiftbpp;
					*(ptr++) = (*(y++)) << -shiftbpp;

				}
			}
		} else {
			for (row = 0; row < opjimage->comps[selcomp].h; row++) {
				for (col = 0; col < opjimage->comps[selcomp].w; col++) {
					
					*(ptr++) = *(y);
					*(ptr++) = *(y);
					*(ptr++) = *(y++);

				}
			}
		}
	}


}