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
// Name:        sashtest.cpp
// Purpose:     Layout/sash sample
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: sashtest.cpp,v 1.18 2005/08/23 15:54:35 ABX Exp $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Name:        treetest.cpp
// Purpose:     wxTreeCtrl sample
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: treetest.cpp,v 1.110 2006/11/04 11:26:51 VZ Exp $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Name:        dialogs.cpp
// Purpose:     Common dialogs demo
// Author:      Julian Smart
// Modified by: ABX (2004) - adjustements for conditional building + new menu
// Created:     04/01/98
// RCS-ID:      $Id: dialogs.cpp,v 1.163 2006/11/04 10:57:24 VZ Exp $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Name:        thread.cpp
// Purpose:     wxWidgets thread sample
// Author:      Guilhem Lavaux, Vadim Zeitlin
// Modified by:
// Created:     06/16/98
// RCS-ID:      $Id: thread.cpp,v 1.26 2006/10/02 05:36:28 PC Exp $
// Copyright:   (c) 1998-2002 wxWidgets team
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Name:        samples/image/image.cpp
// Purpose:     sample showing operations with wxImage
// Author:      Robert Roebling
// Modified by:
// Created:     1998
// RCS-ID:      $Id: image.cpp,v 1.120 2006/12/06 17:13:11 VZ Exp $
// Copyright:   (c) 1998-2005 Robert Roebling
// License:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Name:        samples/console/console.cpp
// Purpose:     A sample console (as opposed to GUI) program using wxWidgets
// Author:      Vadim Zeitlin
// Modified by:
// Created:     04.10.99
// RCS-ID:      $Id: console.cpp,v 1.206 2006/11/12 19:55:19 VZ Exp $
// Copyright:   (c) 1999 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Name:        samples/notebook/notebook.cpp
// Purpose:     a sample demonstrating notebook usage
// Author:      Julian Smart
// Modified by: Dimitri Schoolwerth
// Created:     26/10/98
// RCS-ID:      $Id: notebook.cpp,v 1.49 2006/11/04 18:24:07 RR Exp $
// Copyright:   (c) 1998-2002 wxWidgets team
// License:     wxWindows license
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Name:        dialogs.cpp
// Purpose:     Common dialogs demo
// Author:      Julian Smart
// Modified by: ABX (2004) - adjustements for conditional building + new menu
// Created:     04/01/98
// RCS-ID:      $Id: dialogs.cpp,v 1.163 2006/11/04 10:57:24 VZ Exp $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Name:        dnd.cpp
// Purpose:     Drag and drop sample
// Author:      Vadim Zeitlin
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: dnd.cpp,v 1.107 2006/10/30 20:23:41 VZ Exp $
// Copyright:
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Name:        test.cpp
// Purpose:     wxHtml testing example
/////////////////////////////////////////////////////////////////////////////


#include "OPJViewer.h"

IMPLEMENT_APP(OPJViewerApp)

// For drawing lines in a canvas
long xpos = -1;
long ypos = -1;

int winNumber = 1;

// Initialise this in OnInit, not statically
bool OPJViewerApp::OnInit(void)
{
#if wxUSE_UNICODE

    wxChar **wxArgv = new wxChar *[argc + 1];

	int n;
    for (n = 0; n < argc; n++ ) {
        wxMB2WXbuf warg = wxConvertMB2WX((char *) argv[n]);
        wxArgv[n] = wxStrdup(warg);
    }

    wxArgv[n] = NULL;

#else // !wxUSE_UNICODE

    #define wxArgv argv

#endif // wxUSE_UNICODE/!wxUSE_UNICODE

#if wxUSE_CMDLINE_PARSER

    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        { wxCMD_LINE_SWITCH, _T("h"), _T("help"), _T("show this help message"),
            wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },

        { wxCMD_LINE_PARAM,  NULL, NULL, _T("input file"),
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },

        { wxCMD_LINE_NONE }
    };

    wxCmdLineParser parser(cmdLineDesc, argc, wxArgv);

    /*parser.AddOption(_T("project_name"), _T(""), _T("full path to project file"),
                     wxCMD_LINE_VAL_STRING,
                     wxCMD_LINE_OPTION_MANDATORY | wxCMD_LINE_NEEDS_SEPARATOR);*/

    switch (parser.Parse()) {
    case -1:
        wxLogMessage(wxT("Help was given, terminating."));
        break;

    case 0:
        ShowCmdLine(parser);
        break;

    default:
        wxLogMessage(wxT("Syntax error detected."));
        break;
    }

#endif // wxUSE_CMDLINE_PARSER

    //wxInitAllImageHandlers();
#if wxUSE_LIBJPEG
  wxImage::AddHandler( new wxJPEGHandler );
#endif
#if wxUSE_LIBOPENJPEG
  wxImage::AddHandler( new wxJ2KHandler );
  wxImage::AddHandler( new wxJP2Handler );
  wxImage::AddHandler( new wxMJ2Handler );
#endif
#if OPJ_MANYFORMATS
  wxImage::AddHandler( new wxBMPHandler );
  wxImage::AddHandler( new wxPNGHandler );
  wxImage::AddHandler( new wxGIFHandler );
  wxImage::AddHandler( new wxPNMHandler );
  wxImage::AddHandler( new wxTIFFHandler );
#endif
    // we use a XPM image in our HTML page
    wxImage::AddHandler(new wxXPMHandler);

	// memory file system
    wxFileSystem::AddHandler(new wxMemoryFSHandler);

	// set decoding engine parameters
	m_enabledeco = true;
	m_resizemethod = 0;
	m_reducefactor = 0;
	m_qualitylayers = 0;
	m_components = 0;
	m_framenum = 0;
#ifdef USE_JPWL
	m_enablejpwl = true;
	m_expcomps = JPWL_EXPECTED_COMPONENTS;
	m_maxtiles = JPWL_MAXIMUM_TILES;
#endif // USE_JPWL

	// Create the main frame window
  OPJFrame *frame = new OPJFrame(NULL, wxID_ANY, OPJ_APPLICATION_TITLEBAR,
					  wxDefaultPosition, wxSize(800, 600),
                      wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE |
                      wxHSCROLL | wxVSCROLL);

  // Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
  frame->SetIcon(wxIcon(wxT("OPJViewer16")));
#endif

  frame->Show(true);

  SetTopWindow(frame);

	// if there are files on the command line, open them
	if (!(m_filelist.IsEmpty())) {
		//wxLogMessage(wxT("Habemus files!!!"));
		wxArrayString paths, filenames;
		for (unsigned int f = 0; f < wxGetApp().m_filelist.GetCount(); f++) {
			paths.Add(wxFileName(wxGetApp().m_filelist[f]).GetFullPath());
			filenames.Add(wxFileName(wxGetApp().m_filelist[f]).GetFullName());
		}
		//wxLogMessage(paths[0]);
		frame->OpenFiles(paths, filenames);
	}

  return true;
}

void OPJViewerApp::ShowCmdLine(const wxCmdLineParser& parser)
{
    wxString s = wxT("Command line parsed successfully:\nInput files: ");

    size_t count = parser.GetParamCount();
    for (size_t param = 0; param < count; param++) {
        s << parser.GetParam(param) << ';';
		m_filelist.Add(parser.GetParam(param));
    }

    //wxLogMessage(s);
}

// OPJFrame events
BEGIN_EVENT_TABLE(OPJFrame, wxMDIParentFrame)
    EVT_MENU(OPJFRAME_HELPABOUT, OPJFrame::OnAbout)
    EVT_MENU(OPJFRAME_FILEOPEN, OPJFrame::OnFileOpen)
    EVT_SIZE(OPJFrame::OnSize)
    EVT_MENU(OPJFRAME_FILEEXIT, OPJFrame::OnQuit)
    EVT_MENU(OPJFRAME_FILECLOSE, OPJFrame::OnClose)
    EVT_MENU(OPJFRAME_VIEWZOOM, OPJFrame::OnZoom)
    EVT_MENU(OPJFRAME_VIEWFIT, OPJFrame::OnFit)
    EVT_MENU(OPJFRAME_VIEWRELOAD, OPJFrame::OnReload)
    EVT_MENU(OPJFRAME_FILETOGGLEB, OPJFrame::OnToggleBrowser)
    EVT_MENU(OPJFRAME_FILETOGGLEP, OPJFrame::OnTogglePeeker)
    EVT_MENU(OPJFRAME_SETSENCO, OPJFrame::OnSetsEnco)
    EVT_MENU(OPJFRAME_SETSDECO, OPJFrame::OnSetsDeco)
    EVT_SASH_DRAGGED_RANGE(OPJFRAME_BROWSEWIN, OPJFRAME_LOGWIN, OPJFrame::OnSashDrag)
    EVT_NOTEBOOK_PAGE_CHANGED(LEFT_NOTEBOOK_ID, OPJFrame::OnNotebook)
END_EVENT_TABLE()

// this is the frame constructor
OPJFrame::OPJFrame(wxWindow *parent, const wxWindowID id, const wxString& title,
				   const wxPoint& pos, const wxSize& size, const long style)
		: wxMDIParentFrame(parent, id, title, pos, size, style)
{
	// file menu and its items
	wxMenu *file_menu = new wxMenu;

	file_menu->Append(OPJFRAME_FILEOPEN, wxT("&Open\tCtrl+O"));
	file_menu->SetHelpString(OPJFRAME_FILEOPEN, wxT("Open one or more files"));

	file_menu->Append(OPJFRAME_FILETOGGLEB, wxT("Toggle &browser\tCtrl+B"));
	file_menu->SetHelpString(OPJFRAME_FILETOGGLEB, wxT("Toggle the left browsing pane"));

	file_menu->Append(OPJFRAME_FILETOGGLEP, wxT("Toggle &peeker\tCtrl+P"));
	file_menu->SetHelpString(OPJFRAME_FILETOGGLEP, wxT("Toggle the bottom peeking pane"));

	file_menu->Append(OPJFRAME_FILECLOSE, wxT("&Close\tCtrl+C"));
	file_menu->SetHelpString(OPJFRAME_FILECLOSE, wxT("Close current image"));

	file_menu->Append(OPJFRAME_FILEEXIT, wxT("&Exit\tCtrl+Q"));
	file_menu->SetHelpString(OPJFRAME_FILEEXIT, wxT("Quit this program"));

	// view menu and its items
	wxMenu *view_menu = new wxMenu;

	view_menu->Append(OPJFRAME_VIEWZOOM, wxT("&Zoom\tCtrl+Z"));
	view_menu->SetHelpString(OPJFRAME_VIEWZOOM, wxT("Rescale the image"));

	view_menu->Append(OPJFRAME_VIEWFIT, wxT("Zoom to &fit\tCtrl+F"));
	view_menu->SetHelpString(OPJFRAME_VIEWFIT, wxT("Fit the image in canvas"));

	view_menu->Append(OPJFRAME_VIEWRELOAD, wxT("&Reload image\tCtrl+R"));
	view_menu->SetHelpString(OPJFRAME_VIEWRELOAD, wxT("Reload the current image"));

	// settings menu and its items
	wxMenu *sets_menu = new wxMenu;

	sets_menu->Append(OPJFRAME_SETSENCO, wxT("&Encoder\tCtrl+E"));
	sets_menu->SetHelpString(OPJFRAME_SETSENCO, wxT("Encoder settings"));

	sets_menu->Append(OPJFRAME_SETSDECO, wxT("&Decoder\tCtrl+D"));
	sets_menu->SetHelpString(OPJFRAME_SETSDECO, wxT("Decoder settings"));

	// help menu and its items
	wxMenu *help_menu = new wxMenu;

	help_menu->Append(OPJFRAME_HELPABOUT, wxT("&About\tF1"));
	help_menu->SetHelpString(OPJFRAME_HELPABOUT, wxT("Basic info on the program"));

	// the whole menubar
	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, wxT("&File"));
	menu_bar->Append(view_menu, wxT("&View"));
	menu_bar->Append(sets_menu, wxT("&Settings"));
	menu_bar->Append(help_menu, wxT("&Help"));

	// Associate the menu bar with the frame
	SetMenuBar(menu_bar);

	// the status bar
	CreateStatusBar();

	// the logging window
	loggingWindow = new wxSashLayoutWindow(this, OPJFRAME_LOGWIN,
											wxDefaultPosition, wxSize(400, 130),
											wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN
											);
	loggingWindow->SetDefaultSize(wxSize(1000, 130));
	loggingWindow->SetOrientation(wxLAYOUT_HORIZONTAL);
	loggingWindow->SetAlignment(wxLAYOUT_BOTTOM);
	//loggingWindow->SetBackgroundColour(wxColour(0, 0, 255));
	loggingWindow->SetSashVisible(wxSASH_TOP, true);

	// create the bottom notebook
	m_bookCtrlbottom = new wxNotebook(loggingWindow, BOTTOM_NOTEBOOK_ID,
								wxDefaultPosition, wxDefaultSize,
								wxBK_LEFT);

	// create the text control of the logger
	m_textCtrl = new wxTextCtrl(m_bookCtrlbottom, wxID_ANY, wxT(""),
								wxDefaultPosition, wxDefaultSize,
								wxTE_MULTILINE | wxSUNKEN_BORDER | wxTE_READONLY
								);
	m_textCtrl->SetValue(_T("Logging window\n"));

	// add it to the notebook
	m_bookCtrlbottom->AddPage(m_textCtrl, wxT("Log"));

	// create the text control of the browser
	m_textCtrlbrowse = new wxTextCtrl(m_bookCtrlbottom, wxID_ANY, wxT(""),
								wxDefaultPosition, wxDefaultSize,
								wxTE_MULTILINE | wxSUNKEN_BORDER | wxTE_READONLY | wxTE_RICH
								);
	wxFont *browsefont = new wxFont(wxNORMAL_FONT->GetPointSize(),
		wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_textCtrlbrowse->SetDefaultStyle(wxTextAttr(wxNullColour, wxNullColour, *browsefont));
	m_textCtrlbrowse->AppendText(wxT("Browsing window\n"));

	// add it the notebook
	m_bookCtrlbottom->AddPage(m_textCtrlbrowse, wxT("Peek"), false);

	// the browser window
	markerTreeWindow = new wxSashLayoutWindow(this, OPJFRAME_BROWSEWIN,
											  wxDefaultPosition, wxSize(300, 30),
											  wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN
											  );
	markerTreeWindow->SetDefaultSize(wxSize(300, 1000));
	markerTreeWindow->SetOrientation(wxLAYOUT_VERTICAL);
	markerTreeWindow->SetAlignment(wxLAYOUT_LEFT);
	//markerTreeWindow->SetBackgroundColour(wxColour(0, 255, 0));
	markerTreeWindow->SetSashVisible(wxSASH_RIGHT, true);
	markerTreeWindow->SetExtraBorderSize(0);

	// create the browser notebook
	m_bookCtrl = new wxNotebook(markerTreeWindow, LEFT_NOTEBOOK_ID,
								wxDefaultPosition, wxDefaultSize,
								wxBK_TOP);

#ifdef __WXMOTIF__
	// For some reason, we get a memcpy crash in wxLogStream::DoLogStream
	// on gcc/wxMotif, if we use wxLogTextCtl. Maybe it's just gcc?
	delete wxLog::SetActiveTarget(new wxLogStderr);
#else
	// set our text control as the log target
	wxLogTextCtrl *logWindow = new wxLogTextCtrl(m_textCtrl);
	delete wxLog::SetActiveTarget(logWindow);
#endif

	// associate drop targets with the controls
	SetDropTarget(new OPJDnDFile(this));

}

// this is the frame destructor
OPJFrame::~OPJFrame(void)
{
	// delete all possible things
	delete m_bookCtrl;
	m_bookCtrl = NULL;

	delete markerTreeWindow;
	markerTreeWindow = NULL;

	delete m_textCtrl;
	m_textCtrl = NULL;

	delete m_bookCtrlbottom;
	m_bookCtrlbottom = NULL;

	delete loggingWindow;
	loggingWindow = NULL;
}

void OPJFrame::OnNotebook(wxNotebookEvent& event)
{
	int sel = event.GetSelection();
	long childnum;

	m_bookCtrl->GetPageText(sel).ToLong(&childnum);

	if (m_childhash[childnum])
		m_childhash[childnum]->Activate();

	//wxLogMessage(wxT("Selection changed (now %d --> %d)"), childnum, m_childhash[childnum]->m_winnumber);

}


void OPJFrame::Resize(int number)
{
	wxSize size = GetClientSize();
}

void OPJFrame::OnSetsEnco(wxCommandEvent& event)
{
    OPJEncoderDialog dialog(this, event.GetId());

    if (dialog.ShowModal() == wxID_OK) {

	};
}

void OPJFrame::OnSetsDeco(wxCommandEvent& event)
{
    OPJDecoderDialog dialog(this, event.GetId());

    if (dialog.ShowModal() == wxID_OK) {

		// load settings
		wxGetApp().m_enabledeco = dialog.m_enabledecoCheck->GetValue();
		wxGetApp().m_resizemethod = dialog.m_resizeBox->GetSelection();
		wxGetApp().m_reducefactor = dialog.m_reduceCtrl->GetValue();
		wxGetApp().m_qualitylayers = dialog.m_layerCtrl->GetValue();
		wxGetApp().m_components = dialog.m_numcompsCtrl->GetValue();
		wxGetApp().m_framenum = dialog.m_framenumCtrl->GetValue();
#ifdef USE_JPWL
		wxGetApp().m_enablejpwl = dialog.m_enablejpwlCheck->GetValue();
		wxGetApp().m_expcomps = dialog.m_expcompsCtrl->GetValue();
		wxGetApp().m_maxtiles = dialog.m_maxtilesCtrl->GetValue();
#endif // USE_JPWL

	};
}

void OPJFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void OPJFrame::OnClose(wxCommandEvent& WXUNUSED(event))
{
	// current frame
	OPJChildFrame *currframe = (OPJChildFrame *) GetActiveChild();

	if (!currframe)
		return;

	wxCloseEvent e;
	currframe->OnClose(e);
}

void OPJFrame::OnFit(wxCommandEvent& WXUNUSED(event))
{
	// current child
	OPJChildFrame *currchild = (OPJChildFrame *) GetActiveChild();
	if (!currchild)
		return;

	// current canvas
	OPJCanvas *currcanvas = currchild->m_canvas;

	// find a fit-to-width zoom
	int zooml, wzooml, hzooml;
	wxSize clientsize = currcanvas->GetClientSize();
	wzooml = (int) ceil(100.0 * (double) (clientsize.GetWidth() - 2 * OPJ_CANVAS_BORDER) / (double) (currcanvas->m_image100.GetWidth()));
	hzooml = (int) ceil(100.0 * (double) (clientsize.GetHeight() - 2 * OPJ_CANVAS_BORDER) / (double) (currcanvas->m_image100.GetHeight()));
	zooml = wxMin(100, wxMin(wzooml, hzooml));

	// fit to width
	Rescale(zooml, currchild);
}

void OPJFrame::OnZoom(wxCommandEvent& WXUNUSED(event))
{
	// current frame
	OPJChildFrame *currframe = (OPJChildFrame *) GetActiveChild();

	if (!currframe)
		return;

	// get the preferred zoom
	long zooml = wxGetNumberFromUser(wxT("Choose a scale between 5% and 300%"),
		wxT("Zoom (%)"),
		wxT("Image scale"),
		currframe->m_canvas->m_zooml, 5, 300, NULL, wxDefaultPosition);

	// rescale current frame image if necessary
	if (zooml >= 5) {
		Rescale(zooml, currframe);
		wxLogMessage(wxT("zoom to %d%%"), zooml);
	}
}

void OPJFrame::Rescale(int zooml, OPJChildFrame *currframe)
{
	wxImage new_image = currframe->m_canvas->m_image100.ConvertToImage();
	if (zooml != 100)
		new_image.Rescale((int) ((double) zooml * (double) new_image.GetWidth() / 100.0),
			(int) ((double) zooml * (double) new_image.GetHeight() / 100.0),
			wxGetApp().m_resizemethod ? wxIMAGE_QUALITY_HIGH : wxIMAGE_QUALITY_NORMAL);
    currframe->m_canvas->m_image = wxBitmap(new_image);
	currframe->m_canvas->SetScrollbars(20,
										20,
										(int)(0.5 + (double) new_image.GetWidth() / 20.0),
										(int)(0.5 + (double) new_image.GetHeight() / 20.0)
										);
	currframe->m_canvas->Refresh();

	// update zoom
	currframe->m_canvas->m_zooml = zooml;
}


void OPJFrame::OnReload(wxCommandEvent& event)
{
	OPJChildFrame *currframe = (OPJChildFrame *) GetActiveChild();

    OPJDecoThread *dthread = currframe->m_canvas->CreateDecoThread();

    if (dthread->Run() != wxTHREAD_NO_ERROR)
        wxLogMessage(wxT("Can't start deco thread!"));
    else
		wxLogMessage(wxT("New deco thread started."));

	currframe->m_canvas->Refresh();

	// update zoom
	//currframe->m_canvas->m_zooml = zooml;
}


// about window for the frame
void OPJFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
#ifdef OPJ_HTMLABOUT
#include "about_htm.h"
#include "opj_logo.xpm"

    wxBoxSizer *topsizer;
    wxHtmlWindow *html;
    wxDialog dlg(this, wxID_ANY, wxString(_("About")));

    wxMemoryFSHandler::AddFile(wxT("opj_logo.xpm"), wxBitmap(opj_logo), wxBITMAP_TYPE_XPM);

    topsizer = new wxBoxSizer(wxVERTICAL);

    html = new wxHtmlWindow(&dlg, wxID_ANY, wxDefaultPosition, wxSize(320, 250), wxHW_SCROLLBAR_NEVER);
    html->SetBorders(0);
    //html->LoadPage(wxT("about/about.htm"));
	//html->SetPage("<html><body>Hello, world!</body></html>");
	html->SetPage(htmlaboutpage);
    html->SetSize(html->GetInternalRepresentation()->GetWidth(),
                    html->GetInternalRepresentation()->GetHeight());

    topsizer->Add(html, 1, wxALL, 10);

    topsizer->Add(new wxStaticLine(&dlg, wxID_ANY), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    wxButton *bu1 = new wxButton(&dlg, wxID_OK, wxT("OK"));
    bu1->SetDefault();

    topsizer->Add(bu1, 0, wxALL | wxALIGN_RIGHT, 15);

    dlg.SetSizer(topsizer);
    topsizer->Fit(&dlg);

    dlg.ShowModal();

#else

	wxMessageBox(wxString::Format(OPJ_APPLICATION_TITLEBAR
								  wxT("\n\n")
								  wxT("Built with %s and OpenJPEG ")
								  wxT(OPENJPEG_VERSION)
								  wxT("\non ") wxT(__DATE__) wxT(", ") wxT(__TIME__)
								  wxT("\nRunning under %s\n\n")
								  OPJ_APPLICATION_COPYRIGHT,
								  wxVERSION_STRING,
								  wxGetOsDescription().c_str()),
				 wxT("About ") OPJ_APPLICATION_NAME,
				 wxOK | wxICON_INFORMATION,
				 this
				 );

#endif

}

void OPJFrame::OnToggleBrowser(wxCommandEvent& WXUNUSED(event))
{
    if (markerTreeWindow->IsShown())
        markerTreeWindow->Show(false);
    else
        markerTreeWindow->Show(true);

    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
}

void OPJFrame::OnTogglePeeker(wxCommandEvent& WXUNUSED(event))
{
    if (loggingWindow->IsShown())
        loggingWindow->Show(false);
    else
        loggingWindow->Show(true);

    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
}

void OPJFrame::OnSashDrag(wxSashEvent& event)
{
    if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;

    switch (event.GetId()) {
		case OPJFRAME_BROWSEWIN:
		{
			markerTreeWindow->SetDefaultSize(wxSize(event.GetDragRect().width, 1000));
			break;
		}
		case OPJFRAME_LOGWIN:
		{
			loggingWindow->SetDefaultSize(wxSize(1000, event.GetDragRect().height));
			break;
		}
    }

    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);

    // Leaves bits of itself behind sometimes
    GetClientWindow()->Refresh();
}

// physically open the files
void OPJFrame::OpenFiles(wxArrayString paths, wxArrayString filenames)
{

	size_t count = paths.GetCount();
	for (size_t n = 0; n < count; n++) {

		wxString msg, s;
		s.Printf(_T("File %d: %s (%s)\n"), (int)n, paths[n].c_str(), filenames[n].c_str());

		msg += s;

		/*wxMessageDialog dialog2(this, msg, _T("Selected files"));
		dialog2.ShowModal();*/

		// Make another frame, containing a canvas
		OPJChildFrame *subframe = new OPJChildFrame(this,
													paths[n],
													winNumber,
													wxT("Canvas Frame"),
													wxDefaultPosition, wxSize(300, 300),
													wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE
													);
		m_childhash[winNumber] = subframe;

		// create own marker tree
		m_treehash[winNumber] = new OPJMarkerTree(m_bookCtrl, subframe, paths[n], wxT("Parsing..."), TreeTest_Ctrl,
												  wxDefaultPosition, wxDefaultSize,
												  wxTR_DEFAULT_STYLE | wxSUNKEN_BORDER
												  );

		m_bookCtrl->AddPage(m_treehash[winNumber], wxString::Format(wxT("%u"), winNumber), false);

		for (unsigned int p = 0; p < m_bookCtrl->GetPageCount(); p++) {
			if (m_bookCtrl->GetPageText(p) == wxString::Format(wxT("%u"), winNumber)) {
				m_bookCtrl->ChangeSelection(p);
				break;
			}
		}

		winNumber++;
	}
}

void OPJFrame::OnFileOpen(wxCommandEvent& WXUNUSED(event))
{
    wxString wildcards =
#ifdef __WXMOTIF__
	wxT("JPEG 2000 files (*.jp2,*.j2k,*.j2c,*.mj2)|*.*j*2*");
#else
#if wxUSE_LIBOPENJPEG
	wxT("JPEG 2000 files (*.jp2,*.j2k,*.j2c,*.mj2)|*.jp2;*.j2k;*.j2c;*.mj2")
#endif
#if wxUSE_LIBJPEG
		wxT("|JPEG files (*.jpg)|*.jpg")
#endif
#if OPJ_MANYFORMATS
		wxT("|BMP files (*.bmp)|*.bmp")
		wxT("|PNG files (*.png)|*.png")
		wxT("|GIF files (*.gif)|*.gif")
		wxT("|PNM files (*.pnm)|*.pnm")
		wxT("|TIFF files (*.tif,*.tiff)|*.tif*")
#endif
		wxT("|All files|*");
#endif
    wxFileDialog dialog(this, _T("Open image file(s)"),
                        wxEmptyString, wxEmptyString, wildcards,
                        wxFD_OPEN|wxFD_MULTIPLE);

    if (dialog.ShowModal() == wxID_OK) {
        wxArrayString paths, filenames;

        dialog.GetPaths(paths);
        dialog.GetFilenames(filenames);

		OpenFiles(paths, filenames);
    }

}

BEGIN_EVENT_TABLE(OPJCanvas, wxScrolledWindow)
    EVT_MOUSE_EVENTS(OPJCanvas::OnEvent)
END_EVENT_TABLE()

// Define a constructor for my canvas
OPJCanvas::OPJCanvas(wxFileName fname, wxWindow *parent, const wxPoint& pos, const wxSize& size)
        : wxScrolledWindow(parent, wxID_ANY, pos, size,
                           wxSUNKEN_BORDER | wxNO_FULL_REPAINT_ON_RESIZE)
{
    SetBackgroundColour(OPJ_CANVAS_COLOUR);

	m_fname = fname;
	m_childframe = (OPJChildFrame *) parent;
	// 100% zoom
	m_zooml = 100;


    OPJDecoThread *dthread = CreateDecoThread();

    if (dthread->Run() != wxTHREAD_NO_ERROR)
        wxLogMessage(wxT("Can't start deco thread!"));
    else
		wxLogMessage(wxT("New deco thread started."));

	// 100% zoom
	//m_zooml = 100;

}

OPJDecoThread *OPJCanvas::CreateDecoThread(void)
{
    OPJDecoThread *dthread = new OPJDecoThread(this);

    if (dthread->Create() != wxTHREAD_NO_ERROR)
		wxLogError(wxT("Can't create deco thread!"));

    wxCriticalSectionLocker enter(wxGetApp().m_deco_critsect);
    wxGetApp().m_deco_threads.Add(dthread);

    return dthread;
}

#define activeoverlay 0
// Define the repainting behaviour
void OPJCanvas::OnDraw(wxDC& dc)
{
	if (m_image.Ok()) {
		dc.DrawBitmap(m_image, OPJ_CANVAS_BORDER, OPJ_CANVAS_BORDER);

		if (activeoverlay) {
			dc.SetPen(*wxRED_PEN);
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			//int tw, th;
			dc.DrawRectangle(OPJ_CANVAS_BORDER, OPJ_CANVAS_BORDER,
				(unsigned long int) (0.5 + (double) m_zooml * (double) m_childframe->m_twidth / 100.0),
				(unsigned long int) (0.5 + (double) m_zooml * (double) m_childframe->m_theight / 100.0));
		}

	} else {
		dc.SetFont(*wxSWISS_FONT);
		dc.SetPen(*wxBLACK_PEN);
		dc.DrawText(_T("Decoding image, please wait..."), 40, 50);
	}
}

// This implements a tiny doodling program! Drag the mouse using
// the left button.
void OPJCanvas::OnEvent(wxMouseEvent& event)
{
#if USE_PENCIL_ON_CANVAS
  wxClientDC dc(this);
  PrepareDC(dc);

  wxPoint pt(event.GetLogicalPosition(dc));

  if ((xpos > -1) && (ypos > -1) && event.Dragging()) {
    dc.SetPen(*wxRED_PEN);
    dc.DrawLine(xpos, ypos, pt.x, pt.y);
  }
  xpos = pt.x;
  ypos = pt.y;
#endif
}

void OPJFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
}

// Note that OPJFRAME_FILEOPEN and OPJFRAME_HELPABOUT commands get passed
// to the parent window for processing, so no need to
// duplicate event handlers here.

BEGIN_EVENT_TABLE(OPJChildFrame, wxMDIChildFrame)
  /*EVT_MENU(SASHTEST_CHILD_QUIT, OPJChildFrame::OnQuit)*/
  EVT_CLOSE(OPJChildFrame::OnClose)
  EVT_SET_FOCUS(OPJChildFrame::OnGotFocus)
  EVT_KILL_FOCUS(OPJChildFrame::OnLostFocus)
END_EVENT_TABLE()

OPJChildFrame::OPJChildFrame(OPJFrame *parent, wxFileName fname, int winnumber, const wxString& title, const wxPoint& pos, const wxSize& size,
const long style):
  wxMDIChildFrame(parent, wxID_ANY, title, pos, size, style)
{
	m_frame = (OPJFrame  *) parent;
	m_canvas = NULL;
	//my_children.Append(this);
	m_fname = fname;
	m_winnumber = winnumber;
	SetTitle(wxString::Format(_T("%d: "), m_winnumber) + m_fname.GetFullName());

	  // Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
	SetIcon(wxIcon(wxT("OPJChild16")));
#endif

	// Give it a status line
	/*CreateStatusBar();*/

	int width, height;
	GetClientSize(&width, &height);

	OPJCanvas *canvas = new OPJCanvas(fname, this, wxPoint(0, 0), wxSize(width, height));
#if USE_PENCIL_ON_CANVAS
	canvas->SetCursor(wxCursor(wxCURSOR_PENCIL));
#endif
	m_canvas = canvas;

	// Give it scrollbars
	canvas->SetScrollbars(20, 20, 5, 5);

	Show(true);
	Maximize(true);

	/*wxLogError(wxString::Format(wxT("Created tree %d (0x%x)"), m_winnumber, m_frame->m_treehash[m_winnumber]));*/

}

OPJChildFrame::~OPJChildFrame(void)
{
  //my_children.DeleteObject(this);
}


void OPJChildFrame::OnClose(wxCloseEvent& event)
{
	for (unsigned int p = 0; p < m_frame->m_bookCtrl->GetPageCount(); p++) {
		if (m_frame->m_bookCtrl->GetPageText(p) == wxString::Format(wxT("%u"), m_winnumber)) {
			m_frame->m_bookCtrl->DeletePage(p);
			break;
		}
	}
	Destroy();

	wxLogMessage(wxT("Closed: %d"), m_winnumber);
}

void OPJChildFrame::OnActivate(wxActivateEvent& event)
{
  /*if (event.GetActive() && m_canvas)
    m_canvas->SetFocus();*/
}

void OPJChildFrame::OnGotFocus(wxFocusEvent& event)
{
	// we need to check if the notebook is being destroyed or not
	if (!m_frame->m_bookCtrl)
		return;

	for (unsigned int p = 0; p < m_frame->m_bookCtrl->GetPageCount(); p++) {

		if (m_frame->m_bookCtrl->GetPageText(p) == wxString::Format(wxT("%u"), m_winnumber)) {
			m_frame->m_bookCtrl->ChangeSelection(p);
			break;
		}

	}

	//wxLogMessage(wxT("Got focus: %d (%x)"), m_winnumber, event.GetWindow());
}

void OPJChildFrame::OnLostFocus(wxFocusEvent& event)
{
	//wxLogMessage(wxT("Lost focus: %d (%x)"), m_winnumber, event.GetWindow());
}

#if USE_GENERIC_TREECTRL
BEGIN_EVENT_TABLE(OPJMarkerTree, wxGenericTreeCtrl)
#else
BEGIN_EVENT_TABLE(OPJMarkerTree, wxTreeCtrl)
#endif
    /*EVT_TREE_BEGIN_DRAG(TreeTest_Ctrl, OPJMarkerTree::OnBeginDrag)
    EVT_TREE_BEGIN_RDRAG(TreeTest_Ctrl, OPJMarkerTree::OnBeginRDrag)
    EVT_TREE_END_DRAG(TreeTest_Ctrl, OPJMarkerTree::OnEndDrag)*/
    /*EVT_TREE_BEGIN_LABEL_EDIT(TreeTest_Ctrl, OPJMarkerTree::OnBeginLabelEdit)
    EVT_TREE_END_LABEL_EDIT(TreeTest_Ctrl, OPJMarkerTree::OnEndLabelEdit)*/
    /*EVT_TREE_DELETE_ITEM(TreeTest_Ctrl, OPJMarkerTree::OnDeleteItem)*/
#if 0       // there are so many of those that logging them causes flicker
    /*EVT_TREE_GET_INFO(TreeTest_Ctrl, OPJMarkerTree::OnGetInfo)*/
#endif
    /*EVT_TREE_SET_INFO(TreeTest_Ctrl, OPJMarkerTree::OnSetInfo)
    EVT_TREE_ITEM_EXPANDED(TreeTest_Ctrl, OPJMarkerTree::OnItemExpanded)*/
    EVT_TREE_ITEM_EXPANDING(TreeTest_Ctrl, OPJMarkerTree::OnItemExpanding)
    /*EVT_TREE_ITEM_COLLAPSED(TreeTest_Ctrl, OPJMarkerTree::OnItemCollapsed)
    EVT_TREE_ITEM_COLLAPSING(TreeTest_Ctrl, OPJMarkerTree::OnItemCollapsing)*/

    EVT_TREE_SEL_CHANGED(TreeTest_Ctrl, OPJMarkerTree::OnSelChanged)
    /*EVT_TREE_SEL_CHANGING(TreeTest_Ctrl, OPJMarkerTree::OnSelChanging)*/
    /*EVT_TREE_KEY_DOWN(TreeTest_Ctrl, OPJMarkerTree::OnTreeKeyDown)*/
    /*EVT_TREE_ITEM_ACTIVATED(TreeTest_Ctrl, OPJMarkerTree::OnItemActivated)*/

    // so many differents ways to handle right mouse button clicks...
    /*EVT_CONTEXT_MENU(OPJMarkerTree::OnContextMenu)*/
    // EVT_TREE_ITEM_MENU is the preferred event for creating context menus
    // on a tree control, because it includes the point of the click or item,
    // meaning that no additional placement calculations are required.
    EVT_TREE_ITEM_MENU(TreeTest_Ctrl, OPJMarkerTree::OnItemMenu)
    /*EVT_TREE_ITEM_RIGHT_CLICK(TreeTest_Ctrl, OPJMarkerTree::OnItemRClick)*/

    /*EVT_RIGHT_DOWN(OPJMarkerTree::OnRMouseDown)
    EVT_RIGHT_UP(OPJMarkerTree::OnRMouseUp)
    EVT_RIGHT_DCLICK(OPJMarkerTree::OnRMouseDClick)*/
END_EVENT_TABLE()

// OPJMarkerTree implementation
#if USE_GENERIC_TREECTRL
IMPLEMENT_DYNAMIC_CLASS(OPJMarkerTree, wxGenericTreeCtrl)
#else
IMPLEMENT_DYNAMIC_CLASS(OPJMarkerTree, wxTreeCtrl)
#endif

OPJMarkerTree::OPJMarkerTree(wxWindow *parent, OPJChildFrame *subframe, wxFileName fname, wxString name, const wxWindowID id,
           const wxPoint& pos, const wxSize& size, long style)
          : wxTreeCtrl(parent, id, pos, size, style)
{
    m_reverseSort = false;
	m_fname = fname;

	m_peektextCtrl = ((OPJFrame *) (parent->GetParent()->GetParent()))->m_textCtrlbrowse;
    CreateImageList();

    // Add some items to the tree
    //AddTestItemsToTree(5, 5);
    int image = wxGetApp().ShowImages() ? OPJMarkerTree::TreeCtrlIcon_Folder : -1;
    wxTreeItemId rootId = AddRoot(name,
                                  image, image,
                                  new OPJMarkerData(name));

    OPJParseThread *pthread = CreateParseThread(0x00, subframe);
    if (pthread->Run() != wxTHREAD_NO_ERROR)
        wxLogMessage(wxT("Can't start parse thread!"));
    else
		wxLogMessage(wxT("New parse thread started."));

	m_childframe = subframe;
}

void OPJMarkerTree::CreateImageList(int size)
{
    if (size == -1) {
        SetImageList(NULL);
        return;
    }
    if (size == 0)
        size = m_imageSize;
    else
        m_imageSize = size;

    // Make an image list containing small icons
    wxImageList *images = new wxImageList(size, size, true);

    // should correspond to TreeCtrlIcon_xxx enum
    wxBusyCursor wait;
    wxIcon icons[5];
    icons[0] = wxIcon(icon1_xpm);
    icons[1] = wxIcon(icon2_xpm);
    icons[2] = wxIcon(icon3_xpm);
    icons[3] = wxIcon(icon4_xpm);
    icons[4] = wxIcon(icon5_xpm);

    int sizeOrig = icons[0].GetWidth();
    for (size_t i = 0; i < WXSIZEOF(icons); i++) {
        if (size == sizeOrig) {
            images->Add(icons[i]);
        } else {
            images->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size)));
        }
    }

    AssignImageList(images);
}

#if USE_GENERIC_TREECTRL || !defined(__WXMSW__)
void OPJMarkerTree::CreateButtonsImageList(int size)
{
    if ( size == -1 ) {
        SetButtonsImageList(NULL);
        return;
    }

    // Make an image list containing small icons
    wxImageList *images = new wxImageList(size, size, true);

    // should correspond to TreeCtrlIcon_xxx enum
    wxBusyCursor wait;
    wxIcon icons[4];
    icons[0] = wxIcon(icon3_xpm);   // closed
    icons[1] = wxIcon(icon3_xpm);   // closed, selected
    icons[2] = wxIcon(icon5_xpm);   // open
    icons[3] = wxIcon(icon5_xpm);   // open, selected

    for ( size_t i = 0; i < WXSIZEOF(icons); i++ ) {
        int sizeOrig = icons[i].GetWidth();
        if ( size == sizeOrig ) {
            images->Add(icons[i]);
        } else {
            images->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size)));
        }
    }

    AssignButtonsImageList(images);
#else
void OPJMarkerTree::CreateButtonsImageList(int WXUNUSED(size))
{
#endif
}

void OPJParseThread::LoadFile(wxFileName fname)
{
	wxTreeItemId rootid;

	// this is the root node
	int image = wxGetApp().ShowImages() ? m_tree->TreeCtrlIcon_Folder : -1;

	if (this->m_parentid) {
		// leaf of a tree
		rootid = m_parentid;
		m_tree->SetItemText(rootid, wxT("Parsing..."));

	} else {

		// delete the existing tree hierarchy
		m_tree->DeleteAllItems();

		// new tree
		rootid = m_tree->AddRoot(wxT("Parsing..."),
			image,
			image,
			new OPJMarkerData(fname.GetFullPath())
			);
		//m_tree->SetItemFont(rootid, *wxITALIC_FONT);
		m_tree->SetItemBold(rootid);
	}

	// open the file
	wxFile m_file(fname.GetFullPath().c_str(), wxFile::read);

	// what is the extension?
	if ((fname.GetExt() == wxT("j2k")) || (fname.GetExt() == wxT("j2c"))) {

		// parse the file
		ParseJ2KFile(&m_file, 0, m_file.Length(), rootid);

	} else if ((fname.GetExt() == wxT("jp2")) || (fname.GetExt() == wxT("mj2"))) {

		// parse the file
		if (this->m_parentid) {
			//WriteText(wxT("Only a subsection of jp2"));
			OPJMarkerData *data = (OPJMarkerData *) m_tree->GetItemData(rootid);
			ParseJ2KFile(&m_file, data->m_start, data->m_length, rootid);
			m_tree->Expand(rootid);

		} else
			// as usual
			ParseJP2File(&m_file, 0, m_file.Length(), rootid);

	} else {

		// unknown extension
		WriteText(wxT("Unknown file format!"));

	}

	// this is the root node
	if (this->m_parentid)
		m_tree->SetItemText(rootid, wxT("Codestream"));
	else
		//m_tree->SetItemText(rootid, wxString::Format(wxT("%s (%d B)"), fname.GetFullName(), m_file.Length()));
		m_tree->SetItemText(rootid, fname.GetFullName());

	// close the file
	m_file.Close();

	WriteText(wxT("Parsing finished!"));
}

/*int OPJMarkerTree::OnCompareItems(const wxTreeItemId& item1,
                               const wxTreeItemId& item2)
{
    if ( m_reverseSort )
    {
        // just exchange 1st and 2nd items
        return wxTreeCtrl::OnCompareItems(item2, item1);
    }
    else
    {
        return wxTreeCtrl::OnCompareItems(item1, item2);
    }
}*/

/*void OPJMarkerTree::AddItemsRecursively(const wxTreeItemId& idParent,
                                     size_t numChildren,
                                     size_t depth,
                                     size_t folder)
{
    if ( depth > 0 )
    {
        bool hasChildren = depth > 1;

        wxString str;
        for ( size_t n = 0; n < numChildren; n++ )
        {
            // at depth 1 elements won't have any more children
            if ( hasChildren )
                str.Printf(wxT("%s child %u"), wxT("Folder"), unsigned(n + 1));
            else
                str.Printf(wxT("%s child %u.%u"), wxT("File"), unsigned(folder), unsigned(n + 1));

            // here we pass to AppendItem() normal and selected item images (we
            // suppose that selected image follows the normal one in the enum)
            int image, imageSel;
            if ( wxGetApp().ShowImages() )
            {
                image = depth == 1 ? TreeCtrlIcon_File : TreeCtrlIcon_Folder;
                imageSel = image + 1;
            }
            else
            {
                image = imageSel = -1;
            }
            wxTreeItemId id = AppendItem(idParent, str, image, imageSel,
                                         new OPJMarkerData(str));

            // and now we also set the expanded one (only for the folders)
            if ( hasChildren && wxGetApp().ShowImages() )
            {
                SetItemImage(id, TreeCtrlIcon_FolderOpened,
                             wxTreeItemIcon_Expanded);
            }

            // remember the last child for OnEnsureVisible()
            if ( !hasChildren && n == numChildren - 1 )
            {
                m_lastItem = id;
            }

            AddItemsRecursively(id, numChildren, depth - 1, n + 1);
        }
    }
    //else: done!
}*/

/*void OPJMarkerTree::AddTestItemsToTree(size_t numChildren,
                                    size_t depth)
{
    int image = wxGetApp().ShowImages() ? OPJMarkerTree::TreeCtrlIcon_Folder : -1;
    wxTreeItemId rootId = AddRoot(wxT("Root"),
                                  image, image,
                                  new OPJMarkerData(wxT("Root item")));
    if ( image != -1 )
    {
        SetItemImage(rootId, TreeCtrlIcon_FolderOpened, wxTreeItemIcon_Expanded);
    }

    AddItemsRecursively(rootId, numChildren, depth, 0);

    // set some colours/fonts for testing
    SetItemFont(rootId, *wxITALIC_FONT);

    wxTreeItemIdValue cookie;
    wxTreeItemId id = GetFirstChild(rootId, cookie);
    SetItemTextColour(id, *wxBLUE);

    id = GetNextChild(rootId, cookie);
    id = GetNextChild(rootId, cookie);
    SetItemTextColour(id, *wxRED);
    SetItemBackgroundColour(id, *wxLIGHT_GREY);
}*/

/*void OPJMarkerTree::GetItemsRecursively(const wxTreeItemId& idParent,
                                     wxTreeItemIdValue cookie)
{
    wxTreeItemId id;

    if ( !cookie )
        id = GetFirstChild(idParent, cookie);
    else
        id = GetNextChild(idParent, cookie);

    if ( !id.IsOk() )
        return;

    wxString text = GetItemText(id);
    wxLogMessage(text);

    if (ItemHasChildren(id))
        GetItemsRecursively(id);

    GetItemsRecursively(idParent, cookie);
}*/

/*void OPJMarkerTree::DoToggleIcon(const wxTreeItemId& item)
{
    int image = (GetItemImage(item) == TreeCtrlIcon_Folder)
                    ? TreeCtrlIcon_File
                    : TreeCtrlIcon_Folder;
    SetItemImage(item, image, wxTreeItemIcon_Normal);

    image = (GetItemImage(item) == TreeCtrlIcon_FolderSelected)
                    ? TreeCtrlIcon_FileSelected
                    : TreeCtrlIcon_FolderSelected;
    SetItemImage(item, image, wxTreeItemIcon_Selected);
}*/

void OPJMarkerTree::LogEvent(const wxChar *name, const wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    wxString text;
    if ( item.IsOk() )
        text << wxT('"') << GetItemText(item).c_str() << wxT('"');
    else
        text = wxT("invalid item");
    wxLogMessage(wxT("%s(%s)"), name, text.c_str());
}

OPJParseThread *OPJMarkerTree::CreateParseThread(wxTreeItemId parentid, OPJChildFrame *subframe)
{
    OPJParseThread *pthread = new OPJParseThread(this, parentid);

    if (pthread->Create() != wxTHREAD_NO_ERROR)
		wxLogError(wxT("Can't create parse thread!"));

    wxCriticalSectionLocker enter(wxGetApp().m_parse_critsect);
    wxGetApp().m_parse_threads.Add(pthread);

    return pthread;
}


/*// avoid repetition
#define TREE_EVENT_HANDLER(name)                                 \
void OPJMarkerTree::name(wxTreeEvent& event)                        \
{                                                                \
    LogEvent(_T(#name), event);                                  \
    SetLastItem(wxTreeItemId());                                 \
    event.Skip();                                                \
}*/

/*TREE_EVENT_HANDLER(OnBeginRDrag)*/
/*TREE_EVENT_HANDLER(OnDeleteItem)*/
/*TREE_EVENT_HANDLER(OnGetInfo)
TREE_EVENT_HANDLER(OnSetInfo)*/
/*TREE_EVENT_HANDLER(OnItemExpanded)
TREE_EVENT_HANDLER(OnItemExpanding)*/
/*TREE_EVENT_HANDLER(OnItemCollapsed)*/
/*TREE_EVENT_HANDLER(OnSelChanged)
TREE_EVENT_HANDLER(OnSelChanging)*/

/*#undef TREE_EVENT_HANDLER*/

void OPJMarkerTree::OnItemExpanding(wxTreeEvent& event)
{
	wxTreeItemId item = event.GetItem();
	OPJMarkerData* data = (OPJMarkerData *) GetItemData(item);
	wxString text;

	if (item.IsOk())
		text << wxT('"') << GetItemText(item).c_str() << wxT('"');
	else
		text = wxT("invalid item");

	if (wxStrcmp(data->GetDesc1(), wxT("INFO-CSTREAM")))
		return;

	wxLogMessage(wxT("Expanding... (%s -> %s, %s, %d, %d)"),
		text.c_str(), data->GetDesc1(), data->GetDesc2(),
		data->m_start, data->m_length);

	// the codestream box is being asked for expansion
	wxTreeItemIdValue cookie;
	if (!GetFirstChild(item, cookie).IsOk()) {
		OPJParseThread *pthread = CreateParseThread(item);
		if (pthread->Run() != wxTHREAD_NO_ERROR)
			wxLogMessage(wxT("Can't start parse thread!"));
		else
			wxLogMessage(wxT("New parse thread started."));
	}
}

void OPJMarkerTree::OnSelChanged(wxTreeEvent& event)
{
#define BUNCH_LINESIZE	16
#define BUNCH_NUMLINES	7

	wxTreeItemId item = event.GetItem();
	OPJMarkerData* data = (OPJMarkerData *) GetItemData(item);
	wxString text;
	int l, c, pos = 0, pre_pos;
	unsigned char buffer[BUNCH_LINESIZE * BUNCH_NUMLINES];

	m_peektextCtrl->Clear();

	/*text << wxString::Format(wxT("Selected... (%s -> %s, %s, %d, %d)"),
		text.c_str(), data->GetDesc1(), data->GetDesc2(),
		data->m_start, data->m_length) << wxT("\n");*/

	// open the file and browse a little
	wxFile *fp = new wxFile(m_fname.GetFullPath().c_str(), wxFile::read);

	// go to position claimed
	fp->Seek(data->m_start, wxFromStart);

	// read a bunch
	int max_read = wxMin(wxFileOffset(WXSIZEOF(buffer)), data->m_length - data->m_start + 1);
	fp->Read(buffer, max_read);

	// write the file data between start and stop
	pos = 0;
	for (l = 0; l < BUNCH_NUMLINES; l++) {

		text << wxString::Format(wxT("%010d:"), data->m_start + pos);

		pre_pos = pos;

		// add hex browsing text
		for (c = 0; c < BUNCH_LINESIZE; c++) {

			if (!(c % 8))
				text << wxT(" ");

			if (pos < max_read) {
				text << wxString::Format(wxT("%02X "), buffer[pos]);
			} else
				text << wxT("   ");
			pos++;
		}

		text << wxT("    ");

		// add char browsing text
		for (c = 0; c < BUNCH_LINESIZE; c++) {

			if (pre_pos < max_read) {
				if ((buffer[pre_pos] == '\n') ||
					(buffer[pre_pos] == '\t') ||
					(buffer[pre_pos] == '\0') ||
					(buffer[pre_pos] == 0x0D) ||
					(buffer[pre_pos] == 0x0B))
					buffer[pre_pos] = ' ';
				text << wxString::Format(wxT("%c."), wxChar(buffer[pre_pos]));
			} else
				text << wxT("  ");
			pre_pos++;
		}

		text << wxT("\n");

	}

	// close the file
	fp->Close();

	m_peektextCtrl->WriteText(text);
}

/*void LogKeyEvent(const wxChar *name, const wxKeyEvent& event)
{
    wxString key;
    long keycode = event.GetKeyCode();
    {
        switch ( keycode )
        {
            case WXK_BACK: key = wxT("BACK"); break;
            case WXK_TAB: key = wxT("TAB"); break;
            case WXK_RETURN: key = wxT("RETURN"); break;
            case WXK_ESCAPE: key = wxT("ESCAPE"); break;
            case WXK_SPACE: key = wxT("SPACE"); break;
            case WXK_DELETE: key = wxT("DELETE"); break;
            case WXK_START: key = wxT("START"); break;
            case WXK_LBUTTON: key = wxT("LBUTTON"); break;
            case WXK_RBUTTON: key = wxT("RBUTTON"); break;
            case WXK_CANCEL: key = wxT("CANCEL"); break;
            case WXK_MBUTTON: key = wxT("MBUTTON"); break;
            case WXK_CLEAR: key = wxT("CLEAR"); break;
            case WXK_SHIFT: key = wxT("SHIFT"); break;
            case WXK_ALT: key = wxT("ALT"); break;
            case WXK_CONTROL: key = wxT("CONTROL"); break;
            case WXK_MENU: key = wxT("MENU"); break;
            case WXK_PAUSE: key = wxT("PAUSE"); break;
            case WXK_CAPITAL: key = wxT("CAPITAL"); break;
            case WXK_END: key = wxT("END"); break;
            case WXK_HOME: key = wxT("HOME"); break;
            case WXK_LEFT: key = wxT("LEFT"); break;
            case WXK_UP: key = wxT("UP"); break;
            case WXK_RIGHT: key = wxT("RIGHT"); break;
            case WXK_DOWN: key = wxT("DOWN"); break;
            case WXK_SELECT: key = wxT("SELECT"); break;
            case WXK_PRINT: key = wxT("PRINT"); break;
            case WXK_EXECUTE: key = wxT("EXECUTE"); break;
            case WXK_SNAPSHOT: key = wxT("SNAPSHOT"); break;
            case WXK_INSERT: key = wxT("INSERT"); break;
            case WXK_HELP: key = wxT("HELP"); break;
            case WXK_NUMPAD0: key = wxT("NUMPAD0"); break;
            case WXK_NUMPAD1: key = wxT("NUMPAD1"); break;
            case WXK_NUMPAD2: key = wxT("NUMPAD2"); break;
            case WXK_NUMPAD3: key = wxT("NUMPAD3"); break;
            case WXK_NUMPAD4: key = wxT("NUMPAD4"); break;
            case WXK_NUMPAD5: key = wxT("NUMPAD5"); break;
            case WXK_NUMPAD6: key = wxT("NUMPAD6"); break;
            case WXK_NUMPAD7: key = wxT("NUMPAD7"); break;
            case WXK_NUMPAD8: key = wxT("NUMPAD8"); break;
            case WXK_NUMPAD9: key = wxT("NUMPAD9"); break;
            case WXK_MULTIPLY: key = wxT("MULTIPLY"); break;
            case WXK_ADD: key = wxT("ADD"); break;
            case WXK_SEPARATOR: key = wxT("SEPARATOR"); break;
            case WXK_SUBTRACT: key = wxT("SUBTRACT"); break;
            case WXK_DECIMAL: key = wxT("DECIMAL"); break;
            case WXK_DIVIDE: key = wxT("DIVIDE"); break;
            case WXK_F1: key = wxT("F1"); break;
            case WXK_F2: key = wxT("F2"); break;
            case WXK_F3: key = wxT("F3"); break;
            case WXK_F4: key = wxT("F4"); break;
            case WXK_F5: key = wxT("F5"); break;
            case WXK_F6: key = wxT("F6"); break;
            case WXK_F7: key = wxT("F7"); break;
            case WXK_F8: key = wxT("F8"); break;
            case WXK_F9: key = wxT("F9"); break;
            case WXK_F10: key = wxT("F10"); break;
            case WXK_F11: key = wxT("F11"); break;
            case WXK_F12: key = wxT("F12"); break;
            case WXK_F13: key = wxT("F13"); break;
            case WXK_F14: key = wxT("F14"); break;
            case WXK_F15: key = wxT("F15"); break;
            case WXK_F16: key = wxT("F16"); break;
            case WXK_F17: key = wxT("F17"); break;
            case WXK_F18: key = wxT("F18"); break;
            case WXK_F19: key = wxT("F19"); break;
            case WXK_F20: key = wxT("F20"); break;
            case WXK_F21: key = wxT("F21"); break;
            case WXK_F22: key = wxT("F22"); break;
            case WXK_F23: key = wxT("F23"); break;
            case WXK_F24: key = wxT("F24"); break;
            case WXK_NUMLOCK: key = wxT("NUMLOCK"); break;
            case WXK_SCROLL: key = wxT("SCROLL"); break;
            case WXK_PAGEUP: key = wxT("PAGEUP"); break;
            case WXK_PAGEDOWN: key = wxT("PAGEDOWN"); break;
            case WXK_NUMPAD_SPACE: key = wxT("NUMPAD_SPACE"); break;
            case WXK_NUMPAD_TAB: key = wxT("NUMPAD_TAB"); break;
            case WXK_NUMPAD_ENTER: key = wxT("NUMPAD_ENTER"); break;
            case WXK_NUMPAD_F1: key = wxT("NUMPAD_F1"); break;
            case WXK_NUMPAD_F2: key = wxT("NUMPAD_F2"); break;
            case WXK_NUMPAD_F3: key = wxT("NUMPAD_F3"); break;
            case WXK_NUMPAD_F4: key = wxT("NUMPAD_F4"); break;
            case WXK_NUMPAD_HOME: key = wxT("NUMPAD_HOME"); break;
            case WXK_NUMPAD_LEFT: key = wxT("NUMPAD_LEFT"); break;
            case WXK_NUMPAD_UP: key = wxT("NUMPAD_UP"); break;
            case WXK_NUMPAD_RIGHT: key = wxT("NUMPAD_RIGHT"); break;
            case WXK_NUMPAD_DOWN: key = wxT("NUMPAD_DOWN"); break;
            case WXK_NUMPAD_PAGEUP: key = wxT("NUMPAD_PAGEUP"); break;
            case WXK_NUMPAD_PAGEDOWN: key = wxT("NUMPAD_PAGEDOWN"); break;
            case WXK_NUMPAD_END: key = wxT("NUMPAD_END"); break;
            case WXK_NUMPAD_BEGIN: key = wxT("NUMPAD_BEGIN"); break;
            case WXK_NUMPAD_INSERT: key = wxT("NUMPAD_INSERT"); break;
            case WXK_NUMPAD_DELETE: key = wxT("NUMPAD_DELETE"); break;
            case WXK_NUMPAD_EQUAL: key = wxT("NUMPAD_EQUAL"); break;
            case WXK_NUMPAD_MULTIPLY: key = wxT("NUMPAD_MULTIPLY"); break;
            case WXK_NUMPAD_ADD: key = wxT("NUMPAD_ADD"); break;
            case WXK_NUMPAD_SEPARATOR: key = wxT("NUMPAD_SEPARATOR"); break;
            case WXK_NUMPAD_SUBTRACT: key = wxT("NUMPAD_SUBTRACT"); break;
            case WXK_NUMPAD_DECIMAL: key = wxT("NUMPAD_DECIMAL"); break;

            default:
            {
               if ( keycode < 128 && wxIsprint((int)keycode) )
                   key.Printf(wxT("'%c'"), (char)keycode);
               else if ( keycode > 0 && keycode < 27 )
                   key.Printf(_("Ctrl-%c"), wxT('A') + keycode - 1);
               else
                   key.Printf(wxT("unknown (%ld)"), keycode);
            }
        }
    }

    wxLogMessage(wxT("%s event: %s (flags = %c%c%c%c)"),
                  name,
                  key.c_str(),
                  event.ControlDown() ? wxT('C') : wxT('-'),
                  event.AltDown() ? wxT('A') : wxT('-'),
                  event.ShiftDown() ? wxT('S') : wxT('-'),
                  event.MetaDown() ? wxT('M') : wxT('-'));
}

void OPJMarkerTree::OnTreeKeyDown(wxTreeEvent& event)
{
    LogKeyEvent(wxT("Tree key down "), event.GetKeyEvent());

    event.Skip();
}*/

/*void OPJMarkerTree::OnBeginDrag(wxTreeEvent& event)
{
    // need to explicitly allow drag
    if ( event.GetItem() != GetRootItem() )
    {
        m_draggedItem = event.GetItem();

        wxLogMessage(wxT("OnBeginDrag: started dragging %s"),
                     GetItemText(m_draggedItem).c_str());

        event.Allow();
    }
    else
    {
        wxLogMessage(wxT("OnBeginDrag: this item can't be dragged."));
    }
}

void OPJMarkerTree::OnEndDrag(wxTreeEvent& event)
{
    wxTreeItemId itemSrc = m_draggedItem,
                 itemDst = event.GetItem();
    m_draggedItem = (wxTreeItemId)0l;

    // where to copy the item?
    if ( itemDst.IsOk() && !ItemHasChildren(itemDst) )
    {
        // copy to the parent then
        itemDst = GetItemParent(itemDst);
    }

    if ( !itemDst.IsOk() )
    {
        wxLogMessage(wxT("OnEndDrag: can't drop here."));

        return;
    }

    wxString text = GetItemText(itemSrc);
    wxLogMessage(wxT("OnEndDrag: '%s' copied to '%s'."),
                 text.c_str(), GetItemText(itemDst).c_str());

    // just do append here - we could also insert it just before/after the item
    // on which it was dropped, but this requires slightly more work... we also
    // completely ignore the client data and icon of the old item but could
    // copy them as well.
    //
    // Finally, we only copy one item here but we might copy the entire tree if
    // we were dragging a folder.
    int image = wxGetApp().ShowImages() ? TreeCtrlIcon_File : -1;
    AppendItem(itemDst, text, image);
}*/

/*void OPJMarkerTree::OnBeginLabelEdit(wxTreeEvent& event)
{
    wxLogMessage(wxT("OnBeginLabelEdit"));

    // for testing, prevent this item's label editing
    wxTreeItemId itemId = event.GetItem();
    if ( IsTestItem(itemId) )
    {
        wxMessageBox(wxT("You can't edit this item."));

        event.Veto();
    }
    else if ( itemId == GetRootItem() )
    {
        // test that it is possible to change the text of the item being edited
        SetItemText(itemId, _T("Editing root item"));
    }
}

void OPJMarkerTree::OnEndLabelEdit(wxTreeEvent& event)
{
    wxLogMessage(wxT("OnEndLabelEdit"));

    // don't allow anything except letters in the labels
    if ( !event.GetLabel().IsWord() )
    {
        wxMessageBox(wxT("The new label should be a single word."));

        event.Veto();
    }
}*/

/*void OPJMarkerTree::OnItemCollapsing(wxTreeEvent& event)
{
    wxLogMessage(wxT("OnItemCollapsing"));

    // for testing, prevent the user from collapsing the first child folder
    wxTreeItemId itemId = event.GetItem();
    if ( IsTestItem(itemId) )
    {
        wxMessageBox(wxT("You can't collapse this item."));

        event.Veto();
    }
}*/

/*void OPJMarkerTree::OnItemActivated(wxTreeEvent& event)
{
    // show some info about this item
    wxTreeItemId itemId = event.GetItem();
    OPJMarkerData *item = (OPJMarkerData *)GetItemData(itemId);

    if ( item != NULL )
    {
        item->ShowInfo(this);
    }

    wxLogMessage(wxT("OnItemActivated"));
}*/

void OPJMarkerTree::OnItemMenu(wxTreeEvent& event)
{
    /*wxTreeItemId itemId = event.GetItem();
    OPJMarkerData *item = itemId.IsOk() ? (OPJMarkerData *)GetItemData(itemId)
                                         : NULL;

    wxLogMessage(wxT("OnItemMenu for item \"%s\""), item ? item->GetDesc()
                                                         : _T(""));*/

	//wxLogMessage(wxT("EEEEEEEEEE"));

    //event.Skip();
}

/*void OPJMarkerTree::OnContextMenu(wxContextMenuEvent& event)
{
    wxPoint pt = event.GetPosition();
    wxTreeItemId item;
    wxLogMessage(wxT("OnContextMenu at screen coords (%i, %i)"), pt.x, pt.y);

    // check if event was generated by keyboard (MSW-specific?)
    if ( pt.x == -1 && pt.y == -1 ) //(this is how MSW indicates it)
    {
        if ( !HasFlag(wxTR_MULTIPLE) )
            item = GetSelection();

        // attempt to guess where to show the menu
        if ( item.IsOk() )
        {
            // if an item was clicked, show menu to the right of it
            wxRect rect;
            GetBoundingRect(item, rect, true );// only the label
            pt = wxPoint(rect.GetRight(), rect.GetTop());
        }
        else
        {
            pt = wxPoint(0, 0);
        }
    }
    else // event was generated by mouse, use supplied coords
    {
        pt = ScreenToClient(pt);
        item = HitTest(pt);
    }

    ShowMenu(item, pt);
}*/

/*void OPJMarkerTree::ShowMenu(wxTreeItemId id, const wxPoint& pt)
{
    wxString title;
    if ( id.IsOk() )
    {
        title << wxT("Menu for ") << GetItemText(id);
    }
    else
    {
        title = wxT("Menu for no particular item");
    }

#if wxUSE_MENUS
    wxMenu menu(title);
    menu.Append(TreeTest_About, wxT("&About..."));
    menu.AppendSeparator();
    menu.Append(TreeTest_Highlight, wxT("&Highlight item"));
    menu.Append(TreeTest_Dump, wxT("&Dump"));

    PopupMenu(&menu, pt);
#endif // wxUSE_MENUS
}*/

/*void OPJMarkerTree::OnItemRClick(wxTreeEvent& event)
{
    wxTreeItemId itemId = event.GetItem();
    OPJMarkerData *item = itemId.IsOk() ? (OPJMarkerData *)GetItemData(itemId)
                                         : NULL;

    wxLogMessage(wxT("Item \"%s\" right clicked"), item ? item->GetDesc()
                                                        : _T(""));

    event.Skip();
}*/

/*
void OPJMarkerTree::OnRMouseDown(wxMouseEvent& event)
{
    wxLogMessage(wxT("Right mouse button down"));

    event.Skip();
}

void OPJMarkerTree::OnRMouseUp(wxMouseEvent& event)
{
    wxLogMessage(wxT("Right mouse button up"));

    event.Skip();
}

void OPJMarkerTree::OnRMouseDClick(wxMouseEvent& event)
{
    wxTreeItemId id = HitTest(event.GetPosition());
    if ( !id )
        wxLogMessage(wxT("No item under mouse"));
    else
    {
        OPJMarkerData *item = (OPJMarkerData *)GetItemData(id);
        if ( item )
            wxLogMessage(wxT("Item '%s' under mouse"), item->GetDesc());
    }

    event.Skip();
}
*/

static inline const wxChar *Bool2String(bool b)
{
    return b ? wxT("") : wxT("not ");
}

void OPJMarkerData::ShowInfo(wxTreeCtrl *tree)
{
    wxLogMessage(wxT("Item '%s': %sselected, %sexpanded, %sbold,\n")
                 wxT("%u children (%u immediately under this item)."),
                 m_desc.c_str(),
                 Bool2String(tree->IsSelected(GetId())),
                 Bool2String(tree->IsExpanded(GetId())),
                 Bool2String(tree->IsBold(GetId())),
                 unsigned(tree->GetChildrenCount(GetId())),
                 unsigned(tree->GetChildrenCount(GetId(), false)));
}

/////////////////////////////////////////////////////////////////////
// Decoding thread class
/////////////////////////////////////////////////////////////////////

OPJDecoThread::OPJDecoThread(OPJCanvas *canvas)
        : wxThread()
{
    m_count = 0;
    m_canvas = canvas;
}

void OPJDecoThread::WriteText(const wxString& text)
{
    wxString msg;

    // before doing any GUI calls we must ensure that this thread is the only
    // one doing it!

#ifndef __WXGTK__ 
    wxMutexGuiEnter();
#endif // __WXGTK__

    msg << text;
    m_canvas->WriteText(msg);

#ifndef __WXGTK__ 
    wxMutexGuiLeave();
#endif // __WXGTK__
}

void OPJDecoThread::OnExit()
{
    wxCriticalSectionLocker locker(wxGetApp().m_deco_critsect);

    wxArrayThread& dthreads = wxGetApp().m_deco_threads;
    dthreads.Remove(this);

    if (dthreads.IsEmpty() )
    {
        // signal the main thread that there are no more threads left if it is
        // waiting for us
        if (wxGetApp().m_deco_waitingUntilAllDone) {
            wxGetApp().m_deco_waitingUntilAllDone = false;
            wxGetApp().m_deco_semAllDone.Post();
        }
    }
}

void *OPJDecoThread::Entry()
{

    wxString text;

	srand(GetId());
	//int m_countnum = rand() % 9;
    //text.Printf(wxT("Deco thread 0x%lx started (priority = %u, time = %d)."),
    //            GetId(), GetPriority(), m_countnum);
    text.Printf(wxT("Deco thread %d started"), m_canvas->m_childframe->m_winnumber);

    WriteText(text);

    wxBitmap bitmap(100, 100);
    wxImage image(100, 100, true); //= bitmap.ConvertToImage();
    image.Destroy();

	WriteText(m_canvas->m_fname.GetFullPath());


	// set handler properties
	wxJ2KHandler *j2kkkhandler = (wxJ2KHandler *) wxImage::FindHandler( wxBITMAP_TYPE_J2K);
	j2kkkhandler->m_reducefactor = wxGetApp().m_reducefactor;
	j2kkkhandler->m_qualitylayers = wxGetApp().m_qualitylayers;
	j2kkkhandler->m_components = wxGetApp().m_components;
#ifdef USE_JPWL
	j2kkkhandler->m_enablejpwl = wxGetApp().m_enablejpwl;
	j2kkkhandler->m_expcomps = wxGetApp().m_expcomps;
	j2kkkhandler->m_maxtiles = wxGetApp().m_maxtiles;
#endif // USE_JPWL

	wxJP2Handler *jp222handler = (wxJP2Handler *) wxImage::FindHandler( wxBITMAP_TYPE_JP2);
	jp222handler->m_reducefactor = wxGetApp().m_reducefactor;
	jp222handler->m_qualitylayers = wxGetApp().m_qualitylayers;
	jp222handler->m_components = wxGetApp().m_components;
#ifdef USE_JPWL
	jp222handler->m_enablejpwl = wxGetApp().m_enablejpwl;
	jp222handler->m_expcomps = wxGetApp().m_expcomps;
	jp222handler->m_maxtiles = wxGetApp().m_maxtiles;
#endif // USE_JPWL

	wxMJ2Handler *mj222handler = (wxMJ2Handler *) wxImage::FindHandler( wxBITMAP_TYPE_MJ2);
	mj222handler->m_reducefactor = wxGetApp().m_reducefactor;
	mj222handler->m_qualitylayers = wxGetApp().m_qualitylayers;
	mj222handler->m_components = wxGetApp().m_components;
	mj222handler->m_framenum = wxGetApp().m_framenum;
#ifdef USE_JPWL
	mj222handler->m_enablejpwl = wxGetApp().m_enablejpwl;
	mj222handler->m_expcomps = wxGetApp().m_expcomps;
	mj222handler->m_maxtiles = wxGetApp().m_maxtiles;
#endif // USE_JPWL

	if (wxGetApp().m_enabledeco) {

		// load the file
		if (!image.LoadFile(m_canvas->m_fname.GetFullPath(), wxBITMAP_TYPE_ANY, 0)) {
			WriteText(wxT("Can't load image"));
			return NULL;
		}

	} else {

		// display a macaron
		if (!image.Create(300, 5, false)) {
			WriteText(wxT("Can't create image"));
			return NULL;
		}

	}

	// assign 100% image
    m_canvas->m_image100 = wxBitmap(image);

	// find a fit-to-width zoom
	int zooml, wzooml, hzooml;
	wxSize clientsize = m_canvas->GetClientSize();
	wzooml = (int) floor(100.0 * (double) clientsize.GetWidth() / (double) (2 * OPJ_CANVAS_BORDER + image.GetWidth()));
	hzooml = (int) floor(100.0 * (double) clientsize.GetHeight() / (double) (2 * OPJ_CANVAS_BORDER + image.GetHeight()));
	zooml = wxMin(100, wxMin(wzooml, hzooml));

	// fit to width
#ifndef __WXGTK__
	m_canvas->m_childframe->m_frame->Rescale(zooml, m_canvas->m_childframe);
#endif // __WXGTK__

	//m_canvas->m_image = m_canvas->m_image100;
	//m_canvas->Refresh();
	//m_canvas->SetScrollbars(20, 20, (int)(0.5 + (double) image.GetWidth() / 20.0), (int)(0.5 + (double) image.GetHeight() / 20.0));

    //text.Printf(wxT("Deco thread 0x%lx finished."), GetId());
    text.Printf(wxT("Deco thread %d finished"), m_canvas->m_childframe->m_winnumber);
    WriteText(text);
    return NULL;

}

/////////////////////////////////////////////////////////////////////
// Parsing thread class
/////////////////////////////////////////////////////////////////////

OPJParseThread::OPJParseThread(OPJMarkerTree *tree, wxTreeItemId parentid)
        : wxThread()
{
    m_count = 0;
    m_tree = tree;
	m_parentid = parentid;
}

void OPJParseThread::WriteText(const wxString& text)
{
    wxString msg;

    // before doing any GUI calls we must ensure that this thread is the only
    // one doing it!

#ifndef __WXGTK__ 
    wxMutexGuiEnter();
#endif // __WXGTK

    msg << text;
    m_tree->WriteText(msg);

#ifndef __WXGTK__ 
    wxMutexGuiLeave();
#endif // __WXGTK
}

void OPJParseThread::OnExit()
{
    wxCriticalSectionLocker locker(wxGetApp().m_parse_critsect);

    wxArrayThread& threads = wxGetApp().m_parse_threads;
    threads.Remove(this);

    if (threads.IsEmpty()) {
        // signal the main thread that there are no more threads left if it is
        // waiting for us
        if (wxGetApp().m_parse_waitingUntilAllDone) {
            wxGetApp().m_parse_waitingUntilAllDone = false;
            wxGetApp().m_parse_semAllDone.Post();
        }
    }
}

void *OPJParseThread::Entry()
{

	printf("Entering\n\n");

    wxString text;

	srand(GetId());
	int m_countnum = rand() % 9;
    text.Printf(wxT("Parse thread 0x%lx started (priority = %u, time = %d)."),
            GetId(), GetPriority(), m_countnum);
    WriteText(text);
    LoadFile(m_tree->m_fname);
    text.Printf(wxT("Parse thread 0x%lx finished."), GetId());
    WriteText(text);


    //wxLogMessage(wxT("Entering\n")); //test wxLog thread safeness

	//wxBusyCursor wait;
	//wxBusyInfo wait(wxT("Decoding image ..."));


    /*for ( m_count = 0; m_count < m_countnum; m_count++ )
    {
        // check if we were asked to exit
        if ( TestDestroy() )
            break;

        text.Printf(wxT("[%u] Parse thread 0x%lx here."), m_count, GetId());
        WriteText(text);

        // wxSleep() can't be called from non-GUI thread!
        wxThread::Sleep(10);
    }*/

    // wxLogMessage(text); -- test wxLog thread safeness

	printf("Exiting\n\n");

    return NULL;
}







// ----------------------------------------------------------------------------
// OPJDecoderDialog
// ----------------------------------------------------------------------------

IMPLEMENT_CLASS(OPJDecoderDialog, wxPropertySheetDialog)

BEGIN_EVENT_TABLE(OPJDecoderDialog, wxPropertySheetDialog)
#ifdef USE_JPWL
	EVT_CHECKBOX(OPJDECO_ENABLEDECO, OPJDecoderDialog::OnEnableDeco)
	EVT_CHECKBOX(OPJDECO_ENABLEJPWL, OPJDecoderDialog::OnEnableJPWL)
#endif // USE_JPWL
END_EVENT_TABLE()

OPJDecoderDialog::OPJDecoderDialog(wxWindow* win, int dialogType)
{
	SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);

	Create(win, wxID_ANY, wxT("Decoder settings"),
		wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE| (int) wxPlatform::IfNot(wxOS_WINDOWS_CE, wxRESIZE_BORDER)
		);

	CreateButtons(wxOK | wxCANCEL | (int)wxPlatform::IfNot(wxOS_WINDOWS_CE, wxHELP));

	m_settingsNotebook = GetBookCtrl();

	wxPanel* mainSettings = CreateMainSettingsPage(m_settingsNotebook);
	wxPanel* jpeg2000Settings = CreatePart1SettingsPage(m_settingsNotebook);
	if (!wxGetApp().m_enabledeco)
		jpeg2000Settings->Enable(false);
	wxPanel* mjpeg2000Settings = CreatePart3SettingsPage(m_settingsNotebook);
	if (!wxGetApp().m_enabledeco)
		mjpeg2000Settings->Enable(false);
#ifdef USE_JPWL
	wxPanel* jpwlSettings = CreatePart11SettingsPage(m_settingsNotebook);
	if (!wxGetApp().m_enabledeco)
		jpwlSettings->Enable(false);
#endif // USE_JPWL

	m_settingsNotebook->AddPage(mainSettings, wxT("Display"), false);
	m_settingsNotebook->AddPage(jpeg2000Settings, wxT("JPEG 2000"), false);
	m_settingsNotebook->AddPage(mjpeg2000Settings, wxT("MJPEG 2000"), false);
#ifdef USE_JPWL
	m_settingsNotebook->AddPage(jpwlSettings, wxT("JPWL"), false);
#endif // USE_JPWL

	LayoutDialog();
}

OPJDecoderDialog::~OPJDecoderDialog()
{
}

wxPanel* OPJDecoderDialog::CreateMainSettingsPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

	// top sizer
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

		// sub top sizer
		wxBoxSizer *subtopSizer = new wxBoxSizer(wxVERTICAL);

		// add decoding enabling check box
		subtopSizer->Add(
			m_enabledecoCheck = new wxCheckBox(panel, OPJDECO_ENABLEDECO, wxT("Enable decoding"), wxDefaultPosition, wxDefaultSize),
			0, wxGROW | wxALL, 5);
		m_enabledecoCheck->SetValue(wxGetApp().m_enabledeco);

			// resize settings, column
			wxString choices[] = {wxT("Low quality"), wxT("High quality")};
			m_resizeBox = new wxRadioBox(panel, OPJDECO_RESMETHOD,
				wxT("Resize method"),
				wxDefaultPosition, wxDefaultSize,
				WXSIZEOF(choices),
				choices,
				1,
				wxRA_SPECIFY_ROWS);
			m_resizeBox->SetSelection(wxGetApp().m_resizemethod);

		subtopSizer->Add(m_resizeBox, 0, wxGROW | wxALL, 5);

	topSizer->Add(subtopSizer, 1, wxGROW | wxALIGN_CENTRE | wxALL, 5);

	// assign top and fit it
    panel->SetSizer(topSizer);
    topSizer->Fit(panel);

    return panel;
}

wxPanel* OPJDecoderDialog::CreatePart3SettingsPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

	// top sizer
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

	// add some space
	//topSizer->AddSpacer(5);

		// sub top sizer
		wxBoxSizer *subtopSizer = new wxBoxSizer(wxVERTICAL);

			// frame settings, column
			wxStaticBox* frameBox = new wxStaticBox(panel, wxID_ANY, wxT("Frame"));
			wxBoxSizer* frameSizer = new wxStaticBoxSizer(frameBox, wxVERTICAL);

				// selected frame number, row
				wxBoxSizer* framenumSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				framenumSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Displayed frame:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

				// add some horizontal space
				framenumSizer->Add(5, 5, 1, wxALL, 0);

				// add the value control
				framenumSizer->Add(
					m_framenumCtrl = new wxSpinCtrl(panel, OPJDECO_FRAMENUM,
								wxString::Format(wxT("%d"), wxGetApp().m_framenum),
								wxDefaultPosition, wxSize(80, wxDefaultCoord),
								wxSP_ARROW_KEYS,
								1, 100000, wxGetApp().m_framenum),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

			frameSizer->Add(framenumSizer, 0, wxGROW | wxALL, 5);

		subtopSizer->Add(frameSizer, 0, wxGROW | wxALL, 5);

	topSizer->Add(subtopSizer, 1, wxGROW | wxALIGN_CENTRE | wxALL, 5);

	// assign top and fit it
    panel->SetSizer(topSizer);
    topSizer->Fit(panel);

    return panel;
}

wxPanel* OPJDecoderDialog::CreatePart1SettingsPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

	// top sizer
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

	// add some space
	//topSizer->AddSpacer(5);

		// sub top sizer
		wxBoxSizer *subtopSizer = new wxBoxSizer(wxVERTICAL);

			// resolutions settings, column
			wxStaticBox* resolutionBox = new wxStaticBox(panel, wxID_ANY, wxT("Resolutions"));
			wxBoxSizer* resolutionSizer = new wxStaticBoxSizer(resolutionBox, wxVERTICAL);

				// reduce factor sizer, row
				wxBoxSizer* reduceSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				reduceSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Reduce factor:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

				// add some horizontal space
				reduceSizer->Add(5, 5, 1, wxALL, 0);

				// add the value control
				reduceSizer->Add(
					m_reduceCtrl = new wxSpinCtrl(panel, OPJDECO_REDUCEFACTOR,
					wxString::Format(wxT("%d"), wxGetApp().m_reducefactor),
								wxDefaultPosition, wxSize(80, wxDefaultCoord),
								wxSP_ARROW_KEYS,
								0, 10000, wxGetApp().m_reducefactor),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

			resolutionSizer->Add(reduceSizer, 0, wxGROW | wxALL, 5);

		subtopSizer->Add(resolutionSizer, 0, wxGROW | wxALL, 5);

			// quality layer settings, column
			wxStaticBox* layerBox = new wxStaticBox(panel, wxID_ANY, wxT("Layers"));
			wxBoxSizer* layerSizer = new wxStaticBoxSizer(layerBox, wxVERTICAL);

				// quality layers sizer, row
				wxBoxSizer* qualitySizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				qualitySizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Quality layers:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

				// add some horizontal space
				qualitySizer->Add(5, 5, 1, wxALL, 0);

				// add the value control
				qualitySizer->Add(
					m_layerCtrl = new wxSpinCtrl(panel, OPJDECO_QUALITYLAYERS,
								wxString::Format(wxT("%d"), wxGetApp().m_qualitylayers),
								wxDefaultPosition, wxSize(80, wxDefaultCoord),
								wxSP_ARROW_KEYS,
								0, 100000, wxGetApp().m_qualitylayers),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

			layerSizer->Add(qualitySizer, 0, wxGROW | wxALL, 5);

		subtopSizer->Add(layerSizer, 0, wxGROW | wxALL, 5);

			// component settings, column
			wxStaticBox* compoBox = new wxStaticBox(panel, wxID_ANY, wxT("Components"));
			wxBoxSizer* compoSizer = new wxStaticBoxSizer(compoBox, wxVERTICAL);

				// quality layers sizer, row
				wxBoxSizer* numcompsSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				numcompsSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Component displayed:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

				// add some horizontal space
				numcompsSizer->Add(5, 5, 1, wxALL, 0);

				// add the value control
				numcompsSizer->Add(
					m_numcompsCtrl = new wxSpinCtrl(panel, OPJDECO_NUMCOMPS,
								wxString::Format(wxT("%d"), wxGetApp().m_components),
								wxDefaultPosition, wxSize(80, wxDefaultCoord),
								wxSP_ARROW_KEYS,
								0, 100000, wxGetApp().m_components),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
				m_numcompsCtrl->Enable(true);

			compoSizer->Add(numcompsSizer, 0, wxGROW | wxALL, 5);

		subtopSizer->Add(compoSizer, 0, wxGROW | wxALL, 5);

	topSizer->Add(subtopSizer, 1, wxGROW | wxALIGN_CENTRE | wxALL, 5);

	// assign top and fit it
    panel->SetSizer(topSizer);
    topSizer->Fit(panel);

    return panel;
}

#ifdef USE_JPWL
wxPanel* OPJDecoderDialog::CreatePart11SettingsPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

	// top sizer
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

	// add some space
	//topSizer->AddSpacer(5);

		// sub top sizer
		wxBoxSizer *subtopSizer = new wxBoxSizer(wxVERTICAL);

		// add JPWL enabling check box
		subtopSizer->Add(
			m_enablejpwlCheck = new wxCheckBox(panel, OPJDECO_ENABLEJPWL, wxT("Enable JPWL"), wxDefaultPosition, wxDefaultSize),
			0, wxGROW | wxALL, 5);
		m_enablejpwlCheck->SetValue(wxGetApp().m_enablejpwl);

			// component settings, column
			wxStaticBox* compoBox = new wxStaticBox(panel, wxID_ANY, wxT("Components"));
			wxBoxSizer* compoSizer = new wxStaticBoxSizer(compoBox, wxVERTICAL);

				// expected components sizer, row
				wxBoxSizer* expcompsSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				expcompsSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Expected comps.:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

				// add some horizontal space
				expcompsSizer->Add(5, 5, 1, wxALL, 0);

				// add the value control
				expcompsSizer->Add(
					m_expcompsCtrl = new wxSpinCtrl(panel, OPJDECO_EXPCOMPS,
								wxString::Format(wxT("%d"), wxGetApp().m_expcomps),
								wxDefaultPosition, wxSize(80, wxDefaultCoord),
								wxSP_ARROW_KEYS,
								1, 100000, wxGetApp().m_expcomps),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
				m_expcompsCtrl->Enable(wxGetApp().m_enablejpwl);

			compoSizer->Add(expcompsSizer, 0, wxGROW | wxALL, 5);

		subtopSizer->Add(compoSizer, 0, wxGROW | wxALL, 5);

			// tiles settings, column
			wxStaticBox* tileBox = new wxStaticBox(panel, wxID_ANY, wxT("Tiles"));
			wxBoxSizer* tileSizer = new wxStaticBoxSizer(tileBox, wxVERTICAL);

				// maximum tiles sizer, row
				wxBoxSizer* maxtileSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				maxtileSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Max. no. of tiles:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

				// add some horizontal space
				maxtileSizer->Add(5, 5, 1, wxALL, 0);

				// add the value control
				maxtileSizer->Add(
					m_maxtilesCtrl = new wxSpinCtrl(panel, OPJDECO_MAXTILES,
								wxString::Format(wxT("%d"), wxGetApp().m_maxtiles),
								wxDefaultPosition, wxSize(80, wxDefaultCoord),
								wxSP_ARROW_KEYS,
								1, 100000, wxGetApp().m_maxtiles),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
				m_maxtilesCtrl->Enable(wxGetApp().m_enablejpwl);

			tileSizer->Add(maxtileSizer, 0, wxGROW | wxALL, 5);

		subtopSizer->Add(tileSizer, 0, wxGROW | wxALL, 5);

	topSizer->Add(subtopSizer, 1, wxGROW | wxALIGN_CENTRE | wxALL, 5);

	// assign top and fit it
    panel->SetSizer(topSizer);
    topSizer->Fit(panel);

    return panel;
}

void OPJDecoderDialog::OnEnableDeco(wxCommandEvent& event)
{
	size_t pp;

	if (event.IsChecked()) {
		wxLogMessage(wxT("Decoding enabled"));
		m_resizeBox->Enable(true);
		// enable all tabs except ourselves
		for (pp = 0; pp < m_settingsNotebook->GetPageCount(); pp++) {
			if (m_settingsNotebook->GetPageText(pp) != wxT("Display"))
				m_settingsNotebook->GetPage(pp)->Enable(true);
		}
	} else {
		wxLogMessage(wxT("Decoding disabled"));
		m_resizeBox->Enable(false);
		// disable all tabs except ourselves
		for (pp = 0; pp < m_settingsNotebook->GetPageCount(); pp++) {
			if (m_settingsNotebook->GetPageText(pp) != wxT("Display"))
				m_settingsNotebook->GetPage(pp)->Enable(false);
		}
	}

}

void OPJDecoderDialog::OnEnableJPWL(wxCommandEvent& event)
{
	if (event.IsChecked()) {
		wxLogMessage(wxT("JPWL enabled"));
		m_expcompsCtrl->Enable(true);
		m_maxtilesCtrl->Enable(true);
	} else {
		wxLogMessage(wxT("JPWL disabled"));
		m_expcompsCtrl->Enable(false);
		m_maxtilesCtrl->Enable(false);
	}

}

#endif // USE_JPWL

bool OPJDnDFile::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
    /*size_t nFiles = filenames.GetCount();
    wxString str;
    str.Printf( _T("%d files dropped\n"), (int)nFiles);
    for ( size_t n = 0; n < nFiles; n++ ) {
        str << filenames[n] << wxT("\n");
    }
    wxLogMessage(str);*/
	m_pOwner->OpenFiles(filenames, filenames);

    return true;
}





// ----------------------------------------------------------------------------
// OPJEncoderDialog
// ----------------------------------------------------------------------------

IMPLEMENT_CLASS(OPJEncoderDialog, wxPropertySheetDialog)

BEGIN_EVENT_TABLE(OPJEncoderDialog, wxPropertySheetDialog)
#ifdef USE_JPWL
	EVT_CHECKBOX(OPJENCO_ENABLEJPWL, OPJEncoderDialog::OnEnableJPWL)
#endif // USE_JPWL
END_EVENT_TABLE()

OPJEncoderDialog::OPJEncoderDialog(wxWindow* win, int dialogType)
{
	SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);

	Create(win, wxID_ANY, wxT("Encoder settings"),
		wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE| (int) wxPlatform::IfNot(wxOS_WINDOWS_CE, wxRESIZE_BORDER)
		);

	CreateButtons(wxOK | wxCANCEL | (int)wxPlatform::IfNot(wxOS_WINDOWS_CE, wxHELP));

	m_settingsNotebook = GetBookCtrl();

	wxPanel* mainSettings = CreateMainSettingsPage(m_settingsNotebook);
	wxPanel* jpeg2000Settings = CreatePart1SettingsPage(m_settingsNotebook);
/*	if (!wxGetApp().m_enabledeco)
		jpeg2000Settings->Enable(false);
	wxPanel* mjpeg2000Settings = CreatePart3SettingsPage(m_settingsNotebook);
	if (!wxGetApp().m_enabledeco)
		mjpeg2000Settings->Enable(false);
#ifdef USE_JPWL
	wxPanel* jpwlSettings = CreatePart11SettingsPage(m_settingsNotebook);
	if (!wxGetApp().m_enabledeco)
		jpwlSettings->Enable(false);
#endif // USE_JPWL
*/

	m_settingsNotebook->AddPage(mainSettings, wxT("General"), false);
	m_settingsNotebook->AddPage(jpeg2000Settings, wxT("JPEG 2000"), false);
/*	m_settingsNotebook->AddPage(mjpeg2000Settings, wxT("MJPEG 2000"), false);
#ifdef USE_JPWL
	m_settingsNotebook->AddPage(jpwlSettings, wxT("JPWL"), false);
#endif // USE_JPWL
*/
	LayoutDialog();
}

OPJEncoderDialog::~OPJEncoderDialog()
{
}

wxPanel* OPJEncoderDialog::CreateMainSettingsPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

	// top sizer
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

		// sub top sizer
		wxBoxSizer *subtopSizer = new wxBoxSizer(wxVERTICAL);

	topSizer->Add(subtopSizer, 1, wxGROW | wxALIGN_CENTRE | wxALL, 5);

	// assign top and fit it
    panel->SetSizer(topSizer);
    topSizer->Fit(panel);

    return panel;
}

wxPanel* OPJEncoderDialog::CreatePart1SettingsPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

	// top sizer
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

	// add some space
	//topSizer->AddSpacer(5);

		// sub top sizer
		wxFlexGridSizer *subtopSizer = new wxFlexGridSizer(2, 3, 3);

			// image settings, column
			wxStaticBox* imageBox = new wxStaticBox(panel, wxID_ANY, wxT("Image"));
			wxBoxSizer* imageSizer = new wxStaticBoxSizer(imageBox, wxVERTICAL);

				// subsampling factor sizer, row
				wxBoxSizer* subsSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				subsSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Subsampling:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				subsSizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				subsSizer->Add(
					/*m_rateCtrl = */new wxTextCtrl(panel, OPJENCO_SUBSAMPLING,
								wxT("1,1"),
								wxDefaultPosition, wxSize(120, wxDefaultCoord),
								wxTE_LEFT),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			imageSizer->Add(subsSizer, 0, wxGROW | wxALL, 3);

				// origin sizer, row
				wxBoxSizer* imorigSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				imorigSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Origin:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				imorigSizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				imorigSizer->Add(
					/*m_rateCtrl = */new wxTextCtrl(panel, OPJENCO_IMORIG,
								wxT("0,0"),
								wxDefaultPosition, wxSize(120, wxDefaultCoord),
								wxTE_LEFT),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			imageSizer->Add(imorigSizer, 0, wxGROW | wxALL, 3);

		subtopSizer->Add(imageSizer, 0, wxGROW | wxALL, 3);

			// layer settings, column
			wxStaticBox* layerBox = new wxStaticBox(panel, wxID_ANY, wxT("Layers"));
			wxBoxSizer* layerSizer = new wxStaticBoxSizer(layerBox, wxVERTICAL);

				// rate factor sizer, row
				wxBoxSizer* rateSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				rateSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Rate values:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				rateSizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				rateSizer->Add(
					/*m_rateCtrl = */new wxTextCtrl(panel, OPJENCO_RATEFACTOR,
								wxT("20,10,5"),
								wxDefaultPosition, wxSize(120, wxDefaultCoord),
								wxTE_LEFT),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			layerSizer->Add(rateSizer, 0, wxGROW | wxALL, 3);

				// quality factor sizer, row
				wxBoxSizer* qualitySizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				qualitySizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Quality values:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				qualitySizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				qualitySizer->Add(
					/*m_rateCtrl = */new wxTextCtrl(panel, OPJENCO_QUALITYFACTOR,
								wxT("30,35,40"),
								wxDefaultPosition, wxSize(120, wxDefaultCoord),
								wxTE_LEFT),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			layerSizer->Add(qualitySizer, 0, wxGROW | wxALL, 3);

		subtopSizer->Add(layerSizer, 0, wxGROW | wxALL, 3);

			// wavelet settings, column
			wxStaticBox* waveletBox = new wxStaticBox(panel, wxID_ANY, wxT("Transform"));
			wxBoxSizer* waveletSizer = new wxStaticBoxSizer(waveletBox, wxVERTICAL);

			// irreversible check box
			waveletSizer->Add(
				/*m_enabledecoCheck =*/ new wxCheckBox(panel, OPJENCO_ENABLEIRREV, wxT("Irreversible"),
				wxDefaultPosition, wxDefaultSize),
				0, wxGROW | wxALL, 3);
			/*m_enabledecoCheck->SetValue(wxGetApp().m_enabledeco);*/

				// resolution number sizer, row
				wxBoxSizer* resnumSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				resnumSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Resolutions:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				resnumSizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				resnumSizer->Add(
					/*m_layerCtrl =*/ new wxSpinCtrl(panel, OPJENCO_RESNUMBER,
								wxT("6"),
								wxDefaultPosition, wxSize(80, wxDefaultCoord),
								wxSP_ARROW_KEYS,
								0, 256, 6),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			waveletSizer->Add(resnumSizer, 0, wxGROW | wxALL, 3);

				// codeblock sizer, row
				wxBoxSizer* codeblockSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				codeblockSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Codeblocks size:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				codeblockSizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				codeblockSizer->Add(
					/*m_rateCtrl = */new wxTextCtrl(panel, OPJENCO_CODEBLOCKSIZE,
								wxT("32,32"),
								wxDefaultPosition, wxSize(120, wxDefaultCoord),
								wxTE_LEFT),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			waveletSizer->Add(codeblockSizer, 0, wxGROW | wxALL, 3);

				// precinct sizer, row
				wxBoxSizer* precinctSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				precinctSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Precincts size:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				precinctSizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				precinctSizer->Add(
					/*m_rateCtrl = */new wxTextCtrl(panel, OPJENCO_PRECINCTSIZE,
								wxT("[128,128],[128,128]"),
								wxDefaultPosition, wxSize(120, wxDefaultCoord),
								wxTE_LEFT),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			waveletSizer->Add(precinctSizer, 0, wxGROW | wxALL, 3);

		subtopSizer->Add(waveletSizer, 0, wxGROW | wxALL, 3);

			// tile settings, column
			wxStaticBox* tileBox = new wxStaticBox(panel, wxID_ANY, wxT("Tiles"));
			wxBoxSizer* tileSizer = new wxStaticBoxSizer(tileBox, wxVERTICAL);

				// tile size sizer, row
				wxBoxSizer* tilesizeSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				tilesizeSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Size:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				tilesizeSizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				tilesizeSizer->Add(
					/*m_rateCtrl = */new wxTextCtrl(panel, OPJENCO_TILESIZE,
								wxT(""),
								wxDefaultPosition, wxSize(120, wxDefaultCoord),
								wxTE_LEFT),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			tileSizer->Add(tilesizeSizer, 0, wxGROW | wxALL, 3);

				// tile origin sizer, row
				wxBoxSizer* tilorigSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				tilorigSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Origin:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				tilorigSizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				tilorigSizer->Add(
					/*m_rateCtrl = */new wxTextCtrl(panel, OPJENCO_TILORIG,
								wxT("0,0"),
								wxDefaultPosition, wxSize(120, wxDefaultCoord),
								wxTE_LEFT),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			tileSizer->Add(tilorigSizer, 0, wxGROW | wxALL, 3);

		subtopSizer->Add(tileSizer, 0, wxGROW | wxALL, 3);

			// progression settings, column
			wxString choices[] = {wxT("LRCP"), wxT("RLCP"), wxT("RPCL"), wxT("PCRL"), wxT("CPRL")};
			wxRadioBox *progressionBox = new wxRadioBox(panel, OPJENCO_PROGRESSION,
				wxT("Progression"),
				wxDefaultPosition, wxDefaultSize,
				WXSIZEOF(choices),
				choices,
				4,
				wxRA_SPECIFY_COLS);
			progressionBox->SetSelection(0);

		subtopSizer->Add(progressionBox, 0, wxGROW | wxALL, 3);

			// resilience settings, column
			wxStaticBox* resilBox = new wxStaticBox(panel, wxID_ANY, wxT("Resilience"));
			wxBoxSizer* resilSizer = new wxStaticBoxSizer(resilBox, wxVERTICAL);

				// resil2 sizer, row
				wxBoxSizer* resil2Sizer = new wxBoxSizer(wxHORIZONTAL);

				// SOP check box
				resil2Sizer->Add(
					/*m_enabledecoCheck =*/ new wxCheckBox(panel, OPJENCO_ENABLESOP, wxT("SOP"),
					wxDefaultPosition, wxDefaultSize),
					0, wxGROW | wxALL, 3);
				/*m_enabledecoCheck->SetValue(wxGetApp().m_enabledeco);*/

				// EPH check box
				resil2Sizer->Add(
					/*m_enabledecoCheck =*/ new wxCheckBox(panel, OPJENCO_ENABLEEPH, wxT("EPH"),
					wxDefaultPosition, wxDefaultSize),
					0, wxGROW | wxALL, 3);
				/*m_enabledecoCheck->SetValue(wxGetApp().m_enabledeco);*/

			resilSizer->Add(resil2Sizer, 0, wxGROW | wxALL, 3);

			// separation
			resilSizer->Add(new wxStaticLine(panel, wxID_ANY), 0, wxEXPAND | wxLEFT | wxRIGHT, 3);

				// resil3 sizer, row
				wxFlexGridSizer* resil3Sizer = new wxFlexGridSizer(3, 3, 3);

				// BYPASS check box
				resil3Sizer->Add(
					/*m_enabledecoCheck =*/ new wxCheckBox(panel, OPJENCO_ENABLEBYPASS, wxT("BYPASS"),
					wxDefaultPosition, wxDefaultSize),
					0, wxGROW | wxALL, 3);
				/*m_enabledecoCheck->SetValue(wxGetApp().m_enabledeco);*/

				// RESET check box
				resil3Sizer->Add(
					/*m_enabledecoCheck =*/ new wxCheckBox(panel, OPJENCO_ENABLERESET, wxT("RESET"),
					wxDefaultPosition, wxDefaultSize),
					0, wxGROW | wxALL, 3);
				/*m_enabledecoCheck->SetValue(wxGetApp().m_enabledeco);*/

				// RESTART check box
				resil3Sizer->Add(
					/*m_enabledecoCheck =*/ new wxCheckBox(panel, OPJENCO_ENABLERESTART, wxT("RESTART"),
					wxDefaultPosition, wxDefaultSize),
					0, wxGROW | wxALL, 3);
				/*m_enabledecoCheck->SetValue(wxGetApp().m_enabledeco);*/

				// VSC check box
				resil3Sizer->Add(
					/*m_enabledecoCheck =*/ new wxCheckBox(panel, OPJENCO_ENABLEVSC, wxT("VSC"),
					wxDefaultPosition, wxDefaultSize),
					0, wxGROW | wxALL, 3);
				/*m_enabledecoCheck->SetValue(wxGetApp().m_enabledeco);*/

				// ERTERM check box
				resil3Sizer->Add(
					/*m_enabledecoCheck =*/ new wxCheckBox(panel, OPJENCO_ENABLEERTERM, wxT("ERTERM"),
					wxDefaultPosition, wxDefaultSize),
					0, wxGROW | wxALL, 3);
				/*m_enabledecoCheck->SetValue(wxGetApp().m_enabledeco);*/

				// SEGMARK check box
				resil3Sizer->Add(
					/*m_enabledecoCheck =*/ new wxCheckBox(panel, OPJENCO_ENABLESEGMARK, wxT("SEGMARK"),
					wxDefaultPosition, wxDefaultSize),
					0, wxGROW | wxALL, 3);
				/*m_enabledecoCheck->SetValue(wxGetApp().m_enabledeco);*/

			resilSizer->Add(resil3Sizer, 0, wxGROW | wxALL, 3);

		subtopSizer->Add(resilSizer, 0, wxGROW | wxALL, 3);

			// ROI settings, column
			wxStaticBox* roiBox = new wxStaticBox(panel, wxID_ANY, wxT("ROI"));
			wxBoxSizer* roiSizer = new wxStaticBoxSizer(roiBox, wxVERTICAL);

				// component number sizer, row
				wxBoxSizer* roicompSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				roicompSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Component:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				roicompSizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				roicompSizer->Add(
					/*m_layerCtrl =*/ new wxSpinCtrl(panel, OPJENCO_ROICOMP,
								wxT("0"),
								wxDefaultPosition, wxSize(80, wxDefaultCoord),
								wxSP_ARROW_KEYS,
								0, 256, 0),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			roiSizer->Add(roicompSizer, 0, wxGROW | wxALL, 3);

				// upshift sizer, row
				wxBoxSizer* roishiftSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				roishiftSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Upshift:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				roishiftSizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				roishiftSizer->Add(
					/*m_layerCtrl =*/ new wxSpinCtrl(panel, OPJENCO_ROISHIFT,
								wxT("0"),
								wxDefaultPosition, wxSize(80, wxDefaultCoord),
								wxSP_ARROW_KEYS,
								0, 37, 0),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			roiSizer->Add(roishiftSizer, 0, wxGROW | wxALL, 3);

		subtopSizer->Add(roiSizer, 0, wxGROW | wxALL, 3);

			// ROI settings, column
			wxStaticBox* indexBox = new wxStaticBox(panel, wxID_ANY, wxT("Indexing"));
			wxBoxSizer* indexSizer = new wxStaticBoxSizer(indexBox, wxVERTICAL);

			// indexing check box
			indexSizer->Add(
				/*m_enabledecoCheck =*/ new wxCheckBox(panel, OPJENCO_ENABLEINDEX, wxT("Enabled"),
				wxDefaultPosition, wxDefaultSize),
				0, wxGROW | wxALL, 3);
			/*m_enabledecoCheck->SetValue(wxGetApp().m_enabledeco);*/

				// index file sizer, row
				wxBoxSizer* indexnameSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				indexnameSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&File name:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

				// add some horizontal space
				indexnameSizer->Add(3, 3, 1, wxALL, 0);

				// add the value control
				indexnameSizer->Add(
					/*m_rateCtrl = */new wxTextCtrl(panel, OPJENCO_INDEXNAME,
								wxT(""),
								wxDefaultPosition, wxSize(120, wxDefaultCoord),
								wxTE_LEFT),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

			indexSizer->Add(indexnameSizer, 0, wxGROW | wxALL, 3);

		subtopSizer->Add(indexSizer, 0, wxGROW | wxALL, 3);

/*			// component settings, column
			wxStaticBox* compoBox = new wxStaticBox(panel, wxID_ANY, wxT("Components"));
			wxBoxSizer* compoSizer = new wxStaticBoxSizer(compoBox, wxVERTICAL);

				// quality layers sizer, row
				wxBoxSizer* numcompsSizer = new wxBoxSizer(wxHORIZONTAL);

				// add some text
				numcompsSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("&Component displayed:")),
								0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

				// add some horizontal space
				numcompsSizer->Add(5, 5, 1, wxALL, 0);

				// add the value control
				numcompsSizer->Add(
					m_numcompsCtrl = new wxSpinCtrl(panel, OPJDECO_NUMCOMPS,
								wxString::Format(wxT("%d"), wxGetApp().m_components),
								wxDefaultPosition, wxSize(80, wxDefaultCoord),
								wxSP_ARROW_KEYS,
								0, 100000, wxGetApp().m_components),
					0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
				m_numcompsCtrl->Enable(true);

			compoSizer->Add(numcompsSizer, 0, wxGROW | wxALL, 5);

		subtopSizer->Add(compoSizer, 0, wxGROW | wxALL, 5);
*/
	topSizer->Add(subtopSizer, 1, wxGROW | wxALIGN_CENTRE | wxALL, 5);

	// assign top and fit it
    panel->SetSizer(topSizer);
    topSizer->Fit(panel);

    return panel;
}

#ifdef USE_JPWL
void OPJEncoderDialog::OnEnableJPWL(wxCommandEvent& event)
{
	/*if (event.IsChecked()) {
		wxLogMessage(wxT("JPWL enabled"));
		m_expcompsCtrl->Enable(true);
		m_maxtilesCtrl->Enable(true);
	} else {
		wxLogMessage(wxT("JPWL disabled"));
		m_expcompsCtrl->Enable(false);
		m_maxtilesCtrl->Enable(false);
	}*/

}
#endif // USE_JPWL
