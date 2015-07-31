#ifdef OPJ_HAVE_LIBPNG

#ifndef _FLVIEWER_PNG_HH_
#define _FLVIEWER_PNG_HH_

extern void PNG_load(Canvas *canvas, FILE *reader, const char *read_idf,
	int64_t fsize);
extern void PNG_save_file(Canvas *canvas, const char *write_idf);

#endif /* _FLVIEWER_PNG_HH_ */

#endif /* OPJ_HAVE_LIBPNG */
