/*
* Copyright (c) 2003-2004, Yannick Verschueren
* Copyright (c) 2003-2004,  Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
* All rights reserved.
*
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "j2k.h"
#include "jp2.h"
#include "cio.h"

#define JPIP_JPIP 0x6a706970

#define JP2_JP   0x6a502020
#define JP2_FTYP 0x66747970
#define JP2_JP2H 0x6a703268
#define JP2_IHDR 0x69686472
#define JP2_COLR 0x636f6c72
#define JP2_JP2C 0x6a703263
#define JP2_URL  0x75726c20
#define JP2_DBTL 0x6474626c
#define JP2_BPCC 0x62706363
#define JP2_JP2  0x6a703220

/*
* 
* Read box headers
*
*/

int jp2_read_boxhdr(jp2_box_t * box)
{
  box->init_pos = cio_tell();
  box->length = cio_read(4);
  box->type = cio_read(4);
  if (box->length == 1) {
    if (cio_read(4) != 0) {
      fprintf(stderr, "Error: Cannot handle box sizes higher than 2^32\n");
      return 1;
    }
    box->length = cio_read(4);
    if (box->length == 0)
      box->length = cio_numbytesleft() + 12;
  } else if (box->length == 0) {
    box->length = cio_numbytesleft() + 8;
  }
  return 0;
}

/*
* 
* Initialisation of a Standard JP2 structure
*/

int jp2_init_stdjp2(jp2_struct_t * jp2_struct)
{

  jp2_struct->comps =
    (jp2_comps_t *) malloc(jp2_struct->numcomps * sizeof(jp2_comps_t));

  jp2_struct->precedence = 0;   /* PRECEDENCE*/
  jp2_struct->approx = 0;   /* APPROX*/

  jp2_struct->brand = JP2_JP2;	/* BR         */
  jp2_struct->minversion = 0;	/* MinV       */
  jp2_struct->numcl = 1;
  jp2_struct->cl = (unsigned int *) malloc(jp2_struct->numcl * sizeof(int));
  jp2_struct->cl[0] = JP2_JP2;	/* CL0 : JP2  */

  jp2_struct->C = 7;      /* C : Always 7*/
  jp2_struct->UnkC = 0;      /* UnkC, colorspace specified in colr box*/
  jp2_struct->IPR = 0;      /* IPR, no intellectual property*/

  return 0;
}


void jp2_write_url(char *Idx_file)
{
  unsigned int i;
  char str[256];
  jp2_box_t box;

  sprintf(str, "%s", Idx_file);


  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(JP2_URL, 4);   /* DBTL*/
  cio_write(0, 1);      /* VERS*/
  cio_write(0, 3);      /* FLAG*/

  for (i = 0; i < strlen(str); i++) {
    cio_write(str[i], 1);
  }

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/*    L       */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the IHDR box
*
* Image Header box
*
*/
int jp2_read_ihdr(jp2_struct_t * jp2_struct)
{
  jp2_box_t box;

  jp2_read_boxhdr(&box);
  if (JP2_IHDR != box.type) {
    fprintf(stderr, "Error: Expected IHDR Marker\n");
    return 1;
  }

  jp2_struct->h = cio_read(4);   /* HEIGHT*/
  jp2_struct->w = cio_read(4);   /* WIDTH*/
  jp2_struct->numcomps = cio_read(2);   /* NC*/

  jp2_struct->bpc = cio_read(1);   /* BPC*/

  jp2_struct->C = cio_read(1);   /* C */
  jp2_struct->UnkC = cio_read(1);   /* UnkC*/
  jp2_struct->IPR = cio_read(1);   /* IPR*/

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with IHDR Box\n");
    return 1;
  }
  return 0;
}

void jp2_write_ihdr(jp2_struct_t * jp2_struct)
{
  jp2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(JP2_IHDR, 4);   /* IHDR*/

  cio_write(jp2_struct->h, 4);   /* HEIGHT*/
  cio_write(jp2_struct->w, 4);   /* WIDTH*/
  cio_write(jp2_struct->numcomps, 2);   /* NC*/

  cio_write(jp2_struct->bpc, 1);   /* BPC  */

  cio_write(jp2_struct->C, 1);   /* C : Always 7*/
  cio_write(jp2_struct->UnkC, 1);   /* UnkC, colorspace unknow*/
  cio_write(jp2_struct->IPR, 1);   /* IPR, no intellectual property*/

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/*    L       */
  cio_seek(box.init_pos + box.length);
}


void jp2_write_bpcc(jp2_struct_t * jp2_struct)
{
  unsigned int i;
  jp2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(JP2_BPCC, 4);   /* BPCC*/

  for (i = 0; i < jp2_struct->numcomps; i++)
    cio_write(jp2_struct->comps[i].bpcc, 1);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/*    L       */
  cio_seek(box.init_pos + box.length);
}


int jp2_read_bpcc(jp2_struct_t * jp2_struct)
{
  unsigned int i;
  jp2_box_t box;

  jp2_read_boxhdr(&box);
  if (JP2_BPCC != box.type) {
    fprintf(stderr, "Error: Expected BPCC Marker\n");
    return 1;
  }

  for (i = 0; i < jp2_struct->numcomps; i++)
    jp2_struct->comps[i].bpcc = cio_read(1);

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with BPCC Box\n");
    return 1;
  }
  return 0;
}

void jp2_write_colr(jp2_struct_t * jp2_struct)
{
  jp2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(JP2_COLR, 4);   /* COLR*/

  cio_write(jp2_struct->meth, 1);   /* METH*/
  cio_write(jp2_struct->precedence, 1);   /* PRECEDENCE*/
  cio_write(jp2_struct->approx, 1);   /* APPROX*/

  if (jp2_struct->meth == 1)
    cio_write(jp2_struct->enumcs, 4);   /* EnumCS*/
  else
    cio_write(0, 1);      /* PROFILE (??)*/

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/*    L       */
  cio_seek(box.init_pos + box.length);
}

int jp2_read_colr(jp2_struct_t * jp2_struct)
{
  jp2_box_t box;
  int skip_len;

  jp2_read_boxhdr(&box);
  do {
    if (JP2_COLR != box.type) {
      cio_skip(box.length - 8);
      jp2_read_boxhdr(&box);
    }
  } while (JP2_COLR != box.type);

  jp2_struct->meth = cio_read(1);   /* METH*/
  jp2_struct->precedence = cio_read(1);   /* PRECEDENCE*/
  jp2_struct->approx = cio_read(1);   /* APPROX*/

  if (jp2_struct->meth == 1)
    jp2_struct->enumcs = cio_read(4);   /* EnumCS*/
  else {
    /* SKIP PROFILE     */
    skip_len = box.init_pos + box.length - cio_tell();
    if (skip_len < 0) {
      fprintf(stderr, "Error with JP2H box size\n");
      return 1;
    }
    cio_skip(box.init_pos + box.length - cio_tell());
  }

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with BPCC Box\n");
    return 1;
  }
  return 0;
}

/*
* Write the JP2H box
*
* JP2 Header box
*
*/
void jp2_write_jp2h(jp2_struct_t * jp2_struct)
{
  jp2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);;
  cio_write(JP2_JP2H, 4);	/* JP2H */

  jp2_write_ihdr(jp2_struct);

  if (jp2_struct->bpc == 255)
    jp2_write_bpcc(jp2_struct);
  jp2_write_colr(jp2_struct);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/*    L       */
  cio_seek(box.init_pos + box.length);
}


/*
* Read the JP2H box
*
* JP2 Header box
*
*/
int jp2_read_jp2h(jp2_struct_t * jp2_struct)
{
  jp2_box_t box;
  int skip_len;

  jp2_read_boxhdr(&box);
  do {
    if (JP2_JP2H != box.type) {
      if (box.type == JP2_JP2C) {
	fprintf(stderr, "Error: Expected JP2H Marker\n");
	return 1;
      }
      cio_skip(box.length - 8);
      jp2_read_boxhdr(&box);
    }
  } while (JP2_JP2H != box.type);

  if (jp2_read_ihdr(jp2_struct))
    return 1;

  if (jp2_struct->bpc == 255) {
    if (jp2_read_bpcc(jp2_struct))
      return 1;
  }

  if (jp2_read_colr(jp2_struct))
    return 1;

  skip_len = box.init_pos + box.length - cio_tell();
  if (skip_len < 0) {
    fprintf(stderr, "Error with JP2H box size\n");
    return 1;
  }
  cio_skip(box.init_pos + box.length - cio_tell());

  return 0;
}

/*
* Write the FTYP box
*
* File type box
*
*/
void jp2_write_ftyp(jp2_struct_t * jp2_struct)
{
  unsigned int i;
  jp2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(JP2_FTYP, 4);	/* FTYP       */

  cio_write(jp2_struct->brand, 4);	/* BR         */
  cio_write(jp2_struct->minversion, 4);	/* MinV       */

  for (i = 0; i < jp2_struct->numcl; i++)
    cio_write(jp2_struct->cl[i], 4);	/* CL           */

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/*    L       */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the FTYP box
*
* File type box
*
*/
int jp2_read_ftyp(jp2_struct_t * jp2_struct)
{
  int i;
  jp2_box_t box;

  jp2_read_boxhdr(&box);

  if (JP2_FTYP != box.type) {
    fprintf(stderr, "Error: Excpected FTYP Marker\n");
    return 1;
  }

  jp2_struct->brand = cio_read(4);	/* BR              */
  jp2_struct->minversion = cio_read(4);	/* MinV            */
  jp2_struct->numcl = (box.length - 16) / 4;
  jp2_struct->cl =
    (unsigned int *) malloc(jp2_struct->numcl * sizeof(unsigned int));

  for (i = 0; i < (int) jp2_struct->numcl; i++)
    jp2_struct->cl[i] = cio_read(4);	/* CLi */

  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with FTYP Box\n");
    return 1;
  }
  return 0;
}

int jp2_write_jp2c(int j2k_codestream_len, int *j2k_codestream_offset,
		   char *j2k_codestream)
{
  jp2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(JP2_JP2C, 4);   /* JP2C*/

  *j2k_codestream_offset = cio_tell();
  memcpy(cio_getbp(), j2k_codestream, j2k_codestream_len);

  box.length = 8 + j2k_codestream_len;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/*    L       */
  cio_seek(box.init_pos + box.length);

  return box.length;
}


int jp2_read_jp2c(unsigned int *j2k_codestream_len,
		  unsigned int *j2k_codestream_offset)
{
  jp2_box_t box;

  jp2_read_boxhdr(&box);
  do {
    if (JP2_JP2C != box.type) {
      cio_skip(box.length - 8);
      jp2_read_boxhdr(&box);
    }
  } while (JP2_JP2C != box.type);

  *j2k_codestream_offset = cio_tell();
  *j2k_codestream_len = box.length - 8;

  return 0;
}

void jp2_write_jp()
{
  jp2_box_t box;

  box.init_pos = cio_tell();
  cio_skip(4);
  cio_write(JP2_JP, 4);      /* JP*/
  cio_write(0x0d0a870a, 4);

  box.length = cio_tell() - box.init_pos;
  cio_seek(box.init_pos);
  cio_write(box.length, 4);	/*    L       */
  cio_seek(box.init_pos + box.length);
}

/*
* Read the JP box
*
* JPEG 2000 signature
*
* return 1 if error else 0
*/
int jp2_read_jp()
{
  jp2_box_t box;

  jp2_read_boxhdr(&box);
  if (JP2_JP != box.type) {
    fprintf(stderr, "Error: Expected JP Marker\n");
    return 1;
  }
  if (0x0d0a870a != cio_read(4)) {
    fprintf(stderr, "Error with JP Marker\n");
    return 1;
  }
  if (cio_tell() - box.init_pos != box.length) {
    fprintf(stderr, "Error with JP Box size\n");
    return 1;
  }
  return 0;

}


int jp2_read_struct(unsigned char *src, jp2_struct_t * jp2_struct, int len)
{
  cio_init(src, len);

  if (jp2_read_jp())
    return 1;
  if (jp2_read_ftyp(jp2_struct))
    return 1;
  if (jp2_read_jp2h(jp2_struct))
    return 1;
  if (jp2_read_jp2c
      (&jp2_struct->j2k_codestream_len,
       &jp2_struct->j2k_codestream_offset))
    return 1;
  return 0;
}

int jp2_wrap_j2k(jp2_struct_t * jp2_struct, char *j2k_codestream,
		 char *output)
{
  (void)output;
  jp2_write_jp();
  jp2_write_ftyp(jp2_struct);
  jp2_write_jp2h(jp2_struct);

  jp2_write_jp2c(jp2_struct->j2k_codestream_len,
		 &jp2_struct->j2k_codestream_offset, j2k_codestream);

  return cio_tell();
}
