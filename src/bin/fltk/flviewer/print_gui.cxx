#include <config.h>
/*
 * author(s) and license
*/
/* 
 * TODO: background_color other than FL_WHITE for transparent pixels
 * TODO: minimal margin: 6.35 mm (18 dots); 6.703 mm (19 dots)
*/
#ifdef _WIN32
#include <windows.h>

#define popen _popen
#define pclose _pclose
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_ask.H>
#include <FL/filename.H>


#include "lang/dialog_lang.h_utf8"
#include "print_gui.hh"
#include "paper_sizes.hh"

extern char *root_dir;
/* configurable option ? */
#define SHOW_PRINTER_LOC

#define LOCAL_PRINTER

#ifdef LOCAL_PRINTER
#define LOCAL_CMD   "pdq -P"
#define LOCAL_NAME  "postscript"
#define LOCAL_LOC   "Kyocera FS-920"
#endif

#define PNAME_POS 0
#define PRINT_CMD "lpr -P"

#define WIN_W 640
#define WIN_H 380
#define TAB_W (WIN_W - 10)
#define GROUP_W TAB_W
#define GROUP_H (WIN_H - 60)
#define BOX_W (GROUP_W - 10)


/* lpadmin -L "location" */

typedef struct printer_info
{
    char *cmd;
    char *name;
    char *loc;
}PrinterInfo; 

static PrinterInfo *printer_info;

static void fill_paper_size(PrintInfo *print)
{
	Fl_Choice *c = print->paper_size;
	int i;

	i = 0;
	while(paper_sizes[i].name)
   {
	c->add(paper_sizes[i].name);
	++i;
   }
	c->value(PSIZE_POS);
	print->paper_size_pos = PSIZE_POS;
}

static void paper_size_cb(Fl_Widget *wid, void *v)
{
	PrintInfo *print = (PrintInfo*)v;
	print->paper_size_pos = print->paper_size->value();
}

static PrinterInfo *append(PrinterInfo *tab, const char *n)
{
    const char *o;
    int i = 0;

    if(tab)
   {
    while((o = tab[i].name))
  {
    if(strcmp(o, n) == 0) return tab;
    ++i;
  }
   }
    tab = (PrinterInfo*)realloc(tab, (i+2)*sizeof(PrinterInfo));

	if(tab != NULL)
   {
	tab[i].cmd = strdup(PRINT_CMD);
    tab[i].name = strdup(n); 

	tab[i].loc = strdup(n);

	tab[i+1].name = NULL; tab[i+1].loc = NULL; tab[i+1].cmd = NULL;
   }
    return tab;
}
#ifndef _WIN32
/*

lpstat -p

printer FS_920 is idle.  enabled since ...
printer HP_Desk is idle.  enabled since ...
*/
static void add_lpr_printer_names(void)
{
    FILE *p;
    char buf[1024];

    if((p = popen("lpstat -p 2> /dev/null", "r")) == NULL) 
   {
	fputs("\n    Can not print\n", stderr);
	return;
   }
    while(fgets(buf, sizeof(buf), p) != NULL)
   {
    char *start, *end;

    start = buf;
    if(strncmp(start, "printer ", 8) != 0) continue;
    start += 8;
    if(strstr(start, "enabled") == NULL) continue;
    end = start;
    while(*end && !isspace(*end)) ++end;
    *end = 0;
    printer_info = append(printer_info, start);
   }
    pclose(p);
}
#endif /* _WIN32 */

static void free_printer_info()
{
    int i = 0;

    while(printer_info[i].name)
   {
	free(printer_info[i].cmd);
    free(printer_info[i].name);

    free(printer_info[i].loc);

    ++i;
   }
	free(printer_info);

}

static void fill_printer_names(PrintInfo *print)
{
	Fl_Choice *c = print->printer_name;
	int i = 0;

#ifdef LOCAL_PRINTER
	printer_info = (PrinterInfo*)calloc(2, sizeof(PrinterInfo));

	printer_info[0].cmd = strdup(LOCAL_CMD);
	printer_info[0].name = strdup(LOCAL_NAME);
	printer_info[0].loc = strdup(LOCAL_LOC);

	if(printer_info == NULL
	|| printer_info[0].cmd == NULL
	|| printer_info[0].name == NULL
	|| printer_info[0].loc == NULL)
   {
	fprintf(stderr,"print_gui.cxx:%d:\n\tmemory out\n",__LINE__);
	exit(1);
   }
#endif

#ifndef _WIN32
	add_lpr_printer_names();
#endif /* _WIN32 */

	while(printer_info[i].name)
   {
	c->add(printer_info[i].name);
	++i;
   }
	c->value(PNAME_POS);
	print->printer_pos = PNAME_POS;
}

static void printer_name_cb(Fl_Widget *w, void *v)
{
	PrintInfo *print = (PrintInfo*)v;
	print->printer_pos = print->printer_name->value();

#ifdef SHOW_PRINTER_LOC
	print->loc->value(printer_info[print->printer_pos].loc);
#endif
}

static void to_printer_cb(Fl_Widget *wid, void *data)
{
	Fl_Radio_Button *b = (Fl_Radio_Button*)wid;
	PrintInfo *print = (PrintInfo*)data;

	if(!b->value())
   {
	print->printer_group->deactivate();
	print->file_group->activate();
	print->fname->value("image.ps");
   }
	else
   {
	print->printer_group->activate();
	print->fname->value("");
	print->file_group->deactivate();
   }
}

static void to_file_cb(Fl_Widget *wid, void *data)
{
	Fl_Radio_Button *b = (Fl_Radio_Button*)wid;
	PrintInfo *print = (PrintInfo*)data;

    if(!b->value())
   {
	print->fname->value("");
	print->file_group->deactivate();
	print->printer_group->activate();
   }
    else
   {
	print->fname->value("image.ps");
	print->file_group->activate();
	print->printer_group->deactivate();
   }

}

static void portrait_cb(Fl_Widget *w, void *data)
{
	PrintInfo *info = (PrintInfo*)data;
	info->paper_layout[0] = 'P';
}

static void landscape_cb(Fl_Widget *w, void *data)
{
	PrintInfo *info = (PrintInfo*)data;
	info->paper_layout[0] = 'L';
}

static void lowres_cb(Fl_Widget *w, void *data)
{
	PrintInfo *info = (PrintInfo*)data;
	info->resolution = 300;
}

static void hires_cb(Fl_Widget *w, void *data)
{
	PrintInfo *info = (PrintInfo*)data;
	info->resolution = 600;
}

static void color_cb(Fl_Widget *w, void *data)
{
	PrintInfo *info = (PrintInfo*)data;
	info->pcolor[0] = 'C';
}

static void gray_cb(Fl_Widget *w, void *data)
{
	PrintInfo *info = (PrintInfo*)data;
	info->pcolor[0] = 'G';
}

static void ok_cb(Fl_Widget *w, void *data)
{
	PrintInfo *print = (PrintInfo*)data;
	const char *cs;
	struct stat sb;

	if(print->to_file->value())
   {
	cs = print->fname->value();
	if(*cs == 0)
  {
	fl_message("%s", NO_FILENAME_FOUND_s);
	return;
  }
	if(stat(cs, &sb) == 0)
  {
	if((sb.st_mode & S_IFMT) == S_IFDIR)
 {
	fl_message("%s", NAME_IS_DIR_s);
	return;
 }
	if(fl_choice(CONTINUE_PRINT_s, "NO", "YES", NULL) == 0) return;
  }
	print->fname_s = strdup(cs);

	if(print->fname_s == NULL)
  {
	fprintf(stderr,"print_gui.cxx:%d:\n\tmemory out\n",__LINE__);
	exit(1);
  }
   }/* if(print->to_file->value()) */
	else
	if(print->to_printer->value())
   {
	size_t len = strlen(root_dir) + 20;

	print->printer_s = strdup(print->printer_name->text());
	print->command_s = strdup(printer_info[print->printer_pos].cmd);
	print->fname_s = (char*)malloc(len);

	if(print->printer_s == NULL 
	|| print->command_s == NULL 
	|| print->fname == NULL)
   {
	fprintf(stderr,"print_gui.cxx:%d:\n\tmemory out\n",__LINE__);
	exit(1);
   }
#ifdef _WIN32
	sprintf_s(print->fname_s, len, "%s\\.flimage.ps", root_dir);
#else
	snprintf(print->fname_s, len, "%s/.flimage.ps", root_dir);
#endif
	print->remove_fname = 1;
   }/* if(print->to_printer->value()) */

	print->format_s = strdup(paper_sizes[print->paper_size_pos].name);
	print->media_s = strdup(paper_sizes[print->paper_size_pos].media_s);

	if(print->format_s == NULL || print->media_s == NULL)
   {
	fprintf(stderr,"print_gui.cxx:%d:\n\tmemory out\n",__LINE__);
	if(print->format_s) free(print->format_s);

	exit(1);
   }
	print->fpaper_w = paper_sizes[print->paper_size_pos].paper_w;
	print->fpaper_h = paper_sizes[print->paper_size_pos].paper_h;

	cs = print->copies->value();

	if(*cs)
	 print->nr_copies = atoi(cs);
	else
	 print->nr_copies = 1;

	print->title_s = strdup(print->title->value());

	if(print->title_s == NULL)
   {
	fprintf(stderr,"print_gui.cxx:%d:\n\tmemory out\n",__LINE__);
	exit(1);
   }
	print->fmargin_lhs = atof(print->margin_lhs->value());
	print->fmargin_rhs = atof(print->margin_rhs->value());
	print->fmargin_top = atof(print->margin_top->value());
	print->fmargin_bot = atof(print->margin_bot->value());

	print->center = print->center_image->value();
	if(print->paper_layout[0] == 'L')
	 print->want_landscape = 1;
	else
	 print->want_portrait = 1;

	if(print-> pcolor[0] == 'C')
	 print->want_color = 1;
	else
	 print->want_gray = 1;

	if(printer_info)
	 free_printer_info();

	delete print->win;

	print->ok = 1;

	print->win_destroy = 1;

}/* ok_cb() */

static void cancel_cb(Fl_Widget *, void *data) 
{
	PrintInfo *print = (PrintInfo*)data;

	if(printer_info)
	 free_printer_info();

	delete print->win;

	print->ok = 0;

	print->win_destroy = 1;
}

void Print_gui_create(PrintInfo *print)
{
	Fl_Window* win;
	Fl_Group *page1;
	int x, y, group_x, group_y, top_y, lhs_x;
	int label_x, label_y;

	print->win = win = new Fl_Window(WIN_W, WIN_H);
	win->box(FL_FLAT_BOX);
	win->color(FL_BACKGROUND_COLOR);
	win->begin();

	group_x = 0; group_y = 20;

	page1 = new Fl_Group(group_x, group_y, GROUP_W, GROUP_H, PRINTERPAGE_s);
	page1->box(FL_FLAT_BOX);
	page1->color(FL_BACKGROUND_COLOR);
	page1->labelfont(FL_HELVETICA_BOLD_ITALIC);
	page1->labelsize(13);
	page1->labelcolor(FL_BLACK);
	page1->begin();

	group_x += 4; group_y += 12; 
   {
	Fl_Group *format;

	x = group_x; y = group_y;
	format = new Fl_Group(x, y, GROUP_W-6, 96);
	format->box(FL_EMBOSSED_BOX);
	format->begin();

	x += 10; y -= 10;
	label_x = x; label_y = y;
	lhs_x = group_x; top_y = group_y + 12;
	x = lhs_x; y = top_y;
  {
    Fl_Round_Button *o;
    Fl_Group *g = new Fl_Group(x, y, 150, 40);
    g->begin();
	x += 3; y += 0;

    Fl_Round_Button *r1 = new Fl_Round_Button(x, y, 150, 20, COLOR_s);
	r1->type(FL_RADIO_BUTTON);
    r1->callback(color_cb, print);
	r1->clear_visible_focus();
    print->color = r1;

    y += 20;
    o = new Fl_Round_Button(x, y, 150, 20, GRAY_s);
    o->value(1);
	o->type(FL_RADIO_BUTTON);
	print->pcolor[0] = 'G';
    o->callback(gray_cb, print);
	o->clear_visible_focus();
    print->gray = o;
    g->end();
	y = g->y() + g->h();
  }
	x = lhs_x;
  {
    Fl_Round_Button *o;
    Fl_Group *g = new Fl_Group(x, y, 150, 40);
    g->begin();

    x += 3; y += 0;
    o = new Fl_Round_Button(x, y, 150, 20, LOWRES_s);
    o->callback(lowres_cb, print);
	o->type(FL_RADIO_BUTTON);
    o->value(1);
	o->clear_visible_focus();
    print->resolution = 300;
    print->lowres = o;

    y += 20;
    o = new Fl_Round_Button(x, y, 150, 20, HIRES_s);
	o->type(FL_RADIO_BUTTON);
    o->callback(hires_cb, print);
	o->clear_visible_focus();
    print->hires = o;

    g->end();
    x = g->x() + g->w() + 10; 
  }
	y = top_y;
	print->paper_size = new Fl_Choice(x,y,110,23);
	print->paper_size->clear_visible_focus();
	fill_paper_size(print);
	print->paper_size->callback(paper_size_cb, print);

	y = top_y; x = format->w() - 154;
  {
    Fl_Round_Button *o;
    Fl_Group *g = new Fl_Group(x, y, 150, 40);
    g->begin();

	x += 0; y += 0;
    o = new Fl_Round_Button(x, y, 150, 20, PORTRAIT_s);
	o->type(FL_RADIO_BUTTON);
    o->callback(portrait_cb, print);
    o->value(1);
	o->clear_visible_focus();
    print->paper_layout[0] = 'P';
    print->portrait = o;

    y += 20;
    o = new Fl_Round_Button(x, y, 150, 20, LANDSCAPE_s);
	o->type(FL_RADIO_BUTTON);
    o->callback(landscape_cb, print);
	o->clear_visible_focus();
    print->landscape = o;

    g->end();
	y = g->y() + g->h();
  }
	x = format->w() - 150; 
    print->center_image = new Fl_Check_Button(x, y, 25, 25, CENTER_s);
	print->center_image->clear_visible_focus();

	format->end(); 
	group_y = format->y() + format->h() + 12;
  {
    Fl_Group *c = new Fl_Group(label_x, label_y, 100, 20, DOCFORMAT_s);
    c->box(FL_EMBOSSED_BOX);
    c->align(FL_ALIGN_INSIDE);
    c->end();
  }
   }
	x = group_x; y = group_y;
   {
	Fl_Group *g;
	g = new Fl_Group(x, y, 178, 38);
	g->box(FL_EMBOSSED_BOX);
	g->begin();

	label_x = x+10; label_y = y-10;
	x += 7; y += 12;
  {
	Fl_Round_Button *o = new Fl_Round_Button(x, y, 96, 22, PRINTER_s);
	o->type(FL_RADIO_BUTTON);
	o->callback(to_printer_cb, print);
	o->value(1);
	o->clear_visible_focus();
	print->to_printer = o;
  }
	x += 100;	
  {
	Fl_Round_Button *o = new Fl_Round_Button(x, y, 67, 22,  FILE_s);
	o->type(FL_RADIO_BUTTON);
	o->callback(to_file_cb, print);
	o->clear_visible_focus();
	print->to_file = o;
  }	
	g->end();
	x = g->x() + g->w() + 1;
  {
    Fl_Group *c = new Fl_Group(label_x, label_y, 100, 20, PRINTTO_s);
    c->box(FL_EMBOSSED_BOX);
    c->align(FL_ALIGN_INSIDE);
    c->end();
  }
   }
	y = group_y;
   {
	Fl_Group *g;
	g = new Fl_Group(x, y, GROUP_W-x-4, 40);
	g->box(FL_EMBOSSED_BOX);
	g->begin();

	label_x = x+10; label_y = y-10;
	x += 70; y += 12;
  {
	print->copies = new Fl_Int_Input(x, y, 50, 24, COPIES_s);
	print->copies->value("1");
  }
	x += 130;
  {
	print->title = new Fl_Input(x, y, g->x() + g->w()-x-4, 24, TITLE_s);
	print->title->value("No title");
  }
	g->end();
	group_y = g->y() + g->h() + 12;
  {
    Fl_Group *c = new Fl_Group(label_x, label_y, 100, 20, PRINT_s);
    c->box(FL_EMBOSSED_BOX);
    c->align(FL_ALIGN_INSIDE);
    c->end();
  }
   }
	x = group_x; y = group_y;
	print->printer_group = new Fl_Group(x, y-10, GROUP_W-6, 50);
	print->printer_group->begin();
   {
	Fl_Choice *o;
	Fl_Group *g;
	g = new Fl_Group(x, y, GROUP_W-6, 40);
	g->box(FL_EMBOSSED_BOX);
    g->begin();

	label_x = x+10; label_y = y-10;
	x += 120; y += 12;
	o = new Fl_Choice(x, y, 140, 24, PRINTERNAMES_s);
	o->align(FL_ALIGN_LEFT);
	o->callback(printer_name_cb, print);
	print->printer_name = o;

	fill_printer_names(print);

#ifdef SHOW_PRINTER_LOC
	x += 240;
	print->loc = new Fl_Output(x, y, 140, 24, LOC_s);
	print->loc->value(printer_info[print->printer_pos].loc);
	print->loc->clear_visible_focus();
#endif
	g->end();
  {
    Fl_Group *c = new Fl_Group(label_x, label_y, 100, 20, PRINTER_s);
    c->box(FL_EMBOSSED_BOX);
    c->align(FL_ALIGN_INSIDE);
    c->end();
  }
   }
	print->printer_group->end();
	group_y = print->printer_group->y() + print->printer_group->h() + 12;

	x = group_x; y = group_y;
	print->file_group = new Fl_Group(x, y-10, GROUP_W-6, 50);
	print->file_group->deactivate();
	print->file_group->begin();
   {
	Fl_Group *g;
	g = new Fl_Group(x, y, GROUP_W-6, 40);
	g->box(FL_EMBOSSED_BOX);
    g->begin();

	label_x = x+10; label_y = y-10;
	x += 120; y += 12;
	print->fname = new Fl_Input(x, y, 280, 24, NAME_s);

    g->end();
  {
    Fl_Group *c = new Fl_Group(label_x, label_y, 100, 20, FILE_s);
    c->box(FL_EMBOSSED_BOX);
    c->align(FL_ALIGN_INSIDE);
    c->end();
  }

   }
	print->file_group->end();
    group_y = print->file_group->y() + print->file_group->h() + 12;

	x = group_x; y = group_y;
   {
    Fl_Group *g = new Fl_Group(x, y, GROUP_W-6, 40);
    g->box(FL_EMBOSSED_BOX);
    g->begin();

	label_x = x+10; label_y = y-10;
	x += 80; y += 12;
	print->margin_lhs = new Fl_Float_Input(x, y, 50, 24, LEFT_s);
	print->margin_lhs->value("25.51");

	x += 130;
	print->margin_rhs = new Fl_Float_Input(x, y, 50, 24, RIGHT_s);
	print->margin_rhs->value("6.35");

	x += 130;
	print->margin_top = new Fl_Float_Input(x, y, 50, 24, TOP_s);
	print->margin_top->value("6.35");

	x += 130;
	print->margin_bot = new Fl_Float_Input(x, y, 50, 24, BOTTOM_s);
	print->margin_bot->value("6.35");

    g->end();
  {
    Fl_Group *c = new Fl_Group(label_x, label_y, 100, 20, MARGINS_s);
    c->box(FL_EMBOSSED_BOX);
    c->align(FL_ALIGN_INSIDE);
    c->end();
  }

   }
	page1->end();

	y = WIN_H - 35;
	int dx = (WIN_W - 200)/3;
	x = dx;
   {
    Fl_Button* o = new Fl_Button(x, y, 100, 30, PRINT_s);
    o->callback(ok_cb, print);
   }
	x += 100 + dx;
   {
    Fl_Button* o = new Fl_Button(x, y, 100, 30, CANCEL_s);
	o->color(FL_YELLOW);
    o->callback(cancel_cb, print);
   }
	win->end();
	win->show();

	while( !print->win_destroy)
	 Fl::wait(1.0);

}/* Print_gui_create() */
