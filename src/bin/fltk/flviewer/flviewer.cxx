#include <config.h>
#include "opj_apps_config.h"
/*
 * author(s) and license
*/
//------------ see in main() ------------
#define HAVE_OPTION_FNFC_USES_GTK
//---------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <process.h>
//FIXME remove #define snprintf sprintf_s
#else /* not _WIN32 */
#include <unistd.h>
#include <sys/sem.h>
#define __STDC_FORMAT_MACROS
#include <stdint.h>
#endif /* _WIN32 */

#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>


#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>

#include <FL/fl_ask.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Double_Window.H>

#include <FL/Fl_Button.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Shared_Image.H>

#include <FL/Fl_Group.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Box.H>

#include <FL/filename.H>
#include <FL/x.H>
#include <FL/Fl_Scroll.H>

#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Spinner.H>


#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Overlay_Window.H>

#include <FL/Fl_Tree.H>
#include <FL/Fl_Pixmap.H>

#include "opj_stdint.h"

#include "flviewer.hh"
#include "tree.hh"
#include "viewerdefs.hh"
#include "lang/viewer_lang.h_utf8"
#include "rgb_color.hh"

#include "print_gui.hh"
#include "ps_image.hh"

#ifdef OPJ_HAVE_LIBPNG
#include "PNG.hh"
#endif

#include "OPENJPEG.hh"
#ifdef WANT_PGX
#include "PGX.hh"
#endif
#include "OPENMJ2.hh"
#include "JP2.hh"

#ifndef OPJ_PATH_LEN
#define OPJ_PATH_LEN 4096
#endif

//#define RESET_REDUCTION

#define USE_CUSTOM_ICONS
#define MAX_SPINVALUE 6.

#define DEFAULT_BACKGROUND FL_LIGHT1
#define DEFAULT_CANVAS_BACKGROUND FL_DARK2

static const char *IMG_PATTERN =
#ifdef WANT_PGX
 "*.{png,j2k,j2c,jp2,jpg2,jpc,jpx,jpf,jpm,mj2,mjp2,pgx}";
#else
 "*.{png,j2k,j2c,jp2,jpg2,jpc,jpx,jpf,jpm,mj2,mjp2}";
#endif

static Canvas *canvas;
static short movie_runs;

static char url_buf[512];

#define MAX_COLORBUF 31
static char bgcolor_buf[MAX_COLORBUF + 1];
static char vbuf[48];

//------------------- FORWARD ---------------------
static void print_cb(Fl_Widget *wid, void *v);
static void save_as_opj_cb(Fl_Widget *wid, void *v);

#ifdef OPJ_HAVE_LIBPNG
static void save_as_png_cb(Fl_Widget *wid, void *v);
#endif

static Fl_Double_Window *crop_win;
static Fl_Box *crop_box;
static Fl_RGB_Image *crop_img;

static int crop_win_done;
static int crop_lhs_top_x, crop_lhs_top_y;
static int crop_width, crop_height;

static int start_overlay;

static void exit_cb(Fl_Widget *wid, void *v);
static void pause_cb(Fl_Widget *wid, void *v);
static void resume_cb(Fl_Widget *wid, void *v);

static void restart_cb(Fl_Widget *wid, void *v);
static void forward_cb(Fl_Widget *wid, void *v);
static void backward_cb(Fl_Widget *wid, void *v);

static void fin_cb(Fl_Widget *wid, void *v);
static void browse_cb(Fl_Widget *wid, void *v);

static void goto_frame_cb(Fl_Widget *wid, void *v);
static void goto_track_cb(Fl_Widget *w, void *v);

static void chooser_cb(const char *cs);

#ifdef HAVE_THREADS
static int nr_threads;
#endif

char *root_dir;

static char *read_idf;
static FILE *reader;
static Fl_Spinner *threads_spinner;

static int win_w, win_h;
static Fl_Group *tiles_group;

static Fl_Button *pause_but, *resume_but, *restart_but, *backward_but,
    *forward_but, *fin_but;

static Fl_Output *url_out;
static Fl_Input *goto_frame_in, *goto_track_in;
static Fl_Output *all_frames_out, *all_tracks_out;

static Fl_Group *tracks_group, *frames_group;

static Fl_Input *layer_in, *component_in;
static Fl_Output *layer_out, *component_out;
static int mj2_max_tracks, max_components, max_layers;
static int cur_component, cur_layers;

static Fl_Scroll *scroll;

static Fl_Input *area_in, *tile_in;

static Fl_Group *reduct_group;
static Fl_Input *reduct_in;
static Fl_Output *tiles_out;

static Fl_Output *reduct_out;
static Fl_Button * reload_jp2;

static int max_tiles, max_reduction, cur_tile, cur_reduction;
static int user_changed_tile, user_changed_reduct;
static int reloaded;

static int cmd_line_reduction;

static int area_x0, area_y0, area_x1, area_y1;
static int user_changed_area;

static char *jp2_file;
static char *mj2_file;

static double fscale = 1.0;

#ifdef USE_CUSTOM_ICONS

static const char *L_open_xpm[] =
{
    "11 11 2 1",
    ".  c None",
    "@  c #000000",
    "...@.......",
    "...@@......",
    "...@@@.....",
    "...@@@@....",
    "...@@@@@...",
    "...@@@@@@..",
    "...@@@@@...",
    "...@@@@....",
    "...@@@.....",
    "...@@......",
    "...@......."
};
static Fl_Pixmap L_openpixmap(L_open_xpm);

static const char *L_close_xpm[] =
{
    "11 11 2 1",
    ".  c None",
    "@  c #000000",
    "...........",
    "...........",
    "...........",
    "...........",
    "...........",
    "@@@@@@@@@@@",
    ".@@@@@@@@@.",
    "..@@@@@@@..",
    "...@@@@@...",
    "....@@@....",
    ".....@....."
};
static Fl_Pixmap L_closepixmap(L_close_xpm);

#endif //USE_CUSTOM_ICONS

//----------------- end FORWARD -------------------
UserTree *tree = NULL;

int UserTree::handle(int event)
{
#ifdef DEBUG_TREE
fprintf(stderr,"%s:%d:\n\tUserTree(%d,%d,%d,%d) event(%d,%d) ex(%d) ey(%d)\n",
__FILE__,__LINE__,x(),y(),x()+w(),y()+h(),
Fl::event_x(),Fl::event_y(), Fl::event_x()-x(),Fl::event_y()-y() );
#endif
    if(!Fl::event_inside(x(), y(), w(), h() ) )
   {
#ifdef DEBUG_TREE
fprintf(stderr,"%s:%d:\n\tNOT INSIDE TREE\n",__FILE__,__LINE__);
#endif
    return 0;
   }
	if(event == FL_LEAVE) return 1;

	return Fl_Tree::handle(event);
}

class ImageBox : public Fl_Box
{

public:
    int start_x, start_y;
    int lhs_top_x, lhs_top_y, rhs_top_x, rhs_top_y;
    int lhs_bot_x, lhs_bot_y, rhs_bot_x, rhs_bot_y;
	int lhs_x, top_y;

    ImageBox(int xx, int yy, int ww, int hh)
    : Fl_Box(xx, yy, ww, hh, NULL)
   {
	lhs_x = xx; top_y = yy;

    lhs_top_x = lhs_top_y = 0;
    rhs_top_x = rhs_top_y = 0;
    lhs_bot_x = lhs_bot_y = 0;
    rhs_bot_x = rhs_bot_y = 0;
   } ;
    virtual int handle(int event);
};

static ImageBox *scroll_box;

class Overlay : public Fl_Overlay_Window
{

public:
    Overlay(int xx, int yy, int ww, int hh)
	: Fl_Overlay_Window(xx, yy, ww, hh) {};

	void draw_overlay();
};

static Overlay *main_win;

void Overlay::draw_overlay() 
{
	if(movie_runs || !canvas->cbuf) return;
   
	if(start_overlay) 
   { 
	crop_width = crop_height = 0;
	start_overlay = 0;
   }
	fl_color(FL_YELLOW); 
	fl_rect(scroll_box->x() + crop_lhs_top_x, 
			scroll_box->y() + crop_lhs_top_y, 
			crop_width, crop_height);
}

int ImageBox::handle(int event)
{
	int ex, ey;
#ifdef DEBUG_IMAGEBOX
fprintf(stderr,"%s:%d:\n\tImageBox(%d,%d,%d,%d) event(%d,%d) ex(%d) ey(%d)\n",
__FILE__,__LINE__,x(),y(),x()+w(),y()+h(),
Fl::event_x(),Fl::event_y(), Fl::event_x()-x(),Fl::event_y()-y() );
#endif

	if(!Fl::event_inside(x(), y(), w(), h() ) ) 
   {
#ifdef DEBUG_IMAGEBOX
fprintf(stderr,"%s:%d:\n\tNOT INSIDE IMAGEBOX\n",__FILE__,__LINE__);
#endif
	return 0;
   }

	if(event == FL_LEAVE) return 1;

	if(movie_runs || !canvas->cbuf) return 1;

	ex = Fl::event_x() - x(); ey = Fl::event_y() - y();

	switch(event)
   {
	case FL_PUSH:
		lhs_top_x = lhs_top_y = rhs_top_x = rhs_top_y = 0;
		lhs_bot_x = lhs_bot_y = rhs_bot_x = rhs_bot_y = 0;
		start_x = ex; start_y = ey;

		start_overlay = 1;
		main_win->redraw_overlay();
		return 1;

	case FL_DRAG:
		if(ex > start_x) //rhs
	   {
		if(ey < start_y) //up
	  {
		lhs_top_x = start_x; lhs_top_y = ey;
		rhs_top_x = ex; rhs_top_y = ey;
		lhs_bot_x = start_x; lhs_bot_y = start_y;
		lhs_bot_x = ex; rhs_bot_y = start_y;
	  }
		else
		if(ey > start_y) //down
	  {
		lhs_top_x = start_x; lhs_top_y = start_y;
		rhs_top_x = ex; rhs_top_y = start_y;
		lhs_bot_x = start_x; lhs_bot_y = ey;
		rhs_bot_x = ex; rhs_bot_y = ey;
	  }
	   }
		else
		if(ex < start_x) //lhs
	   {
		if(ey < start_y) //up
	  {
		lhs_top_x = ex; lhs_top_y = ey;
		rhs_top_x = start_x; rhs_top_y = ey;
		lhs_bot_x = ex; lhs_bot_y = start_y;
		rhs_bot_x = start_x; rhs_bot_y = start_y;
	  }
		else
		if(ey > start_y) //down
	  {
		lhs_top_x = ex; lhs_top_y = start_y;
		rhs_top_x = start_x; rhs_top_y = start_y;
		lhs_bot_x = ex; lhs_bot_y = ey;
		rhs_bot_x = start_x; rhs_bot_y = ey;
	  }
	   }
		crop_lhs_top_x = lhs_top_x;
		crop_lhs_top_y = lhs_top_y;
		crop_width = rhs_top_x - lhs_top_x;
		crop_height = rhs_bot_y - rhs_top_y;

		main_win->redraw_overlay();

		return 1;

	case FL_RELEASE:
		return 1;
   }
	return Fl_Box::handle(event);
}

static void crop_cancel_cb(Fl_Widget *wid, void *v)
{
	crop_win_done = 1;
	crop_win->hide();
}

static void crop_print_cb(Fl_Widget *wid, void *v)
{
	const void *cbuf;
	int iw, ih, ic;

	cbuf = canvas->cbuf;

	iw = canvas->new_iwidth;
	ih = canvas->new_iheight;
	ic = canvas->new_idepth;

	canvas->cbuf = crop_img->data()[0];
	canvas->new_iwidth = crop_img->w();
	canvas->new_iheight = crop_img->h();
	canvas->new_idepth = crop_img->d();

	print_cb(wid, v);

	canvas->cbuf = cbuf;
	canvas->new_iwidth = iw;
	canvas->new_iheight = ih;
	canvas->new_idepth = ic;
}

#ifdef OPJ_HAVE_LIBPNG
static void crop_save_as_png_cb(Fl_Widget *wid, void *v)
{
	const void *cbuf;
	int iw, ih, ic;
	
	cbuf = canvas->cbuf;
	
	iw = canvas->new_iwidth;
	ih = canvas->new_iheight;
	ic = canvas->new_idepth;
	
	canvas->cbuf = crop_img->data()[0];
	canvas->new_iwidth = crop_img->w();
	canvas->new_iheight = crop_img->h();
	canvas->new_idepth = crop_img->d();
	
	save_as_png_cb(wid, v);
	
	canvas->cbuf = cbuf;
	canvas->new_iwidth = iw;
	canvas->new_iheight = ih;
	canvas->new_idepth = ic;
}
#endif

static void crop_save_as_opj_cb(Fl_Widget *wid, void *v)
{
	const void *cbuf;
	int iw, ih, ic;
	
	cbuf = canvas->cbuf;
	
	iw = canvas->new_iwidth;
	ih = canvas->new_iheight;
	ic = canvas->new_idepth;
	
	canvas->cbuf = crop_img->data()[0];
	canvas->new_iwidth = crop_img->w();
	canvas->new_iheight = crop_img->h();
	canvas->new_idepth = crop_img->d();
	
	save_as_opj_cb(wid, v);
	
	canvas->cbuf = cbuf;
	canvas->new_iwidth = iw;
	canvas->new_iheight = ih;
	canvas->new_idepth = ic;
}

Fl_Menu_Item crop_popup_items[] =
{
 {EXIT_s,  0, &crop_cancel_cb,  NULL, FL_MENU_DIVIDER, 0, 4, 15, FL_YELLOW},
 {PRINT_s, 0, &crop_print_cb, NULL, FL_MENU_DIVIDER, 0, 4, 15, 0},
#ifdef OPJ_HAVE_LIBPNG
 {SAVE_AS_PNG_s, 0, &crop_save_as_png_cb, NULL, FL_MENU_DIVIDER, 0, 4, 15, 0},
#endif
 {SAVE_AS_OPJ_s, 0, &crop_save_as_opj_cb, NULL, FL_MENU_DIVIDER, 0, 4, 15, 0},
 {NULL, 0, NULL, NULL, 0, 0, 0, 0, 0}
};

static void crop_popup_create(int x, int y, int ww, int hh)
{
	Fl_Menu_Button *popup;

	popup = new Fl_Menu_Button(x, y, ww, hh);
	popup->type(Fl_Menu_Button::POPUP3);
	popup->menu(crop_popup_items);
}

static void crop_window_cb(Fl_Widget *wid, void *v)
{
	unsigned char *dst, *d;
	const unsigned char *s;
	int ww, hh, idepth, i, iwidth, s_step, d_step;

	if(canvas->cbuf == NULL) return;

	if(crop_width == 0 && crop_height == 0) return;
//One crop window only:
	if(crop_win != NULL) return;

	idepth = canvas->new_idepth;
	dst = (unsigned char*)malloc(crop_width * crop_height * idepth);

	if(dst == NULL) return;

	hh = crop_height + 10; ww = crop_width + 10;

    crop_win = new Fl_Double_Window(220, 220, ww, hh);
	crop_win->box(FL_FLAT_BOX);
	crop_win->color(main_win->color());
	crop_win->begin();

	crop_box = new Fl_Box(5, 5, crop_width, crop_height);
	crop_box->box(FL_FLAT_BOX);
	crop_box->color(scroll_box->color());
	crop_box->align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER);

//	crop_win->resizable(crop_box);

	crop_popup_create(0, 0, ww, hh);
    crop_win_done = 0;
    crop_win->end();
    crop_win->show();

	iwidth = canvas->new_iwidth;

	s = (const unsigned char*)canvas->cbuf 
	 + (iwidth * crop_lhs_top_y + crop_lhs_top_x) * idepth;

	s_step = iwidth * idepth;
	d_step = crop_width * idepth;
	d = dst;

	for(i = 0; i < crop_height; ++i)
   {
	memcpy(d, s, d_step);
	d += d_step;
	s += s_step;
   }
	crop_img = new 
	 Fl_RGB_Image(dst, crop_width, crop_height, idepth);

	crop_box->image(crop_img);
	crop_box->redraw();

	while(crop_win_done == 0)
	 Fl::wait();

	crop_win_done = 0;
    delete crop_img;
    crop_img = NULL;

    delete crop_win;
    crop_win = NULL;
}

#ifdef _WIN32
CRITICAL_SECTION CriticalSection;

#else /* not _WIN32 */

static int semid;
static struct sembuf sema;

static union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
} su;
#endif /* _WIN32 */

#ifdef _WIN32

#define IDLE_SPIN 0x00000400

#else /* not _WIN32 */

static void critical(int id, int op)
{
	sema.sem_op = op; sema.sem_flg = SEM_UNDO;
	if(semop(id, &sema, 1) == -1)
	 fprintf(stderr,"%s:%d:semop %d failed\n",__FILE__,__LINE__,op); 	
}

#define ENTER_CRITICAL(i) critical(i, -1)
#define LEAVE_CRITICAL(i) critical(i, 1)

#endif /* _WIN32 */

void FLViewer_clear_tree()
{
	if(tree) tree->clear();
}

void FLViewer_header_deactivate()
{
    tile_in->value("");
    tiles_out->value("");
    tiles_group->deactivate();

	reduct_in->value("");
	reduct_out->value("");
    reduct_group->deactivate();

    reload_jp2->deactivate();
    area_in->deactivate();

    FLViewer_area_activate(0);
	FLViewer_layer_component_activate(0);

    pause_but->deactivate();
    resume_but->deactivate();
    restart_but->deactivate();
    fin_but->deactivate();

    forward_but->deactivate();
    backward_but->deactivate();

    goto_frame_in->value("");
    all_frames_out->value("");
    frames_group->deactivate();

    goto_track_in->value("");
    all_tracks_out->value("");
    tracks_group->deactivate();

}//FLViewer_header_activate()

#ifdef HAVE_THREADS
void FLViewer_threads_activate(int v)
{
    if(v) 
	 threads_spinner->activate(); 
	else 
	 threads_spinner->deactivate();
}
#endif

void FLViewer_movie_runs(int v)
{
	movie_runs = v;
}

void FLViewer_close_reader(void)
{
	if(reader)
   {
	fclose(reader); reader = NULL;
   }
}

static void spinner_cb(Fl_Widget *wid, void *v)
{
	if( !wid->contains(Fl::belowmouse())) return;

#ifdef HAVE_THREADS
   {
	Fl_Spinner *spin = (Fl_Spinner*)wid;

	if(spin->value() < 0. || spin->value() > MAX_SPINVALUE)
	 spin->value(0);

	nr_threads = (int)spin->value();
   }
#endif
}

int FLViewer_nr_threads(void)
{
#ifdef HAVE_THREADS
	threads_spinner->value(nr_threads);
	return nr_threads;
#else
	threads_spinner->value(0);
	return 0;
#endif
}

static void zoomin_cb(Fl_Widget *w, void *v)
{
	if(movie_runs) return;

   fscale += 0.05;
   chooser_cb(read_idf);
}

static void zoomin_50_cb(Fl_Widget *w, void *v)
{
	if(movie_runs) return;

	fscale += 0.5;
	chooser_cb(read_idf);
}

static void zoomin_25_cb(Fl_Widget *w, void *v)
{
	if(movie_runs) return;

	fscale += 0.25;
	chooser_cb(read_idf);
}

static void zoomout_cb(Fl_Widget *w, void *v)
{
	if(movie_runs) return;

	if(fscale < 0.06) return;

	fscale -= 0.05;
	chooser_cb(read_idf);
}

static void zoomout_50_cb(Fl_Widget *w, void *v)
{
	if(movie_runs) return;

	if(fscale < 0.6) return;

	fscale -= 0.5;
	chooser_cb(read_idf);
}

static void zoomout_25_cb(Fl_Widget *w, void *v)
{
	if(movie_runs) return;

	if(fscale < 0.3) return;

	fscale -= 0.25;
	chooser_cb(read_idf);
}

static void reset_zoom_cb(Fl_Widget *w, void *v)
{
	if(movie_runs) return;

   fscale = 1.0;
   chooser_cb(read_idf);
}

static void tile_in_cb(Fl_Widget *w, void *v)
{
	const char *cs;
	int i;

	if( !w->contains(Fl::belowmouse())) return;

	cs = ((Fl_Input*)w)->value();

    if(cs == NULL || *cs == 0)
     i = -1;
    else
     i = atoi(cs);

    if(i >= max_tiles) i = max_tiles - 1;

    cur_tile = i;

    user_changed_tile = user_changed_reduct = 1;
    user_changed_area = 0;

    chooser_cb(read_idf);

    if(i < 0)
     tile_in->value("");
    else
   {
    snprintf(vbuf, sizeof(vbuf)-1, "%d", i);
    tile_in->value(vbuf);
   }
    tile_in->redraw();
}

static void area_in_cb(Fl_Widget *w, void *v)
{
	const char *cs;

	if( !w->contains(Fl::belowmouse())) return;

	cs = ((Fl_Input*)w)->value();

    if(cs == NULL || *cs == 0)
   {
    area_in->value("x0,y0,x1,y1");
    area_in->redraw();
    return;
   }
	area_x0 = area_y0 = area_x1 = area_y1 = 0;

	if(sscanf(cs, "%d,%d,%d,%d", &area_x0,&area_y0,&area_x1,&area_y1) != 4)
   {
	fprintf(stderr,"AREA(%s) unusable\n",cs);
	return;
   }
	if(area_x0 < 0 || area_x1 < 0 || area_y0 < 0 || area_y1 < 0) return;
	if(area_x1 <= area_x0 || area_y1 <= area_y0) return;

	user_changed_area = 1;
	user_changed_tile = user_changed_reduct = 0;

    tile_in->value("");
    tile_in->redraw();
    cur_tile = -1;

    chooser_cb(read_idf);

    snprintf(vbuf, sizeof(vbuf)-1, "%d,%d,%d,%d", area_x0, area_y0,
	 area_x1, area_y1);
    area_in->value(vbuf);
    area_in->redraw();
}

static void reduct_in_cb(Fl_Widget *w, void *v)
{
	const char *cs;
	int i;

	if( !w->contains(Fl::belowmouse())) return;

	cs = ((Fl_Input*)w)->value();

    if(cs == NULL || *cs == 0)
     i = 0;
    else
     i = atoi(cs);

    if(i < 0) return;

    if(max_reduction > 0 && i >= max_reduction) return;

    cur_reduction = i;

    user_changed_tile = user_changed_reduct = 1;

    chooser_cb(read_idf);

    snprintf(vbuf, sizeof(vbuf)-1, "%d", i);
    reduct_in->value(vbuf);
    reduct_in->redraw();
}

static void test_new_jp2_file()
{
    int new_jp2_file;

    if(reloaded)
   {
    reloaded = 0;

    fscale = 1.0;
    user_changed_area = 0;

    area_x0 = area_x1 = area_y0 = area_y1 = 0;
	area_in->value("x0,y0,x1,y1");
	area_in->redraw();

    max_tiles = max_reduction = 0;

    tile_in->value("");
    tile_in->redraw();
    reduct_in->value("0");
    reduct_in->redraw();

    user_changed_tile = 0; user_changed_reduct = 0;

    max_tiles = max_reduction = -1;

    cur_tile = -1; cur_reduction = 0;

    cur_layers = max_layers = 0;
    cur_component = max_components = 0;
    component_in->value("");
    component_out->value("");

    if(JPEG2000_build_tree(read_idf) == 0)
  {
    fprintf(stderr,"%s:%d:\n\t=== NO TREE BUILT ===\n",__FILE__,__LINE__);
  }

    return;
   }

    if(jp2_file)
   {
    if(strcmp(jp2_file, read_idf))/* New file */
  {
    new_jp2_file = 1;
  }
    else
  {
    new_jp2_file = 0;
  }
   }
    else
   {
    new_jp2_file = 1;
   }

    if(new_jp2_file)
   {
    if(jp2_file) free(jp2_file);

    jp2_file = strdup(read_idf);

	
    fscale = 1.0;
    user_changed_area = 0;
    area_x0 = area_y0 = area_x1 = area_y1 = 0;
	area_in->value("x0,y0,x1,y1");
	area_in->redraw();

    tile_in->value("");
    tile_in->redraw();
    cur_tile = -1;

/* To get the new values:
*/
	max_tiles = max_reduction = 0;
	user_changed_tile = 0;
#ifdef RESET_REDUCTION
    cur_reduction = 0;
    reduct_in->value("");
    reduct_out->value("");
	user_changed_reduct = 0;
#endif

	layer_in->value("");
	layer_out->value("");
	cur_layers = max_layers = 0;

	component_in->value("");
	component_out->value("");
	cur_component = max_components = 0;

#ifdef HAVE_THREADS
    nr_threads = 0;
    threads_spinner->value(0);
    threads_spinner->deactivate();
#endif

    if(JPEG2000_build_tree(read_idf) == 0)
  {
    fprintf(stderr,"%s:%d:\n\t=== NO TREE BUILT ===\n",__FILE__,__LINE__);
  }
   }
}//test_new_jp2_file()

static void test_new_mj2_file()
{
    int new_file;

//FLViewer_movie_runs(1);

    if(mj2_file)
   {
    if(strcmp(mj2_file, read_idf))
  { 
    free(mj2_file);
    new_file = 1;
  } 
    else
  { 
    new_file = 0;
  }
   }
    else
   {
    new_file = 1;
   }
   
    if(new_file)
   {
    mj2_file = strdup(read_idf);
    
    layer_in->value("");
    layer_in->deactivate();

    layer_out->value("");
    layer_out->deactivate();

    component_in->activate();
    component_out->activate();

	OPENMJ2_free_tracks();

    if(JPEG2000_build_tree(read_idf) == 0)
  {
    fprintf(stderr,"%s:%d:\n\t=== NO TREE BUILT ===\n",__FILE__,__LINE__);
  }
   }
}//test_new_mj2_file()

static void reload_jp2_cb(Fl_Widget *w, void *v)
{
    tile_in->value("");
    tile_in->redraw();
    cur_tile = -1;

    cur_reduction = 0;

    user_changed_tile = user_changed_reduct = 0;

    reloaded = 1;

    chooser_cb(read_idf);

    reduct_in->value("0");
    reduct_in->redraw();
}

void FLViewer_get_max_tiles_and_reduction(int *out_mx_tiles, int *out_mx_reduct)
{
	*out_mx_tiles = max_tiles; *out_mx_reduct = max_reduction;
}

void FLViewer_set_max_reduction(int mx_reduct)
{
	max_reduction = mx_reduct;
}

void FLViewer_set_max_tiles(int mx_tiles)
{
	max_tiles = mx_tiles;
}

void FLViewer_put_max_tiles_and_reduction(int mx_tiles, int mx_reduct)
{
    tiles_out->value("");
    tiles_out->redraw();

    reduct_out->value("");
    reduct_out->redraw();

    max_tiles = mx_tiles;
    max_reduction = mx_reduct;
    user_changed_tile = 1; user_changed_reduct = 1;

	snprintf(vbuf, sizeof(vbuf)-1, "%d", cur_reduction);
    reduct_in->value(vbuf);
    reduct_in->redraw();
}

void FLViewer_reset_tiles_and_reduction()
{
	tiles_out->value("");
	tiles_out->redraw();

	reduct_out->value("");
	reduct_out->redraw();

	user_changed_tile = user_changed_reduct = 0;
	cur_tile = -1; cur_reduction = 0;
	max_tiles = -1;
	max_reduction = -1;

	snprintf(vbuf, sizeof(vbuf)-1, "%d", cur_reduction);
	reduct_in->value(vbuf);
	reduct_in->redraw();
}

static void tiles_write_out(void)
{
    if(user_changed_tile && user_changed_reduct)
   {
    if(max_tiles > 0)
  {
    snprintf(vbuf, sizeof(vbuf)-1, "%d", max_tiles - 1);
    tiles_out->value(vbuf);
    tiles_out->redraw();
  }

	if(cur_reduction > -1)
  {
	snprintf(vbuf, sizeof(vbuf)-1, "%d", cur_reduction);
	reduct_in->value(vbuf);
	reduct_in->redraw();
  }
    if(max_reduction > 0)
  {
    snprintf(vbuf, sizeof(vbuf)-1, "%d", max_reduction - 1);
    reduct_out->value(vbuf);
    reduct_out->redraw();
  }
   }
}

int FLViewer_has_tile_and_reduction(void)
{
	return (user_changed_tile && user_changed_reduct);
}

void FLViewer_get_tile_and_reduction(int *out_tile, int *out_reduct)
{
	*out_tile = cur_tile; *out_reduct = cur_reduction;
}

int FLViewer_has_area_values(void)
{
	return user_changed_area;
}

void FLViewer_get_area_values(int *out_x0, int *out_y0,
	int *out_x1, int *out_y1)
{
	*out_x0 = area_x0; *out_y0 = area_y0;
	*out_x1 = area_x1; *out_y1 = area_y1;
}

void FLViewer_set_max_mj2_tracks(unsigned int n)
{
	int s;

	if((s = canvas->selected_track) < 1)
	 s = 1;

	mj2_max_tracks = (int)n-1;
    snprintf(vbuf, sizeof(vbuf)-1, "%d/%d", s, mj2_max_tracks);
    all_tracks_out->value(vbuf);
}

void FLViewer_set_max_components(unsigned int n)
{
	max_components = (int)n;
    snprintf(vbuf, sizeof(vbuf)-1, "%d", n);
	component_out->value(vbuf);
}

void FLViewer_set_max_layers(unsigned int n)
{
	max_layers = (int)n;
    snprintf(vbuf,sizeof(vbuf)-1, "%d", max_layers);
	layer_out->value(vbuf);
	layer_in->value("0");
}

int FLViewer_layers()
{
	return cur_layers;
}

int FLViewer_component()
{
	return cur_component;
}

void FLViewer_layer_component_activate(int v)
{
	if(v)
   {
	component_in->activate();
	component_out->activate();

    snprintf(vbuf, sizeof(vbuf)-1,"%d", cur_component);
    component_in->value(vbuf);

    snprintf(vbuf,sizeof(vbuf)-1,"%d", max_components);
    component_out->value(vbuf);

	layer_in->activate();
	layer_out->activate();

    snprintf(vbuf, sizeof(vbuf)-1, "%d", cur_layers);
	layer_in->value(vbuf);

    snprintf(vbuf, sizeof(vbuf)-1, "%d", max_layers);
    layer_out->value(vbuf);
   }
	else
   {
	layer_in->value("");
	layer_out->value("");
	layer_in->deactivate();
	layer_out->deactivate();
   }
}

static void layer_in_cb(Fl_Widget *wid, void *v)
{
	int i;

	if(max_layers == 0) 
   {
	layer_in->value("");
	return;
   }
	i = atoi(layer_in->value());

	if(i < 0 || i > max_layers) 
   {
    snprintf(vbuf, sizeof(vbuf)-1, "%d", cur_layers);
    layer_in->value(vbuf);

	return;
   }
	cur_layers = i;

	chooser_cb(read_idf);	
}

static void component_in_cb(Fl_Widget *wid, void *v)
{
	int i;

	if(max_components == 0) 
   {
	component_in->value("");
	return;
   }
	i = atoi(component_in->value());

	if(i < 0 || i > max_components) 
   {
    snprintf(vbuf, sizeof(vbuf)-1, "%d", cur_component);
    component_in->value(vbuf);

	return;
   }
	cur_component = i;

	chooser_cb(read_idf);
}

class Header : public Fl_Group 
{

public:
	Header(int x, int y, int w, int h, const char *l);
	virtual int handle(int event);
};

static Header *header;

Header::Header(int X, int Y, int W, int H, const char *l)
	: Fl_Group(X, Y, W, H, l)
{
	int xx, yy, ww, bx, bw, gw, gh;
	begin();

	xx = X; yy = Y;
   {
	int ux = xx + BUTTON_W + 10;
	url_out =
	 new Fl_Output(ux, yy, xx + W - ux, BUTTON_H, FILENAME_s);
	url_out->textsize(LABEL_SIZE);
	url_out->labelsize(LABEL_SIZE);
	url_out->clear_visible_focus();
   }
	xx = X; yy += BUTTON_H + 3; 
	gw = 70 + FRAMES_W + 60 + FRAMES_W/2 + 4; gh = BUTTON_H + 4;

	frames_group = new Fl_Group(xx, yy, gw, gh);
	frames_group->box(FL_EMBOSSED_BOX);
	frames_group->begin();

	xx += 70; yy += 2;
	all_frames_out =
	 new Fl_Output(xx, yy, FRAMES_W, BUTTON_H, ALLFRAMES_s);
	all_frames_out->labelsize(LABEL_SIZE);
	all_frames_out->textsize(LABEL_SIZE);
	all_frames_out->clear_visible_focus();

	xx += FRAMES_W + 60;
	goto_frame_in =
	 new Fl_Input(xx, yy,  FRAMES_W/2, BUTTON_H, START_s);
	goto_frame_in->callback(goto_frame_cb);
	goto_frame_in->when(FL_WHEN_ENTER_KEY_ALWAYS);
	goto_frame_in->labelsize(LABEL_SIZE);
	goto_frame_in->textsize(LABEL_SIZE);

	frames_group->end();

	xx = frames_group->x() + frames_group->w() + 5;
	yy = frames_group->y();
	ww = TRACKS_W/2;
	gw = 70 + ww + 60 + ww + 4; gh = BUTTON_H + 4;

	tracks_group = new Fl_Group(xx, yy, gw, gh);
	tracks_group->box(FL_EMBOSSED_BOX);
	tracks_group->begin();

	xx += 70; yy += 2;
    all_tracks_out =
     new Fl_Output(xx, yy, ww, BUTTON_H, ALLTRACKS_s);
    all_tracks_out->labelsize(LABEL_SIZE);
    all_tracks_out->textsize(LABEL_SIZE);
    all_tracks_out->clear_visible_focus();

    xx += ww + 60;
    goto_track_in = 
     new Fl_Input(xx, yy,  ww, BUTTON_H, START_s);
    goto_track_in->callback(goto_track_cb);
    goto_track_in->when(FL_WHEN_ENTER_KEY_ALWAYS);
    goto_track_in->labelsize(LABEL_SIZE);
    goto_track_in->textsize(LABEL_SIZE);

	tracks_group->end();

    xx = tracks_group->x() + tracks_group->w() + 10;
	yy = tracks_group->y() + 2;

   {
    Fl_Group *g = new Fl_Group(xx, yy, 115, BUTTON_H);
    g->box(FL_FLAT_BOX);
    g->color(DEFAULT_BACKGROUND+1);
    g->begin();

    xx += 70;
    threads_spinner = new Fl_Spinner(xx, yy, 40, BUTTON_H, "Threads:");
    threads_spinner->labelsize(LABEL_SIZE);
    threads_spinner->minimum(0);
    threads_spinner->maximum(6);
    threads_spinner->step(1);
    threads_spinner->value(0);
    threads_spinner->callback(spinner_cb);
    threads_spinner->deactivate();

    g->end();
	xx = g->x() + g->w();
   }
	xx += 10;
    reload_jp2 = new Fl_Button(xx, yy, BUTTON_W + 15, BUTTON_H, RELOAD_s);
    reload_jp2->callback(reload_jp2_cb);
    reload_jp2->labelsize(LABEL_SIZE);
    reload_jp2->deactivate();

	xx = X; yy = reload_jp2->y() + reload_jp2->h() + 5;
	ww = 70 + 40 + 40 + 40;

	tiles_group = new Fl_Group(xx, yy, ww, BUTTON_H);
	tiles_group->begin();

	xx += 70;
	tile_in =
	 new Fl_Input(xx, yy, 40, BUTTON_H, TILE_s);
	tile_in->callback(tile_in_cb);
	tile_in->when(FL_WHEN_RELEASE_ALWAYS);
	tile_in->labelsize(LABEL_SIZE);
	tile_in->textsize(LABEL_SIZE);
    tile_in->value("");
    tile_in->redraw();

	xx += 40 + 40;
	tiles_out =
	 new Fl_Output(xx, yy, 40, BUTTON_H, OF_s);
	tiles_out->labelsize(LABEL_SIZE);
	tiles_out->textsize(LABEL_SIZE);
	tiles_out->clear_visible_focus();
    tiles_out->value("");
    tiles_out->redraw();

	tiles_group->end();
	tiles_group->deactivate();

	xx = tiles_group->x() + tiles_group->w() + 5;
	yy = tiles_group->y();
	ww = 115 + 30 + 40 + 30;

	reduct_group = new Fl_Group(xx, yy, ww, BUTTON_H);
	reduct_group->begin();

	xx += 115;
	reduct_in =
	 new Fl_Input(xx, yy, 30, BUTTON_H, REDUCTION_s);
	reduct_in->callback(reduct_in_cb);
	reduct_in->when(FL_WHEN_RELEASE_ALWAYS);
	reduct_in->labelsize(LABEL_SIZE);
	reduct_in->textsize(LABEL_SIZE);
	reduct_in->value(" ");
	reduct_in->redraw();

	xx += 30 + 40;
	reduct_out =
	 new Fl_Output(xx, yy, 30, BUTTON_H, OF_s);
	reduct_out->labelsize(LABEL_SIZE);
	reduct_out->textsize(LABEL_SIZE);
	reduct_out->clear_visible_focus();
	reduct_out->value(" ");
	reduct_out->redraw();

	reduct_group->end();
	reduct_group->deactivate();

	xx = reduct_group->x() + reduct_group->w() + 10;
	ww = 70 + 97;

	xx += 75;
	area_in =
	 new Fl_Input(xx, yy, 218, BUTTON_H, AREA_s);
	area_in->callback(area_in_cb);
	area_in->labelsize(LABEL_SIZE);
	area_in->textsize(LABEL_SIZE);
	area_in->value("x0,y0,x1,y1");
	area_in->when(FL_WHEN_RELEASE_ALWAYS);
	area_in->deactivate();

	user_changed_tile = user_changed_reduct = 0;
	max_tiles = 0; cur_tile = -1;
	max_reduction = 0; cur_reduction = 0;

	yy = tiles_group->y() + tiles_group->h() + 5;
	xx = X;

	bx = xx;

	pause_but = new Fl_Button(bx,yy,BUTTON_W,BUTTON_H,STOP_s);
	pause_but->callback(pause_cb);
	pause_but->labelsize(LABEL_SIZE);
	bx += BUTTON_W;

	resume_but = new Fl_Button(bx,yy,BUTTON_W,BUTTON_H,GOON_s);
	resume_but->callback(resume_cb);
	resume_but->labelsize(LABEL_SIZE);
	bx += BUTTON_W;

	restart_but = new Fl_Button(bx,yy,BUTTON_W,BUTTON_H,RESTART_s);
	restart_but->callback(restart_cb);
	restart_but->labelsize(LABEL_SIZE);
	bx += BUTTON_W;

	fin_but = new Fl_Button(bx,yy,BUTTON_W,BUTTON_H,CLOSEFILE_s);
	fin_but->callback(fin_cb);
	fin_but->labelsize(LABEL_SIZE);
	bx += BUTTON_W;

	xx = fin_but->x() + fin_but->w() + 5;
	yy = fin_but->y();

	bx = xx;

	backward_but = new Fl_Button(bx,yy,BUTTON_W,BUTTON_H,BACKWARD_s);
	backward_but->callback(backward_cb);
	backward_but->labelsize(LABEL_SIZE);
	bx += BUTTON_W;

	forward_but = new Fl_Button(bx,yy,BUTTON_W,BUTTON_H,FORWARD_s);
	forward_but->callback(forward_cb);
	forward_but->labelsize(LABEL_SIZE);
	bx += BUTTON_W;

    xx = forward_but->x() + forward_but->w() + 10;
    yy = forward_but->y();
	bw = BUTTON_W + 3;
   {
    Fl_Button *b = new Fl_Button(xx, yy, bw, BUTTON_H, CROP_s);
    b->callback(crop_window_cb);
    b->labelsize(LABEL_SIZE);
   }
	xx += bw + 5;
   {
	Fl_Button *b = new Fl_Button(xx,yy,BUTTON_W,BUTTON_H,BROWSE_s);
	b->callback(browse_cb);
	yy = b->y() + b->h() + 5;
   }
	xx = X + 70;
	layer_in = new Fl_Input(xx, yy, 30, BUTTON_H, LAYER_s);
	layer_in->callback(layer_in_cb);
	layer_in->deactivate();

	xx += layer_in->w() + 10 + 30;
	layer_out = new Fl_Output(xx, yy, 30, BUTTON_H, OF_s);
	layer_out->deactivate();

	xx += layer_out->w() + 10 + 100;
	component_in = new Fl_Input(xx, yy, 30, BUTTON_H, COMPONENT_s);
	component_in->callback(component_in_cb);
	component_in->deactivate();

	xx += component_in->w() + 10 + 30;
	component_out = new Fl_Output(xx, yy, 30, BUTTON_H, OF_s);
	component_out->deactivate();

	
	FLViewer_frames_animation(0);

	end();
}/* Header::Header() */

int Header::handle(int event)
{
	if(!Fl::event_inside(x(), y(), w(), h() ) ) return 0;

	if(event == FL_LEAVE) return 1;

	return Fl_Group::handle(event);
}

static unsigned char *print_buffer(size_t len)
{
	unsigned char *buf;

	if((buf = (unsigned char*)malloc(len)))
	 memcpy(buf, canvas->cbuf, len);

	return buf;
}

static void print_cb(Fl_Widget *wid, void *v)
{
	const char *cs;
	FILE *writer;
	int ok, iw, ih, ic;
	PSInfo ps;
	PrintInfo print;

	if(movie_runs || !canvas->cbuf)
   {
	fl_alert("%s", NO_FILE_TO_PRINT_s);
	return;
   }
	memset(&ps, 0, sizeof(PSInfo));
	memset(&print, 0, sizeof(PrintInfo));
/*----------------------------*/
	Print_gui_create(&print);
/*----------------------------*/
	if( !print.ok) return;

	writer = fopen(print.fname_s, "wb");

	if(writer == NULL)
   {
	cs = CANNOT_OPEN_s;
   }
	else
   {
	cs = SUCCESS_WITH_s;
	ps.writer = writer;

	iw = canvas->new_iwidth;
	ih = canvas->new_iheight;
	ic = canvas->new_idepth;

	ps.src_buf = print_buffer(iw*ih*ic);

	if(ps.src_buf == NULL)
  {
	fclose(writer); ps.writer = NULL;
	return;
  }
	ps.bg_red = 255; ps.bg_green = 255; ps.bg_blue = 255;

	ps.title_s = print.title_s;
	ps.media_s = print.media_s;
	ps.center = print.center;
	ps.landscape = print.want_landscape;
	ps.portrait = print.want_portrait;
/*	want_color, want_gray */
	ps.fmedia_w = (int)(print.fpaper_w * 72./25.4);
	ps.fmedia_h = (int)(print.fpaper_h * 72./25.4);

	ps.image_w = iw; ps.image_h = ih;
	ps.image_channels = ic;
	ps.fmargin_lhs = (int)(print.fmargin_lhs * 72./25.4);
	ps.fmargin_top = (int)(print.fmargin_top * 72./25.4);
	ps.fmargin_rhs = (int)(print.fmargin_rhs * 72./25.4);
	ps.fmargin_bot = (int)(print.fmargin_bot * 72./25.4);
/*===============================*/
	ok = PS_image_draw(&ps);
/*===============================*/
	fclose(writer);
	free(ps.src_buf);

	if( !ok)
	 cs = FAILURE_WITH_s;
	else
	if(print.command_s)
  {
#ifdef _WIN32
	fl_message("%s\n%s",USE_SYSTEM_COMMAND_TO_PRINT_s, print.fname_s);
#else
	char *cmd;
	size_t len;
	int ret;

	len = strlen(print.command_s) + strlen(print.printer_s)
	 + strlen(print.fname_s) + 32;
	cmd = (char*)malloc(len);
/*-----
'man system':

 Do  not  use  system()  from a program with set-user-ID or set-group-ID
 privileges, because strange values for some environment variables might
 be  used  to subvert system integrity.  Use the exec(3) family of func-
 tions instead, but not execlp(3) or execvp(3)
------*/
	while(print.nr_copies)
 {
	snprintf(cmd, len, "%s%s %s", print.command_s, print.printer_s,
	 print.fname_s);
	ret = system(cmd);

	if(WIFSIGNALED(ret)
	&& (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
	 break;

	--print.nr_copies;
 }
	free(cmd);
#endif /* _WIN32 */
  }
   }
	fl_message("%s\n%s", cs, print.fname_s);

	if(print.remove_fname)
	 remove(print.fname_s);

	free(print.fname_s);
	free(print.printer_s);
	free(print.command_s);
	free(print.format_s);
	free(print.title_s);
	free(print.media_s);

}/* print_cb() */

#ifdef OPJ_HAVE_LIBPNG

static void save_as_png_cb(Fl_Widget *w, void *v)
{
	const char *cs, *period;
	char *write_idf;
	int ok;
	struct stat sb;

	if(movie_runs || !canvas->cbuf)
   {
	fl_alert("%s", NO_FILE_TO_SAVE_s);
	return;
   }
	cs = fl_input(ENTER_PNG_TO_SAVE_s, "name.png");

	if(cs == NULL || *cs == 0) return;

	if(stat(cs, &sb) == 0)
   {
	if(fl_choice(FILE_EXISTS_s, "NO", "YES", NULL) == 0) return;
   }
	ok =
	(   ((period = strrchr(cs, '.')) != NULL)
	 && (strncasecmp(period, ".png", 4) == 0));

	if(!ok)
   {
	if(fl_choice(MISSING_PNG_EXT_s, "NO", "YES", NULL) == 0)
	 return;
   }
    write_idf = strdup(cs);

    PNG_save_file(canvas, write_idf);

    free(write_idf);
}
#endif // OPJ_HAVE_LIBPNG 

static void save_as_opj_cb(Fl_Widget *w, void *v)
{
    const char *cs;
    char *write_idf;
    struct stat sb;

    if(movie_runs || !canvas->cbuf)
   {
    fl_alert("%s", NO_FILE_TO_SAVE_s);
    return;
   }
    cs = fl_input(ENTER_OPJ_TO_SAVE_s, "name.[jp2|j2k|jpc|j2c]");

    if(cs == NULL || *cs == 0) return;

    if(stat(cs, &sb) == 0)
   {
    if(fl_choice(FILE_EXISTS_s, "NO", "YES", NULL) == 0) return;
   }
    write_idf = strdup(cs);

    JP2_save_file(canvas, write_idf);

    free(write_idf);
}

/*
{"IDF",shortcut,CB,user_data,FLAGS,labeltype,LABELFONT,LABELSIZE,labelcolor}
*/
Fl_Menu_Item popup_items[] =
{
 {EXIT_s,  0, &exit_cb,  NULL, FL_MENU_DIVIDER, 0, 4, 15, FL_YELLOW},
 {PRINT_s, 0, &print_cb, NULL, FL_MENU_DIVIDER, 0, 4, 15, 0},
 {"  -5%",  0, &zoomout_cb, NULL, FL_NORMAL_LABEL, 0, 4, 15, 0},
 {"  -25%", 0, &zoomout_25_cb, NULL, FL_NORMAL_LABEL, 0, 4, 15, 0},
 {"  -50%", 0, &zoomout_50_cb, NULL, FL_MENU_DIVIDER, 0, 4, 15, 0},
 {"  +5%",  0, &zoomin_cb, NULL, FL_NORMAL_LABEL, 0, 4, 15, 0},
 {"  +25%", 0, &zoomin_25_cb, NULL, FL_NORMAL_LABEL, 0, 4, 15, 0},
 {"  +50%", 0, &zoomin_50_cb, NULL, FL_MENU_DIVIDER, 0, 4, 15, 0},
 {"  100%", 0, &reset_zoom_cb, NULL, FL_MENU_DIVIDER, 0, 4, 15, 0},
#ifdef OPJ_HAVE_LIBPNG
 {SAVE_AS_PNG_s, 0, &save_as_png_cb, NULL, FL_MENU_DIVIDER, 0, 4, 15, 0},
#endif
 {SAVE_AS_OPJ_s, 0, &save_as_opj_cb, NULL, FL_MENU_DIVIDER, 0, 4, 15, 0},
 {NULL, 0, NULL, NULL, 0, 0, 0, 0, 0}
};

static void popup_create(int x, int y)
{
	Fl_Menu_Button *popup;

	popup = new Fl_Menu_Button(x, y, MAX_SCROLLER_W, MAX_SCROLLER_H);
	popup->type(Fl_Menu_Button::POPUP3);
	popup->menu(popup_items);
}

static void show_image(Fl_Image* img)
{
	int scroll_h, scroll_w, iw, ih, win_w, win_h;
	int has_hbar, has_vbar, bar_size;
	unsigned char type;

	if((bar_size = scroll->scrollbar_size()) == 0) bar_size = BAR_SIZE;

#ifdef _WIN32
	EnterCriticalSection(&CriticalSection);
#else
	ENTER_CRITICAL(semid);
#endif

	has_hbar = has_vbar = 0;
	iw = img->w(); ih = img->h();
/* movie without scrollbars, the whole image is visible: 
*/
	if( !movie_runs)
   {
	if(iw > MAX_SCROLLER_W) 
  {
	has_hbar = bar_size;

	if(ih + bar_size > MAX_SCROLLER_H)
	 has_vbar = bar_size;
  }
	else
	if(ih > MAX_SCROLLER_H)
  {
	has_vbar = bar_size;

	if(iw + bar_size > MAX_SCROLLER_W)
	 has_hbar = bar_size;
  }
   }

	if(has_vbar)
	 scroll_h = MAX_SCROLLER_H;
	else
	 scroll_h = ih + has_hbar;

	if(has_hbar)
	 scroll_w = MAX_SCROLLER_W;
	else
	 scroll_w = iw + has_vbar;

	win_h = HEADER_H + scroll_h + BORDER_SIZE;
	win_w = SCROLLER_X + scroll_w + BORDER_SIZE;

	if(win_w < WINDOW_MIN_W) win_w = WINDOW_MIN_W;
	if(win_h < WINDOW_MIN_H) win_h = WINDOW_MIN_H;

	main_win->size(win_w, win_h);
	scroll->size(scroll_w, scroll_h);
	scroll_box->size(iw, ih);
	scroll_box->image(img);

	type = 0;
	if(has_hbar) type |= Fl_Scroll::HORIZONTAL;
	if(has_vbar) type |= Fl_Scroll::VERTICAL;

	scroll->type(type);
	scroll->scroll_to(0,0);

	main_win->redraw();

#ifdef _WIN32
	LeaveCriticalSection(&CriticalSection);
#else
	LEAVE_CRITICAL(semid);
#endif

}/* show_image() */

void FLViewer_use_buffer(const void *b, unsigned int w, unsigned int h,
    int depth)
{
	Fl_RGB_Image *rgb, *new_rgb;

	rgb = canvas->new_rgb;

	scroll_box->image(NULL);
	canvas->new_rgb = NULL;

	new_rgb = new Fl_RGB_Image((unsigned char*)b, (int)w, (int)h, depth);

	if(new_rgb)
   {
	if(fscale != 1.0)
  {
	Fl_Image *temp;
	double fw, fh;

	fw = (double)new_rgb->w() * fscale;
	fh = (double)new_rgb->h() * fscale;

	temp = new_rgb->copy((int)fw, (int)fh);

	delete new_rgb;
	new_rgb = (Fl_RGB_Image*)temp;
  }
	canvas->new_idepth = new_rgb->d();
	canvas->new_iwidth = new_rgb->w();
	canvas->new_iheight = new_rgb->h();

	canvas->cbuf = new_rgb->data()[0];

	show_image((Fl_Image*)new_rgb);
   }
	if(rgb) delete rgb;

	canvas->new_rgb = new_rgb;

}/* FLViewer_use_buffer() */

void FLViewer_area_activate(int v)
{
	if(v)
   {
	area_in->activate();
   }
	else
   {
	area_in->deactivate();
   }
}

void FLViewer_tiles_activate(int v)
{
	if(v)
   {
	tiles_group->activate();
	reduct_group->activate();
	reload_jp2->activate();

	tiles_write_out();
   }
	else
   {
	tiles_out->value("");
	tiles_out->redraw();
	tiles_group->deactivate();

	cur_reduction = 0; max_reduction = 0;
    reduct_in->value("");
    reduct_out->value("");
    reduct_group->deactivate();

	reload_jp2->deactivate();
   }
}

void FLViewer_frames_animation(int v)
{
	FLViewer_animation(v);

	goto_frame_in->value("");
	goto_frame_in->redraw();
	all_frames_out->value("");
	all_frames_out->redraw();

	goto_track_in->value("");
	goto_track_in->redraw();
	all_tracks_out->value("");
	all_tracks_out->redraw();

	if(v)
   {
	frames_group->activate();
	tracks_group->activate();
   }
	else
   {
	frames_group->deactivate();
	tracks_group->deactivate();

    tiles_out->value("");
    tile_in->value("");
    tiles_group->deactivate();

	cur_reduction = 0; max_reduction = 0;
    reduct_in->value("");
    reduct_out->value("");
    reduct_group->deactivate();
   }
	if(canvas)
   {
	canvas->selected_frame = -1;
	canvas->top_frame = -1;

	canvas->selected_track = -1;
	canvas->top_track = -1;
   }
}

void FLViewer_animation(int v)
{
    if(v)
   {
    pause_but->activate();
    resume_but->activate();
    restart_but->activate();
    fin_but->activate();

    backward_but->activate();
    forward_but->activate();
   }
    else
   {
    pause_but->deactivate();
    resume_but->deactivate();
    restart_but->deactivate();
    fin_but->deactivate();

    backward_but->deactivate();
    forward_but->deactivate();
   }
}

void FLViewer_mj2_animation(int v)
{
	FLViewer_frames_animation(v);
}

void FLViewer_show_frame(int n, int m)
{
	char all_buf[32];

	snprintf(all_buf, sizeof(all_buf)-1, "%d/%d", n, m);
	all_frames_out->value(all_buf);
}

void FLViewer_show_track(int n, int m)
{
	char all_buf[32];
	int s;

	if((s = canvas->selected_track) < 1)
	 s = 1;

	snprintf(all_buf,sizeof(all_buf)-1,"%d/%d", s, m);
	all_tracks_out->value(all_buf);
}

void FLViewer_clear_wait(void)
{
	url_out->value(""); url_out->redraw();
}

void FLViewer_wait(void)
{
	url_out->value(WAITPLEASE_s);
}

void FLViewer_url(const char *fname, int w, int h)
{
	const char *cs = NULL;
#ifdef _WIN32
	cs = strrchr(fname, '\\');
#else
	cs = strrchr(fname, '/');
#endif
	if(cs) ++cs; else cs = fname;

	snprintf(url_buf, 511, "%s (%d x %d)",cs,w,h);
	url_out->value(url_buf);
}

static void load_image(FILE *fp, const char *fname, uint64_t fsize)
{
	unsigned char magic_buf[32];

    memset(magic_buf, 0, 32);
    (void)fread(magic_buf, 1, 32, fp);
    rewind(fp);

	FLViewer_header_deactivate();

#ifdef OPJ_HAVE_LIBPNG
	if(memcmp(magic_buf, PNG_MAGIC, 8) == 0)
   {
	if(tree) tree->clear();

	component_in->value("");
	component_in->deactivate();
	component_out->value("");
	component_out->deactivate();

	PNG_load(canvas, fp, fname, fsize);

	return;
   }
#endif

	if(memcmp(magic_buf, JP2_RFC3745_MAGIC, 12) == 0
	&& memcmp(magic_buf+20, "\x6d\x6a\x70\x32", 4) == 0)
   {
    FLViewer_close_reader();

    if(strlen(read_idf) > OPJ_PATH_LEN - 2)
  {
    fl_alert("%s:\n%s", read_idf, FILENAME_TOO_LONG_s);
    return;
  }
	FLViewer_reset_tiles_and_reduction();

	test_new_mj2_file();

	OPENMJ2_load(canvas, fname, fsize);

	return;
   }

	if(memcmp(magic_buf, JP2_RFC3745_MAGIC, 12) == 0
	|| memcmp(magic_buf, JP2_MAGIC, 4) == 0
	|| memcmp(magic_buf, J2K_CODESTREAM_MAGIC, 4) == 0
	  )
   {
	unsigned char *src;
	FTYPInfo ftyp;
	int ret;

    if(strlen(read_idf) > OPJ_PATH_LEN - 2)
  {
    fl_alert("%s:\n%s", read_idf, FILENAME_TOO_LONG_s);
    FLViewer_close_reader();
    return;
  }

    if((src = (unsigned char*)malloc(fsize)) == NULL)
  {
    FLViewer_close_reader();
    return;
  }
    (void)fread(src, 1, fsize, fp);
    FLViewer_close_reader();

    ret = JPEG2000_test_ftyp(fname, src, fsize, &ftyp);

	free(src); src = NULL;

    if(ret == 0)
  {
    if(ftyp.is_j2k == 0)
 {
    fprintf(stderr,"%s:%d:\n\tload_image(%s) FAILED\n",
     __FILE__,__LINE__,fname);
    return;
 }
  }

	FLViewer_layer_component_activate(1);

	test_new_jp2_file();

    if(cmd_line_reduction > 0)
  {
    user_changed_tile = 1; user_changed_reduct = 1;
    cur_reduction = cmd_line_reduction;

    FLViewer_tiles_activate(1);

    cmd_line_reduction = 0;
  }

	OPENJPEG_load(canvas, fname, src, fsize, ftyp.decod_format);

	return;
   }
#ifdef WANT_PGX
	if(memcmp(buf, "PG", 2) == 0)
   {
	if(tree) tree->clear();

	PGX_load(canvas, fp, fname, fsize);

	return;
   }
#endif //WANT_PGX

	fl_alert("%s\n%s\n %s", CANNOT_USE_FILE_s, fname,
		NO_DRIVER_FOUND_s);
} /* load_image() */

static void chooser_cb(const char *cs)
{
	struct stat sb;

	if(cs == NULL || *cs == 0) return;

	if(stat(cs, &sb) < 0)
   {
	fl_alert("%s\n%s", FILE_NOT_FOUND_s, cs);
	return;
   }
	if((sb.st_mode & S_IFMT) == S_IFDIR) return;

   {
	char dbuf[4096];

#ifdef _WIN32
	strncpy(dbuf, cs, 4095); // WIN32 without soft link
#else
	ssize_t n;
	unsigned int reg;
	char sbuf[4096];

	strncpy(sbuf, cs, 4095); *dbuf = 0;

	if(lstat(sbuf, &sb) == 0)
  {
	reg = ((S_IFREG & sb.st_mode) == (S_IFMT & sb.st_mode));

	if( !reg && (n = readlink(sbuf, dbuf, 4095)) > 0)
	 dbuf[n] = 0;
	else
	 strncpy(dbuf, cs, 4095);
  }
#endif

	if(stat(dbuf, &sb) < 0)
  {
	fl_alert("%s\n%s", FILE_NOT_FOUND_s, dbuf);
	return;
  }

	if(sb.st_size == 0)
  {
	fl_alert("%s\n%s", FILE_SIZE_ZERO_s, dbuf);
	return;
  }

	if(sb.st_size > FILESIZE_LIMIT)
  {
	fl_alert("%s\n%s\n", dbuf, FILESIZE_TOO_LONG_s);
	return;
  }

	start_overlay = 1;
	main_win->redraw_overlay();

	if(canvas->cleanup)
  {
	canvas->cleanup();
	canvas->cleanup = NULL;
  }
	FLViewer_close_reader();

	if(read_idf)
  {
	free(read_idf); read_idf = NULL;
  }
/*-------------------------------*/
	reader = fopen(dbuf, "rb");

	if(reader == NULL)
  {
	perror(dbuf);
	fl_alert("%s\n%s", FILE_NOT_FOUND_s, dbuf);

	return;
  }
	read_idf = strdup(dbuf);
	movie_runs = 0;

	load_image(reader, read_idf, (uint64_t)sb.st_size);
   }   
}/* chooser_cb() */

static void goto_frame_cb(Fl_Widget *w, void *v)
{
	const char *cs;
	int i;

	if( !w->contains(Fl::belowmouse())) return;

	cs = ((Fl_Input*)w)->value();

	if(cs && *cs)
   {
	i = atoi(cs);

	if(i < 1) return;
	if(i > canvas->top_frame) i = canvas->top_frame;

	canvas->selected_frame = i;
   }
}

static void goto_track_cb(Fl_Widget *w, void *v)
{
	const char *cs;
	int i;

	if( !w->contains(Fl::belowmouse())) return;

	cs = ((Fl_Input*)w)->value();

	if(cs && *cs)
   {
	i = atoi(cs);

	if(i < 1) { goto_track_in->value(""); return; }

	if(i > canvas->top_track) 
  {
	if(canvas->top_track < 1)
	 canvas->top_track = 1;

	i = canvas->top_track;

    snprintf(vbuf, sizeof(vbuf)-1, "%d", i);
    goto_track_in->value(vbuf);
	goto_track_in->redraw();
  }
	canvas->selected_track = i;
   }
}

int FLViewer_frame_selected(void)
{
	int n;

	if((n = canvas->selected_frame) >= 1)
   {
	canvas->selected_frame = -1;
	return n;
   }
	return -1;
}

// see Fl::option(Fl::OPTION_FNFC_USES_GTK, false) in main()
//
static void browse_cb(Fl_Widget *wid, void *v)
{
    Fl_Native_File_Chooser native;

    native.title("Select Image File");
    native.type(Fl_Native_File_Chooser::BROWSE_FILE);
    native.filter(IMG_PATTERN);
    native.preset_file(read_idf);

#ifdef _WIN32
    if(canvas->cleanup)
   {
    canvas->cleanup();
    canvas->cleanup = NULL;
   }
#endif //_WIN32

    switch(native.show())
   {
    case -1:
        fprintf(stderr, "FILE_CHOOSER_ERROR: %s\n", native.errmsg());
        break;

    case  1: /* Cancel */
        break;

    default:
        if(native.filename())
         chooser_cb(native.filename());
        break;
   }
}

static void pause_cb(Fl_Widget *wid, void *v)
{
	if(canvas->pause) canvas->pause();
}

static void resume_cb(Fl_Widget *wid, void *v)
{
	if(canvas->resume) canvas->resume();
}

static void restart_cb(Fl_Widget *wid, void *v)
{
	if(canvas->restart) canvas->restart();
}

static void backward_cb(Fl_Widget *wid, void *v)
{
	if(canvas->backward) canvas->backward();
}

static void forward_cb(Fl_Widget *wid, void *v)
{
	if(canvas->forward) canvas->forward();
}

static void fin_cb(Fl_Widget *wid, void *v)
{
	if(canvas->fin) canvas->fin();
}

static void exit_cb(Fl_Widget *wid, void *v)
{
	if(canvas->cleanup)
   {
	canvas->cleanup();
   }

#ifdef _WIN32
	DeleteCriticalSection(&CriticalSection);
#else /* not _WIN32 */
	if(semctl(semid, 0, IPC_RMID, 0) == -1)
	 fprintf(stderr,"%s:%d: semctl IPC_RMID failed\n",__FILE__,__LINE__);
#endif /* _WIN32 */

	if(read_idf != NULL) free(read_idf);
	if(jp2_file != NULL) free(jp2_file);
	if(mj2_file != NULL) free(mj2_file);
	if(reader != NULL) fclose(reader);

	free(root_dir);
	delete main_win;

	exit(0);
}

static void usage(void)
{
	const char *bar=
"\n------------------------------------------------------------------\n";

	fputs(bar, stderr);

	fprintf(stderr,"%s\n"
		"USAGE: flviewer [--bg BACKGROUND_COLOR] [--r INT] [FILE]\n",
		PACKAGE_STRING);

	fputs("\twith BACKGROUND_COLOR = "
	"(\"TEXT\" | \"#RGB\" | \"rgb:R/G/B\" )\n"
	  "\t     e.g. (--bg \"red\" | --bg \"#ff0000\" "
	  "| --bg \"rgb:ff/00/00\" )", stderr);
	fputs("\n\t--r INT means the reduction on start, e.g. --r 2\n", stderr);
	fputs(bar, stderr);
}

static void shrink_name(char *buf)
{
	char *s, *d;
	int ch;

	s = d = buf;
	while((ch = *s++))
  {
	if(isspace(ch)) continue;
	*d++ = (char)tolower(ch);
  }
	*d = 0;
}

int main(int argc, char *argv[])
{
	const char *fname, *s;
	int i, x, y, found_bgcolor, reduct;
	unsigned char rc, gc, bc;

	cmd_line_reduction = reduct = 0;
#ifdef _WIN32
	if(!InitializeCriticalSectionAndSpinCount(&CriticalSection,
	 IDLE_SPIN) )
   {
	fprintf(stderr,"%s:%d: InitializeCriticalSectionAndSpinCount failed\n",
	__FILE__,__LINE__);

	return 1;
   }
#else /* not _WIN32 */
	semid = semget(getpid(), 1, S_IRWXU|S_IRWXG|S_IRWXO|IPC_CREAT|IPC_EXCL);

	if(semid == -1)
   {
	fprintf(stderr,"%s:%d:semget failed\n",__FILE__,__LINE__);
	perror(NULL);
	return 1;
   }
/* triggers BUG in PowerPC: if(semctl(semid, 0, SETVAL, 1) == -1) */
	su.val = 1;

	if(semctl(semid, 0, SETVAL, su) == -1)
   {
	fprintf(stderr,"%s:%d:semctl SETVAL failed\n",__FILE__,__LINE__);
	perror(NULL);
	return 1;
   }
#endif /* _WIN32 */

	Fl::visual(FL_RGB);
	fl_register_images();

#ifndef _WIN32
#ifdef HAVE_OPTION_FNFC_USES_GTK
	Fl::option(Fl::OPTION_FNFC_USES_GTK, false);
#endif
#endif

	fname = NULL; found_bgcolor = 0;

	for(i = 1; i < argc; ++i)
   {
	s = argv[i];

	if(strcmp(s, "--help") == 0
	|| strcmp(s, "-help") == 0
	|| *s == '?'
	|| strcmp(s, "--version") == 0
	|| strcmp(s, "-V") == 0
	  )
  {
	usage();
	return 0;
  }
	if(strcasecmp(s, "--bg") == 0)
  {
	++i;
	s = argv[i];

	if(*s == '#' || strncasecmp(s, "rgb:", 4) == 0 || isalpha(*s))
 {
	strncpy(bgcolor_buf, s, MAX_COLORBUF);
	bgcolor_buf[MAX_COLORBUF] = 0;
	found_bgcolor = 1;

	if(*s != '#')
	 shrink_name(bgcolor_buf);
	continue;
 }
	continue;
  }
    if(strcasecmp(s, "--r") == 0)
  {
    ++i;
    s = argv[i];
    reduct = atoi(s);
    continue;
  }
	if(*s != '-')
  {
	fname = s; continue;
  }
   }/* for() */
#ifdef _WIN32
	if((root_dir = getenv("UserProfile")))
	 root_dir = strdup(root_dir);
	else
	 root_dir = strdup("C:\\Windows\\Temp");
#else
	root_dir = strdup(getenv("HOME"));
#endif
	start_overlay = 1;

	win_w = WINDOW_MIN_W; win_h = WINDOW_MIN_H;

	main_win = new Overlay(WIN_X, WIN_Y, win_w, win_h);
	main_win->color(DEFAULT_BACKGROUND);
	main_win->begin();

	x = BORDER_SIZE; y = BORDER_SIZE;

	header = new Header(x, y, HEADER_W, HEADER_H - BORDER_SIZE, NULL);
	header->resizable(NULL);

	tree = new UserTree(TREE_X, TREE_Y, TREE_W, TREE_H);
	tree->begin();

    tree->box(FL_DOWN_BOX);
    tree->color((Fl_Color)55);
    tree->selection_color((Fl_Color)55);
    tree->labeltype(FL_NORMAL_LABEL);
    tree->labelfont(0);
    tree->labelsize(12);
    tree->item_labelsize(12);
    tree->labelcolor(FL_FOREGROUND_COLOR);
    tree->align(Fl_Align(FL_ALIGN_TOP));
    tree->when(FL_WHEN_RELEASE);
#ifdef USE_CUSTOM_ICONS
    tree->openicon(&L_openpixmap);
    tree->closeicon(&L_closepixmap);
#endif
    tree->connectorstyle(FL_TREE_CONNECTOR_SOLID);
    tree->marginleft(0);
    tree->connectorwidth(7);

#if FLTK_ABI_VERSION >= 10304
	tree->item_reselect_mode(FL_TREE_SELECTABLE_ALWAYS);
#endif

	tree->root_label(" ");
    tree->end();

	canvas = (Canvas*)calloc(1, sizeof(Canvas));

	if(found_bgcolor)
   {
	if( !rgb_color_values(bgcolor_buf, rc, gc, bc))
	 fl_parse_color(bgcolor_buf, rc, gc, bc);
   }
	else 
	 Fl::get_color(DEFAULT_CANVAS_BACKGROUND, rc, gc, bc);

	canvas->bg_red = rc; canvas->bg_green = gc;
	canvas->bg_blue = bc;

	scroll = new
	 Fl_Scroll(SCROLLER_X, SCROLLER_Y, MIN_SCROLLER_W,MIN_SCROLLER_H);
	scroll->box(FL_FLAT_BOX);
	scroll->begin();

	scroll_box = new
	 ImageBox(SCROLLER_X, SCROLLER_Y, MIN_SCROLLER_W,MIN_SCROLLER_H);
	scroll_box->box(FL_FLAT_BOX);
	scroll_box->align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER);
	scroll_box->color(fl_rgb_color(rc, gc, bc));

	scroll->end();
	scroll->resizable(scroll_box);
	scroll->type(0);

	popup_create(SCROLLER_X, SCROLLER_Y);

	main_win->end();
/*---------
	main_win->size_range(main_win->w(), main_win->h(),
	 0, 0, 0, 0);
-----------*/
	main_win->show();
	
	if(fname)
   {
    if(reduct > 0)
  {
    cmd_line_reduction = reduct;

    user_changed_tile = 1; user_changed_reduct = 1;
    cur_reduction = reduct;

    FLViewer_tiles_activate(1);

  }
	chooser_cb(fname);
   }
	return Fl::run();
}//main()
