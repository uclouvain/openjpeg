#ifndef _JP2_HH_
#define _JP2_HH_

#define JP2_FILE 1
#define MJ2_FILE 0

extern void JP2_save_file(Canvas *canvas, const char *write_idf);
extern unsigned char *JP2_decode(const char *read_idf, unsigned char *src,
	uint64_t fsize, int is_still, int decod_format, 
	unsigned int *out_width, unsigned int *out_height, 
	int *out_type, int *out_selected);
extern void JP2_file_from_rgb(const unsigned char *buf, unsigned int width,
	unsigned int height, unsigned int numcomps, const char *write_idf);

#endif // _JP2_HH_
