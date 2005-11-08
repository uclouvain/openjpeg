/*
 * Copyright (c) 2001-2002, David Janssens
 * Copyright (c) 2002-2004, Yannick Verschueren
 * Copyright (c) 2002-2004, Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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

#include "t2.h"
#include "tcd.h"
#include "bio.h"
#include "j2k.h"
#include "pi.h"
#include "tgt.h"
#include "int.h"
#include "cio.h"
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>

#define RESTART 0x04

extern jmp_buf j2k_error;

void t2_putcommacode(int n)
{
  while (--n >= 0) {
    bio_write(1, 1);
  }
  bio_write(0, 1);
}

int t2_getcommacode()
{
  int n;
  for (n = 0; bio_read(1); n++) {
  }
  return n;
}

/* <summary> */
/* Variable length code for signalling delta Zil (truncation point) */
/* <val> n : delta Zil */
/* <\summary> */
void t2_putnumpasses(int n)
{
  if (n == 1) {
    bio_write(0, 1);
  } else if (n == 2) {
    bio_write(2, 2);
  } else if (n <= 5) {
    bio_write(0xc | (n - 3), 4);
  } else if (n <= 36) {
    bio_write(0x1e0 | (n - 6), 9);
  } else if (n <= 164) {
    bio_write(0xff80 | (n - 37), 16);
  }
}

int t2_getnumpasses()
{
  int n;
  if (!bio_read(1))
    return 1;
  if (!bio_read(1))
    return 2;
  if ((n = bio_read(2)) != 3)
    return 3 + n;
  if ((n = bio_read(5)) != 31)
    return 6 + n;
  return 37 + bio_read(7);
}

/*
 * Encode a packet of a tile to a destination buffer
 *
 * Tile    : the tile for which to write the packets
 * tcp     : the tile coding parameters
 * compno  : Identity of the packet --> component value
 * resno   : Identity of the packet --> resolution level value
 * precno  : Identity of the packet --> precinct value
 * layno   : Identity of the packet --> quality layer value
 * dest    : the destination buffer
 * len     : the length of the destination buffer
 * info_IM : structure to create an index file
 * tileno  : number of the tile encoded
*/
int t2_encode_packet(tcd_tile_t * tile, j2k_tcp_t * tcp, int compno,
		     int resno, int precno, int layno, unsigned char *dest,
		     int len, info_image * info_IM, int tileno)
{
  int bandno, cblkno;
  unsigned char *sop = 0, *eph = 0;
  tcd_tilecomp_t *tilec = &tile->comps[compno];
  tcd_resolution_t *res = &tilec->resolutions[resno];
  unsigned char *c = dest;

  /* <SOP 0xff91> */
  if (tcp->csty & J2K_CP_CSTY_SOP) {
    sop = (unsigned char *) malloc(6 * sizeof(unsigned char));
    sop[0] = 255;
    sop[1] = 145;
    sop[2] = 0;
    sop[3] = 4;
    sop[4] = (info_IM->num % 65536) / 256;
    sop[5] = (info_IM->num % 65536) % 256;
    memcpy(c, sop, 6);
    free(sop);
    c += 6;
  }
  /* </SOP> */

  if (!layno) {
    for (bandno = 0; bandno < res->numbands; bandno++) {
      tcd_band_t *band = &res->bands[bandno];
      tcd_precinct_t *prc = &band->precincts[precno];
      tgt_reset(prc->incltree);
      tgt_reset(prc->imsbtree);
      for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
	tcd_cblk_t *cblk = &prc->cblks[cblkno];
	cblk->numpasses = 0;
	tgt_setvalue(prc->imsbtree, cblkno, band->numbps - cblk->numbps);
      }
    }
  }

  bio_init_enc(c, len);
  bio_write(1, 1);		/* Empty header bit */

  /* Writing Packet header */
  for (bandno = 0; bandno < res->numbands; bandno++) {
    tcd_band_t *band = &res->bands[bandno];
    tcd_precinct_t *prc = &band->precincts[precno];
    for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
      tcd_cblk_t *cblk = &prc->cblks[cblkno];
      tcd_layer_t *layer = &cblk->layers[layno];
      if (!cblk->numpasses && layer->numpasses) {
	tgt_setvalue(prc->incltree, cblkno, layno);
      }
    }
    for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
      tcd_cblk_t *cblk = &prc->cblks[cblkno];
      tcd_layer_t *layer = &cblk->layers[layno];
      int increment = 0;
      int nump = 0;
      int len = 0, passno;
      /* cblk inclusion bits */
      if (!cblk->numpasses) {
	tgt_encode(prc->incltree, cblkno, layno + 1);
      } else {
	bio_write(layer->numpasses != 0, 1);
      }
      /* if cblk not included, go to the next cblk  */
      if (!layer->numpasses) {
	continue;
      }
      /* if first instance of cblk --> zero bit-planes information */
      if (!cblk->numpasses) {
	cblk->numlenbits = 3;
	tgt_encode(prc->imsbtree, cblkno, 999);
      }
      /* number of coding passes included */
      t2_putnumpasses(layer->numpasses);

      /* computation of the increase of the length indicator and insertion in the header     */
      for (passno = cblk->numpasses;
	   passno < cblk->numpasses + layer->numpasses; passno++) {
	tcd_pass_t *pass = &cblk->passes[passno];
	nump++;
	len += pass->len;
	if (pass->term
	    || passno == (cblk->numpasses + layer->numpasses) - 1) {
	  increment =
	    int_max(increment,
		    int_floorlog2(len) + 1 -
		    (cblk->numlenbits + int_floorlog2(nump)));
	  len = 0;
	  nump = 0;
	}
      }
      t2_putcommacode(increment);
      /* computation of the new Length indicator */
      cblk->numlenbits += increment;
      /* insertion of the codeword segment length */

      for (passno = cblk->numpasses;
	   passno < cblk->numpasses + layer->numpasses; passno++) {
	tcd_pass_t *pass = &cblk->passes[passno];
	nump++;
	len += pass->len;
	if (pass->term
	    || passno == (cblk->numpasses + layer->numpasses) - 1) {
	  bio_write(len, cblk->numlenbits + int_floorlog2(nump));
	  len = 0;
	  nump = 0;
	}
      }
    }
  }

  if (bio_flush())
    return -999;		/* modified to eliminate longjmp !! */

  c += bio_numbytes();

  /* <EPH 0xff92> */
  if (tcp->csty & J2K_CP_CSTY_EPH) {
    eph = (unsigned char *) malloc(2 * sizeof(unsigned char));
    eph[0] = 255;
    eph[1] = 146;
    memcpy(c, eph, 2);
    free(eph);
    c += 2;
  }
  /* </EPH> */

  /* Writing the packet body */

  for (bandno = 0; bandno < res->numbands; bandno++) {
    tcd_band_t *band = &res->bands[bandno];
    tcd_precinct_t *prc = &band->precincts[precno];
    for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
      tcd_cblk_t *cblk = &prc->cblks[cblkno];
      tcd_layer_t *layer = &cblk->layers[layno];
      if (!layer->numpasses) {
	continue;
      }
      if (c + layer->len > dest + len) {
	return -999;
      }

      memcpy(c, layer->data, layer->len);
      cblk->numpasses += layer->numpasses;
      c += layer->len;
      /* ADD for index Cfr. Marcela --> delta disto by packet */
      if (info_IM->index_write && info_IM->index_on) {
	info_tile *info_TL = &info_IM->tile[tileno];
	info_packet *info_PK = &info_TL->packet[info_IM->num];
	info_PK->disto += layer->disto;
	if (info_IM->D_max < info_PK->disto)
	  info_IM->D_max = info_PK->disto;
      }				/* </ADD> */
    }
  }
  return c - dest;
}

void t2_init_seg(tcd_seg_t * seg, int cblksty, int first)
{
  seg->numpasses = 0;
  seg->len = 0;
  if (cblksty & J2K_CCP_CBLKSTY_TERMALL)
    seg->maxpasses = 1;
  else if (cblksty & J2K_CCP_CBLKSTY_LAZY) {
    if (first)
      seg->maxpasses = 10;
    else
      seg->maxpasses = (((seg - 1)->maxpasses == 1)
			|| ((seg - 1)->maxpasses == 10)) ? 2 : 1;
  } else
    seg->maxpasses = 109;
}

/*  
 * Decode a packet of a tile from a source buffer
 *
 * src          : the source buffer
 * len          : the length of the source buffer
 * tile         : the tile for which to write the packets
 * cp           : the image coding parameters
 * tcp          : the tile coding parameters
 * compno  : Identity of the packet --> component value
 * resno      : Identity of the packet --> resolution level value
 * precno    : Identity of the packet --> precinct value
 * layno      : Identity of the packet --> quality layer value
 */
int t2_decode_packet(unsigned char *src, int len, tcd_tile_t * tile,
		     j2k_cp_t * cp, j2k_tcp_t * tcp, int compno, int resno,
		     int precno, int layno)
{
  int bandno, cblkno;
  tcd_tilecomp_t *tilec = &tile->comps[compno];
  tcd_resolution_t *res = &tilec->resolutions[resno];
  unsigned char *c = src;
  unsigned char *hd = NULL;
  int present;

  if (layno == 0) {
    for (bandno = 0; bandno < res->numbands; bandno++) {
      tcd_band_t *band = &res->bands[bandno];
      tcd_precinct_t *prc = &band->precincts[precno];
      
      //Add Antonin : sizebug1
      if ((band->x1-band->x0 == 0)||(band->y1-band->y0 == 0)) continue;
      //ddA
      
      tgt_reset(prc->incltree);
      tgt_reset(prc->imsbtree);
      for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
	tcd_cblk_t *cblk = &prc->cblks[cblkno];
	cblk->numsegs = 0;
      }
    }
  }
  
  // SOP markers
  if (tcp->csty & J2K_CP_CSTY_SOP) {
    if ((*c) != 0xff || (*(c + 1) != 0x91)) {
      printf("Warning : expected SOP marker\n");
    } else {
      c += 6;
    }
    //TODO : check the Nsop value
  }

  /* When the marker PPT/PPM is used the packet header are store in PPT/PPM marker
  This part deal with this caracteristic
  step 1: Read packet header in the saved structure
  step 2: Return to codestream for decoding */

  if (cp->ppm == 1) {		/* PPM */
    hd = cp->ppm_data;
    bio_init_dec(hd, cp->ppm_len); //Mod Antonin : ppmbug1
  } else if (tcp->ppt == 1) {	/* PPT */
    hd = tcp->ppt_data;
    bio_init_dec(hd, tcp->ppt_len);  //Mod Antonin : ppmbug1
  } else {			/* Normal Case */
    hd = c;
    bio_init_dec(hd, src+len-hd);
  }
  
  present = bio_read(1);

  if (!present) {
    bio_inalign();
    hd += bio_numbytes();

    // EPH markers
    if (tcp->csty & J2K_CP_CSTY_EPH) {
      if ((*hd) != 0xff || (*(hd + 1) != 0x92)) {
	printf("Error : expected EPH marker\n");
      } else {
	hd += 2;
      }
    }

    if (cp->ppm == 1) {		/* PPM case */
      cp->ppm_len+=cp->ppm_data-hd;
      cp->ppm_data = hd;
      return c - src;
    }
    if (tcp->ppt == 1) {	/* PPT case */
      tcp->ppt_len+=tcp->ppt_data-hd;
      tcp->ppt_data = hd;
      return c - src;
    }

    return hd - src;
  }

  for (bandno = 0; bandno < res->numbands; bandno++) {
    tcd_band_t *band = &res->bands[bandno];
    tcd_precinct_t *prc = &band->precincts[precno];

    //Add Antonin : sizebug1
    if ((band->x1-band->x0 == 0)||(band->y1-band->y0 == 0)) continue;
    //ddA

    for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
      int included, increment, n;
      tcd_cblk_t *cblk = &prc->cblks[cblkno];
      tcd_seg_t *seg;
      /* if cblk not yet included before --> inclusion tagtree */
      if (!cblk->numsegs) {
	included = tgt_decode(prc->incltree, cblkno, layno + 1);
	/* else one bit */
      } else {
	included = bio_read(1);
      }
      /* if cblk not included */
      if (!included) {
	cblk->numnewpasses = 0;
	continue;
      }
      /* if cblk not yet included --> zero-bitplane tagtree */
      if (!cblk->numsegs) {
	int i, numimsbs;
	for (i = 0; !tgt_decode(prc->imsbtree, cblkno, i); i++) {
	}
	numimsbs = i - 1;
	cblk->numbps = band->numbps - numimsbs;
	cblk->numlenbits = 3;
      }
      /* number of coding passes */
      cblk->numnewpasses = t2_getnumpasses();
      increment = t2_getcommacode();
      /* length indicator increment */
      cblk->numlenbits += increment;
      if (!cblk->numsegs) {
	seg = &cblk->segs[0];
	t2_init_seg(seg, tcp->tccps[compno].cblksty, 1);
      } else {
	seg = &cblk->segs[cblk->numsegs - 1];
	if (seg->numpasses == seg->maxpasses) {
	  t2_init_seg(++seg, tcp->tccps[compno].cblksty, 0);
	}
      }
      n = cblk->numnewpasses;

      do {
	seg->numnewpasses = int_min(seg->maxpasses - seg->numpasses, n);
	seg->newlen =
	  bio_read(cblk->numlenbits + int_floorlog2(seg->numnewpasses));
	n -= seg->numnewpasses;
	if (n > 0) {
	  t2_init_seg(++seg, tcp->tccps[compno].cblksty, 0);
	}
      } while (n > 0);
    }
  }
  if (bio_inalign())
    return -999;

  hd += bio_numbytes();

  // EPH markers
  if (tcp->csty & J2K_CP_CSTY_EPH) {
    if ((*hd) != 0xff || (*(hd + 1) != 0x92)) {
      printf("Error : expected EPH marker\n");
    } else {
      hd += 2;
    }
  }

  if (cp->ppm==1) {
    cp->ppm_len+=cp->ppm_data-hd;
    cp->ppm_data = hd;
  } else if (tcp->ppt == 1) {
    tcp->ppt_len+=tcp->ppt_data-hd;
    tcp->ppt_data = hd;
  } else {
    c=hd;
  }

  for (bandno = 0; bandno < res->numbands; bandno++) {
    tcd_band_t *band = &res->bands[bandno];
    tcd_precinct_t *prc = &band->precincts[precno];

    //Add Antonin : sizebug1
    if ((band->x1-band->x0 == 0)||(band->y1-band->y0 == 0)) continue;
    //ddA

    for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
      tcd_cblk_t *cblk = &prc->cblks[cblkno];
      tcd_seg_t *seg;
      if (!cblk->numnewpasses)
	continue;
      if (!cblk->numsegs) {
	seg = &cblk->segs[0];
	cblk->numsegs++;
	cblk->len = 0;
      } else {
	seg = &cblk->segs[cblk->numsegs - 1];
	if (seg->numpasses == seg->maxpasses) {
	  seg++;
	  cblk->numsegs++;
	}
      }
      do {
	if (c + seg->newlen > src + len) {
	  return -999;
	}

	memcpy(cblk->data + cblk->len, c, seg->newlen);
	if (seg->numpasses == 0) {
	  seg->data = cblk->data + cblk->len;
	}
	c += seg->newlen;
	cblk->len += seg->newlen;
	seg->len += seg->newlen;
	seg->numpasses += seg->numnewpasses;
	cblk->numnewpasses -= seg->numnewpasses;
	if (cblk->numnewpasses > 0) {
	  seg++;
	  cblk->numsegs++;
	}
      } while (cblk->numnewpasses > 0);
    }
  }

  return c - src;
}



/*
 * Encode the packets of a tile to a destination buffer
 *
 * img        : the source image
 * cp         : the image coding parameters
 * tileno     : number of the tile encoded
 * tile       : the tile for which to write the packets
 * maxlayers  : maximum number of layers
 * dest       : the destination buffer
 * len        : the length of the destination buffer
 * info_IM    : structure to create an index file
 */
int t2_encode_packets(j2k_image_t * img, j2k_cp_t * cp, int tileno,
		      tcd_tile_t * tile, int maxlayers,
		      unsigned char *dest, int len, info_image * info_IM)
{
  unsigned char *c = dest;
  int e = 0;
  pi_iterator_t *pi;
  int pino, compno;

  pi = pi_create(img, cp, tileno);

  for (pino = 0; pino <= cp->tcps[tileno].numpocs; pino++) {
    while (pi_next(&pi[pino])) {
      if (pi[pino].layno < maxlayers) {
	e = t2_encode_packet(tile, &cp->tcps[tileno],
			     pi[pino].compno, pi[pino].resno,
			     pi[pino].precno, pi[pino].layno, c,
			     dest + len - c, info_IM, tileno);
	if (e == -999) {
	  break;
	} else
	  c += e;
	/* INDEX >> */
	if (info_IM->index_write && info_IM->index_on) {
	  info_tile *info_TL = &info_IM->tile[tileno];
	  info_packet *info_PK = &info_TL->packet[info_IM->num];
	  if (!info_IM->num) {
	    info_PK->start_pos = info_TL->end_header + 1;
	  } else {
	    info_PK->start_pos =
	      info_TL->packet[info_IM->num - 1].end_pos + 1;
	  }
	  info_PK->end_pos = info_PK->start_pos + e - 1;

	}
	/* << INDEX */
	if ((info_IM->index_write
	     && cp->tcps[tileno].csty & J2K_CP_CSTY_SOP)
	    || (info_IM->index_write && info_IM->index_on)) {
	  info_IM->num++;
	}
      }

    }

    /* FREE space memory taken by pi */
    for (compno = 0; compno < pi[pino].numcomps; compno++) {
      free(pi[pino].comps[compno].resolutions);
    }
    free(pi[pino].comps);
  }
  free(pi[0].include);
  free(pi);
  if (e == -999)
    return e;
  else
    return c - dest;
}



/*
 * Decode the packets of a tile from a source buffer
 *
 * src: the source buffer
 * len: length of the source buffer
 * img: destination image
 * cp: image coding parameters
 * tileno: number that identifies the tile for which to decode the packets
 * tile: tile for which to decode the packets
 */
int t2_decode_packets(unsigned char *src, int len, j2k_image_t * img,
		      j2k_cp_t * cp, int tileno, tcd_tile_t * tile)
{
  unsigned char *c = src;
  pi_iterator_t *pi;
  int pino, compno, e = 0;
  int n = 0;

  pi = pi_create(img, cp, tileno);

  for (pino = 0; pino <= cp->tcps[tileno].numpocs; pino++) {
    while (pi_next(&pi[pino])) {
      e = t2_decode_packet(c, src + len - c, tile, cp,
			   &cp->tcps[tileno], pi[pino].compno,
			   pi[pino].resno, pi[pino].precno,
			   pi[pino].layno);

      /* progression in resolution */
      img->comps[pi[pino].compno].resno_decoded =
	e > 0 ? int_max(pi[pino].resno,
			img->comps[pi[pino].compno].
			resno_decoded) : img->comps[pi[pino].
						    compno].resno_decoded;
      n++;

      if (e == -999) {		/* ADD */
	break;
      } else
	c += e;
    }

    /* FREE space memory taken by pi */
    for (compno = 0; compno < pi[pino].numcomps; compno++) {
      free(pi[pino].comps[compno].resolutions);
    }
    free(pi[pino].comps);
  }
  free(pi[0].include);
  free(pi);

  if (e == -999)
    return e;
  else
    return c - src;
}
