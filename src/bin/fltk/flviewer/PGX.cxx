#include <config.h>
/*
 * author(s) and license
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "flviewer.hh"
#include "PGX.hh"

#define PGX_MIN_LEN 12

static unsigned char *pgx_buf;

static void PGX_postlude(void)
{
	if(pgx_buf) { free(pgx_buf); pgx_buf = NULL; }
}

/* ML == big_endian; LM == little_endian 
*/
static unsigned short readushort(unsigned char *s, int bigendian)
{
	unsigned short c0, c1;

	c0 = *s; c1 = s[1];

	if(bigendian)
	 return (unsigned short)((c0<<8) + c1);

	return (unsigned short)((c1<<8) + c0);
}

static unsigned int readuint(unsigned char *s, int bigendian)
{
	unsigned int c0, c1, c2, c3;

	c0 = *s; c1 = s[1]; c2 = s[2]; c3 = s[3];

	if(bigendian)
	 return (c0 << 24) + (c1 << 16) + (c2 << 8) + c3;

	return (c3 << 24) + (c2 << 16) + (c1 << 8) + c0;
}

void PGX_load(Canvas *canvas, FILE *reader, const char *read_idf,
	size_t fsize)
{
	unsigned char *src, *s, *dst_buf, *d;
	int width, height, prec, sgnd, adjustS, adjustG ;
	int i, max, bigendian;
	int ushift, dshift;
	char endian1, endian2;
	char signbuf[80], white[80];

	if(fsize <= PGX_MIN_LEN)
   {
	fprintf(stderr,"%s:%d: PGX_load\n\t%s\n\tis too short: len is %lu\n",
	__FILE__,__LINE__,read_idf,(unsigned long)fsize);
	FLViewer_close_reader();
	return;
   }
    src = (unsigned char*) calloc(1, fsize + 8);

	if(src == NULL)
   {
	fprintf(stderr,"PGX.cxx:%d:\n\tmemory out\n",__LINE__);
	return;
   }
	(void)fread(src, 1, fsize, reader);
    FLViewer_close_reader();

    width = height = 0;
/*--------------------------------------*/
	i = sscanf((char*)src, "PG%[ \t]%c%c%[ \t%+-]%d%[ \t]%d%[ \t]%d",
		white, &endian1, &endian2, signbuf, &prec, white,
		&width, white, &height);

	if(i != 9)
   {
	fprintf(stderr, "%s:%d:PGX_load\n\t %s\n\tis not a valid PGX file.\n\t"
	"%d elements read, 9 expected.\n",__FILE__,__LINE__,read_idf, i);
	goto fails;
   }
	if(prec <= 0 || prec > 32)
   {
	fprintf(stderr, "%s:%d:PGX_load\n\tprec %d is invalid\n",
	__FILE__,__LINE__,prec);
	goto fails;
   }
	s = (unsigned char*)strchr((char*)src, '\n');
	assert(s);
	++s;

	i = 0; sgnd = 0;

	while(signbuf[i]) 
   {
	if(signbuf[i] == '-') sgnd = 1;
	i++;
   }

	if(endian1 == 'M' && endian2 == 'L')
	 bigendian = 1;
	else 
	if(endian1 == 'L' && endian2 == 'M')
	 bigendian = 0;
	else 
   {
	fprintf(stderr, "%s:%d:PGX_load\n\tneither 'ML' nor 'LM' found "
	"in header.\n",__FILE__,__LINE__);
	goto fails;
   }
	if(prec > 8) adjustG = prec - 8; else adjustG = 0;

	if(sgnd) adjustS = (1<<(prec - 1)); else adjustS = 0;

	if(prec < 8)
   {
	ushift = 8 - prec; dshift = prec - ushift;
   }
	else
   {
	ushift = dshift = 0;
   }
	max = width * height;
	d = dst_buf = (unsigned char*)calloc(1, max * 3);

	if(dst_buf == NULL)
   {
	fprintf(stderr,"PGX.cxx:%d:\n\tmemory out\n",__LINE__);
	goto fails;
   }
	for(i = 0; i < max; i++) 
   {
	int g;

	g = 0;

	if(prec < 8)
  {
	g = s[0] + adjustS; ++s;

	g = (g<<ushift) + (g>>dshift);

	*d++ = (unsigned char)g; *d++ = (unsigned char)g; *d++ = (unsigned char)g;
	continue;
  }

	if(prec == 8) 
  {
	if(sgnd) g = (char)s[0]; else g = s[0]; ++s;
  } 
	else 
	if(prec <= 16) 
  {
	if(sgnd) 
	 g = (short) readushort(s, bigendian);
	else
	 g = readushort(s, bigendian);

	s += 2;
  } 
	else 
  {
	if(sgnd)
	 g = (int) readuint(s, bigendian);
	else
	 g = readuint(s, bigendian);

	s += 4;
  }
	g += adjustS;
	if(adjustG)
	 g = ((g >> adjustG)+((g >> (adjustG - 1))%2));
	if(g > 255) g = 255; else if(g < 0) g = 0;

	*d++ = (unsigned char)g; 
	*d++ = (unsigned char)g;
	*d++ = (unsigned char)g;
   }//for(i

	FLViewer_url(read_idf, width, height);

	FLViewer_use_buffer(dst_buf, width, height, 3);

	pgx_buf = dst_buf;
	canvas->cleanup = &PGX_postlude;

fails:
	free(src);

}/* PGX_load() */
