#ifndef _FLVIEWER_HH_
#define _FLVIEWER_HH_

#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_RGB_Image.H>
#include <limits.h>

/* off_t st_size; */
#define FILESIZE_LIMIT (LONG_MAX - 100)

#define IS_MOVIE 0
#define IS_STILL 1

#define FULLY_OPAQUE 255
#define FULLY_TRANSPARENT 0

#ifdef _WIN32
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define snprintf _snprintf
#define sprintf _scprintf
#define strdup _strdup
#define popen _popen
#define pclose _pclose
#endif /* _WIN32 */

typedef struct canvas
{
	Fl_RGB_Image *new_rgb;
	const char *read_idf;
	const void *cbuf;
	int new_iwidth, new_iheight, new_idepth;
	int selected_frame, top_frame;
	int selected_track, top_track;
	unsigned char bg_red, bg_green, bg_blue;

	void (*pause)(void);
	void (*resume)(void);
	void (*restart)(void);
	void (*forward)(void);
	void (*backward)(void);
	void (*fin)(void);
	void (*cleanup)(void);
	void (*current)(void);

} Canvas;

extern char *root_dir;

/* PROTOTYPES */

extern void FLViewer_url(const char *fname, int w, int h);
extern void FLViewer_animation(int v);
extern void FLViewer_frames_animation(int v);

extern void FLViewer_show_frame(int n, int m);
extern int FLViewer_frame_selected(void);
extern void FLViewer_wait(void);

extern void FLViewer_clear_wait(void);
extern void FLViewer_use_buffer(const void *b, unsigned int w, unsigned int h,
	int depth);

extern void FLViewer_movie_runs(int v);
extern void FLViewer_close_reader(void);

extern void FLViewer_mj2_animation(int v);

extern int FLViewer_has_tile_and_reduction(void);
extern void FLViewer_put_max_tiles_and_reduction(int mx_tiles, int mx_reduct);
extern void FLViewer_get_max_tiles_and_reduction(int *out_mx_tiles,
    int *out_mx_reduct);

extern void FLViewer_get_tile_and_reduction(int *out_tile, int *out_reduct);
extern void FLViewer_tiles_activate(int v);

extern void FLViewer_area_activate(int v);
extern int FLViewer_has_area_values();
extern void FLViewer_get_area_values(int *out_x0, int *out_y0,
	int *out_x1, int *out_y1);

extern void FLViewer_reset_tiles_and_reduction();
extern void FLViewer_set_max_reduction(int mx_reduct);
extern void FLViewer_set_max_tiles(int mx_tiles);

extern int FLViewer_nr_threads(void);
extern void FLViewer_canvas_top_frame(int v);

extern void FLViewer_threads_activate(int v);
extern void FLViewer_header_deactivate();

extern void FLViewer_layer_component_activate(int v);

extern void FLViewer_clear_tree();
extern void FLViewer_show_track(int n, int m);

#endif /* _FLVIEWER_HH_ */
