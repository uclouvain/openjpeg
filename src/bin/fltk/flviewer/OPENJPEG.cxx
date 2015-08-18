#include <config.h>
/*
 * author(s) and license
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else /* not _WIN32 */
#define __STDC_FORMAT_MACROS
#include <stdint.h>
#endif /* _WIN32 */

#include <FL/fl_ask.H>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#include "openjpeg.h"
#include "color.hh"
           }
#else
#include "openjpeg.h"
#include "color.hh"
#endif /* __cplusplus */

#include "flviewer.hh"
#include "viewerdefs.hh"
#include "tree.hh"
#include "lang/opj_lang.h_utf8"

#include "JP2.hh"
#include "OPENJPEG.hh"

static unsigned char *openjpeg_buf;

static void OPENJPEG_postlude(void)
{
	if(openjpeg_buf) { free(openjpeg_buf); openjpeg_buf = NULL; }
}

void OPENJPEG_load(Canvas *canvas, const char *read_idf,
	unsigned char *src, uint64_t fsize, int decod_format)
{
	unsigned char *dst;
	int selected_component = 0;
	unsigned int width, height;
	int type;

	dst = 
	JP2_decode(read_idf, src, fsize, IS_STILL, decod_format, 
		&width, &height, &type, &selected_component);

	FLViewer_clear_wait();
	FLViewer_url(read_idf, width, height);

	if( !dst) return;

	canvas->read_idf = read_idf;

	FLViewer_use_buffer(dst, width, height, type);

    FLViewer_tiles_activate(1);

	FLViewer_area_activate(1);

	openjpeg_buf = dst;

	canvas->cleanup = &OPENJPEG_postlude;

}/* OPENJPEG_load() */
