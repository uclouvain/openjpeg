/* Copyright (c) 2001 David Janssens
 * Copyright (c) 2002-2003 Yannick Verschueren
 * Copyright (c) 2002-2003 Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
 * 
 * All rights reserved. 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */



#include <openjpeg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ceildiv(int a, int b)
{
  return (a + b - 1) / b;
}

int main(int argc, char **argv)
{
  FILE *f;
  char *src, *src_name;
  char *dest, S1, S2, S3;
  int len;
  j2k_image_t *img;
  j2k_cp_t *cp;
  j2k_option_t option;
  int w, wr, wrr, h, hr, hrr, max;
  int i, image_type = -1, compno, pad;
  int adjust;
  jp2_struct_t * jp2_struct;

  if (argc < 3) {
    fprintf(stderr,
	    "usage: %s j2k-file image-file -reduce n (<- optional)\n",
	    argv[0]);
    return 1;
  }

  f = fopen(argv[1], "rb");
  if (!f) {
    fprintf(stderr, "failed to open %s for reading\n", argv[1]);
    return 1;
  }

  dest = argv[2];

  option.reduce_on = 0;
  option.reduce_value = 0;

  /* OPTION REDUCE IS ACTIVE */
  if (argc == 5) {
    if (strcmp(argv[3], "-reduce")) {
      fprintf(stderr,
	      "usage: options " "-reduce n"
	      " where n is the factor of reduction [%s]\n", argv[3]);
      return 1;
    }
    option.reduce_on = 1;
    sscanf(argv[4], "%d", &option.reduce_value);
  }

  while (*dest) {
    dest++;
  }
  dest--;
  S3 = *dest;
  dest--;
  S2 = *dest;
  dest--;
  S1 = *dest;

  if ((S1 == 'p' && S2 == 'g' && S3 == 'x')
      || (S1 == 'P' && S2 == 'G' && S3 == 'X')) {
    image_type = 0;
  }

  if ((S1 == 'p' && S2 == 'n' && S3 == 'm')
      || (S1 == 'P' && S2 == 'N' && S3 == 'M') || (S1 == 'p' && S2 == 'g'
						   && S3 == 'm')
      || (S1 == 'P' && S2 == 'G' && S3 == 'M') || (S1 == 'P' && S2 == 'P'
						   && S3 == 'M')
      || (S1 == 'p' && S2 == 'p' && S3 == 'm')) {
    image_type = 1;
  }

  if ((S1 == 'b' && S2 == 'm' && S3 == 'p')
      || (S1 == 'B' && S2 == 'M' && S3 == 'P')) {
    image_type = 2;
  }

  if (image_type == -1) {
    fprintf(stderr,
	    "!! Unrecognized format for infile : %c%c%c [accept only *.pnm, *.pgm, *.ppm, *.pgx or *.bmp] !!\n\n",
	    S1, S2, S3);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  len = ftell(f);
  fseek(f, 0, SEEK_SET);
  src = (char *) malloc(len);
  fread(src, 1, len, f);
  fclose(f);

  src_name = argv[1];
  while (*src_name) {
    src_name++;
  }
  src_name--;
  S3 = *src_name;
  src_name--;
  S2 = *src_name;
  src_name--;
  S1 = *src_name;

  /* J2K format */
  if ((S1 == 'j' && S2 == '2' && S3 == 'k') || (S1 == 'J' && S2 == '2' && S3 == 'K')) {
    if (!j2k_decode(src, len, &img, &cp, option)) {
      fprintf(stderr, "j2k_to_image: failed to decode image!\n");
      return 1;
    }
  }
  /* JP2 format */
  else if ((S1 == 'j' && S2 == 'p' && S3 == '2') || (S1 == 'J' && S2 == 'P' && S3 == '2')) {
    jp2_struct = (jp2_struct_t *) malloc(sizeof(jp2_struct_t));
    if (jp2_decode(src,len,jp2_struct,&cp, option)) {
      fprintf(stderr, "j2k_to_image: failed to decode image!\n");
      return 1;
    }
    img = jp2_struct->image;
  }
  /* JPT format */
  else if ((S1 == 'j' && S2 == 'p' && S3 == 't') || (S1 == 'J' && S2 == 'P' && S3 == 'T')){
      if (!j2k_decode_jpt_stream(src, len, &img, &cp)) {
	fprintf(stderr, "j2k_to_image: failed to decode image!\n");
	return 1;
      }
    else {
      fprintf(stderr,
	      "j2k_to_image : Unknown format image *.%c%c%c [only *.j2k, *.jp2 or *.jpt]!! \n",
	      S1, S2, S3);
      return 1;
    }
  }

  free(src);
  /* ------------------  CREATE OUT IMAGE WITH THE RIGHT FORMAT ----------------------- */

  /* ---------------------------- / */
  /* /                            / */
  /* /  FORMAT : PNM, PGM or PPM  / */
  /* /                            / */
  /* ---------------------------- / */

  switch (image_type) {
  case 1:			/* PNM PGM PPM */
    if (img->numcomps == 3 && img->comps[0].dx == img->comps[1].dx
	&& img->comps[1].dx == img->comps[2].dx
	&& img->comps[0].dy == img->comps[1].dy
	&& img->comps[1].dy == img->comps[2].dy
	&& img->comps[0].prec == img->comps[1].prec
	&& img->comps[1].prec == img->comps[2].prec) {
      f = fopen(argv[2], "wb");
      w = ceildiv(img->x1 - img->x0, img->comps[0].dx);
      // wr = ceildiv(int_ceildivpow2(img->x1 - img->x0,img->factor),img->comps[0].dx);
      wr = img->comps[0].w;
      wrr = int_ceildivpow2(img->comps[0].w, img->comps[0].factor);

      h = ceildiv(img->y1 - img->y0, img->comps[0].dy);
      // hr = ceildiv(int_ceildivpow2(img->y1 - img->y0,img->factor), img->comps[0].dy);
      hr = img->comps[0].h;
      hrr = int_ceildivpow2(img->comps[0].h, img->comps[0].factor);

      max = img->comps[0].prec > 8 ? 255 : (1 << img->comps[0].prec) - 1;

      img->comps[0].x0 =
	int_ceildivpow2(img->comps[0].x0 -
			int_ceildiv(img->x0, img->comps[0].dx),
			img->comps[0].factor);
      img->comps[0].y0 =
	int_ceildivpow2(img->comps[0].y0 -
			int_ceildiv(img->y0, img->comps[0].dy),
			img->comps[0].factor);


      fprintf(f, "P6\n# %d %d %d %d %d\n%d %d\n%d\n",
	      cp->tcps[cp->tileno[0]].tccps[0].numresolutions, w, h,
	      img->comps[0].x0, img->comps[0].y0, wrr, hrr, max);
      adjust = img->comps[0].prec > 8 ? img->comps[0].prec - 8 : 0;
      for (i = 0; i < wrr * hrr; i++) {
	char r, g, b;
	r = img->comps[0].data[i / wrr * wr + i % wrr];
	r += (img->comps[0].sgnd ? 1 << (img->comps[0].prec - 1) : 0);
	r = r >> adjust;

	g = img->comps[1].data[i / wrr * wr + i % wrr];
	g += (img->comps[1].sgnd ? 1 << (img->comps[1].prec - 1) : 0);
	g = g >> adjust;

	b = img->comps[2].data[i / wrr * wr + i % wrr];
	b += (img->comps[2].sgnd ? 1 << (img->comps[2].prec - 1) : 0);
	b = b >> adjust;

	fprintf(f, "%c%c%c", r, g, b);
      }
      fclose(f);
    } else {
      for (compno = 0; compno < img->numcomps; compno++) {
	char name[256];
	if (img->numcomps > 1) {
	  sprintf(name, "%d.%s", compno, argv[2]);
	} else {
	  sprintf(name, "%s", argv[2]);
	}
	f = fopen(name, "wb");
	w = ceildiv(img->x1 - img->x0, img->comps[compno].dx);
	// wr = ceildiv(int_ceildivpow2(img->x1 - img->x0,img->factor),img->comps[compno].dx);
	wr = img->comps[compno].w;
	wrr =
	  int_ceildivpow2(img->comps[compno].w, img->comps[compno].factor);

	h = ceildiv(img->y1 - img->y0, img->comps[compno].dy);
	// hr = ceildiv(int_ceildivpow2(img->y1 - img->y0,img->factor), img->comps[compno].dy);
	hr = img->comps[compno].h;
	hrr =
	  int_ceildivpow2(img->comps[compno].h, img->comps[compno].factor);

	max =
	  img->comps[compno].prec >
	  8 ? 255 : (1 << img->comps[compno].prec) - 1;

	img->comps[compno].x0 =
	  int_ceildivpow2(img->comps[compno].x0 -
			  int_ceildiv(img->x0,
				      img->comps[compno].dx),
			  img->comps[compno].factor);
	img->comps[compno].y0 =
	  int_ceildivpow2(img->comps[compno].y0 -
			  int_ceildiv(img->y0,
				      img->comps[compno].dy),
			  img->comps[compno].factor);

	fprintf(f, "P5\n# %d %d %d %d %d\n%d %d\n%d\n",
		cp->tcps[cp->tileno[0]].tccps[compno].
		numresolutions, w, h, img->comps[compno].x0,
		img->comps[compno].y0, wrr, hrr, max);
	adjust =
	  img->comps[compno].prec > 8 ? img->comps[compno].prec - 8 : 0;
	for (i = 0; i < wrr * hrr; i++) {
	  char l;
	  l = img->comps[compno].data[i / wrr * wr + i % wrr];
	  l += (img->comps[compno].
		sgnd ? 1 << (img->comps[compno].prec - 1) : 0);
	  l = l >> adjust;
	  fprintf(f, "%c", l);
	}
	fclose(f);
      }
    }
    break;

    /* ------------------------ / */
    /* /                        / */
    /* /     FORMAT : PGX       / */
    /* /                        / */
    /* /----------------------- / */
  case 0:			/* PGX */
    for (compno = 0; compno < img->numcomps; compno++) {
      j2k_comp_t *comp = &img->comps[compno];
      char name[256];
      if (img->numcomps > 1)
	sprintf(name, "%d_%s", compno, argv[2]);
      else
	sprintf(name, "%s", argv[2]);

      f = fopen(name, "wb");
      // w = ceildiv(img->x1 - img->x0, comp->dx);
      // wr = ceildiv(int_ceildivpow2(img->x1 - img->x0,img->factor), comp->dx);
      w = img->comps[compno].w;
      wr = int_ceildivpow2(img->comps[compno].w,
			   img->comps[compno].factor);

      // h = ceildiv(img->y1 - img->y0, comp->dy);
      // hr = ceildiv(int_ceildivpow2(img->y1 - img->y0,img->factor), comp->dy);
      h = img->comps[compno].h;
      hr = int_ceildivpow2(img->comps[compno].h,
			   img->comps[compno].factor);

      fprintf(f, "PG LM %c %d %d %d\n", comp->sgnd ? '-' : '+',
	      comp->prec, wr, hr);
      for (i = 0; i < wr * hr; i++) {
	int v = img->comps[compno].data[i / wr * w + i % wr];
	if (comp->prec <= 8) {
	  char c = (char) v;
	  fwrite(&c, 1, 1, f);
	} else if (comp->prec <= 16) {
	  short s = (short) v;
	  fwrite(&s, 2, 1, f);
	} else {
	  fwrite(&v, 4, 1, f);
	}
      }
      fclose(f);
    }
    break;

    /* ------------------------ / */
    /* /                        / */
    /* /     FORMAT : BMP       / */
    /* /                        / */
    /* /----------------------- / */

  case 2:			/* BMP */
    if (img->numcomps == 3 && img->comps[0].dx == img->comps[1].dx
	&& img->comps[1].dx == img->comps[2].dx
	&& img->comps[0].dy == img->comps[1].dy
	&& img->comps[1].dy == img->comps[2].dy
	&& img->comps[0].prec == img->comps[1].prec
	&& img->comps[1].prec == img->comps[2].prec) {
      /* -->> -->> -->> -->>

         24 bits color

         <<-- <<-- <<-- <<-- */

      f = fopen(argv[2], "wb");
      // w = ceildiv(img->x1 - img->x0, img->comps[0].dx);
      // wr = ceildiv(int_ceildivpow2(img->x1 - img->x0,img->factor), img->comps[0].dx);
      w = img->comps[0].w;
      wr = int_ceildivpow2(img->comps[0].w, img->comps[0].factor);

      // h = ceildiv(img->y1 - img->y0, img->comps[0].dy);
      // hr = ceildiv(int_ceildivpow2(img->y1 - img->y0,img->factor), img->comps[0].dy);
      h = img->comps[0].h;
      hr = int_ceildivpow2(img->comps[0].h, img->comps[0].factor);

      fprintf(f, "BM");

      /* FILE HEADER */
      /* ------------- */
      fprintf(f, "%c%c%c%c",
	      (unsigned char) (hr * wr * 3 + 3 * hr * (wr % 2) +
			       54) & 0xff,
	      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)
			       >> 8) & 0xff,
	      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)
			       >> 16) & 0xff,
	      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2) + 54)
			       >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	      ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (54) & 0xff, ((54) >> 8) & 0xff,
	      ((54) >> 16) & 0xff, ((54) >> 24) & 0xff);

      /* INFO HEADER   */
      /* ------------- */
      fprintf(f, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,
	      ((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (unsigned char) ((wr) & 0xff),
	      (unsigned char) ((wr) >> 8) & 0xff,
	      (unsigned char) ((wr) >> 16) & 0xff,
	      (unsigned char) ((wr) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (unsigned char) ((hr) & 0xff),
	      (unsigned char) ((hr) >> 8) & 0xff,
	      (unsigned char) ((hr) >> 16) & 0xff,
	      (unsigned char) ((hr) >> 24) & 0xff);
      fprintf(f, "%c%c", (1) & 0xff, ((1) >> 8) & 0xff);
      fprintf(f, "%c%c", (24) & 0xff, ((24) >> 8) & 0xff);
      fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	      ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c",
	      (unsigned char) (3 * hr * wr +
			       3 * hr * (wr % 2)) & 0xff,
	      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >>
			       8) & 0xff,
	      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >>
			       16) & 0xff,
	      (unsigned char) ((hr * wr * 3 + 3 * hr * (wr % 2)) >>
			       24) & 0xff);
      fprintf(f, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
	      ((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
	      ((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	      ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	      ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);

      for (i = 0; i < wr * hr; i++) {
	unsigned char R, G, B;
	/* a modifier */
	// R = img->comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
	R = img->comps[0].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
	// G = img->comps[1].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
	G = img->comps[1].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
	// B = img->comps[2].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
	B = img->comps[2].data[w * hr - ((i) / (wr) + 1) * w + (i) % (wr)];
	fprintf(f, "%c%c%c", B, G, R);

	if ((i + 1) % wr == 0) {
	  for (pad = (3 * wr) % 4 ? 4 - (3 * wr) % 4 : 0; pad > 0; pad--)	/* ADD */
	    fprintf(f, "%c", 0);
	}
      }
      fclose(f);
    } else {			/* Gray-scale */

      /* -->> -->> -->> -->>

         8 bits non code (Gray scale)

         <<-- <<-- <<-- <<-- */
      f = fopen(argv[2], "wb");
      // w = ceildiv(img->x1 - img->x0, img->comps[0].dx);
      // wr = ceildiv(int_ceildivpow2(img->x1 - img->x0,img->factor), img->comps[0].dx);
      w = img->comps[0].w;
      wr = int_ceildivpow2(img->comps[0].w, img->comps[0].factor);

      // h = ceildiv(img->y1 - img->y0, img->comps[0].dy);
      // hr = ceildiv(int_ceildivpow2(img->y1 - img->y0,img->factor), img->comps[0].dy);
      h = img->comps[0].h;
      hr = int_ceildivpow2(img->comps[0].h, img->comps[0].factor);

      fprintf(f, "BM");

      /* FILE HEADER */
      /* ------------- */
      fprintf(f, "%c%c%c%c",
	      (unsigned char) (hr * wr + 54 + 1024 +
			       hr * (wr % 2)) & 0xff,
	      (unsigned char) ((hr * wr + 54 + 1024 + hr * (wr % 2))
			       >> 8) & 0xff,
	      (unsigned char) ((hr * wr + 54 + 1024 + hr * (wr % 2))
			       >> 16) & 0xff,
	      (unsigned char) ((hr * wr + 54 + 1024 + wr * (wr % 2))
			       >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	      ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (54 + 1024) & 0xff,
	      ((54 + 1024) >> 8) & 0xff, ((54 + 1024) >> 16) & 0xff,
	      ((54 + 1024) >> 24) & 0xff);

      /* INFO HEADER */
      /* ------------- */
      fprintf(f, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,
	      ((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (unsigned char) ((wr) & 0xff),
	      (unsigned char) ((wr) >> 8) & 0xff,
	      (unsigned char) ((wr) >> 16) & 0xff,
	      (unsigned char) ((wr) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (unsigned char) ((hr) & 0xff),
	      (unsigned char) ((hr) >> 8) & 0xff,
	      (unsigned char) ((hr) >> 16) & 0xff,
	      (unsigned char) ((hr) >> 24) & 0xff);
      fprintf(f, "%c%c", (1) & 0xff, ((1) >> 8) & 0xff);
      fprintf(f, "%c%c", (8) & 0xff, ((8) >> 8) & 0xff);
      fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
	      ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c",
	      (unsigned char) (hr * wr + hr * (wr % 2)) & 0xff,
	      (unsigned char) ((hr * wr + hr * (wr % 2)) >> 8) &
	      0xff,
	      (unsigned char) ((hr * wr + hr * (wr % 2)) >> 16) &
	      0xff,
	      (unsigned char) ((hr * wr + hr * (wr % 2)) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
	      ((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
	      ((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff,
	      ((256) >> 16) & 0xff, ((256) >> 24) & 0xff);
      fprintf(f, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff,
	      ((256) >> 16) & 0xff, ((256) >> 24) & 0xff);
    }

    for (i = 0; i < 256; i++) {
      fprintf(f, "%c%c%c%c", i, i, i, 0);
    }

    for (i = 0; i < wr * hr; i++) {
      /* a modifier !! */
      // fprintf(f, "%c", img->comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)]);
      fprintf(f, "%c",
	      img->comps[0].data[w * hr - ((i) / (wr) + 1) * w +
				 (i) % (wr)]);
      /*if (((i + 1) % w == 0 && w % 2))
         fprintf(f, "%c", 0); */
      if ((i + 1) % wr == 0) {
	for (pad = wr % 4 ? 4 - wr % 4 : 0; pad > 0; pad--)	/* ADD */
	  fprintf(f, "%c", 0);
      }
    }
    break;
  default:
    break;
  }

  return 0;
}
