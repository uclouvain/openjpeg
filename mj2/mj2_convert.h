#include "mj2.h"

#ifndef __MJ2_CONVERT_H
#define __MJ2_CONVERT_H

int imagetoyuv(j2k_image_t * img, j2k_cp_t * cp, char *outfile);

int imagetobmp(j2k_image_t * img, j2k_cp_t * cp, char *outfile);

int yuvtoimage(FILE *yuvfile, mj2_tk_t * tk, j2k_image_t * img, 
	       int frame_num, int subsampling_dx, int subsampling_dy);

int yuv_num_frames(mj2_tk_t * tk, FILE *f);


#endif