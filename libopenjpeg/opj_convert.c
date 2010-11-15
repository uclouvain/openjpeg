#include "opj_includes.h"

/*--------------------------------------------------------
Matrix für sYCC, Amendment 1 to IEC 61966-2-1

Y :   0.299   0.587    0.114   :R
Cb:  -0.1687 -0.3312   0.5     :G
Cr:   0.5    -0.4187  -0.0812  :B

Inverse:

R: 1        -3.68213e-05    1.40199      :Y
G: 1.00003  -0.344125      -0.714128     :Cb - 2^(prec - 1)
B: 0.999823  1.77204       -8.04142e-06  :Cr - 2^(prec - 1)

-----------------------------------------------------------*/
static void sycc_to_rgb(int offset, int upb, int y, int cb, int cr,
	int *out_r, int *out_g, int *out_b)
{
	int r, g, b;

	cb -= offset; cr -= offset;
	r = y + (int)(1.402 * (float)cr);
	if(r < 0) r = 0; else if(r > upb) r = upb; *out_r = r;

	g = y - (int)(0.344 * (float)cb + 0.714 * (float)cr);
	if(g < 0) g = 0; else if(g > upb) g = upb; *out_g = g;

	b = y + (int)(1.772 * (float)cb);
	if(b < 0) b = 0; else if(b > upb) b = upb; *out_b = b;
}

static void opj_convert_sycc444(opj_image_t *img)
{
	int *d0, *d1, *d2, *r, *g, *b;
	const int *y, *cb, *cr;
	int maxw, maxh, max, i, offset, upb;

	i = img->comps[0].prec;
	offset = 1<<(i - 1); upb = (1<<i)-1;

	maxw = img->comps[0].w; maxh = img->comps[0].h;
	max = maxw * maxh;

	y = img->comps[0].data;
	cb = img->comps[1].data;
	cr = img->comps[2].data;

	d0 = r = (int*)opj_malloc(sizeof(int) * max);
	d1 = g = (int*)opj_malloc(sizeof(int) * max);
	d2 = b = (int*)opj_malloc(sizeof(int) * max);

	for(i = 0; i < max; ++i)
   {
	sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);	

	++y; ++cb; ++cr; ++r; ++g; ++b;
   }	
	opj_free(img->comps[0].data); img->comps[0].data = d0;
	opj_free(img->comps[1].data); img->comps[1].data = d1;
	opj_free(img->comps[2].data); img->comps[2].data = d2;

}/* opj_convert_sycc444() */

static void opj_convert_sycc422(opj_image_t *img)
{	
	int *d0, *d1, *d2, *r, *g, *b;
	const int *y, *cb, *cr;
	int maxw, maxh, max, offset, upb;
	int i, j;

	i = img->comps[0].prec;
	offset = 1<<(i - 1); upb = (1<<i)-1;

	maxw = img->comps[0].w; maxh = img->comps[0].h;
	max = maxw * maxh;

	y = img->comps[0].data;
	cb = img->comps[1].data;
	cr = img->comps[2].data;

	d0 = r = (int*)opj_malloc(sizeof(int) * max);
	d1 = g = (int*)opj_malloc(sizeof(int) * max);
	d2 = b = (int*)opj_malloc(sizeof(int) * max);

	for(i=0; i < maxh; ++i)
   {
	for(j=0; j < maxw; j += 2)
  {
	sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);

	++y; ++r; ++g; ++b;

	sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);

	++y; ++r; ++g; ++b; ++cb; ++cr;
  }
   }
	opj_free(img->comps[0].data); img->comps[0].data = d0;
	opj_free(img->comps[1].data); img->comps[1].data = d1;
	opj_free(img->comps[2].data); img->comps[2].data = d2;

	img->comps[1].w = maxw; img->comps[1].h = maxh;
	img->comps[2].w = maxw; img->comps[2].h = maxh;
	img->comps[1].dx = img->comps[0].dx;
	img->comps[2].dx = img->comps[0].dx;
	img->comps[1].dy = img->comps[0].dy;
	img->comps[2].dy = img->comps[0].dy;

}/* opj_convert_sycc422() */

static void opj_convert_sycc420(opj_image_t *img)
{
	int *d0, *d1, *d2, *r, *g, *b, *nr, *ng, *nb;
	const int *y, *cb, *cr, *ny;
	int maxw, maxh, max, offset, upb;
	int i, j;

	i = img->comps[0].prec;
	offset = 1<<(i - 1); upb = (1<<i)-1;

	maxw = img->comps[0].w; maxh = img->comps[0].h;
	max = maxw * maxh;

	y = img->comps[0].data;
	cb = img->comps[1].data;
	cr = img->comps[2].data;

	d0 = r = (int*)opj_malloc(sizeof(int) * max);
	d1 = g = (int*)opj_malloc(sizeof(int) * max);
	d2 = b = (int*)opj_malloc(sizeof(int) * max);

	for(i=0; i < maxh; i += 2)
   {
	ny = y + maxw;
	nr = r + maxw; ng = g + maxw; nb = b + maxw;

	for(j=0; j < maxw;  j += 2)
  {
	sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);

	++y; ++r; ++g; ++b;

	sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);

	++y; ++r; ++g; ++b;

	sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);

	++ny; ++nr; ++ng; ++nb;

	sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);

	++ny; ++nr; ++ng; ++nb; ++cb; ++cr;
  }
	y += maxw; r += maxw; g += maxw; b += maxw;
   }
	opj_free(img->comps[0].data); img->comps[0].data = d0;
	opj_free(img->comps[1].data); img->comps[1].data = d1;
	opj_free(img->comps[2].data); img->comps[2].data = d2;

	img->comps[1].w = maxw; img->comps[1].h = maxh;
	img->comps[2].w = maxw; img->comps[2].h = maxh;
	img->comps[1].dx = img->comps[0].dx;
	img->comps[2].dx = img->comps[0].dx;
	img->comps[1].dy = img->comps[0].dy;
	img->comps[2].dy = img->comps[0].dy;

}/* opj_convert_sycc420() */

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
	&& (img->comps[2].dy == 2))/* horizontal and vertical sub-sample */
  {
	opj_convert_sycc420(img);
  }
	else
	if((img->comps[0].dx == 1)
	&& (img->comps[1].dx == 2)
	&& (img->comps[2].dx == 2)
	&& (img->comps[0].dy == 1)
	&& (img->comps[1].dy == 1)
	&& (img->comps[2].dy == 1))/* horizontal sub-sample only */
  {
	opj_convert_sycc422(img);
  }
	else
	if((img->comps[0].dx == 1)
	&& (img->comps[1].dx == 1)
	&& (img->comps[2].dx == 1)
	&& (img->comps[0].dy == 1)
	&& (img->comps[1].dy == 1)
	&& (img->comps[2].dy == 1))/* no sub-sample */
  {
	opj_convert_sycc444(img);
  }
	else
  {
	fprintf(stderr,"%s:%d:opj_convert_sycc_to_rgb\n\tCAN NOT CONVERT\n",
	 __FILE__,__LINE__);
	return;
  }
	img->color_space = CLRSPC_SRGB;

}/* opj_convert_sycc_to_rgb() */
