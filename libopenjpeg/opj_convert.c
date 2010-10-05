#include "opj_includes.h"
#include "opj_convert.h"
/*
 * The yuv code is based on cinelerra-VERSION/quicktime/rtjpeg_core.c
*/
#define KcrR 76284
#define KcrG 53281
#define KcbG 25625
#define KcbB 132252
#define Ky 76284

static void opj_convert_yuv444(opj_image_t *img)
{
    int *d0, *d1, *d2, *red, *green, *blue;
    const int *sY, *sCb, *sCr;
	int maxw, maxh, max, i;
	int R, G, B, C, D, E;
    int shiftR, shiftG, shiftB;
    int addR, addG, addB;

    maxw = img->comps[0].w; maxh = img->comps[0].h;
    max = maxw * maxh;

    sY = img->comps[0].data;
    sCb = img->comps[1].data;
    sCr = img->comps[2].data;

    if(img->comps[0].prec > 8)
     shiftR = img->comps[0].prec - 8;
    else
     shiftR = 0;

    if(img->comps[1].prec > 8)
     shiftG = img->comps[1].prec - 8;
    else
     shiftG = 0;

    if(img->comps[2].prec > 8)
     shiftB = img->comps[2].prec - 8;
    else
     shiftB = 0;

    addR = (img->comps[0].sgnd ? 1 << (img->comps[0].prec - 1) : 0);
    addG = (img->comps[1].sgnd ? 1 << (img->comps[1].prec - 1) : 0);
    addB = (img->comps[2].sgnd ? 1 << (img->comps[2].prec - 1) : 0);

    d0 = red = (int*)opj_malloc(sizeof(int) * max);
    d1 = green = (int*)opj_malloc(sizeof(int) * max);
    d2 = blue = (int*)opj_malloc(sizeof(int) * max);

    for(i = 0; i < max; ++i)
   {
    C = ((*sY++ + addR) >> shiftR) - 16;
    D = ((*sCb++ + addG) >> shiftG) - 128;
    E = ((*sCr++ + addB) >> shiftB) - 128;


	R = (298 * C + 409 * E + 128)>>8;
	if(R < 0) R = 0; else if(R > 255) R = 255;

	G = (298 * C  - 100 * D - 208 * E + 128)>>8;
	if(G < 0) G = 0; else if(G > 255) G = 255;

	B = (298 * C + 516 * D + 128)>>8;
	if(B < 0) B = 0; else if(B > 255) B = 255;

	*d0++ = R; *d1++ = G; *d2++ = B;

   }
	opj_free(img->comps[0].data); img->comps[0].data = red;
    opj_free(img->comps[1].data); img->comps[1].data = green;
    opj_free(img->comps[2].data); img->comps[2].data = blue;

	img->comps[0].prec = 8; img->comps[1].prec = 8; img->comps[2].prec = 8;
}/* opj_convert_yuv444() */

static void yuv422rgb(int *src_y, int *src_cb, int *src_cr, int *red,
	int *green, int *blue, int width, int height)
{	
	int i, j, tmp;
	int y, crR, crG, cbG, cbB;

	for(i=0; i<height; i++)
   {
	for(j=0; j<width; j+=2)
  {
	crR=(*src_cr - 128)*KcrR;
	crG=(*src_cr - 128)*KcrG; ++src_cr;
	cbG=(*src_cb - 128)*KcbG;
	cbB=(*src_cb - 128)*KcbB; ++src_cb;

	y=(src_y[j]-16)*Ky;

	tmp=(y+crR)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*red++ = tmp;

	tmp=(y-crG-cbG)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*green++ = tmp;

	tmp=(y+cbB)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*blue = tmp;

	y=(src_y[j+1]-16)*Ky;

	tmp=(y+crR)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*red++ = tmp;

	tmp=(y-crG-cbG)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*green++ = tmp;

	tmp=(y+cbB)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*blue++ = tmp;
  }
	src_y += width;
   }
}/* yuv422rgb() */

static void yuv420rgb(const int *src_y, const int *src_cb, 
	const int *src_cr, int *red, int *green, int *blue,
	int width, int height)
{
	int *red_e, *green_e, *blue_e, *red_o, *green_o, *blue_o;
	int  y, crR, crG, cbG, cbB, i, j, tmp;
	int yskip;

	yskip = width;
	red_e = red; green_e = green; blue_e = blue;
	red_o = red + width; green_o = green + width; blue_o = blue + width;

	for(i=0; i < (height>>1); i++)
   {
	for(j=0; j < width; j+=2)
  {
	crR=(*src_cr-128)*KcrR;
	crG=(*(src_cr++)-128)*KcrG;
	cbG=(*src_cb-128)*KcbG;
	cbB=(*(src_cb++)-128)*KcbB;

	y=(src_y[j]-16)*Ky;

	tmp=(y+crR)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*red_e++ = tmp;//*(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);

	tmp=(y-crG-cbG)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*green_e++ = tmp;//*(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);

	tmp=(y+cbB)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*blue_e++ = tmp;//*(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);

	y=(src_y[j+1]-16)*Ky;

	tmp=(y+crR)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*red_e++ = tmp;//*(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);

	tmp=(y-crG-cbG)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*green_e++ = tmp;//*(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);

	tmp=(y+cbB)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*blue_e++ = tmp;//*(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);

	y=(src_y[j+yskip]-16)*Ky;

	tmp=(y+crR)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*red_o++ = tmp;//*(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);

	tmp=(y-crG-cbG)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*green_o++ = tmp;//*(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);

	tmp=(y+cbB)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*blue_o++ = tmp;//*(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);

	y=(src_y[j+1+yskip]-16)*Ky;

	tmp=(y+crR)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*red_o++ = tmp;//*(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);

	tmp=(y-crG-cbG)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*green_o++ = tmp;//*(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);

	tmp=(y+cbB)>>16;
	if(tmp < 0) tmp = 0; else if(tmp > 255) tmp = 255;
	*blue_o++ = tmp;//*(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);

  }
	red_e += width; green_e += width; blue_e += width;
	red_o += width; green_o += width; blue_o += width;
	src_y += width<<1;
   }
}/* yuv420rgb() */

static void opj_convert_yuv422(opj_image_t *img)
{
	int *y, *cb, *cr, *red, *green, *blue;
	int maxy, maxw, maxh;

	maxw = img->comps[0].w; maxh = img->comps[0].h;
	maxy = maxw * maxh;

	y = img->comps[0].data;
	cb = img->comps[1].data; 
	cr = img->comps[2].data;

//YUV422 -> YUV444 -> RGB
	red = (int*)opj_calloc(sizeof(int), maxy);
	green = (int*)opj_calloc(sizeof(int), maxy);
	blue = (int*)opj_calloc(sizeof(int), maxy);

	yuv422rgb(y, cb, cr, red, green, blue, maxw, maxh);

	opj_free(img->comps[0].data); img->comps[0].data = red;
	opj_free(img->comps[1].data); img->comps[1].data = green;
	opj_free(img->comps[2].data); img->comps[2].data = blue;

	img->comps[1].w = maxw; img->comps[2].h = maxh;
	img->comps[1].dx = img->comps[0].dx; 
	img->comps[2].dx = img->comps[0].dx;
	img->comps[1].dy = img->comps[0].dy; 
	img->comps[2].dy = img->comps[0].dy;

	img->comps[0].prec = 8; img->comps[1].prec = 8; img->comps[2].prec = 8;
}

static void opj_convert_yuv420(opj_image_t *img)
{
	int *y, *cb, *cr, *red, *green, *blue;
	int maxy, maxw, maxh;

	maxw = img->comps[0].w; maxh = img->comps[0].h;
	maxy = maxw * maxh;

	y = img->comps[0].data;
	cb = img->comps[1].data; 
	cr = img->comps[2].data;

//YUV420 -> YUV422 -> YUV444 -> RGB
	 red = (int*)opj_calloc(sizeof(int), maxy);
	 green = (int*)opj_calloc(sizeof(int), maxy);
	 blue = (int*)opj_calloc(sizeof(int), maxy);

	yuv420rgb(y, cb, cr, red, green, blue, maxw, maxh);

	opj_free(img->comps[0].data); img->comps[0].data = red;
	opj_free(img->comps[1].data); img->comps[1].data = green;
	opj_free(img->comps[2].data); img->comps[2].data = blue;

	img->comps[1].w = maxw; img->comps[2].h = maxh;
    img->comps[1].dx = img->comps[0].dx; 
    img->comps[2].dx = img->comps[0].dx;
    img->comps[1].dy = img->comps[0].dy; 
    img->comps[2].dy = img->comps[0].dy;

	img->comps[0].prec = 8; img->comps[1].prec = 8; img->comps[2].prec = 8;
}

void opj_convert_sycc_to_rgb(opj_image_t *img)
{
	if(img->numcomps < 3) 
   {
	img->color_space = CLRSPC_GRAY;
	return;
   }
    if((img->comps[0].dx == 1)
    && (img->comps[1].dx == 2)
    && (img->comps[2].dx == 2)
    && (img->comps[0].dy == 1)
    && (img->comps[1].dy == 2)
    && (img->comps[2].dy == 2))// horizontal and vertical
  {
    opj_convert_yuv420(img);
  }
    else
    if((img->comps[0].dx == 1)
    && (img->comps[1].dx == 2)
    && (img->comps[2].dx == 2)
    && (img->comps[0].dy == 1)
    && (img->comps[1].dy == 1)
    && (img->comps[2].dy == 1))// horizontal only
  {
    opj_convert_yuv422(img);
  }
    else
    if((img->comps[0].dx == 1)
    && (img->comps[1].dx == 1)
    && (img->comps[2].dx == 1)
    && (img->comps[0].dy == 1)
    && (img->comps[1].dy == 1)
    && (img->comps[2].dy == 1))
  {
    opj_convert_yuv444(img);
  }
	else
  {
fprintf(stderr,"%s:%d:opj_convert_sycc_to_rgb\n\tCAN NOT CONVERT\n",
__FILE__,__LINE__);
	return;
  }
	img->color_space = CLRSPC_SRGB;
}/* opj_convert_sycc_to_rgb() */
