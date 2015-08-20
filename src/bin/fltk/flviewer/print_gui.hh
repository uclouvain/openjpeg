#ifndef _FLVIEWER_PRINT_GUI_HH_
#define _FLVIEWER_PRINT_GUI_HH_

#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>

typedef struct print_info
{
    Fl_Window* win;

    Fl_Choice *paper_size, *printer_name;
    Fl_Round_Button *portrait, *landscape;

    Fl_Round_Button *lowres, *hires;

    Fl_Round_Button *color, *gray;

    Fl_Group *printer_group, *file_group;
    Fl_Round_Button *to_printer, *to_file;

	Fl_Output *loc;

    Fl_Input *title, *fname;
    Fl_Button *center_image;
	Fl_Int_Input *copies;
    Fl_Float_Input *margin_lhs, *margin_rhs, *margin_top, *margin_bot;

	char ok, remove_fname, center;
	char want_portrait, want_landscape, want_color, want_gray;
    int resolution, nr_copies;
    int paper_size_pos, printer_pos;
	int win_destroy;

    char paper_layout[2];
    char pcolor[2];

	double fmargin_lhs, fmargin_rhs, fmargin_top, fmargin_bot;
	double fpaper_w, fpaper_h;
	char *fname_s, *command_s, *title_s, *printer_s, *format_s;
	char *media_s;

} PrintInfo;


void Print_gui_create(PrintInfo *p);

#endif /* _FLVIEWER_PRINT_GUI_HH_ */
