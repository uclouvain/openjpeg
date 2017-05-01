#include <config.h>
#include "opj_apps_config.h"

#ifdef OPJ_HAVE_LIBPNG
/*
 * author(s) and license
 *
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#include <png.h>
#include <zlib.h>
           }
#else
#include <png.h>
#include <zlib.h>
#endif /* __cplusplus */

#include "opj_stdint.h"

#include <FL/fl_ask.H>

#include "flviewer.hh"
#include "PNG.hh"
#include "lang/png_lang.h_utf8"

/* #define USE_PNG_IMAGE */

static unsigned char *png_buf;

static void PNG_postlude(void)
{
	if(png_buf) { free(png_buf); png_buf = NULL; }
}

void PNG_load(Canvas *canvas, FILE *reader, const char *read_idf,
	int64_t fsize)
{
#ifndef USE_PNG_IMAGE // uses reader
    png_bytep row;
    png_structp  png;
    png_infop    info;
    unsigned char *buf;
    double gamma, display_exponent;
    int unit, pass, nr_passes, depth;
    png_uint_32 resx, resy;
    png_uint_32  width, height;
    unsigned int i, src_w;
    int color_type, has_alpha;
    int bit_depth, interlace_type,compression_type, filter_type;

/* libpng-VERSION/example.c:
 * PC : screen_gamma = 2.2;
 * Mac: screen_gamma = 1.7 or 1.0;
*/
    display_exponent = 2.2;

    if((png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                    NULL, NULL, NULL)) == NULL)
      goto fin;
    if((info = png_create_info_struct(png)) == NULL)
      goto fin;

    if(setjmp(png_jmpbuf(png)))
      goto fin;

	FLViewer_wait();

    png_init_io(png, reader);
    png_read_info(png, info);

    png_get_IHDR(png, info, &width, &height,
        &bit_depth, &color_type, &interlace_type,
        &compression_type, &filter_type);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_expand(png);
    else
    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
      png_set_expand(png);

    if(png_get_valid(png, info, PNG_INFO_tRNS))
      png_set_expand(png);

    if(bit_depth == 16)
      png_set_strip_16(png);

/* GRAY => RGB; GRAY_ALPHA => RGBA
*/
    if(color_type == PNG_COLOR_TYPE_GRAY
    || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
   {
    png_set_gray_to_rgb(png);
    color_type =
     (color_type == PNG_COLOR_TYPE_GRAY? PNG_COLOR_TYPE_RGB:
        PNG_COLOR_TYPE_RGB_ALPHA);
   }

    if( !png_get_gAMA(png, info, &gamma))
      gamma = 0.45455;

    png_set_gamma(png, display_exponent, gamma);

    nr_passes = png_set_interlace_handling(png);

    png_read_update_info(png, info);

    png_get_pHYs(png, info, &resx, &resy, &unit);

    color_type = png_get_color_type(png, info);

    has_alpha = (color_type == PNG_COLOR_TYPE_RGB_ALPHA);

    if(has_alpha)
     depth = 4;
    else
     depth = 3;

    src_w = width * (3 + has_alpha);
    buf = (unsigned char*)malloc(src_w * height);

    for(pass = 0; pass < nr_passes; pass++)
   {
    row = buf;

    for(i = 0; i < height; i++)
  {
/* libpng.3:
 * If you want the "sparkle" effect, just call png_read_rows() as
 * normal, with the third parameter NULL.
*/
    png_read_rows(png, &row, NULL, 1);

    row += src_w;
  }
   }
    FLViewer_url(read_idf, width, height);

    FLViewer_use_buffer(buf, width, height, depth);

    png_buf = buf;
    canvas->cleanup = &PNG_postlude;

fin:
    if(png)
      png_destroy_read_struct(&png, &info, NULL);
    FLViewer_close_reader();

#else // USE_PNG_IMAGE uses read_idf
	unsigned char *buf;
	png_image image;
	int depth = 3;

	buf = NULL;
	memset(&image, 0, (sizeof image));
	image.version = PNG_IMAGE_VERSION;

	FLViewer_close_reader();

	FLViewer_wait();

	if(png_image_begin_read_from_file(&image, read_idf))
   {
	png_bytep buffer;

	depth = PNG_IMAGE_SAMPLE_CHANNELS(image.format);

	if(depth == 3 || depth == 1)
  {
	image.format = PNG_FORMAT_RGB; depth = 3;
  }
	else
	if(depth == 4 || depth == 2)
  {
	image.format = PNG_FORMAT_RGBA; depth = 4;
  }
	buffer = (png_bytep)malloc(PNG_IMAGE_SIZE(image));

	if(buffer != NULL
	&& png_image_finish_read(&image, NULL/*background*/, 
		buffer,
		0/*row_stride*/, NULL/*colormap*/))
  {
	buf = buffer;
  }
	else
  {
	if(buffer == NULL)
	 png_image_free(&image);
	else
	 free(buffer);
  }
   }
	if(buf == NULL) return;

	FLViewer_url(read_idf, image.width, image.height);

	FLViewer_use_buffer(buf, image.width, image.height, depth);

	png_buf = buf;
	canvas->cleanup = &PNG_postlude;

#endif /* USE_PNG_IMAGE */
}/* PNG_load() */

static int to_png(unsigned char *buf, int width, int height, int nr_comp,
	FILE *writer)
{
	png_structp png;
	png_infop info;
	unsigned char *s;
	int color_type, step;
	int y, ok;
	png_color_8 sig_bit;

	ok = 0;
	info = NULL;
/* Create and initialize the png_struct with the desired error handler
 * functions.  If you want to use the default stderr and longjump method,
 * you can supply NULL for the last three parameters.  We also check that
 * the library version is compatible with the one used at compile time,
 * in case we are using dynamically linked libraries.  REQUIRED.
*/
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);

	if(png == NULL) goto fin;

/* Allocate/initialize the image information data.  REQUIRED 
*/
	info = png_create_info_struct(png);

	if(info == NULL) goto fin;

/* Set error handling.  REQUIRED if you are not supplying your own
 * error handling functions in the png_create_write_struct() call.
*/
	if(setjmp(png_jmpbuf(png))) goto fin;

/* I/O initialization functions is REQUIRED 
*/
	png_init_io(png, writer);

/* Set the image information here.  Width and height are up to 2^31,
 * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
 * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
 * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
 * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
 * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
 * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. 
 * REQUIRED
 *
 * ERRORS:
 *
 * color_type == PNG_COLOR_TYPE_PALETTE && bit_depth > 8
 * color_type == PNG_COLOR_TYPE_RGB && bit_depth < 8
 * color_type == PNG_COLOR_TYPE_GRAY_ALPHA && bit_depth < 8
 * color_type == PNG_COLOR_TYPE_RGB_ALPHA) && bit_depth < 8
 * 
*/
	png_set_compression_level(png, Z_BEST_COMPRESSION);

	if(nr_comp == 4) 
   {
	step = 4 * width;
	sig_bit.alpha = 8;
    sig_bit.red = sig_bit.green = sig_bit.blue = 8;
	color_type = PNG_COLOR_TYPE_RGB_ALPHA;
   }
	else
	if(nr_comp == 3) 
   {
	step = 3 * width;
	sig_bit.alpha = 0; 
    sig_bit.red = sig_bit.green = sig_bit.blue = 8;
	color_type = PNG_COLOR_TYPE_RGB;
   }
	else
	if(nr_comp == 2)
   {
	step = 2 * width;
    sig_bit.gray = 8;
	sig_bit.red = sig_bit.green = sig_bit.blue = 0;
	sig_bit.alpha = 8;
	color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
   }
	else /* 1 */
   {
	step = width;
	sig_bit.gray = 8;
	sig_bit.red = sig_bit.green = sig_bit.blue = sig_bit.alpha = 0;
	color_type = PNG_COLOR_TYPE_GRAY;
   }
	png_set_sBIT(png, info, &sig_bit);

	png_set_IHDR(png, info, width, height, 8, 
	 color_type,
	 PNG_INTERLACE_NONE,
	 PNG_COMPRESSION_TYPE_BASE,  PNG_FILTER_TYPE_BASE);

	png_set_gamma(png, 2.2, 1./2.2);

	if(nr_comp > 2)
   {
	png_set_sRGB(png, info, PNG_sRGB_INTENT_PERCEPTUAL);
   }
	png_write_info(png, info);

	s = buf;

	for(y = 0; y < height; ++y)
   {
	png_write_row(png, s);

	s += step;
   }
	png_write_end(png, info);

	ok = 1;

fin:
	if(png)
   {
    png_destroy_write_struct(&png, &info);
   }
	return ok;
}/* to_png() */

void PNG_save_file(Canvas *canvas, const char *write_idf)
{
	FILE *writer;
	int width, height, nr_comp;

	width = canvas->new_iwidth;
	height = canvas->new_iheight;
	nr_comp = canvas->new_idepth;

	if((writer = fopen(write_idf, "wb")) == NULL) 
   {
	fl_alert(DST_DID_NOT_OPEN_s, write_idf);
	return;
   }

	if(to_png((unsigned char*)canvas->cbuf, 
		width, height, nr_comp, writer) == 0) 
   {
	fl_alert("%s", WRITE_PNG_FAILS_s);
   }
	fclose(writer);

}/* PNG_save_file() */

#endif /* OPJ_HAVE_LIBPNG */
