#ifndef _FLVIEWER_DEFS_HH_
#define _FLVIEWER_DEFS_HH_

//-----------------------------------------
#define LABEL_SIZE 14
#define BAR_SIZE 17

#ifdef _WIN32
#define WIN_X 30
#define WIN_Y 35
#else
#define WIN_X 80
#define WIN_Y 130
#endif

#define BORDER_SIZE 5
#define FRAMES_W 80
#define TRACKS_W 60

#define BUTTON_W 80
#define BUTTON_H 25

#define STEP_GROUP_W BUTTON_W*2

#define HEADER_H 160

#define MIN_SCROLLER_W BAR_SIZE
#define MIN_SCROLLER_H BAR_SIZE

// Only if(movie_runs == 0); still images only:
//
#define MAX_SCROLLER_W 640 //500 // 590
#define MAX_SCROLLER_H 480

#define SCROLLER_X 210
#define SCROLLER_Y HEADER_H

#define TREE_X BORDER_SIZE
#define TREE_Y SCROLLER_Y
#define TREE_W 200
#define TREE_H MAX_SCROLLER_H

#define HEADER_W TREE_W + BORDER_SIZE + MAX_SCROLLER_W -130

#define WINDOW_MIN_W BORDER_SIZE + HEADER_W + BORDER_SIZE
#define WINDOW_MIN_H BORDER_SIZE + HEADER_H + TREE_H + BORDER_SIZE
//-----------------------------------------
#define PNG_MAGIC "\x89PNG\x0d\x0a\x1a\x0a"

/* JPEG2000 */
#define JP2_RFC3745_MAGIC "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"
#define JP2_MAGIC "\x0d\x0a\x87\x0a"
/* position 45: "\xff\x52" */
#define J2K_CODESTREAM_MAGIC "\xff\x4f\xff\x51"

#define J2K_CFMT 0
#define JP2_CFMT 1
#define JPT_CFMT 2

#define PXM_DFMT 10
#define PGX_DFMT 11
#define BMP_DFMT 12
#define YUV_DFMT 13
#define TIF_DFMT 14
#define RAW_DFMT 15
#define TGA_DFMT 16
#define PNG_DFMT 17
#define RAWL_DFMT 18

/* PROTOTYPES */

#endif /* _FLVIEWER_DEFS_HH_ */
