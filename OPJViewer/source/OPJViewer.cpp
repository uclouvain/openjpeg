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
#include "OPJViewer.h"

OPJFrame *frame = NULL;
wxList my_children;

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

    {
        int n;

        for (n = 0; n < argc; n++ )
        {
            wxMB2WXbuf warg = wxConvertMB2WX(argv[n]);
            wxArgv[n] = wxStrdup(warg);
        }

        wxArgv[n] = NULL;
    }
#else // !wxUSE_UNICODE
    #define wxArgv argv
#endif // wxUSE_UNICODE/!wxUSE_UNICODE

#if wxUSE_CMDLINE_PARSER
    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        { wxCMD_LINE_SWITCH, _T("h"), _T("help"), _T("show this help message"),
            wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        /*{ wxCMD_LINE_SWITCH, _T("v"), _T("verbose"), _T("be verbose") },
        { wxCMD_LINE_SWITCH, _T("q"), _T("quiet"),   _T("be quiet") },

        { wxCMD_LINE_OPTION, _T("o"), _T("output"),  _T("output file") },
        { wxCMD_LINE_OPTION, _T("i"), _T("input"),   _T("input dir") },
        { wxCMD_LINE_OPTION, _T("s"), _T("size"),    _T("output block size"),
            wxCMD_LINE_VAL_NUMBER },
        { wxCMD_LINE_OPTION, _T("d"), _T("date"),    _T("output file date"),
            wxCMD_LINE_VAL_DATE },*/

        { wxCMD_LINE_PARAM,  NULL, NULL, _T("input file"),
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },

        { wxCMD_LINE_NONE }
    };

    wxCmdLineParser parser(cmdLineDesc, argc, wxArgv);

    /*parser.AddOption(_T("project_name"), _T(""), _T("full path to project file"),
                     wxCMD_LINE_VAL_STRING,
                     wxCMD_LINE_OPTION_MANDATORY | wxCMD_LINE_NEEDS_SEPARATOR);*/

    switch ( parser.Parse() )
    {
        case -1:
            wxLogMessage(_T("Help was given, terminating."));
            break;

        case 0:
            ShowCmdLine(parser);
            break;

        default:
            wxLogMessage(_T("Syntax error detected."));
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


	// Create the main frame window

  frame = new OPJFrame(NULL, wxID_ANY, OPJ_APPLICATION_TITLEBAR, wxDefaultPosition, wxSize(800, 600),
                      wxDEFAULT_FRAME_STYLE |
                      wxNO_FULL_REPAINT_ON_RESIZE |
                      wxHSCROLL | wxVSCROLL);

  // Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
  frame->SetIcon(wxIcon(_T("OPJViewer16")));
#endif

  frame->Show(true);

  SetTopWindow(frame);

  return true;
}

void OPJViewerApp::ShowCmdLine(const wxCmdLineParser& parser)
{
    wxString s = _T("Command line parsed successfully:\nInput files: ");

    size_t count = parser.GetParamCount();
    for ( size_t param = 0; param < count; param++ )
    {
        s << parser.GetParam(param) << ';';

		m_filelist.Add(parser.GetParam(param));
    }

    /*s << '\n'
      << _T("Verbose:\t") << (parser.Found(_T("v")) ? _T("yes") : _T("no")) << '\n'
      << _T("Quiet:\t") << (parser.Found(_T("q")) ? _T("yes") : _T("no")) << '\n';

    wxString strVal;
    long lVal;
    wxDateTime dt;
    if ( parser.Found(_T("o"), &strVal) )
        s << _T("Output file:\t") << strVal << '\n';
    if ( parser.Found(_T("i"), &strVal) )
        s << _T("Input dir:\t") << strVal << '\n';
    if ( parser.Found(_T("s"), &lVal) )
        s << _T("Size:\t") << lVal << '\n';
    if ( parser.Found(_T("d"), &dt) )
        s << _T("Date:\t") << dt.FormatISODate() << '\n';
    if ( parser.Found(_T("project_name"), &strVal) )
        s << _T("Project:\t") << strVal << '\n';*/

    //wxLogMessage(s);
}

// OPJFrame events
BEGIN_EVENT_TABLE(OPJFrame, wxMDIParentFrame)
    EVT_MENU(SASHTEST_ABOUT, OPJFrame::OnAbout)
    EVT_MENU(SASHTEST_NEW_WINDOW, OPJFrame::OnFileOpen)
    EVT_SIZE(OPJFrame::OnSize)
    EVT_MENU(SASHTEST_QUIT, OPJFrame::OnQuit)
    EVT_MENU(SASHTEST_TOGGLE_WINDOW, OPJFrame::OnToggleWindow)
    EVT_SASH_DRAGGED_RANGE(ID_WINDOW_TOP, ID_WINDOW_BOTTOM, OPJFrame::OnSashDrag)
    EVT_NOTEBOOK_PAGE_CHANGED(LEFT_NOTEBOOK_ID, OPJFrame::OnNotebook)
END_EVENT_TABLE()

// this is the frame constructor
OPJFrame::OPJFrame(wxWindow *parent, const wxWindowID id, const wxString& title,
				   const wxPoint& pos, const wxSize& size, const long style)
		: wxMDIParentFrame(parent, id, title, pos, size, style)
{
	// file menu and its items
	wxMenu *file_menu = new wxMenu;

	file_menu->Append(SASHTEST_NEW_WINDOW, wxT("&Open\tCtrl+O"));
	file_menu->SetHelpString(SASHTEST_NEW_WINDOW, wxT("Open one or more files"));

	file_menu->Append(SASHTEST_TOGGLE_WINDOW, wxT("&Toggle browser\tCtrl+T"));
	file_menu->SetHelpString(SASHTEST_TOGGLE_WINDOW, wxT("Toggle the left browsing pane"));

	file_menu->Append(SASHTEST_QUIT, wxT("&Exit\tCtrl+Q"));
	file_menu->SetHelpString(SASHTEST_QUIT, wxT("Quit this program"));

	// help menu and its items
	wxMenu *help_menu = new wxMenu;

	help_menu->Append(SASHTEST_ABOUT, wxT("&About\tF1"));
	help_menu->SetHelpString(SASHTEST_ABOUT, wxT("Basic info on the program"));

	// the whole menubar
	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, wxT("&File"));
	menu_bar->Append(help_menu, wxT("&Help"));

	// Associate the menu bar with the frame
	SetMenuBar(menu_bar);

	// the status bar
	CreateStatusBar();

	// the logging window
	loggingWindow = new wxSashLayoutWindow(this, ID_WINDOW_BOTTOM,
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
								wxTE_MULTILINE | wxSUNKEN_BORDER | wxTE_READONLY 
								);
	m_textCtrlbrowse->SetValue(_T("Browsing window\n"));

	// add it the notebook
	m_bookCtrlbottom->AddPage(m_textCtrlbrowse, wxT("Peek"));

	// the browser window
	markerTreeWindow = new wxSashLayoutWindow(this, ID_WINDOW_LEFT1,
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


#if wxUSE_LOG
#ifdef __WXMOTIF__
	// For some reason, we get a memcpy crash in wxLogStream::DoLogStream
	// on gcc/wxMotif, if we use wxLogTextCtl. Maybe it's just gcc?
	delete wxLog::SetActiveTarget(new wxLogStderr);
#else
	// set our text control as the log target
	wxLogTextCtrl *logWindow = new wxLogTextCtrl(m_textCtrl);
	delete wxLog::SetActiveTarget(logWindow);
#endif
#endif // wxUSE_LOG

	// if there are files on the command line, open them
	/*if (!wxGetApp().m_filelist.IsEmpty()) {
	wxLogMessage(wxT("Habemus files!!!"));
	wxArrayString paths, filenames;
	for (int f = 0; f < wxGetApp().m_filelist.GetCount(); f++) {
	paths.Add(wxFileName(wxGetApp().m_filelist[f]).GetFullPath());
	filenames.Add(wxFileName(wxGetApp().m_filelist[f]).GetFullName());
	}
	OpenFiles(paths, filenames);
	}*/
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

	m_childhash[childnum]->Activate();

	wxLogMessage(wxString::Format(wxT("Selection changed (now %d --> %d)"), childnum, m_childhash[childnum]->m_winnumber));

}


void OPJFrame::Resize(int number)
{
	wxSize size = GetClientSize();
}

void OPJFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

// about window for the frame
void OPJFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox(wxString::Format(OPJ_APPLICATION_TITLEBAR
								  wxT("\n\n")
								  wxT("Built with %s and OpenJPEG ")
								  wxT(OPENJPEG_VERSION)
								  wxT("\non ") wxT(__DATE__) wxT(", ") wxT(__TIME__)
								  wxT("\nRunning under %s\n\n")
								  OPJ_APPLICATION_COPYRIGHT,
								  wxVERSION_STRING,
								  wxGetOsDescription().c_str()
								  ),
				 wxT("About ") OPJ_APPLICATION_NAME,
				 wxOK | wxICON_INFORMATION,
				 this
				 );
}

void OPJFrame::OnToggleWindow(wxCommandEvent& WXUNUSED(event))
{
    if (markerTreeWindow->IsShown())
    {
        markerTreeWindow->Show(false);
    }
    else
    {
        markerTreeWindow->Show(true);
    }
#if wxUSE_MDI_ARCHITECTURE
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
#endif // wxUSE_MDI_ARCHITECTURE
}

void OPJFrame::OnSashDrag(wxSashEvent& event)
{
    if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;

    switch (event.GetId())
    {
        case ID_WINDOW_LEFT1:
        {
            markerTreeWindow->SetDefaultSize(wxSize(event.GetDragRect().width, 1000));
            break;
        }
        case ID_WINDOW_BOTTOM:
        {
            loggingWindow->SetDefaultSize(wxSize(1000, event.GetDragRect().height));
            break;
        }
    }

#if wxUSE_MDI_ARCHITECTURE
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
#endif // wxUSE_MDI_ARCHITECTURE

    // Leaves bits of itself behind sometimes
    GetClientWindow()->Refresh();
}

// physically open the files
void OPJFrame::OpenFiles(wxArrayString paths, wxArrayString filenames)
{

        size_t count = paths.GetCount();
        for ( size_t n = 0; n < count; n++ )
        {
			wxString msg, s;
            s.Printf(_T("File %d: %s (%s)\n"),
                     (int)n, paths[n].c_str(), filenames[n].c_str());

            msg += s;
			//s.Printf(_T("Filter index: %d"), dialog.GetFilterIndex());
			msg += s;

			/*wxMessageDialog dialog2(this, msg, _T("Selected files"));
			dialog2.ShowModal();*/

			// Make another frame, containing a canvas
			  OPJChildFrame *subframe = new OPJChildFrame(frame,
											  paths[n],
											  winNumber,
											  _T("Canvas Frame"),
											  wxDefaultPosition, wxSize(300, 300),
											  wxDEFAULT_FRAME_STYLE |
											  wxNO_FULL_REPAINT_ON_RESIZE);
			  m_childhash[winNumber] = subframe;

				  // create own marker tree
				long tstyle = wxTR_DEFAULT_STYLE | wxSUNKEN_BORDER | 
			#ifndef NO_VARIABLE_HEIGHT
							 wxTR_HAS_VARIABLE_ROW_HEIGHT /*|*/
			#endif
							 /*wxTR_EDIT_LABELS*/;

				m_treehash[winNumber] = new OPJMarkerTree(m_bookCtrl, paths[n], wxT("Parsing..."), TreeTest_Ctrl,
											wxDefaultPosition, wxDefaultSize,
											tstyle);

				m_bookCtrl->AddPage(m_treehash[winNumber],
										wxString::Format(wxT("%u"), winNumber), false);

				for (int p = 0; p < m_bookCtrl->GetPageCount(); p++) {

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
	wxT("JPEG 2000 files (*.jp2,*.j2k,*.j2c,*.mj2)|*.jp2;*.j2k;*.j2c;*.mj2|JPEG files (*.jpg)|*.jpg|All files|*");
#endif
    wxFileDialog dialog(this, _T("Open JPEG 2000 file(s)"),
                        wxEmptyString, wxEmptyString, wildcards,
                        wxFD_OPEN|wxFD_MULTIPLE);

    if (dialog.ShowModal() == wxID_OK)
    {
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

    OPJDecoThread *dthread = CreateDecoThread();

    if (dthread->Run() != wxTHREAD_NO_ERROR)
        wxLogMessage(wxT("Can't start deco thread!"));
    else
		wxLogMessage(_T("New deco thread started."));

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

// Define the repainting behaviour
void OPJCanvas::OnDraw(wxDC& dc)
{
    /*dc.SetFont(*wxSWISS_FONT);
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawText(_T("Image drawing canvas"), 10, 10);
    dc.DrawLine(8, 22, 300, 22);*/
	if (m_image.Ok()) {
		dc.DrawBitmap(m_image, OPJ_CANVAS_BORDER, OPJ_CANVAS_BORDER);
	} else {
		dc.SetFont(*wxSWISS_FONT);
		dc.SetPen(*wxBLACK_PEN);
		dc.DrawText(_T("Decoding image, please wait..."), 40, 50);
	}

    /*dc.SetFont(*wxSWISS_FONT);
    dc.SetPen(*wxGREEN_PEN);
    dc.DrawLine(0, 0, 200, 200);
    dc.DrawLine(200, 0, 0, 200);

    dc.SetBrush(*wxCYAN_BRUSH);
    dc.SetPen(*wxRED_PEN);
    dc.DrawRectangle(100, 100, 100, 50);
    dc.DrawRoundedRectangle(150, 150, 100, 50, 20);

    dc.DrawEllipse(250, 250, 100, 50);
#if wxUSE_SPLINES
    dc.DrawSpline(50, 200, 50, 100, 200, 10);
#endif // wxUSE_SPLINES
    dc.DrawLine(50, 230, 200, 230);
    dc.DrawText(_T("This is a test string"), 50, 230);

    wxPoint points[3];
    points[0].x = 200; points[0].y = 300;
    points[1].x = 100; points[1].y = 400;
    points[2].x = 300; points[2].y = 400;

    dc.DrawPolygon(3, points);*/
}

// This implements a tiny doodling program! Drag the mouse using
// the left button.
void OPJCanvas::OnEvent(wxMouseEvent& event)
{
  wxClientDC dc(this);
  PrepareDC(dc);

  wxPoint pt(event.GetLogicalPosition(dc));

  if (xpos > -1 && ypos > -1 && event.Dragging())
  {
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawLine(xpos, ypos, pt.x, pt.y);
  }
  xpos = pt.x;
  ypos = pt.y;
}

void OPJFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
#if wxUSE_MDI_ARCHITECTURE
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
#endif // wxUSE_MDI_ARCHITECTURE
}

// Note that SASHTEST_NEW_WINDOW and SASHTEST_ABOUT commands get passed
// to the parent window for processing, so no need to
// duplicate event handlers here.

BEGIN_EVENT_TABLE(OPJChildFrame, wxMDIChildFrame)
  /*EVT_MENU(SASHTEST_CHILD_QUIT, OPJChildFrame::OnQuit)*/
  EVT_CLOSE(OPJChildFrame::OnClose)
  EVT_SET_FOCUS(OPJChildFrame::OnGotFocus)
  /*EVT_KILL_FOCUS(OPJChildFrame::OnLostFocus)*/
END_EVENT_TABLE()

OPJChildFrame::OPJChildFrame(OPJFrame *parent, wxFileName fname, int winnumber, const wxString& title, const wxPoint& pos, const wxSize& size,
const long style):
  wxMDIChildFrame(parent, wxID_ANY, title, pos, size, style)
{
  m_frame = (OPJFrame  *) parent;
  m_canvas = NULL;
  my_children.Append(this);
  m_fname = fname;
  m_winnumber = winnumber;
	SetTitle(wxString::Format(_T("%d: "), m_winnumber) + m_fname.GetFullName());


	  // Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
	  SetIcon(wxIcon(_T("sashtest_icn")));
#endif

#if wxUSE_STATUSBAR
	  // Give it a status line
	  //CreateStatusBar();
#endif // wxUSE_STATUSBAR

	  // Make a menubar
	  /*wxMenu *file_menu = new wxMenu;

	  file_menu->Append(SASHTEST_NEW_WINDOW, _T("&Open\tCtrl+O"));
	  file_menu->Append(SASHTEST_CHILD_QUIT, _T("&Close\tCtrl+C"));
	  file_menu->Append(SASHTEST_QUIT, _T("&Exit\tCtrl+Q"));

	  wxMenu *option_menu = new wxMenu;

	  // Dummy option
	  option_menu->Append(SASHTEST_REFRESH, _T("&Refresh picture"));

	  wxMenu *help_menu = new wxMenu;
	  help_menu->Append(SASHTEST_ABOUT, _T("&About\tF1"));

	  wxMenuBar *menu_bar = new wxMenuBar;

	  menu_bar->Append(file_menu, _T("&File"));
	  menu_bar->Append(option_menu, _T("&Options"));
	  menu_bar->Append(help_menu, _T("&Help"));

	  // Associate the menu bar with the frame
	  SetMenuBar(menu_bar);*/


	  int width, height;
	  GetClientSize(&width, &height);

	  OPJCanvas *canvas = new OPJCanvas(fname, this, wxPoint(0, 0), wxSize(width, height));
	  canvas->SetCursor(wxCursor(wxCURSOR_PENCIL));
	  m_canvas = canvas;

	  // Give it scrollbars
	  canvas->SetScrollbars(20, 20, 5, 5);

	  Show(true);
	  Maximize(true);


    /*wxSize gsize = m_frame->m_bookCtrl->GetClientSize();
    m_frame->m_treehash[m_winnumber]->SetSize(0, 0, gsize.x, gsize.y);*/

    /*m_frame->Resize(m_winnumber);*/
	/*m_frame->m_treehash[0]->Show(false);
	m_frame->m_treehash[m_winnumber]->Show(true);*/
    /*m_frame->Resize(m_winnumber);*/

	/*wxLogError(wxString::Format(wxT("Created tree %d (0x%x)"), m_winnumber, m_frame->m_treehash[m_winnumber]));*/

}

OPJChildFrame::~OPJChildFrame(void)
{
  my_children.DeleteObject(this);
}

/*void OPJChildFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	for (int p = 0; p < m_frame->m_bookCtrl->GetPageCount(); p++) {

		if (m_frame->m_bookCtrl->GetPageText(p) == wxString::Format(wxT("%u"), m_winnumber)) {
			m_frame->m_bookCtrl->DeletePage(p);;
			break;
		}

	}

	Close(true);
}*/

void OPJChildFrame::OnClose(wxCloseEvent& event)
{
	for (int p = 0; p < m_frame->m_bookCtrl->GetPageCount(); p++) {

		if (m_frame->m_bookCtrl->GetPageText(p) == wxString::Format(wxT("%u"), m_winnumber)) {
			m_frame->m_bookCtrl->DeletePage(p);
			break;
		}

	}
	Destroy();

	wxLogMessage(wxString::Format(wxT("Closed: %d"), m_winnumber));

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

	for (int p = 0; p < m_frame->m_bookCtrl->GetPageCount(); p++) {

		if (m_frame->m_bookCtrl->GetPageText(p) == wxString::Format(wxT("%u"), m_winnumber)) {
			m_frame->m_bookCtrl->ChangeSelection(p);
			break;
		}

	}

	wxLogMessage(wxString::Format(wxT("Got focus: %d (%x)"), m_winnumber, event.GetWindow()));
}

/*void OPJChildFrame::OnLostFocus(wxFocusEvent& event)
{
	wxLogMessage(wxString::Format(wxT("Lost focus: %d (%x)"), m_winnumber, event.GetWindow()));

}*/

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
    /*EVT_TREE_ITEM_MENU(TreeTest_Ctrl, OPJMarkerTree::OnItemMenu)*/
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

OPJMarkerTree::OPJMarkerTree(wxWindow *parent, wxFileName fname, wxString name, const wxWindowID id,
                       const wxPoint& pos, const wxSize& size,
                       long style)
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

    OPJParseThread *pthread = CreateParseThread();
    if (pthread->Run() != wxTHREAD_NO_ERROR)
        wxLogMessage(wxT("Can't start parse thread!"));
    else
		wxLogMessage(_T("New parse thread started."));
}

void OPJMarkerTree::CreateImageList(int size)
{
    if ( size == -1 )
    {
        SetImageList(NULL);
        return;
    }
    if ( size == 0 )
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
    for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
    {
        if ( size == sizeOrig )
        {
            images->Add(icons[i]);
        }
        else
        {
            images->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size)));
        }
    }

    AssignImageList(images);
}

#if USE_GENERIC_TREECTRL || !defined(__WXMSW__)
void OPJMarkerTree::CreateButtonsImageList(int size)
{
    if ( size == -1 )
    {
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

    for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
    {
        int sizeOrig = icons[i].GetWidth();
        if ( size == sizeOrig )
        {
            images->Add(icons[i]);
        }
        else
        {
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

	// close the file
	m_file.Close();

	// this is the root node
	if (this->m_parentid)
		m_tree->SetItemText(rootid, wxT("Codestream"));
	else
		m_tree->SetItemText(rootid, fname.GetFullName());
	
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
        text << _T('"') << GetItemText(item).c_str() << _T('"');
    else
        text = _T("invalid item");
    wxLogMessage(wxT("%s(%s)"), name, text.c_str());
}

OPJParseThread *OPJMarkerTree::CreateParseThread(wxTreeItemId parentid)
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

	if (strcmp(data->GetDesc1(), wxT("INFO-CSTREAM")))
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
#define BUNCH_LINESIZE	24
#define BUNCH_NUMLINES	6

	wxTreeItemId item = event.GetItem();
	OPJMarkerData* data = (OPJMarkerData *) GetItemData(item);
	wxString text;
	int l, c, pos = 0, pre_pos;
	unsigned char buffer[BUNCH_LINESIZE * BUNCH_NUMLINES];

	m_peektextCtrl->Clear();

	/*wxTextAttr myattr = m_peektextCtrl->GetDefaultStyle();
	myattr.SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	m_peektextCtrl->SetDefaultStyle(myattr);*/

	text << wxString::Format(wxT("Selected... (%s -> %s, %s, %d, %d)"),
		text.c_str(), data->GetDesc1(), data->GetDesc2(),
		data->m_start, data->m_length) << wxT("\n");

	// open the file and browse a little
	wxFile *fp = new wxFile(m_fname.GetFullPath().c_str(), wxFile::read);

	// go to position claimed
	fp->Seek(data->m_start, wxFromStart);

	// read a bunch
	int max_read = wxMin(WXSIZEOF(buffer), data->m_length - data->m_start + 1);
	fp->Read(buffer, max_read);

	pos = 0;
	for (l = 0; l < BUNCH_NUMLINES; l++) {

		text << wxString::Format(wxT("%08d:"), data->m_start + pos);

		pre_pos = pos;

		// add hex browsing text
		for (c = 0; c < BUNCH_LINESIZE; c++) {

			if (!(c % 8))
				text << wxT(" ");

			if (pos < max_read) {
				text << wxString::Format(wxT("%02X "), wxT(buffer[pos]));
			} else
				text << wxT("   ");
			pos++;
		}

		text << wxT("    ");

		// add char browsing text
		for (c = 0; c < BUNCH_LINESIZE; c++) {

			if (pre_pos < max_read) {
				if ((buffer[pre_pos] == '\n') || (buffer[pre_pos] == '\t'))
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

    wxLogMessage( wxT("%s event: %s (flags = %c%c%c%c)"),
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

/*void OPJMarkerTree::OnItemMenu(wxTreeEvent& event)
{
    wxTreeItemId itemId = event.GetItem();
    OPJMarkerData *item = itemId.IsOk() ? (OPJMarkerData *)GetItemData(itemId)
                                         : NULL;

    wxLogMessage(wxT("OnItemMenu for item \"%s\""), item ? item->GetDesc()
                                                         : _T(""));

    event.Skip();
}*/

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

    wxMutexGuiEnter();

    msg << text;
    m_canvas->WriteText(msg);

    wxMutexGuiLeave();
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
	int m_countnum = rand() % 9;
    text.Printf(wxT("Deco thread 0x%lx started (priority = %u, time = %d)."),
                GetId(), GetPriority(), m_countnum);
    WriteText(text);
    // wxLogMessage(text); -- test wxLog thread safeness

	//wxBusyCursor wait;
	//wxBusyInfo wait(wxT("Decoding image ..."));


    /*for (m_count = 0; m_count < m_countnum; m_count++) {
        // check if we were asked to exit
        if ( TestDestroy() )
            break;

        text.Printf(wxT("[%u] Deco thread 0x%lx here."), m_count, GetId());
        WriteText(text);

        // wxSleep() can't be called from non-GUI thread!
        wxThread::Sleep(10);
    }*/

    wxBitmap bitmap( 100, 100 );
    wxImage image = bitmap.ConvertToImage();
    image.Destroy();

	WriteText(m_canvas->m_fname.GetFullPath());

    if (!image.LoadFile(m_canvas->m_fname.GetFullPath(), wxBITMAP_TYPE_ANY), 0) {
        wxLogError(wxT("Can't load image"));
		return NULL;
	}

    m_canvas->m_image = wxBitmap(image);
	m_canvas->Refresh();
	m_canvas->SetScrollbars(20, 20, (int)(0.5 + (double) image.GetWidth() / 20.0), (int)(0.5 + (double) image.GetHeight() / 20.0));

    text.Printf(wxT("Deco thread 0x%lx finished."), GetId());
    WriteText(text);
    // wxLogMessage(text); -- test wxLog thread safeness

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

    wxMutexGuiEnter();

    msg << text;
    m_tree->WriteText(msg);

    wxMutexGuiLeave();
}

void OPJParseThread::OnExit()
{
    wxCriticalSectionLocker locker(wxGetApp().m_parse_critsect);

    wxArrayThread& threads = wxGetApp().m_parse_threads;
    threads.Remove(this);

    if ( threads.IsEmpty() )
    {
        // signal the main thread that there are no more threads left if it is
        // waiting for us
        if ( wxGetApp().m_parse_waitingUntilAllDone )
        {
            wxGetApp().m_parse_waitingUntilAllDone = false;

            wxGetApp().m_parse_semAllDone.Post();
        }
    }
}

void *OPJParseThread::Entry()
{

    wxString text;

	srand( GetId() );
	int m_countnum = rand() % 9;
    text.Printf(wxT("Parse thread 0x%lx started (priority = %u, time = %d)."),
                GetId(), GetPriority(), m_countnum);
    WriteText(text);
    // wxLogMessage(text); -- test wxLog thread safeness

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


    LoadFile(m_tree->m_fname);

    text.Printf(wxT("Parse thread 0x%lx finished."), GetId());
    WriteText(text);
    // wxLogMessage(text); -- test wxLog thread safeness

    return NULL;
}
