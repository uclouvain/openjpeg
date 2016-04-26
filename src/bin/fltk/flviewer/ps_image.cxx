#include <config.h>
/* 
 * Author(s): Ameet A. Raval, Frans van Hoesel, Andrew Ford, szukw000
 *
 * NOTE: rle16(): files can be sufficiently shorter 
 *                if the image has a small number of colors.
 *       rle8() : extends the file.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>


#include "ps_image.hh"

#define MAX_COL 12
#define TM_BASE_YEAR 1900

/* RLE values */
#define USHORT_BLOCK_MASK 0x8000
#define USHORT_BLOCK_LEN  21845
#define USHORT_MAX_INDEX  21845

#define UCHAR_MAX_INDEX 256

#define FULLY_TRANSPARENT 0
#define FULLY_OPAQUE      255

static void ps_print_hex(PSInfo *ps, unsigned short val, int flush)
{
	static unsigned char hexline[128];
	static unsigned char digit[] = "0123456789abcdef";

	if(!flush)
   {
	hexline[ps->PS_hexi++] = (unsigned char) 
     digit[((unsigned) val >>(unsigned) 4) & (unsigned) 0x0f];
	hexline[ps->PS_hexi++] = (unsigned char) 
	 digit[(unsigned) val & (unsigned) 0x0f];
   }
	if((flush && ps->PS_hexi) || (ps->PS_hexi >77))
   {
	hexline[ps->PS_hexi] = '\0';
	ps->PS_hexi = 0;
	fprintf(ps->writer, "%s\n", hexline);
   }
}

struct ps_color
{
	unsigned char red, green, blue;
	unsigned char dirty;
};

static unsigned char *mono_to_rgb(PSInfo *ps)
{
	unsigned char *buf, *d, *s;
	unsigned int i, max;

	max = ps->image_w * ps->image_h;
	buf = (unsigned char*)malloc(max * 3);

	if(buf == NULL)
   {
	fprintf(stderr,"ps_image.cxx:%d:\n\tmemory out\n",__LINE__);
	return NULL;
   }
	d = buf; s = ps->src_buf;

	for(i = 0; i < max; ++i)
   {
	*d++ = *s; *d++ = *s; *d++ = *s; ++s;
   }
	return buf;
}

/* rgba_to_rgb() uses the alpha_composite()-code from :
 * libpng-VERSION/contrib/gregbook/rpng2-x.c
*/
static unsigned char *rgba_to_rgb(PSInfo *ps)
{
	unsigned char *buf, *d, *s;
	unsigned int i, max;
	unsigned char bg_red, bg_green, bg_blue;
	unsigned short red, green, blue, alpha, c;

	bg_red = ps->bg_red; bg_green = ps->bg_green; bg_blue = ps->bg_blue;
	max = ps->image_w * ps->image_h;
	buf = (unsigned char*)malloc(max * 3);

	if(buf == NULL)
   {
	fprintf(stderr,"ps_image.cxx:%d:\n\tmemory out\n",__LINE__);
	return NULL;
   }
	d = buf; s = ps->src_buf;

	for(i = 0; i < max; ++i)
   {
	red = (unsigned short)*s++;
	green = (unsigned short)*s++;
	blue = (unsigned short)*s++;
	alpha = (unsigned short)*s++;

	if(alpha == FULLY_OPAQUE)
  {
	*d++ = (unsigned char)red;
	*d++ = (unsigned char)green;
	*d++ = (unsigned char)blue;
	continue;
  }
	if(alpha == FULLY_TRANSPARENT)
  {
	*d++ = bg_red;
	*d++ = bg_green;
	*d++ = bg_blue;
	continue;
  }
/* transition */
	c = red * alpha
	  + (unsigned short)bg_red * (FULLY_OPAQUE - alpha) + 128;
	*d++ = (unsigned char)((c + (c>>8))>>8);

	c = green * alpha
	  + (unsigned short)bg_green * (FULLY_OPAQUE - alpha) + 128;
	*d++ = (unsigned char)((c + (c>>8))>>8);

	c = blue * alpha
	  + (unsigned short)bg_blue * (FULLY_OPAQUE - alpha) + 128;
	*d++ = (unsigned char)((c + (c>>8))>>8);

   }/* for(i ) */

	return buf;
}

/*
 * Run-Length-Encoding of image.
 * RLE is done to reduce the file size and therefore the time to send
 * the file to the printer. You get longer processing time instead.
 *
 * RLE is encoded as such:
 *  <count> <value>         # 'run' of count+1 equal pixels
 *  <count | MASK> <count+1 data bytes> # count+1 non-equal pixels
 * count can range between 0 and 127   for uchar
 * and             between 0 and 21845 for ushort
 *
 * returns the length of the RLE line vector
 *
 * Implementation Limits of PostScript: 'string' 65535
 *
 * max. triple: 21845
 *
 * Depending of the image size the encoding is split into:
 *
 *   if len is <= 2^16 - 1: rle16_encode()
*/

static int rle16_encode(unsigned short *scanline,
	unsigned short *rleline, unsigned short *block, int wide)
{
	int  i, j, rlen;
	unsigned short blocklen, isrun;
	unsigned short pix;

	blocklen = isrun = 0; rlen = 0;

	for(i = 0; i < wide; i++)
   {
/*  there are 5 possible states:
 *   0: block empty.
 *   1: block is a run, current pix == previous pix
 *   2: block is a run, current pix != previous pix
 *   3: block not a run, current pix == previous pix
 *   4: block not a run, current pix != previous pix
*/
	pix = scanline[i];
	if(!blocklen)
  {
/* case 0:  empty
*/
	block[blocklen++] = pix;
	isrun = 1;
  }
	else
	if(isrun)
  {
	if(pix == block[blocklen-1])
 {
/*  case 1:  isrun, prev == cur
*/
	block[blocklen++] = pix;
 }
	else
 {
/*  case 2:  isrun, prev != cur
*/
    if(blocklen>1)
   {
/*  we have a run block to flush */
    rleline[rlen++] = blocklen-1;
    rleline[rlen++] = block[0];
/*  start new run block with pix */
    block[0] = pix;
    blocklen = 1;
   }
    else
   {
/*  blocklen<=1, turn into non-run */
    isrun = 0;
    block[blocklen++] = pix;
   }
 }
  }
	else
  {
/* not a run */
	if(pix == block[blocklen-1])
 {
/* case 3: non-run, prev == cur
*/
    if(blocklen>1)
   {
/*  have a non-run block to flush */
    rleline[rlen++] = (blocklen-1) | USHORT_BLOCK_MASK;
    for(j=0; j<blocklen; j++)
        rleline[rlen++] = block[j];
/*  start new run block with pix */
    block[0] = pix;
    blocklen = isrun = 1;
   }
    else
   {
/*  blocklen <= 1 turn into a run */
    isrun = 1;
    block[blocklen++] = pix;
   }
 }
	else
 {
/* case 4:  non-run, prev != cur
*/
	block[blocklen++] = pix;
 }
  }

/* max block length.  flush */
	if(blocklen == USHORT_BLOCK_LEN)
  {
	if(isrun)
 {
	rleline[rlen++] = blocklen-1;
	rleline[rlen++] = block[0];
 }
	else
 {
	rleline[rlen++] = (blocklen-1) | USHORT_BLOCK_MASK;
	for(j=0; j<blocklen; j++)
	 rleline[rlen++] = block[j];
 }
	blocklen = 0;
  }
   }/* for(i) */

/* flush last block */
	if(blocklen)
   {
	if(isrun)
  {
	rleline[rlen++] = blocklen-1;
	rleline[rlen++] = block[0];
  }
	else
  {
	rleline[rlen++] = (blocklen-1) | USHORT_BLOCK_MASK;
	for(j=0; j<blocklen; j++)
	 rleline[rlen++] = block[j];
  }
   }
	return rlen;
}/* rle16_encode() */

/*
 * Writes code for the colormap of the following image.
 * If !color: writes a mono-ized graymap.
*/
static void write_colormap(PSInfo *ps, int nc,
	struct ps_color *ps_colors)
{
	int i;

/*  define the colormap */
	fprintf(ps->writer, "/cmap %d string def\n\n\n", nc * 3);

/*  load up the colormap */
	fputs("currentfile cmap readhexstring\n", ps->writer);

	for(i=0; i < nc; i++)
   {
	fprintf(ps->writer, "%02x%02x%02x", ps_colors[i].red,
         ps_colors[i].green, ps_colors[i].blue);

	if((i%(MAX_COL+1)) == MAX_COL) fputs("\n", ps->writer);
   }
/* lose return values from readhexstring */
	fputs("\npop pop\n", ps->writer); 

}/* write_colormap() */

static int create_image_data(PSInfo *ps, unsigned char *src_buf,
	struct ps_color **out_colors, unsigned short **out_dst,
	int *out_dst_len)
{
	unsigned char *src;
	struct ps_color *ps_colors;
	unsigned short *dst;
	unsigned int src_len, dst_len, i, free_index;
	unsigned int r, g, b;

	src = src_buf;
	src_len = ps->image_w * ps->image_h * 3;

	i = (src_len/3) * (unsigned int)sizeof(struct ps_color);

	dst = (unsigned short*)calloc(1, i);
	ps_colors = (struct ps_color*)calloc(1, i);

	if(dst == NULL || ps_colors == NULL)
   {
	goto fails;
   }
	*out_dst = dst; *out_colors = ps_colors;

	ps_colors[0].red = *src++;
	ps_colors[0].green = *src++;
	ps_colors[0].blue = *src++;
	free_index = 1;
	src_len -= 3; *dst++ = 0; dst_len = 1;

	while(src_len > 0)
   {
	r = *src++; g = *src++; b = *src++; src_len -= 3;
	i = (r<<3) + (r>>11) + g;
	i = (i<<3) + (i>>23) + b;
	i %= free_index;

	while(i < free_index)
  {
	if(ps_colors[i].dirty == 0)
 {
	ps_colors[i].red = (unsigned char)r;
	ps_colors[i].green = (unsigned char)g;
	ps_colors[i].blue = (unsigned char)b;
	ps_colors[i].dirty = 1;
	break;
 }
	if(r == ps_colors[i].red
	&& g == ps_colors[i].green
	&& b == ps_colors[i].blue)
	 break;
	++i;
  }
	if(i == free_index)
  {
	if(++free_index == USHORT_MAX_INDEX) 
 {
/* no colormap, no ps_colors */
	break;
 }
	ps_colors[i].red = (unsigned char)r; 
	ps_colors[i].green = (unsigned char)g; 
	ps_colors[i].blue = (unsigned char)b;
	ps_colors[i].dirty = 1;
  }
	*dst++ = i; ++dst_len;
   }/* while(src_len > 0) */

	*out_dst_len = dst_len;

	return free_index;

fails:
	if(dst != NULL) free(dst);

	fprintf(stderr,"ps_image.cxx:%d:\n\tmemory out\n",__LINE__);
	return 0;

}/* create_image_data() */

const char *prolog_open_cs =
"%%!PS-Adobe-3.0\n"
"%%%%Creator: %s\n"
"%%%%Title: %s\n"
"%%%%LanguageLevel: 2\n"
"%%%%CreationDate: %s\n"
"%%%%Pages: 1\n"
"%%%%PageOrder: Ascend\n";

const char *rle16_cs =
"/RLECMAPIMAGE16 { /buffer 2 string def /rgbval 3 string def\n"
" /block  65535 string def\n"
" { currentfile buffer readhexstring pop pop\n"
"   /bcount buffer 0 get 256 mul buffer 1 get add store\n"
"    bcount 32768 ge\n"
"  {\n"
"     0 1 bcount 32768 sub\n"
"   { currentfile buffer readhexstring pop pop\n"
"     /mapidx buffer 0 get 256 mul buffer 1 get add 3 mul store\n"
"    /rgbval cmap mapidx 3 getinterval store\n"
"       block exch 3 mul rgbval putinterval\n"
"   } for\n"
"     block  0  bcount 32767 sub 3 mul  getinterval\n"
"  }\n"
"  {\n"
"   currentfile buffer readhexstring pop pop\n"
"   /mapidx buffer 0 get 256 mul buffer 1 get add 3 mul store\n"
"   /rgbval cmap mapidx 3 getinterval store\n"
"   0 1 bcount { block exch 3 mul rgbval putinterval } for\n"
"   block 0 bcount 1 add 3 mul getinterval\n"
"  } ifelse\n"
" }\n"
"  false 3 colorimage\n"
"} bind def\n";

const char *ps_date(void)
{
    static char buf[128];
    struct tm *stm;
    time_t t;

    t = time(NULL);
    stm = localtime(&t);
#ifdef _WIN32
    sprintf_s(buf, 128, "%4d-%02d-%02d",
      stm->tm_year + TM_BASE_YEAR, stm->tm_mon+1, stm->tm_mday);
#else
    snprintf(buf, 128, "%4d-%02d-%02d",
      stm->tm_year + TM_BASE_YEAR, stm->tm_mon+1, stm->tm_mday);
#endif

    return buf;
}

static void write_header(PSInfo *ps, int max_index)
{
	double lhs, top, rhs, bot, box_w, box_h, x, y;
	const char *moveto;

	fprintf(ps->writer, prolog_open_cs, PACKAGE_STRING, ps->title_s, 
		ps_date());

	lhs = ps->fmargin_lhs;
	top = ps->fmargin_top;
	rhs = ps->fmargin_rhs;
	bot = ps->fmargin_bot;

	if(ps->portrait)
  {
	box_w = ps->fmedia_w - lhs - rhs;
	box_h = ps->fmedia_h - top - bot;
    ps->fscale_w = ps->image_w; ps->fscale_h = ps->image_h;

    if(ps->fscale_w > box_w)
 {
    ps->fscale_w = box_w;
    ps->fscale_h = box_w  * (double)ps->image_h/(double)ps->image_w;
 }
	else
    if(ps->fscale_h > box_h)
 {
    ps->fscale_w = box_h * (double)ps->image_h/(double)ps->image_w;
    ps->fscale_h = box_h;
 }
    x = lhs; y = box_h + bot;

	if(ps->center)
 {
    if(ps->fscale_h < box_h) y -= (box_h - ps->fscale_h)/2.;
    if(ps->fscale_w < box_w) x += (box_w - ps->fscale_w)/2.;
 }
	fprintf(ps->writer, "%%%%Orientation: Portrait\n"
	"%%%%BoundingBox: %d %d %d %d\n"
	"%%%%DocumentMedia: %s %d %d 0 () ()\n",
	(int)lhs, (int)top, (int)box_w, (int)box_h,
	ps->media_s, (int)ps->fmedia_w, (int)ps->fmedia_h);

	moveto = "save\n%.4f %.4f moveto\n";
  }
	else
  {
	assert(ps->landscape);

    box_w = ps->fmedia_w - lhs - rhs;
    box_h = ps->fmedia_h - top - bot;
    ps->fscale_w = ps->image_w; ps->fscale_h = ps->image_h;

    if(ps->fscale_h > box_w)
 {
    ps->fscale_w = box_w * (double)ps->image_w/(double)ps->image_h;
    ps->fscale_h = box_w;
 }
	else
    if(ps->fscale_w > box_h)
 {
    ps->fscale_w = box_h;
    ps->fscale_h = box_h * (double)ps->image_w/(double)ps->image_h;
 }
    x = lhs; y = bot;

	if(ps->center)
 {
    if(ps->fscale_h < box_w) x += (box_w - ps->fscale_h)/2.;
    if(ps->fscale_w < box_h) y += (box_h - ps->fscale_w)/2.;
 }
	fprintf(ps->writer, "%%%%Orientation: Landscape\n"
	"%%%%BoundingBox: %d %d %d %d\n"
	"%%%%DocumentMedia: %s %d %d 0 () ()\n",
	(int)lhs, (int)top, (int)box_w, (int)box_h,
	ps->media_s, (int)ps->fmedia_w, (int)ps->fmedia_h);

	moveto = "save\n%.4f %.4f translate 90 rotate 0. 0. moveto\n";

  }
	fputs("%%EndComments\n%%BeginProlog\n", ps->writer);

    if(max_index < USHORT_MAX_INDEX)
     fputs(rle16_cs, ps->writer);

	fputs("%%EndProlog\n%%BeginSetup\n%%EndSetup\n%%Page: 1 1\n"
		"%%BeginPageSetup\n%%EndPageSetup\n", ps->writer);

	fprintf(ps->writer, moveto, x, y);
#ifdef DEBUG
fprintf(stderr,"%s:%d:write_header\n\tMARGINS(%.4f,%.4f,%.4f,%.4f)\n\t"
"BOX w(%.4f) h(%.4f) SCALE w(%.4f) h(%.4f)\n\tX(%.4f) Y(%.4f)\n",
__FILE__,__LINE__, top,rhs,bot,lhs,box_w,box_h,ps->fscale_w,
ps->fscale_h,x,y);
#endif
}/* write_header() */

int PS_image_draw(PSInfo *ps)
{
	struct ps_color *ps_colors;
	unsigned short *dst_buf, *src;
	unsigned char *rgb_buf;
	unsigned short *rle_line;
	unsigned short *block;
	int i, j, width, height, rle_len, dst_len;
	int	max_index;

	width = ps->image_w;
	height = ps->image_h;
	dst_buf = NULL; ps_colors = NULL;

	if(ps->image_channels == 1)
	 rgb_buf = mono_to_rgb(ps);
	else
	if(ps->image_channels == 4)
	 rgb_buf = rgba_to_rgb(ps);
	else
	 rgb_buf = ps->src_buf;

	if(rgb_buf == NULL)
   {
	fprintf(stderr,".cxx:%d:\n\tmemory out\n",__LINE__);
	return 0;
   }
	max_index = 
	create_image_data(ps, rgb_buf, &ps_colors, &dst_buf, &dst_len);

	if(max_index == 0)
   {
	if(ps->image_channels != 3) free(rgb_buf);

	if(dst_buf) free(dst_buf);

	return 0;
   }
/* small images without colormap:
*/
	if(max_index < UCHAR_MAX_INDEX)
	 max_index = USHORT_MAX_INDEX;

	write_header(ps, max_index);

	fprintf(ps->writer, "20 dict begin\n/pix %d string def\n\n", width * 3);

/*  position and scaling 
*/
	fprintf(ps->writer, "gsave currentpoint %.4f sub translate"
	 " %.4f %.4f scale\n", ps->fscale_h, ps->fscale_w, ps->fscale_h);

	if(max_index < USHORT_MAX_INDEX)
   {
	write_colormap(ps, max_index, ps_colors);
   }
	fprintf(ps->writer, "%d %d 8\n",  width, height);

/*  mapping matrix 
*/
	fprintf(ps->writer, "[%d 0 0 -%d 0 %d]\n", width, height, height);

	rle_line = NULL; block = NULL;

	if(max_index < USHORT_MAX_INDEX)
   {
	unsigned short val;

	rle_line = (unsigned short*) malloc(width * 2 * sizeof(unsigned short));
	block = (unsigned short*) malloc(width * sizeof(unsigned short));

	if(rle_line == NULL || block == NULL)
  {
	goto fails;
  }
	fputs("RLECMAPIMAGE16\n", ps->writer);

	src = dst_buf;

	for(i=0; i < height; i++) 
  {
	rle_len = rle16_encode(src, rle_line, block, width);
	src += width;
	j = -1;

	while(++j < rle_len)
 {
	val = rle_line[j];
	ps_print_hex(ps, val>>8, 0);
	ps_print_hex(ps, val%256, 0);
 }
	ps_print_hex(ps, '\0', 1); /*  Flush the hex buffer */
  }
	free(rle_line); free(block);
   }
	else /* max_index >= USHORT_MAX_INDEX */
   {
	unsigned char *src;
	int col, src_len;

	fputs("{currentfile pix readhexstring pop}\n"
		"false 3 colorimage\n", ps->writer);

	src = rgb_buf; src_len = width * height * 3;
	col = 0;

	while(src_len > 0)
  {
	fprintf(ps->writer, "%02x%02x%02x", src[0], src[1], src[2]);
	src += 3; src_len -= 3;
	if(++col > MAX_COL) 
 { 
	fputs("\n", ps->writer); col = 0; 
 }
  }
   }
	if(ps->image_channels != 3)
	 free(rgb_buf);
	free(dst_buf); free(ps_colors);

	fputs("end\ngrestore\nrestore\nshowpage\n%%Trailer\n%%EOF\n", ps->writer);

	return 1;

fails:
	if(rle_line != NULL) free(rle_line);

	fprintf(stderr,"ps_image.cxx:%d:\n\tmemory out\n",__LINE__);
	return 0;

} /* PS_image_write() */
