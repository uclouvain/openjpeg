/*
 * Copyright (c) 2001-2002, David Janssens
 * Copyright (c) 2003, Yannick Verschueren
 * Copyright (c) 2003,  Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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
#include <setjmp.h>
#include <math.h>
#include <unistd.h>

#include "j2k.h"
#include "cio.h"
#include "tcd.h"
#include "int.h"
#include "jpip.h"
#include "jp2.h"

#define J2K_MS_SOC 0xff4f
#define J2K_MS_SOT 0xff90
#define J2K_MS_SOD 0xff93
#define J2K_MS_EOC 0xffd9
#define J2K_MS_SIZ 0xff51
#define J2K_MS_COD 0xff52
#define J2K_MS_COC 0xff53
#define J2K_MS_RGN 0xff5e
#define J2K_MS_QCD 0xff5c
#define J2K_MS_QCC 0xff5d
#define J2K_MS_POC 0xff5f
#define J2K_MS_TLM 0xff55
#define J2K_MS_PLM 0xff57
#define J2K_MS_PLT 0xff58
#define J2K_MS_PPM 0xff60
#define J2K_MS_PPT 0xff61
#define J2K_MS_SOP 0xff91
#define J2K_MS_EPH 0xff92
#define J2K_MS_CRG 0xff63
#define J2K_MS_COM 0xff64

#define J2K_STATE_MHSOC 0x0001
#define J2K_STATE_MHSIZ 0x0002
#define J2K_STATE_MH 0x0004
#define J2K_STATE_TPHSOT 0x0008
#define J2K_STATE_TPH 0x0010
#define J2K_STATE_MT 0x0020

jmp_buf j2k_error;

static int j2k_state;
static int j2k_curtileno;
static j2k_tcp_t j2k_default_tcp;
static unsigned char *j2k_eot;

static j2k_image_t *j2k_img;
static j2k_cp_t *j2k_cp;

static unsigned char **j2k_tile_data;
static int *j2k_tile_len;

static info_image_t img;

void j2k_clean() {
  int tileno = 0;
  int compno, resno, precno;
  
  tcd_free(j2k_img,j2k_cp);
  
  for (tileno=0;tileno<j2k_cp->tw*j2k_cp->th;tileno++) {
    info_tile_t *tile_Idx = &img.tile[tileno];
    
    for (compno=0;compno<img.Comp;compno++)
      {
	info_compo_t *compo_Idx = &tile_Idx->compo[compno];
	
	for(resno=0;resno<img.Decomposition+1;resno++)
	  {
	    info_reso_t *reso_Idx = &compo_Idx->reso[resno];
	    for (precno=0;precno<img.tile[tileno].pw*img.tile[tileno].ph;precno++)
	      {
		info_prec_t *prec_Idx = &reso_Idx->prec[precno];
		free(prec_Idx->layer);
	      }
	    free(reso_Idx->prec);
	  }
	free(compo_Idx->reso);
      }
    free(tile_Idx->compo);
  }
  free(img.tile);
  free(img.marker);
}

void j2k_read_soc() {
    j2k_state=J2K_STATE_MHSIZ;
}

void j2k_read_siz() {
    int len, i;
    len=cio_read(2);

    // <INDEX>
    img.marker[img.num_marker].type=J2K_MS_SIZ;
    img.marker[img.num_marker].start_pos=cio_tell()-2;
    img.marker[img.num_marker].len=len;
    img.num_marker++;
    // </INDEX>

    
    cio_read(2);              // Rsiz (capabilities)
    j2k_img->x1=cio_read(4);  // Xsiz
    j2k_img->y1=cio_read(4);  // Ysiz
    j2k_img->x0=cio_read(4);  // X0siz
    j2k_img->y0=cio_read(4);  // Y0siz
    j2k_cp->tdx=cio_read(4);  // XTsiz
    j2k_cp->tdy=cio_read(4);  // YTsiz
    j2k_cp->tx0=cio_read(4);  // XT0siz
    j2k_cp->ty0=cio_read(4);  // YTOsiz
    j2k_img->numcomps=cio_read(2);  // Csiz
    j2k_img->comps=(j2k_comp_t*)malloc(j2k_img->numcomps*sizeof(j2k_comp_t));
    for (i=0; i<j2k_img->numcomps; i++) {
        int tmp, w, h;
        tmp=cio_read(1);
        j2k_img->comps[i].prec=(tmp&0x7f)+1;
        j2k_img->comps[i].sgnd=tmp>>7;
        j2k_img->comps[i].dx=cio_read(1);
        j2k_img->comps[i].dy=cio_read(1);
        w=int_ceildiv(j2k_img->x1-j2k_img->x0, j2k_img->comps[i].dx);
        h=int_ceildiv(j2k_img->y1-j2k_img->y0, j2k_img->comps[i].dy);
        j2k_img->comps[i].data=(int*)malloc(sizeof(int)*w*h);
    }
    j2k_cp->tw=int_ceildiv(j2k_img->x1-j2k_img->x0, j2k_cp->tdx);
    j2k_cp->th=int_ceildiv(j2k_img->y1-j2k_img->y0, j2k_cp->tdy);

    j2k_cp->tcps=(j2k_tcp_t*)calloc(sizeof(j2k_tcp_t), j2k_cp->tw*j2k_cp->th);
    j2k_default_tcp.tccps=(j2k_tccp_t*)calloc(sizeof(j2k_tccp_t), j2k_img->numcomps);
    for (i=0; i<j2k_cp->tw*j2k_cp->th; i++) {
        j2k_cp->tcps[i].tccps=(j2k_tccp_t*)calloc(sizeof(j2k_tccp_t), j2k_img->numcomps);
    }
    j2k_tile_data=(unsigned char**)calloc(j2k_cp->tw*j2k_cp->th, sizeof(char*));
    j2k_tile_len=(int*)calloc(j2k_cp->tw*j2k_cp->th, sizeof(int));
    j2k_state=J2K_STATE_MH;

    // <INDEX>
    img.Im_w=j2k_img->x1-j2k_img->x0;
    img.Im_h=j2k_img->y1-j2k_img->y0;
    img.Tile_x=j2k_cp->tdx;
    img.Tile_y=j2k_cp->tdy;
    img.Comp=j2k_img->numcomps;
    img.tw=j2k_cp->tw;
    img.th=j2k_cp->th;
    img.tile=(info_tile_t*)malloc((img.tw*img.th)*sizeof(info_tile_t));
    //</INDEX>


 }

void j2k_read_com() {
    int len;
    len=cio_read(2);

    // <INDEX>
    if (j2k_state==J2K_STATE_MH)
      {
	if (!img.marker_mul.num_COM)
	  img.marker_mul.COM=(info_marker_t*)malloc(sizeof(info_marker_t));
	else
	  img.marker_mul.COM=realloc(img.marker_mul.COM,(1+img.marker_mul.num_COM)*sizeof(info_marker_t));
	img.marker_mul.COM[img.marker_mul.num_COM].type=J2K_MS_COM;
	img.marker_mul.COM[img.marker_mul.num_COM].start_pos=cio_tell()-2;
	img.marker_mul.COM[img.marker_mul.num_COM].len=len;
	img.marker_mul.num_COM++;
      }
    // </INDEX>

    cio_skip(len-2);
}

void j2k_read_cox(int compno) {
    int i;
    j2k_tcp_t *tcp;
    j2k_tccp_t *tccp;
    tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
    tccp=&tcp->tccps[compno];
    tccp->numresolutions=cio_read(1)+1;

    img.Decomposition=tccp->numresolutions-1; // <INDEX>

    tccp->cblkw=cio_read(1)+2;
    tccp->cblkh=cio_read(1)+2;
    tccp->cblksty=cio_read(1);
    tccp->qmfbid=cio_read(1);
    if (tccp->csty&J2K_CP_CSTY_PRT) {
        for (i=0; i<tccp->numresolutions; i++) {
            int tmp=cio_read(1);
            tccp->prcw[i]=tmp&0xf;
            tccp->prch[i]=tmp>>4; 
        }
    }
}

void j2k_read_cod() {
    int len, i, pos;
    j2k_tcp_t *tcp;
    tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
    len=cio_read(2);

    // <INDEX>
    if (j2k_state==J2K_STATE_MH)
      {
	if (!img.marker_mul.num_COD)
	  img.marker_mul.COD=(info_marker_t*)malloc(sizeof(info_marker_t));
	else
	  img.marker_mul.COD=realloc(img.marker_mul.COD,(1+img.marker_mul.num_COD)*sizeof(info_marker_t));
	img.marker_mul.COD[img.marker_mul.num_COD].type=J2K_MS_COD;
	img.marker_mul.COD[img.marker_mul.num_COD].start_pos=cio_tell()-2;
	img.marker_mul.COD[img.marker_mul.num_COD].len=len;
	img.marker_mul.num_COD++;
      }
    // </INDEX>
    
    
    tcp->csty=cio_read(1);
    tcp->prg=cio_read(1);
    tcp->numlayers=cio_read(2);
    tcp->mct=cio_read(1);
    pos=cio_tell();
    for (i=0; i<j2k_img->numcomps; i++) {
        tcp->tccps[i].csty=tcp->csty&J2K_CP_CSTY_PRT;
        cio_seek(pos);
        j2k_read_cox(i);
    }
    //<INDEX>
    img.Prog=tcp->prg;
    img.Layer=tcp->numlayers;
    //</INDEX>
}

void j2k_read_coc() {
    int len, compno;
    j2k_tcp_t *tcp;
    tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
    len=cio_read(2);
    // <INDEX>
    if (j2k_state==J2K_STATE_MH)
      {
	if (!img.marker_mul.num_COC)
	  img.marker_mul.COC=(info_marker_t*)malloc(sizeof(info_marker_t));
	else
	  img.marker_mul.COC=realloc(img.marker_mul.COC,(1+img.marker_mul.num_COC)*sizeof(info_marker_t));
	img.marker_mul.COC[img.marker_mul.num_COC].type=J2K_MS_COC;
	img.marker_mul.COC[img.marker_mul.num_COC].start_pos=cio_tell()-2;
	img.marker_mul.COC[img.marker_mul.num_COC].len=len;
	img.marker_mul.num_COC++;
      }
    // </INDEX>

    
    compno=cio_read(j2k_img->numcomps<=256?1:2);
    tcp->tccps[compno].csty=cio_read(1);
    j2k_read_cox(compno);
}

void j2k_read_qcx(int compno, int len) {
    int tmp;
    j2k_tcp_t *tcp;
    j2k_tccp_t *tccp;
    int bandno, numbands;
    tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
    tccp=&tcp->tccps[compno];
    tmp=cio_read(1);
    tccp->qntsty=tmp&0x1f;
    tccp->numgbits=tmp>>5;
    numbands=tccp->qntsty==J2K_CCP_QNTSTY_SIQNT?1:(tccp->qntsty==J2K_CCP_QNTSTY_NOQNT?len-1:(len-1)/2);
    for (bandno=0; bandno<numbands; bandno++) {
        int expn, mant;
        if (tccp->qntsty==J2K_CCP_QNTSTY_NOQNT) { // WHY STEPSIZES WHEN NOQNT ?
            expn=cio_read(1)>>3;
            mant=0;
        } else {
            tmp=cio_read(2);
            expn=tmp>>11;
            mant=tmp&0x7ff;
        }
        tccp->stepsizes[bandno].expn=expn;
        tccp->stepsizes[bandno].mant=mant;
    }
}

void j2k_read_qcd() {
    int len, i, pos;
    len=cio_read(2);    

    // <INDEX>
    if (j2k_state==J2K_STATE_MH)
      {
	img.marker[img.num_marker].type=J2K_MS_QCD;
	img.marker[img.num_marker].start_pos=cio_tell()-2;
	img.marker[img.num_marker].len=len;
	img.num_marker++;
      }
    // </INDEX>
    
    
    pos=cio_tell();
    for (i=0; i<j2k_img->numcomps; i++) {
        cio_seek(pos);
        j2k_read_qcx(i, len-2);
    }
}

void j2k_read_qcc() {
  int len, compno;
  len=cio_read(2);  

  // <INDEX>
  if (j2k_state==J2K_STATE_MH)
    {
      if (!img.marker_mul.num_QCC)
	img.marker_mul.QCC=(info_marker_t*)malloc(sizeof(info_marker_t));
      else
	img.marker_mul.QCC=realloc(img.marker_mul.QCC,(1+img.marker_mul.num_QCC)*sizeof(info_marker_t));
      img.marker_mul.QCC[img.marker_mul.num_QCC].type=J2K_MS_QCC;
      img.marker_mul.QCC[img.marker_mul.num_QCC].start_pos=cio_tell()-2;
      img.marker_mul.QCC[img.marker_mul.num_QCC].len=len;
      img.marker_mul.num_QCC++;
    }
  // </INDEX>
  
  
  compno=cio_read(j2k_img->numcomps<=256?1:2);
  j2k_read_qcx(compno, len-2-(j2k_img->numcomps<=256?1:2));
}

void j2k_read_poc() {
  int len, numpchgs, i;
  j2k_tcp_t *tcp;
  fprintf(stderr, "WARNING: POC marker segment processing not fully implemented\n");
  tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
  len=cio_read(2);
  
  // <INDEX>
  if (j2k_state==J2K_STATE_MH)
    {
      img.marker[img.num_marker].type=J2K_MS_POC;
      img.marker[img.num_marker].start_pos=cio_tell()-2;
      img.marker[img.num_marker].len=len;
      img.num_marker++;
    }
  // </INDEX>
  

  numpchgs=(len-2)/(5+2*(j2k_img->numcomps<=256?1:2));
  for (i=0; i<numpchgs; i++) {
    j2k_poc_t *poc;
    poc=&tcp->pocs[i];
    poc->resno0=cio_read(1);
    poc->compno0=cio_read(j2k_img->numcomps<=256?1:2);
    poc->layno1=cio_read(2);
    poc->resno1=cio_read(1);
    poc->compno1=cio_read(j2k_img->numcomps<=256?1:2);
    poc->prg=cio_read(1);
  }
}

void j2k_read_crg() {
    int len;
    len=cio_read(2);

    // <INDEX>
    if (j2k_state==J2K_STATE_MH)
      {
	img.marker[img.num_marker].type=J2K_MS_CRG;
	img.marker[img.num_marker].start_pos=cio_tell()-2;
	img.marker[img.num_marker].len=len;
	img.num_marker++;
      }
    // </INDEX>

    fprintf(stderr, "WARNING: CRG marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_read_tlm() {
    int len;
    len=cio_read(2);

    // <INDEX>
    if (j2k_state==J2K_STATE_MH)
      {
	if (!img.marker_mul.num_TLM)
	  img.marker_mul.TLM=(info_marker_t*)malloc(sizeof(info_marker_t));
	else
	  img.marker_mul.TLM=realloc(img.marker_mul.TLM,(1+img.marker_mul.num_TLM)*sizeof(info_marker_t));
	img.marker_mul.TLM[img.marker_mul.num_TLM].type=J2K_MS_TLM;
	img.marker_mul.TLM[img.marker_mul.num_TLM].start_pos=cio_tell()-2;
	img.marker_mul.TLM[img.marker_mul.num_TLM].len=len;
	img.marker_mul.num_TLM++;
      }
    // </INDEX>
    
    fprintf(stderr, "WARNING: TLM marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_read_plm() {
    int len;
    len=cio_read(2);

    // <INDEX>
    if (j2k_state==J2K_STATE_MH)
      {
	if (!img.marker_mul.num_PLM)
	  img.marker_mul.PLM=(info_marker_t*)malloc(sizeof(info_marker_t));
	else
	  img.marker_mul.PLM=realloc(img.marker_mul.PLM,(1+img.marker_mul.num_PLM)*sizeof(info_marker_t));
	img.marker_mul.PLM[img.marker_mul.num_PLM].type=J2K_MS_PLM;
	img.marker_mul.PLM[img.marker_mul.num_PLM].start_pos=cio_tell()-2;
	img.marker_mul.PLM[img.marker_mul.num_PLM].len=len;
	img.marker_mul.num_PLM++;
      }
    // </INDEX>
    
    fprintf(stderr, "WARNING: PLM marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_read_plt() {
    int len;
    len=cio_read(2);
    fprintf(stderr, "WARNING: PLT marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_read_ppm() {
    int len;
    len=cio_read(2);
    // <INDEX>
    if (j2k_state==J2K_STATE_MH)
      {
	img.marker[img.num_marker].type=J2K_MS_PPM;
	img.marker[img.num_marker].start_pos=cio_tell()-2;
	img.marker[img.num_marker].len=len;
	img.num_marker++;
      }
    // </INDEX>
    
    fprintf(stderr, "WARNING: PPM marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_read_ppt() {
    int len;
    len=cio_read(2);
    fprintf(stderr, "WARNING: PPT marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_read_sot() {
    int len, tileno, totlen, partno, numparts, i;
    j2k_tcp_t *tcp;
    j2k_tccp_t *tmp;
    len=cio_read(2);
    tileno=cio_read(2);
    //<INDEX>
    if (!tileno) img.Main_head_end=cio_tell()-6;  // Correction End = First byte of first SOT
    img.tile[tileno].start_pos=cio_tell()-6;
    img.tile[tileno].num_tile=tileno;
    // </INDEX>
    totlen=cio_read(4); 
    if (!totlen) totlen = cio_numbytesleft() + 8;

    img.tile[tileno].end_pos=totlen+img.tile[tileno].start_pos; // <INDEX>

    partno=cio_read(1);
    numparts=cio_read(1);
    j2k_curtileno=tileno;
    j2k_eot=cio_getbp()-12+totlen;
    j2k_state=J2K_STATE_TPH;
    tcp=&j2k_cp->tcps[j2k_curtileno];
    tmp=tcp->tccps;
    *tcp=j2k_default_tcp;
    tcp->tccps=tmp;
    for (i=0; i<j2k_img->numcomps; i++) {
        tcp->tccps[i]=j2k_default_tcp.tccps[i];
    }
}

void j2k_read_sod() {
    int len;
    unsigned char *data;
    img.tile[j2k_curtileno].end_header=cio_tell()-1;  //<INDEX>
    len=int_min(j2k_eot-cio_getbp(), cio_numbytesleft());
    j2k_tile_len[j2k_curtileno]+=len;
    data=(unsigned char*)realloc(j2k_tile_data[j2k_curtileno], j2k_tile_len[j2k_curtileno]);
    memcpy(data, cio_getbp(), len);
    j2k_tile_data[j2k_curtileno]=data;
    cio_skip(len);
    j2k_state=J2K_STATE_TPHSOT;
   
}

void j2k_read_rgn() {
    int len, compno, roisty;
    j2k_tcp_t *tcp;
    tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
    len=cio_read(2);

    // <INDEX>
    if (j2k_state==J2K_STATE_MH)
      {
	if (!img.marker_mul.num_RGN)
	  img.marker_mul.RGN=(info_marker_t*)malloc(sizeof(info_marker_t));
	else
	  img.marker_mul.RGN=realloc(img.marker_mul.RGN,(1+img.marker_mul.num_RGN)*sizeof(info_marker_t));
	img.marker_mul.RGN[img.marker_mul.num_RGN].type=J2K_MS_RGN;
	img.marker_mul.RGN[img.marker_mul.num_RGN].start_pos=cio_tell()-2;
	img.marker_mul.RGN[img.marker_mul.num_RGN].len=len;
	img.marker_mul.num_RGN++;
      }
    // </INDEX>

    
    compno=cio_read(j2k_img->numcomps<=256?1:2);
    roisty=cio_read(1);
    tcp->tccps[compno].roishift=cio_read(1);
}

void j2k_read_eoc() {
    int tileno;
    tcd_init(j2k_img, j2k_cp, &img);
    for (tileno=0; tileno<j2k_cp->tw*j2k_cp->th; tileno++) {
        tcd_decode_tile(j2k_tile_data[tileno], j2k_tile_len[tileno], tileno, &img);
    }

    j2k_state=J2K_STATE_MT;
     longjmp(j2k_error, 1);

}

void j2k_read_unk() {
    fprintf(stderr, "warning: unknown marker\n");
}

int j2k_index_JPIP(char *Idx_file, char *J2K_file, int len){
  FILE *dest;
  char *index;
  int pos_iptr, end_pos;
  int len_cidx, pos_cidx;
  int len_jp2c, pos_jp2c;
  int len_fidx, pos_fidx;

  dest=fopen(Idx_file, "wb");
  if (!dest) {
    fprintf(stderr,"Failed to open %s for reading !!\n",Idx_file);
    return 0;
  }

 // INDEX MODE JPIP
 index=(char*)malloc(len); 
 cio_init(index, len);
 jp2_write_jp();
 jp2_write_ftyp();
 
 jp2_write_jp2h(j2k_img);
 jp2_write_dbtl(Idx_file);

 pos_iptr=cio_tell();
 cio_skip(24); // IPTR further !
 
 pos_jp2c=cio_tell();
 len_jp2c=jp2_write_jp2c(J2K_file);
  
 pos_cidx=cio_tell();
 len_cidx=jpip_write_cidx(pos_jp2c+8,img, j2k_cp); // Correction len_jp2C --> pos_jp2c+8  
 
 pos_fidx=cio_tell();
 len_fidx=jpip_write_fidx(pos_jp2c, len_jp2c, pos_cidx, len_cidx);
 
 end_pos=cio_tell();
 cio_seek(pos_iptr);
 jpip_write_iptr(pos_fidx,len_fidx);
 cio_seek(end_pos);

 fwrite(index, 1, cio_tell(), dest);
 free(index);

 fclose(dest);
 return 1;
}



typedef struct {
  int id;
    int states;
    void (*handler)();
} j2k_dec_mstabent_t;

j2k_dec_mstabent_t j2k_dec_mstab[]={
    {J2K_MS_SOC, J2K_STATE_MHSOC, j2k_read_soc},
    {J2K_MS_SOT, J2K_STATE_MH|J2K_STATE_TPHSOT, j2k_read_sot},
    {J2K_MS_SOD, J2K_STATE_TPH, j2k_read_sod},
    {J2K_MS_EOC, J2K_STATE_TPHSOT, j2k_read_eoc},
    {J2K_MS_SIZ, J2K_STATE_MHSIZ, j2k_read_siz},
    {J2K_MS_COD, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_cod},
    {J2K_MS_COC, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_coc},
    {J2K_MS_RGN, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_rgn},
    {J2K_MS_QCD, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_qcd},
    {J2K_MS_QCC, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_qcc},
    {J2K_MS_POC, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_poc},
    {J2K_MS_TLM, J2K_STATE_MH, j2k_read_tlm},
    {J2K_MS_PLM, J2K_STATE_MH, j2k_read_plm},
    {J2K_MS_PLT, J2K_STATE_TPH, j2k_read_plt},
    {J2K_MS_PPM, J2K_STATE_MH, j2k_read_ppm},
    {J2K_MS_PPT, J2K_STATE_TPH, j2k_read_ppt},
    {J2K_MS_SOP, 0, 0},
    {J2K_MS_CRG, J2K_STATE_MH, j2k_read_crg},
    {J2K_MS_COM, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_com},
    {0, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_unk}
};

j2k_dec_mstabent_t *j2k_dec_mstab_lookup(int id) {
    j2k_dec_mstabent_t *e;
    for (e=j2k_dec_mstab; e->id!=0; e++) {
        if (e->id==id) {
            break;
        }
    }
    return e;
}

LIBJ2K_API int j2k_decode(unsigned char *src, int len, j2k_image_t **image, j2k_cp_t **cp) {
    if (setjmp(j2k_error)) {
        if (j2k_state!=J2K_STATE_MT) {
            fprintf(stderr, "WARNING: incomplete bitstream\n");
            return 0;
        }
        return cio_numbytes();
    }
    j2k_img=(j2k_image_t*)malloc(sizeof(j2k_image_t));
    j2k_cp=(j2k_cp_t*)malloc(sizeof(j2k_cp_t));
    *image=j2k_img;
    *cp=j2k_cp;
    j2k_state=J2K_STATE_MHSOC;
    cio_init(src, len);
    for (;;) {
        j2k_dec_mstabent_t *e;
        int id=cio_read(2);
        if (id>>8!=0xff) {
            fprintf(stderr, "%.8x: expected a marker instead of %x\n", cio_tell()-2, id);
            return 0;
        }
        e=j2k_dec_mstab_lookup(id);
        if (!(j2k_state & e->states)) {
            fprintf(stderr, "%.8x: unexpected marker %x\n", cio_tell()-2, id);
            return 0;
        }
        if (e->handler) {
            (*e->handler)();
        }
    }

}

#ifdef WIN32
#include <windows.h>

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif


extern info_image_t img;


int main(int argc, char **argv)
{  
  FILE *src;
  int totlen;
  char *j2kfile;
  j2k_image_t *imgg;
  j2k_cp_t *cp;

  if (argc!=3)
    {
      fprintf(stderr,"\nERROR in entry : index_create J2K-file Idx-file\n\n");
      return 0;
    }

  src=fopen(argv[1], "rb");
  if (!src) {
    fprintf(stderr,"Failed to open %s for reading !!\n",argv[1]);
    return 0;
  }

  // length of the codestream
  fseek(src, 0, SEEK_END);
  totlen=ftell(src);
  fseek(src, 0, SEEK_SET);
  
  j2kfile=(char*)malloc(totlen);
  fread(j2kfile, 1, totlen, src);
  
  img.marker=(info_marker_t*)malloc(32*sizeof(info_marker_t));
  img.num_marker=0;
  img.marker_mul.num_COD=0;
  img.marker_mul.num_COC=0;
  img.marker_mul.num_RGN=0;
  img.marker_mul.num_QCC=0;
  img.marker_mul.num_TLM=0;
  img.marker_mul.num_PLM=0;
  img.marker_mul.num_COM=0;

  // decode  

  if (!j2k_decode(j2kfile, totlen, &imgg, &cp)) {
    fprintf(stderr, "Index_creator: failed to decode image!\n");
    return 1;
  }

  free(j2kfile);

  fseek(src, 0, SEEK_SET);
  img.codestream_size=totlen;

  j2k_index_JPIP(argv[2],argv[1],totlen*2>30000?totlen*2:30000);

  fclose(src);

  j2k_clean();
  return 1;
}
