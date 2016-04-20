#ifndef _FLVIEWER_PS_IMAGE_HH_
#define _FLVIEWER_PS_IMAGE_HH_

typedef struct psinfo
{
	char portrait, landscape, center;
	char *format;
/* Normally bg_red = bg_green = bg_blue = 255: 
*/
	unsigned char bg_red, bg_green, bg_blue; 

	unsigned char *src_buf;
	char *title_s;
	char *media_s;

	int image_channels;
	int image_w, image_h;

	double fmedia_w, fmedia_h;
	double fmargin_top, fmargin_rhs, fmargin_bot, fmargin_lhs;
	double fscale_w, fscale_h;

	int PS_hexi;

	FILE *writer;
} PSInfo;


extern int PS_image_draw(PSInfo *psi);

#endif /* _FLVIEWER_PS_IMAGE_HH_ */
