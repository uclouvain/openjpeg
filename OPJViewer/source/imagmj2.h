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
// Name:        imagmj2.h
// Purpose:     wxImage Motion JPEG 2000 file format handler
// Author:      G. Baruffa - based on imagjpeg.h, Vaclav Slavik
// RCS-ID:      $Id: imagmj2.h,v 0.0 2007/02/18 23:45:00 VZ Exp $
// Copyright:   (c) Giuseppe Baruffa
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_IMAGMJ2_H_
#define _WX_IMAGMJ2_H_

#include "wx/defs.h"

//-----------------------------------------------------------------------------
// wxMJ2Handler
//-----------------------------------------------------------------------------

#if wxUSE_LIBOPENJPEG

#include "wx/image.h"
#include "libopenjpeg/openjpeg.h"

#define wxBITMAP_TYPE_MJ2	49

class WXDLLEXPORT wxMJ2Handler: public wxImageHandler
{
public:
    inline wxMJ2Handler()
    {
        m_name = wxT("Motion JPEG 2000 file format");
        m_extension = wxT("mj2");
        m_type = wxBITMAP_TYPE_MJ2;
        m_mime = wxT("image/mj2");

		m_reducefactor = 0;
		m_qualitylayers = 0;
		m_components = 0;
#ifdef USE_JPWL
		m_enablejpwl = true;
		m_expcomps = JPWL_EXPECTED_COMPONENTS;
		m_maxtiles = JPWL_MAXIMUM_TILES;
#endif // USE_JPWL
    }

		// decoding engine parameters
		int m_reducefactor, m_qualitylayers, m_components, m_framenum;
#ifdef USE_JPWL
		bool m_enablejpwl;
		int m_expcomps, m_maxtiles;
#endif // USE_JPWL

#if wxUSE_STREAMS
    virtual bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 );
    virtual bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true );
protected:
    virtual bool DoCanRead( wxInputStream& stream );
#endif

private:
    DECLARE_DYNAMIC_CLASS(wxMJ2Handler)
};

#endif // wxUSE_LIBOPENJPEG

#endif // _WX_IMAGMJ2_H_

