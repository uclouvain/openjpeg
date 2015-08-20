/*
 *  Part 2   Extensions (JPX)
 * fcd15444-2-JPX-2000.pdf
 * fcd15444-2annexm-JPX-2004.pdf
 * fcd15444-2annexn-JPX-2004.pdf
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define snprintf _snprintf
#define sprintf _scprintf
#define strdup _strdup
#else /* not _WIN32 */
#define __STDC_FORMAT_MACROS
#include <stdint.h>
#include <stddef.h>
#endif /* _WIN32 */

#include <FL/Fl.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Pixmap.H>
#include <FL/fl_utf8.h>

#include "opj_inttypes.h"
#include "opj_stdint.h"

#include "viewerdefs.hh"
#include "tree.hh"

 static const char *L_document_xpm[] =
{
 "11 11 3 1",
 ".  c None",
 "x  c #d8d8f8",
 "@  c #202060",
 ".@@@@@@....",
 ".@xxx@.@...",
 ".@xxx@..@..",
 ".@xxx@@@@@.",
 ".@xxxxxxx@.",
 ".@xxxxxxx@.",
 ".@xxxxxxx@.",
 ".@xxxxxxx@.",
 ".@xxxxxxx@.",
 ".@xxxxxxx@.",
 ".@@@@@@@@@."
};
static Fl_Pixmap L_documentpixmap(L_document_xpm);


//FORWARD

static unsigned char *jpx_start;

#define LOG(s) fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,s)

static int cur_composition_layer_w = 100;
static int cur_composition_layer_h = 100;

static char item_name[MAX_ITEMBUF+2];

//--------- end FORWARD

static void add_sub_item()
{
    Fl_Tree_Item *sub = tree->add(item_name);
    sub->usericon(&L_documentpixmap);
}

static unsigned char *read_flst(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t off;
	unsigned int len;
	unsigned short i, n, dr;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Fragment List box",name_buf);
	add_sub_item();

	n = get2(s); s += 2;
	snprintf(item_name, MAX_ITEMBUF, "%s/nr_fragments %u",name_buf,n);
	add_sub_item();

	for(i = 0; i < n; ++i)
   {
	off = get8(s); s += 8;
	len = get4(s); s += 4;
	dr = get2(s); s += 2;

	snprintf(item_name, MAX_ITEMBUF,
	 "%s/frag[%u] off(%"PRIu64") len(%u) dr(%u)", name_buf, i,off,len,dr);
	add_sub_item();
   }

	assert(s == box_end);
	return box_end;
}/* read_flst() */

static unsigned char *read_cref(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep, rtyp;
	char name_buf[MAX_ITEMBUF];
	int pos = -1;

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Cross-Reference box",name_buf);
	add_sub_item();

	rtyp = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/rtyp(%d)",name_buf,rtyp);
	add_sub_item();

	assert(s == box_end);

	box_len = read_boxheader(s, dend, &hstep);
	
	if(memcmp(box_name, "flst", 4) != 0)
   {
LOG("flst missing. STOP.");

	return dend;
   }
   {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] flst",  name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_flst(s + hstep, s + box_len, dend, item_name);

	sup->close();
   }
	assert(s == box_end);	
	return box_end;
}/* read_cref() */

static unsigned char *read_lbl(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned int n;
	unsigned char *buf;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Label box",name_buf);
	add_sub_item();

	n = (unsigned int)(box_end - s); 
	buf = (unsigned char*)malloc(n+1); 
	if(buf)
   {
	buf[n] = 0;
	memcpy(buf, s, n);

	snprintf(item_name, MAX_ITEMBUF, "%s/text(%s)",name_buf,(char*)buf);
	add_sub_item();

	free(buf);
   }
	s += n;
	return box_end;
}

static unsigned char *read_roid(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned int x, y, w, h;
	unsigned short i, nr, r, typ, prio;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/ROI Description box",name_buf);
	add_sub_item();

/*-----------------
//JUNK
//annexm, page 199: Rcp is mentioned
//annexm, page 200:

Rcpi: Region of Interest coding priority. 
	  This value describes the coding priority of the Region of Interest.
      The value 0 means low coding priority and 255 means maximum coding 
	  priority. This value is
      encoded as a 1-byte unsigned integer. In transcoding applications, 
	  bits should be allocated with
      respect to the coding priority of each ROI.

Some lines below in Table M.49, Rcp is missing
---------------------*/ 
	nr = s[0]; ++s;
	snprintf(item_name, MAX_ITEMBUF, "%s/nr_regions %d",name_buf,nr);
	add_sub_item();

	for(i = 0; i < nr; ++i)
   {
	r = s[0]; ++s;
	typ = s[0]; ++s;
	prio = s[0]; ++s;
	x = get4(s); s += 4;
	y = get4(s); s += 4;
	w = get4(s); s += 4;
	h = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/roi[%u] present(%u) typ(%u) prio(%u) "
	 "x(%d) y(%d) w(%d) h(%d)",
	 name_buf, i,r,typ,prio,x,y,w,h);
	add_sub_item();
   }
	assert(s == box_end);// JUNK TEST
	return box_end;
}/* read_roid() */

static unsigned char *read_jpch(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep, nr_channels = 0;
	char name_buf[MAX_ITEMBUF];
	int pos = -1;

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Codestream Header box",name_buf);
	add_sub_item();

	while(s < box_end)
   {
	box_len = read_boxheader(s, dend, &hstep);

	if(memcmp(box_name, "lbl ", 4) == 0)
  {
    Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] lbl ", name_buf, ++pos);
    sup = tree->add(item_name);

	s = read_lbl(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "ihdr", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] ihdr", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_ihdr(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "bpcc", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] bpcc", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_bpcc(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "pclr", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] pclr", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_pclr(s + hstep, s + box_len, &nr_channels, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "cmap", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] cmap", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_cmap(s + hstep, s + box_len, nr_channels, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "roid", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] roid", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_roid(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }
   }
	assert(s == box_end);
	return box_end;

}/* read_jpch() */

static unsigned char *read_cgrp(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep;
	char name_buf[MAX_ITEMBUF];
	int pos = -1;

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Colour Group box",name_buf);
	add_sub_item();

	while(s < box_end)
   {
	Fl_Tree_Item *sup;

	box_len = read_boxheader(s, dend, &hstep);

	if(memcmp(box_name, "colr", 4) != 0) break;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] colr", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_colr(s + hstep, s + box_len, item_name);

	sup->close();
   }
	assert(s == box_end);
	return box_end;

}/* read_cgrp() */

static unsigned char *read_opct(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned short typ, n, i;
	unsigned int siz;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Opacity box",name_buf);
	add_sub_item();

	typ = s[0]; ++s;
	n = s[0]; ++s;
	if(n > 0) 
	 siz = (unsigned int)(box_end - s); 
	else 
	 siz = 0;

	snprintf(item_name, MAX_ITEMBUF, "%s/typ(%u) nr(%u) size(%u)",name_buf,typ,n,siz);
	add_sub_item();

	for(i = 0; i < n; ++i)
   {
/*----------------
CVi: Chroma-key value. This field specifies the value of channel i 
     for the chroma-key colour. Samples
     that match the chroma-key value for all channels shall be 
	 considered fully transparent. 
	 The size of
     this field is specified by the bit depth of the corresponding 
	 channel. 
	 If the value is not a multiple
     of 8, then each CVi value shall be padded to a multiple of 8 bits 
	 with bits equal to the sign bit and
     the actual value shall be stored in the low-order bits of the 
	 padded value. 
	 For example, if the depth
     of a channel is a signed 10-bit value, then the CVi value shall 
	 be stored in the low 10 bits of a 16-bit
     field and the high-order 6 bits shall be all equal to the sign 
	 bit of the value in this CVi field.

-----------------*/
	snprintf(item_name, MAX_ITEMBUF, "%s/chroma-key[%u]%u",name_buf,i,get_size(s, siz));
	add_sub_item();

	s += siz;
   }
	assert(s == box_end);
	return box_end;

}/* read_opct() */

static unsigned char *read_creg(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned short xs, ys, n, i, cdn, xr, yr, xo, yo;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Codestream Registration box",name_buf);
	add_sub_item();

	xs = get2(s); s += 2;
	ys = get2(s); s += 2;
	n = (unsigned short)((box_end - s)/6);

	snprintf(item_name, MAX_ITEMBUF, "%s/xs(%d) ys(%d) n(%d)",name_buf,xs,ys,n);
	add_sub_item();

	for(i = 0; i < n; ++i)
   {
	cdn = get2(s); s += 2;
	xr = s[0]; ++s;
	yr = s[0]; ++s;
	xo = s[0]; ++s;
	yo = s[0]; ++s;

	snprintf(item_name, MAX_ITEMBUF, "%s/stream[%u]cdn(%u) xr(%u) yr(%u) xo(%u) yo(%u)",
	name_buf,i,cdn,xr,yr,xo,yo);
	add_sub_item();
   }
	assert(s == box_end);
	return box_end;

}/* read_creg() */


static unsigned char *read_jplh(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep;
	char name_buf[MAX_ITEMBUF];
	int pos = -1;

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Compositing Layer Header box",name_buf);
	add_sub_item();

	while(s < box_end)
   {
	box_len = read_boxheader(s, dend, &hstep);

	if(memcmp(box_name, "lbl ", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] lbl ", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_lbl(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "cgrp", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] cgrp", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_cgrp(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "opct", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] opct", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_opct(s + hstep, s + box_len, dend, item_name);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);
  }

	if(memcmp(box_name, "cdef", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] cdef", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_cdef(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "creg", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] creg", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_creg(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "res ", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] res ", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_res(s + hstep, s + box_len, dend, (unsigned char*)item_name);

	sup->close();
	continue;
  }
   }
	assert(s == box_end);
	return box_end;

}/* read_jplh() */

static unsigned char *read_inst(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned int tick, x0, y0, w, h, life, next_use,xc,yc,wc,hc;
	unsigned short typ, rept, persist, has_xy, has_wh, has_life, has_crop;
	int i;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Instruction Set box",name_buf);
	add_sub_item();

	typ = get2(s); s += 2;
	rept = get2(s); s += 2;
	tick = get4(s); s += 4;
    has_xy = (typ>>0)&1;
    has_wh = (typ>>1)&1;
    has_life = (typ>>2)&1;
    has_crop = (typ>>5)&1;

#ifdef DEBUG_INST
fprintf(stderr,"read_inst\n\t   typ(%d)(%d,%d,%d,%d, %d,%d,%d,%d)\n"
"\ttyp(%d) rept(%d) tick(%d)\n",typ, (typ>>7)&1,(typ>>6)&1,
has_crop,(typ>>4)&1,(typ>>3)&1,has_life,has_wh,has_xy,
rept,tick);
#endif
	snprintf(item_name, MAX_ITEMBUF, "%s/typ(%d) rept(%d) tick(%d)",name_buf,
	 typ,rept,tick);
	add_sub_item();

	i = -1;
	while(s < box_end)
   {
	++i;
    
    if(has_xy)
  { 
    x0 = get4(s); s += 4;
    y0 = get4(s); s += 4;
  } 
    else
  { 
    x0 = y0 = 0;
  } 
  
    if(has_wh)
  {
    w = get4(s); s += 4;
    h = get4(s); s += 4;
  } 
    else
  { 
    w = cur_composition_layer_w;
    h = cur_composition_layer_h;
  } 
    if(has_life)
  { 
    life = get4(s); s += 4;
    persist = (life>>31)&1;
    life = (life>>31)&0;
    next_use = get4(s); s += 4;
  } 
    else
  { 
    persist = 1;
    life = 0; 
    next_use = 0;
  } 
    if(has_crop)
  { 
    xc = get4(s); s += 4;
    yc = get4(s); s += 4;
    wc = get4(s); s += 4;
    hc = get4(s); s += 4;
  } 
    else
  { 
    xc = yc = 0;
    wc = cur_composition_layer_w;
    hc = cur_composition_layer_h;
  }

/*-------------
fprintf(stderr,"inst[%d]\n\tx0(%d) y0(%d) w(%d) h(%d)\n"
"\tlife(%d) persist(%d) next-use(%d)\n"
"\txc(%d) yc(%d) wc(%d) hc(%d)\n",i,x0,y0,w,h,life,persist,next_use,
 xc,yc,wc,hc);
---------------*/
	snprintf(item_name, MAX_ITEMBUF, "%s/inst[%d] x0(%d) y0(%d) w(%d) h(%d) "
	 "life(%d) persist(%d) next-use(%d) "
	 "xc(%d) yc(%d) wc(%d) hc(%d)",name_buf,i,x0,y0,w,h,life,persist,
	 next_use,xc,yc,wc,hc);
	add_sub_item();

   }
	return s;
	
}/* read_inst() */

static unsigned char *read_copt(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned int h, w; unsigned short loop;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Composition Options box",name_buf);
	add_sub_item();

	h = get4(s); s += 4;
	w = get4(s); s += 4;
	loop = s[0]; ++s;

	snprintf(item_name, MAX_ITEMBUF, "%s/height(%d) width(%d) loop(%d)",
	 name_buf,h,w,loop);
	add_sub_item();

	assert(s == box_end);
	return box_end;
}/* read_copt() */

static unsigned char *read_comp(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep;
	char name_buf[MAX_ITEMBUF];
	int pos = -1;

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Composition box",name_buf);
	add_sub_item();

	box_len = read_boxheader(s, dend, &hstep);

	if(memcmp(box_name, "copt", 4) == 0)
   {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] copt", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_copt(s + hstep, s + box_len, dend, item_name);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);
   }
	if(memcmp(box_name, "inst", 4) == 0)
   {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] inst", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_inst(s + hstep, s + box_len, dend, item_name);

	sup->close();
   }

	return box_end;
}/* read_comp() */

static unsigned char *read_chck(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
/* Digital Signatur Box */
	uint64_t off, len;
	unsigned short styp, ptyp;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Digital Signature box",name_buf);
	add_sub_item();

	off = len = 0;
	styp = s[0]; ++s;
	ptyp = s[0]; ++s;

	if(ptyp == 1)
   {
	off = get8(s); s += 8;
	len = get8(s); s += 8;
   }

	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/styp(%u) ptyp(%u) off(%"PRIu64") len(%"PRIu64") data size[%lu]",
	 name_buf,styp,ptyp,off,len,(unsigned long)(box_end-s));
	add_sub_item();

	assert(box_end == s);
	return box_end;
	
}/* read_chck() */

static unsigned char *read_gtso(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Graphics Technology Standard Output box",name_buf);
	add_sub_item();

/*---------
OUTP: This field shall be a valid Output ICC profile 
	  as defined by the ICC Profile format specification
      ICC-1. Version information is embedded within the 
	  profile itself. Applications that only support
      specific versions of the ICC Profile Format Specifications 
	  can extract the version number from bytes 8-11 of the profile
	  (bytes 8­11 of the contents of the Output ICC Profile box).
------------*/

	
	return box_end;
}/* read_gtso() */

static unsigned char *read_drep(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep;
	char name_buf[MAX_ITEMBUF];
	int pos = -1;

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Desired Reproductions box",name_buf);
	add_sub_item();

	box_len = read_boxheader(s, dend, &hstep);

	if(memcmp(box_name, "gtso", 4) == 0)
   {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] gtso", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_gtso(s + hstep, s + box_len, dend, item_name);

	sup->close();
   }
	return box_end;
}/* read_drep() */

static unsigned char *read_bfil(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned int id, n;
	unsigned char buf[13];
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Binary Filter box",name_buf);
	add_sub_item();

	buf[12] = 0;
	id = get4(s); s += 4;
	memcpy(buf, s, 12); s += 12;
/*---------------------------------------------------------------
 ID     |  UUID-suffix
--------|-----------------------------
EC340B04-74C5-11D4-A729-879EA3548F0E
           Compressed with GZIP. 
           The contents of the DATA field have been compressed using the
           DEFLATE algorithm (as specified in RFC 1951). 
           The compressed data is stored in the binary structure
           defined by the GZIP file format, as specified in RFC 1952.

EC340B04-74C5-11D4-A729-879EA3548F0F 
           Encrypted using DES. 
           The contents of the DATA field has been encrypted using DES as
           defined in ISO 10126-2.


           All other values reserved.


---------------------------------------------------------------*/
	n = (unsigned int)(box_end - s);

	snprintf(item_name, MAX_ITEMBUF, "%s/bfil(%u%s) data[%u]",name_buf,id,buf,n);
	add_sub_item();

	
	return box_end;
}/* read_bfil() */

static unsigned char *read_nlst(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned int n, i, an;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Number List box",name_buf);
	add_sub_item();

	n = (unsigned int)((box_end - s)/4);
	for(i = 0; i < n; ++i)
   {
	an = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/nlst[%u]%u",name_buf,i,an);
	add_sub_item();

   }
	assert(s == box_end);
	return box_end;

}/* read_nlst() */

static unsigned char *read_asoc(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep;
	char name_buf[MAX_ITEMBUF];
	int pos = -1;

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Association box",name_buf);
	add_sub_item();

	while(s < box_end)
   {
	box_len = read_boxheader(s, dend, &hstep);

	if(memcmp(box_name, "nlst", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] nlst", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_nlst(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }
	if(memcmp(box_name, "lbl ", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] lbl ", name_buf, ++pos);
	sup = tree->add(item_name);

	s = read_lbl(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }
	if(memcmp(box_name, "asoc", 4) == 0)
  {
	Fl_Tree_Item *sup;
	++pos;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] asoc", name_buf, pos);

	sup = tree->add(item_name);
	s = read_asoc(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }
	if(memcmp(box_name, "xml ", 4) == 0
	|| memcmp(box_name, "XML ", 4) == 0)
  {
	Fl_Tree_Item *sup, *sub;
	char *buf = NULL;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] xml ", name_buf, ++pos);
	sup = tree->add(item_name);

    snprintf(item_name, MAX_ITEMBUF, "%s/xml1", item_name);
    sub = tree->add(item_name);

	s = read_xml(s + hstep, s + box_len, dend, &buf);

	sub->label(buf);

	sup->close();
	free(buf);
	continue;
  }
  {
     Fl_Tree_Item *sup;
    char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "%s/[%03d] %s", 
	 name_buf, ++pos, (char*)box_name);
    sup = tree->add(iname);

    snprintf(item_name, MAX_ITEMBUF, "%s/Not handled. Skipped.", iname);
    add_sub_item();

    sup->close();

  }
	s = box_end;
   }
	assert(box_end == s);
	return box_end;

}/* read_asoc() */

static unsigned char *read_mdat(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Media Data box",name_buf);
    add_sub_item();

	return box_end;
}//read_mdat()

static unsigned char *read_ftbl(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
    uint64_t box_len;
    unsigned int hstep;
    char name_buf[MAX_ITEMBUF];
    int pos = -1;

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Fragment Table box",name_buf);
    add_sub_item();

	box_len = read_boxheader(s, dend, &hstep);

    if(memcmp(box_name, "flst", 4) != 0)
   {

LOG("flst missing. STOP.");

    return dend;
   }
   {
    Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] flst", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_flst(s + hstep, s + box_len, dend, item_name);

	sup->close();
   }
	return box_end;
}//read_ftbl()

static unsigned char *read_rreq(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
    unsigned int fuam, dcm, vm;
    unsigned short ml, nsf, i, sf, sm, nvf;
    unsigned char uuid[17];
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Data Reference box",name_buf);
	add_sub_item();

    ml = s[0]; ++s;
    fuam = get_size(s, ml); s += ml;
    dcm = get_size(s, ml); s += ml;
    nsf = get2(s); s += 2;

	snprintf(item_name, MAX_ITEMBUF, "%s/ml(%u) fuam(%u) dcm(%u) nsf(%hu)",
	 name_buf, ml,fuam, dcm, nsf);
	add_sub_item();

    for(i = 0; i < nsf; ++i)
   {
    sf = get2(s); s += 2;
    sm = get_size(s, ml); s += ml;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%hd] sf(%hu) sm(%hu)",
	 name_buf,i,sf,sm);
	add_sub_item();
   }

    nvf = get2(s); s += 2;
	snprintf(item_name, MAX_ITEMBUF, "%s/nvf(%hu)",name_buf, nvf);
	add_sub_item();

    uuid[16] = 0;
    for(i = 0; i < nvf; ++i)
   {
    memcpy(uuid, s, 16); s += 16;

    vm = get_size(s, ml); s += ml;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%hu] vm(%u)",name_buf,i,vm);
	add_sub_item();
   }

    return box_end;

}//read_rreq()

int read_jpx(unsigned char *src, unsigned char *parse_start,
	unsigned char *dend)
{
	uint64_t box_len;
	unsigned char *box_end, *s;
	unsigned int hstep;
	int pos = -1;

	jpx_start = src;
	s = parse_start;

	for(;;)
   {
	box_len = read_boxheader(s, dend, &hstep);

	if( box_len == 0) { s = dend; break; }

	if(memcmp(box_name, "ftyp", 4) != 0)
  {

	LOG("ftyp missing. STOP.");

	return 0;
  }
	s += box_len;

	box_len = read_boxheader(s, dend, &hstep);

	if(memcmp(box_name, "rreq", 4) != 0)
  {
	LOG("rreq missing. STOP.");

	return 0;
  }
	break;
   }/* for(;;) */

	s = parse_start;

	while(s < dend)
   {
	box_len = read_boxheader(s, dend, &hstep);

    if((uint64_t)(dend - s) < box_len)
     box_end = dend;
    else
     box_end = s + (ptrdiff_t)box_len;

	if(memcmp(box_name, "ftyp", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] ftyp", ++pos);
	sup = tree->add(item_name);

	s = read_ftyp(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "rreq", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] rreq", ++pos);
	sup = tree->add(item_name);

	s = read_rreq(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "jp2h", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] jp2h", ++pos);
	sup = tree->add(item_name);

	s = read_jp2h(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }
	
	if(memcmp(box_name, "jpch", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] jpch", ++pos);
	sup = tree->add(item_name);

	s = read_jpch(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "jplh", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] jplh", ++pos);
	sup = tree->add(item_name);

	s = read_jplh(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "dtbl", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] dtbl", ++pos);
	sup = tree->add(item_name);

	s = read_dtbl(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "ftbl", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] ftbl", ++pos);
	sup = tree->add(item_name);

	s = read_ftbl(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "jp2c", 4) == 0)
  {
	char iname[MAX_ITEMBUF+2];
	Fl_Tree_Item *sup;

	snprintf(iname, MAX_ITEMBUF, "[%03d] jp2c", ++pos);
	sup = tree->add(iname);

	s = read_jp2c(s + hstep, s + box_len, dend, iname);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "comp", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] comp", ++pos);
	sup = tree->add(item_name);

	s = read_comp(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "drep", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] drep", ++pos);
	sup = tree->add(item_name);

	s = read_drep(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "roid", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] roid", ++pos);
	sup = tree->add(item_name);

	s = read_roid(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "cref", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] cref", ++pos);
	sup = tree->add(item_name);

	s = read_cref(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "asoc", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] asoc", ++pos);
	sup = tree->add(item_name);

	s = read_asoc(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "bfil", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] bfil", ++pos);
	sup = tree->add(item_name);

	s = read_bfil(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "chck", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] chck", ++pos);
	sup = tree->add(item_name);

	s = read_chck(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "xml ", 4) == 0
	|| memcmp(box_name, "XML ", 4) == 0)
  {
    Fl_Tree_Item *sup, *sub;
    char *buf = NULL;

    snprintf(item_name, MAX_ITEMBUF, "[%03d] xml ", ++pos);
    sup = tree->add(item_name);

    snprintf(item_name, MAX_ITEMBUF, "%s/xml1", item_name);
    sub = tree->add(item_name);

    s = read_xml(s + hstep, s + box_len, dend, &buf);

    sub->label(buf);

    sup->close();
	free(buf);
	continue;
  }

	if(memcmp(box_name, "uuid", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] uuid", ++pos);
	sup = tree->add(item_name);

	s = read_uuid(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "uinf", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] uinf", ++pos);
	sup = tree->add(item_name);

	s = read_uinf(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "ftbl", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] ftbl", ++pos);
	sup = tree->add(item_name);

	s = read_ftbl(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "mdat", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] mdat", ++pos);
	sup = tree->add(item_name);

	s = read_mdat(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

	if(memcmp(box_name, "jp2i", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] jp2i", ++pos);
	sup = tree->add(item_name);

	s = read_jp2i(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }

  {
     Fl_Tree_Item *sup;
    char iname[MAX_ITEMBUF];

    snprintf(iname, MAX_ITEMBUF, "[%03d] %s", ++pos, (char*)box_name);
    sup = tree->add(iname);

    snprintf(item_name, MAX_ITEMBUF, "%s/Not handled. Skipped.", iname);
    add_sub_item();

    sup->close();

  }
	s = box_end;	

   }//while(s < dend)

	return 1;

}//read_jpx()
