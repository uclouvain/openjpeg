/*
 * Part 6 Compound image file format, Mixed raster content JPM
 *
 * Problem: Shall 'scal' and 'jp2h' be shown even if 'no_codestream == 1'?
 *          There should not be scal/jp2h boxes. But could be.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
#endif /* _WIN32 */

#include <FL/Fl.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Pixmap.H>
#include <FL/fl_utf8.h>

#include "opj_inttypes.h"
#include "opj_stdint.h"

#include "viewerdefs.hh"
#include "tree.hh"


//#define DEBUG_JPM
//#define DEBUG_JPM_JPX

//--------- FORWARD
static unsigned char *jpm_start;

static int has_thumbnail_jp2h;
static uint64_t thumbnail_jp2hpos; 
static uint64_t thumbnail_jp2hlen;
static unsigned int thumbnail_jp2hstep; 

static int has_thumbnail_jp2c;
static uint64_t thumbnail_jp2cpos;
static uint64_t thumbnail_jp2clen;
static unsigned int thumbnail_jp2cstep;

static unsigned char *read_lbl(unsigned char *s, unsigned char *box_end,
	const char *name_src);
static unsigned char *read_phdr(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src, unsigned short *out_nrid);

static unsigned char *read_lobj(unsigned short cur_id, unsigned short max_id,
    unsigned char *s, unsigned char *box_end, unsigned char *dend,
    const char *name_src);

static unsigned char *read_flst(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src);
static unsigned char *read_ppcl(unsigned char *s, unsigned char *box_end,
    const char *name_src);


#define LOG(s) fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,s)

static const char *otype_values[]={
"The object contains the mask of a layout object",
"The object contains the image of a layout object",
"The object contains the image and mask of a layout object",
"Reserved value"
};

static const char *lhdr_styles[]={
"Separate objects for image and mask components",
"Single object for image and mask components",
"Single object for image only components",
"Single object for mask only components",
"Vendor specific layout object"// 255
};

static const char *pcolor_values[]={
"Page is transparent",
"Page is white",
"Page is black",
"Page color is specified in Base Color box",
"Reserved for ISO use"
};

static const char *or_values[]={
"Orientation not specified",
"Rotate 0 degr clockwise for a right reading image",
"Rotate 90 degr clockwise for a right reading image",
"Rotate 180 degr clockwise for a right reading image",
"Rotate 270 degr clockwise for a right reading image",
"Reserved for ISO use"
};

//--------- END FORWARD

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

static char item_name[MAX_ITEMBUF+2];

static void add_sub_item()
{
    Fl_Tree_Item *sub = tree->add(item_name);
    sub->usericon(&L_documentpixmap);
}

static const char *pagt_flag_values(unsigned short fl)
{
	if(fl == 0) return "'Offset to PCOL box'";
    if(fl & 1) return "'Offset to PAGE box'";
    if(fl & 3) return "'Offset to PAGE box containing thumbnail'";
    if(fl & 4) return "'Offset to auxPCOL box'";
    if(fl & 8) return "'Offset to PAGE or PCOL box containing metadata'";
	if(fl & 9) return "'Offset to PAGE box with meta'";
	if(fl & 12) return "'Offset to auxPCOL box with meta'";

    return "'Unknown'";
}

static unsigned char *read_sdat(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
	uint64_t len, l;
	unsigned short id;
    char name_buf[MAX_ITEMBUF];
	char data[MAX_ITEMBUF+1];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/SDAT box", name_buf);
    add_sub_item();
	
	id = get2(s); s += 2;
	len = (box_end - s);

    snprintf(item_name, MAX_ITEMBUF, "%s/id %hu", name_buf, id);
    add_sub_item();

	l = len; if(l > MAX_ITEMBUF) l = MAX_ITEMBUF;

	memcpy(data, s, l); data[l] = 0;
    snprintf(item_name, MAX_ITEMBUF, "%s/data[%lu](%s)", name_buf,
	 len,data);
    add_sub_item();

	return box_end;
}

static unsigned char *read_sref(unsigned char *s, unsigned char *box_end,
    const char *name_src)
{
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/SREF box", name_buf);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/id %hu", name_buf, get2(s));
    add_sub_item();
	s += 2;

	return box_end;
}

static unsigned char *read_mhdr(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
	uint64_t mpcoff;
	unsigned int np, mpclen;
	unsigned short p, sc, c0, c1;
	char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Compound Image Header box", name_buf);
    add_sub_item();

	np = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/nr_pages(%u)",name_buf,np);
	add_sub_item();

	p = (unsigned short)s[0]; ++s;
	snprintf(item_name, MAX_ITEMBUF, "%s/profile(%hu)",name_buf,p);
	add_sub_item();

	sc = (unsigned short)s[0]; ++s;
    snprintf(item_name, MAX_ITEMBUF, "%s/self-contained(%hu)",name_buf,sc);
    add_sub_item();

    mpcoff = get8(s); s += 8;
    mpclen = get4(s); s += 4;

    snprintf(item_name, MAX_ITEMBUF, "%s/mpcoff(%lu)",name_buf,mpcoff);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/mpclen(%u)",name_buf,mpclen);
    add_sub_item();

    c0 = s[0]; ++s; c1 = 0;
    if(c0&128) { c1 = s[0]; ++s; }

	snprintf(item_name, MAX_ITEMBUF, "%s/MaskCoder(%hu)",name_buf,
	 (c1<<8)|(c0&0xff));
	add_sub_item();

    c0 = s[0]; ++s; c1 = 0;
    if(c0&128) { c1 = s[0]; ++s; }

	snprintf(item_name, MAX_ITEMBUF, "%s/ImgCoder(%hu)",name_buf,
	 (c1<<8)|(c0&0xff));
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/ip_rights(%hu)",name_buf,
	 (unsigned short)s[0]);
    add_sub_item();


#ifdef DEBUG_JPM
fprintf(stderr,"EXIT read_mhdr (s - box_end) ==> %ld\n",
(long)(s - box_end));
#endif

	return box_end;
}// read_mhdr()

unsigned char *read_dtbl(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned char *start;
	uint64_t box_len;
	unsigned int flag, src_len, dst_len, hstep;
	unsigned short i, n, vers;
    char name_buf[MAX_ITEMBUF];
	char dst[2048];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Data Reference box", name_buf);
    add_sub_item();

	n = get2(s); s += 2;
	snprintf(item_name, MAX_ITEMBUF, "%s/nr_ref(%u)",name_buf,n);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF,
     "%s/DR[0]version(0) flag(0)\n\t      loc[0]''",name_buf);
    add_sub_item();

	for(i = 1; i <= n; ++i)
   {
	start = s;
	box_len = read_boxheader(s, dend, &hstep);

	if(memcmp(box_name, "url ", 4) == 0)
  {
	s += hstep;

	vers = (unsigned short)s[0]; ++s;
	flag = get_size(s, 3); s += 3;
	src_len = (unsigned int)strlen((char*)s);
	dst_len = fl_utf8toa((char*)s, src_len, dst, 2047);

	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/DR[%u]version(%u) flag(%u) loc[%u]'%s'",
	 name_buf,i,vers,flag,dst_len,dst);
	add_sub_item();
  }
	s = start + box_len;
   }
#ifdef DEBUG_JPM_JPX
fprintf(stderr,"\tEXIT read_dtbl (s - box_end) ==> %ld\n",(long)(s - box_end));
#endif
	return box_end;
}//read_dtbl()

static unsigned char *read_pagt(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
	unsigned short dr, flag;
	unsigned int i, len, ne;
	uint64_t off;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Page Table box", name_buf);
    add_sub_item();

	ne = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/ne(%u)",name_buf,ne);
	add_sub_item();

	for(i = 0; i < ne; ++i)
   {
	off = get8(s); s += 8;
	len = get4(s); s += 4;
	dr = get2(s); s += 2;
	flag = (unsigned short)s[0]; ++s;

	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/[%2u] off(%lu) len(%u)", name_buf,i,off,len);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/[%2u] dr(%hu)",name_buf,i,dr);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/[%2u] flag[%hu]%s", name_buf,i,flag,pagt_flag_values(flag));
	add_sub_item();
   }
#ifdef DEBUG_JPM
fprintf(stderr,"\tEXIT read_pagt(s - box_end) ==> %ld\n",(long)(s - box_end));
#endif
	return box_end;

}/* read_pagt() */

static unsigned char *read_pcol(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src, uint64_t offset,
	uint64_t len)
{
	uint64_t box_len;
	unsigned int hstep, has_pagt = 0;
    char name_buf[MAX_ITEMBUF];
	char iname[MAX_ITEMBUF];
	int pos = -1;

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Page Collection box", name_buf);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/offset(%ld) len(%ld)", 
	 name_buf, offset, len);
    add_sub_item();

	while(s < box_end)
   {
	box_len = read_boxheader(s, dend, &hstep);

    if(memcmp(box_name, "ppcl", 4) == 0)
  {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] ppcl",name_buf,++pos);
    sup = tree->add(item_name);
//pcol
    s = read_ppcl(s + hstep, s + box_len, item_name);

    sup->close();
    continue;
  }

	if(memcmp(box_name, "lbl ", 4) == 0)
  {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] lbl ",name_buf,++pos);

	sup = tree->add(item_name);

	s = read_lbl(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }
    if(memcmp(box_name, "xml ", 4) == 0
    || memcmp(box_name, "XML ", 4) == 0)
  {
    Fl_Tree_Item *sup, *sub;
    char *buf = NULL;

	snprintf(iname, MAX_ITEMBUF, "%s/[%03d] xml", name_buf, ++pos);
    sup = tree->add(iname);

    snprintf(item_name, MAX_ITEMBUF, "%s/xml1", iname);
    sub = tree->add(item_name);

    s = read_xml(s + hstep, s + box_len, dend, &buf);

    sub->label(buf);

    sup->close();
    free(buf); --pos;
    continue;
  }
    if(memcmp(box_name, "uuid", 4) == 0)
  {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] uuid", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_uuid(s + hstep, s + box_len, dend, item_name);

    sup->close();
    continue;
  }
    if(memcmp(box_name, "uinf", 4) == 0)
  {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] uinf", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_uinf(s + hstep, s + box_len, dend, item_name);

    sup->close();
    continue;
  }
//pcol
	if(memcmp(box_name, "pagt", 4) == 0)
  {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] pagt",name_buf,++pos);

	sup = tree->add(item_name);

	s = read_pagt(s + hstep, s + box_len, item_name);

	sup->close();
	has_pagt = 1;
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

#ifdef DEBUG_JPM
fprintf(stderr,"read_pcol: UNKNOWN BOX name(%s) len(%lu)\n",
(char*)box_name,box_len);
#endif

	s += box_len;

   }

	if(has_pagt == 0) 
   {
LOG("pagt missing. STOP.");
	return dend;
   }
	
	return box_end;
}//read_pcol()

static unsigned char *read_bcvl(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
    unsigned short i, nc, bpc, value, dr, depth, sign;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Base Color Value box", name_buf);
    add_sub_item();


    nc = get2(s); s += 2;
    bpc = (unsigned short)s[0]; ++s;
	depth = (bpc & 0x7f) + 1;
	sign = (bpc & 0x80)?1:0;

    snprintf(item_name, MAX_ITEMBUF, "%s/numcomps(%d)", name_buf, nc);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/bpc(%d)", name_buf, bpc);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/depth(%d)", name_buf, depth);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/signed(%d)", name_buf, sign);
    add_sub_item();

    for(i = 0; i < nc; ++i)
   {
    value = get2(s); s += 2;
    dr = get2(s); s += 2;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%hu]value(%hu) dr(%hu)",
	 name_buf,i,value,dr);
    add_sub_item();

   }
#ifdef DEBUG_JPM
fprintf(stderr,"\n\t\tEXIT read_bcvl (s - box_end) ==> %ld\n",
(long)(s - box_end));
#endif

    return box_end;
}

//no_codestream == 1
static unsigned char *read_bclr(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep;
	unsigned short bcvl_found = 0;;
    char name_buf[MAX_ITEMBUF];
	int pos = -1;

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Base Color box", name_buf);
    add_sub_item();

	while(s < box_end)
   {
    box_len = read_boxheader(s, dend, &hstep);

    if(memcmp(box_name, "bcvl", 4) == 0)
  {
    Fl_Tree_Item *sup;
    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] bcvl",name_buf,++pos);

    sup = tree->add(item_name);

    s = read_bcvl(s + hstep, s + box_len, item_name);

    sup->close();
    ++bcvl_found;
    continue;
  }
    if(memcmp(box_name, "colr", 4) == 0)
  {
    Fl_Tree_Item *sup;
    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] colr",name_buf,++pos);

    sup = tree->add(item_name);

    s = read_colr(s + hstep, s + box_len, item_name);

    sup->close();
	continue;
  }
    if(memcmp(box_name, "bpcc", 4) == 0)
  {
    Fl_Tree_Item *sup;
    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] bpcc",name_buf,++pos);

    sup = tree->add(item_name);

    s = read_bpcc(s + hstep, s + box_len, item_name);

    sup->close();
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

#ifdef DEBUG_JPM
fprintf(stderr,"read_pcol: UNKNOWN BOX name(%s) len(%lu)\n",
(char*)box_name,box_len);
#endif

    s += box_len;

   }//while(s < box_end

    if(bcvl_found == 0)
   {
LOG("bcvl not found. STOP.");
    return dend;
   }
#ifdef DEBUG_JPM
fprintf(stderr,"\t  EXIT read_bclr (s - box_end) ==> %ld\n",
(long)(s-box_end));
#endif

    return box_end;

}/* read_bclr() */

static unsigned char *read_ppcl(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
    uint64_t ppcoff;
    unsigned int ppclen, pix;
    unsigned short ppcdr;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, 
	 "%s/Primary Page Collection Locator box", name_buf);
    add_sub_item();


    ppcoff = get8(s); s += 8;
    snprintf(item_name, MAX_ITEMBUF, "%s/ppcoff(%lu)",name_buf,ppcoff);
    add_sub_item();

    ppclen = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/ppclen(%u)",name_buf,ppclen);
    add_sub_item();

    ppcdr = get2(s); s += 2;
    snprintf(item_name, MAX_ITEMBUF, "%s/ppcdr(%hu)",name_buf,ppcdr);
    add_sub_item();

    pix = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/pix(%u)",name_buf,pix);
    add_sub_item();

    return box_end;

}//read_ppcl()

static unsigned char *read_htxb(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src)
{
    uint64_t box_len;
    unsigned int hstep;
    char name_buf[MAX_ITEMBUF];
	int pos = -1;

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Hidden Text Metadata box", name_buf);
    add_sub_item();

    while(s < box_end)
   {
    box_len = read_boxheader(s, dend, &hstep);

    if(memcmp(box_name, "lbl ", 4) == 0)
  {
    Fl_Tree_Item *sup;
    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] lbl ",name_buf,++pos);

    sup = tree->add(item_name);

    s = read_lbl(s + hstep, s + box_len, item_name);

    sup->close();

    box_len = read_boxheader(s, dend, &hstep);

    if(memcmp(box_name, "xml ", 4) == 0
	|| memcmp(box_name, "XML ", 4) == 0)
 {
    Fl_Tree_Item *sup, *sub;
    char *buf = NULL;
	char iname[MAX_ITEMBUF];

    snprintf(iname, MAX_ITEMBUF, "%s/[%03d] xml", name_buf, ++pos);
    sup = tree->add(iname);

    snprintf(item_name, MAX_ITEMBUF, "%s/xml1", iname);
    sub = tree->add(item_name);

    s = read_xml(s + hstep, s + box_len, dend, &buf);

    sub->label(buf);

    sup->close();
    free(buf); --pos;
	continue;
 }

    if(memcmp(box_name, "uuid", 4) == 0)
 {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] uuid", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_uuid(s + hstep, s + box_len, dend, item_name);

    sup->close();
    continue;
 }

  }	//if(memcmp(box_name, "lbl ", 4) == 0)

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

#ifdef DEBUG_JPM
fprintf(stderr,"read_pcol: UNKNOWN BOX name(%s) len(%lu)\n",
(char*)box_name,box_len);
#endif

    s += box_len;

   }//while(s < box_end)

    return box_end;

}// read_htxb()

static unsigned char *read_phtx(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src)
{
    uint64_t box_len;
    unsigned int hstep;
    unsigned char rtyp[5];
	char name_buf[MAX_ITEMBUF];
	int pos = -1;

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/HTX Reference box", name_buf);
    add_sub_item();

    memcpy(rtyp, s, 4); s += 4; rtyp[4] = 0;
    snprintf(item_name, MAX_ITEMBUF, "%s/phtx_rtyp(%s)",name_buf,rtyp);
    add_sub_item();

    while(s < box_end)
   {
    box_len = read_boxheader(s, dend, &hstep);

    if(memcmp(box_name, "flst", 4) == 0)
  {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] flst",  name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_flst(s + hstep, s + box_len, dend, item_name);

    sup->close();
    continue;
  }

    if(memcmp(box_name, "lbl ", 4) == 0)
  {
    Fl_Tree_Item *sup;
    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] lbl ",name_buf,++pos);

    sup = tree->add(item_name);

    s = read_lbl(s + hstep, s + box_len, item_name);

    sup->close();
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

#ifdef DEBUG_JPM
fprintf(stderr,"read_pcol: UNKNOWN BOX name(%s) len(%lu)\n",
(char*)box_name,box_len);
#endif

    s += box_len;

   }

    return box_end;

}// read_phtx()

static unsigned char *read_page(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src, uint64_t offset,
	uint64_t len)
{
	uint64_t box_len;
	unsigned int hstep;
	unsigned short nr_id = 0, cur_id = 0, has_phdr = 0, has_res = 0;
    char name_buf[MAX_ITEMBUF];
	char iname[MAX_ITEMBUF+2];
	int pos = -1;

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Page box", name_buf);
    add_sub_item();
    snprintf(item_name, MAX_ITEMBUF, "%s/offset(%ld) len(%ld)", 
	 name_buf, offset, len);
    add_sub_item();


	while(s < box_end)
   {
	box_len = read_boxheader(s, dend, &hstep);

	if(memcmp(box_name, "phdr", 4) == 0)
  {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] phdr",name_buf,++pos);
	sup = tree->add(item_name);

	has_phdr = 1;
	nr_id = 0;

	s = read_phdr(s + hstep, s + box_len, dend, item_name, &nr_id);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "res ", 4) == 0)
  {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] res ",name_buf,++pos);
	sup = tree->add(item_name);

	has_res = 1;

	s = read_res(s + hstep, s + box_len, dend, 
		(unsigned char*)item_name);

	sup->close();
	continue;
  }
	if(memcmp(box_name, "lbl ", 4) == 0)
  {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] lbl ",name_buf,++pos);
	sup = tree->add(item_name);

	s = read_lbl(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }
	if(memcmp(box_name, "bclr", 4) == 0)
  {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] bclr",name_buf,++pos);
	sup = tree->add(item_name);

	s = read_bclr(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "xml ", 4) == 0
	|| memcmp(box_name, "XML ", 4) == 0)
  {
    Fl_Tree_Item *sup, *sub;
    char *buf = NULL;

    snprintf(iname, MAX_ITEMBUF, "%s/[%03d] xml", name_buf, ++pos);
    sup = tree->add(iname);

    snprintf(item_name, MAX_ITEMBUF, "%s/xml1", iname);
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

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] uuid", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_uuid(s + hstep, s + box_len, dend, item_name);

    sup->close();
	continue;
  }
	if(memcmp(box_name, "uinf", 4) == 0)
  {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] uinf", name_buf, ++pos);
    sup = tree->add(item_name);

	s = read_uinf(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "lobj", 4) == 0)
  {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] lobj",name_buf,++pos);
	sup = tree->add(item_name);

	s = read_lobj(cur_id, nr_id, s + hstep, s + box_len, dend, item_name);

	++cur_id;
	sup->close();
	continue;
  }

	if(memcmp(box_name, "ppcl", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] ppcl",name_buf,++pos);
	sup = tree->add(item_name);
//page
	s = read_ppcl(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }

    if(memcmp(box_name, "htxb", 4) == 0)
  {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] htxb", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_htxb(s + hstep, s + box_len, dend, item_name);

    sup->close();
    continue;
  }

    if(memcmp(box_name, "phtx", 4) == 0)
  {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] phtx", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_phtx(s + hstep, s + box_len, dend, item_name);

    sup->close();
    continue;
  }

  {
     Fl_Tree_Item *sup;

    snprintf(iname, MAX_ITEMBUF, "%s/[%03d] %s", 
	 name_buf, ++pos, (char*)box_name);
    sup = tree->add(iname);

    snprintf(item_name, MAX_ITEMBUF, "%s/Not handled. Skipped.", iname);
    add_sub_item();

    sup->close();

  }
#ifdef DEBUG_JPM
fprintf(stderr,"read_page: UNKNOWN BOX FOUND name(%s) len(%lu)\n",
(char*)box_name,box_len);
#endif

	s += box_len;
   }

	if(has_phdr == 0)
   {
LOG("phdr missing. STOP.");
	return dend;
   }
	if(has_res == 0)
   {
LOG("res missing. STOP.");
	return dend;
   }
#ifdef DEBUG_JPM
fprintf(stderr,"EXIT read_page (s - box_end) ==> %ld\n\n",(long)(s - box_end));
#endif

	return box_end;

}/* read_page() */

static unsigned char *read_phdr(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src, unsigned short *out_nrid)
{
	unsigned int pheight, pwidth;
	unsigned short nr_lobj, orient, pcolor, v;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Page Header box", name_buf);
    add_sub_item();

	nr_lobj = get2(s); s += 2; *out_nrid = nr_lobj;
	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/nr_layout_objects(%u)",name_buf,nr_lobj);
	add_sub_item();

	pheight = get4(s); s += 4;
	pwidth = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/pwidth(%u) pheight(%u)",name_buf,
	 pwidth,pheight);
	add_sub_item();

	orient = get2(s); s += 2;

	if(orient > 4) v = 5; else v = orient;

	snprintf(item_name, MAX_ITEMBUF, "%s/orientation[%u]'%s'",
	 name_buf,orient,or_values[v]);
	add_sub_item();

	pcolor = get2(s); s += 2;

	if(pcolor > 3) v = 4; else v = pcolor;

	snprintf(item_name, MAX_ITEMBUF, "%s/color[%u]'%s'",
	 name_buf,pcolor,pcolor_values[v]);
	add_sub_item();

#ifdef DEBUG_JPM
fprintf(stderr,"\tEXIT read_phdr (s - box_end) ==> %ld\n",(long)(s - box_end));
#endif
	return box_end;
}//read_phdr()

static unsigned char *read_ohdr(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src, 
	unsigned short *out_no_codestream)
{
	uint64_t off;
	unsigned int voff, hoff, len;
	unsigned short otype, no_cs, dr, o;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Object Header box", name_buf);
    add_sub_item();

	otype = (unsigned short)s[0]; ++s;
	no_cs = (unsigned short)s[0]; ++s;

	if(otype > 2) o = 3; else o = otype;
	snprintf(item_name, MAX_ITEMBUF, "%s/otype(%hu)'%s'",
	 name_buf,otype,otype_values[o]);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/no_codestream(%hu)",
     name_buf,no_cs);
    add_sub_item();

	*out_no_codestream = no_cs;

	if(no_cs == 0)
   {
	voff = get4(s); s += 4;
	hoff = get4(s); s += 4;
	off = get8(s); s += 8;
	len = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/hoff(%u) voff(%u)",
	 name_buf,hoff,voff);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/off(%lu) len(%u)",
     name_buf,off,len);
    add_sub_item();

	dr = get2(s); s += 2;

	snprintf(item_name, MAX_ITEMBUF, "%s/data_reference(%hu)",name_buf,
	 dr);
	add_sub_item();

   }

#ifdef DEBUG_JPM
fprintf(stderr,"\tEXIT read_ohdr(s - box_end) ==> %ld\n",(long)(s - box_end));
#endif

	return box_end;
}/* read_ohdr() */

static unsigned char *read_scal(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned short vrn, vrd, hrn, hrd;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Object Scale box", name_buf);
    add_sub_item();

	vrn = get2(s); s += 2;
	vrd = get2(s); s += 2;
	hrn = get2(s); s += 2;
	hrd = get2(s); s += 2;

	snprintf(item_name, MAX_ITEMBUF, "%s/hrn(%hu) hrd(%hu)",name_buf,
	 hrn,hrd);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/vrn(%hu) vrd(%hu)",name_buf,
	 vrn,vrd);
    add_sub_item();

#ifdef DEBUG_JPM
fprintf(stderr,"\tEXIT read_scal (s - box_end) ==> %ld\n",(long)(s - box_end));
#endif

	return box_end;
}//read_scal()

static unsigned char *read_objc(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep;
	unsigned short has_ohdr = 0, no_codestream = 1;
    char name_buf[MAX_ITEMBUF];
	char iname[MAX_ITEMBUF];
	int pos = -1;

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Object box", name_buf);
    add_sub_item();

	while(s < box_end)
   {
	box_len = read_boxheader(s, dend, &hstep);

	if(memcmp(box_name, "ohdr", 4) == 0)
  {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] ohdr",name_buf,++pos);
	sup = tree->add(item_name);

	has_ohdr = 1;
	s = read_ohdr(s + hstep, s + box_len, dend, item_name, &no_codestream);

	sup->close();
	continue;
  }
	if(no_codestream == 1)
  {
	if(memcmp(box_name, "bclr", 4) == 0)
 {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] bclr",name_buf,++pos);
	sup = tree->add(item_name);

	s = read_bclr(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
 }
  }
    if(memcmp(box_name, "xml ", 4) == 0
    || memcmp(box_name, "XML ", 4) == 0)
  {
    Fl_Tree_Item *sup, *sub;
    char *buf = NULL;

	snprintf(iname, MAX_ITEMBUF, "%s/[%03d] xml", name_buf, ++pos);
    sup = tree->add(iname);

    snprintf(item_name, MAX_ITEMBUF, "%s/xml1", iname);
    sub = tree->add(item_name);

    s = read_xml(s + hstep, s + box_len, dend, &buf);

    sub->label(buf);

    sup->close();
    free(buf); --pos;
    continue;
  }
    if(memcmp(box_name, "uuid", 4) == 0)
  {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] uuid", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_uuid(s + hstep, s + box_len, dend, item_name);

    sup->close();
    continue;
  }
    if(memcmp(box_name, "uinf", 4) == 0)
  {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] uinf", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_uinf(s + hstep, s + box_len, dend, item_name);

    sup->close();
    continue;
  }

	if(memcmp(box_name, "scal", 4) == 0)
  {
	if(no_codestream == 0)//Mandatory
 {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] scal",name_buf,++pos);
	sup = tree->add(item_name);

	s = read_scal(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
 }
	s += box_len;
	continue;
  }

	if(memcmp(box_name, "jp2h", 4) == 0)
  {
	if(no_codestream == 0)//Mandatory
 {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] jp2h",name_buf,++pos);
	sup = tree->add(item_name);

	s = read_jp2h(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
 }
	s += box_len;
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

#ifdef DEBUG_JPM
fprintf(stderr,"read_objc: UNKNOWN BOX FOUND name(%s) len(%lu)\n",
(char*)box_name,box_len);
#endif

    s += box_len;
   }
	if(has_ohdr == 0)
   {
LOG("ohdr missing. STOP.");
	return dend;
   }
#ifdef DEBUG_JPM
fprintf(stderr,"\tEXIT read_objc (s - box_end) ==> %ld\n",(long)(s - box_end));
#endif
	return box_end;

}/* read_objc() */

static unsigned char *read_lhdr(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
	unsigned int h, w, voff, hoff;
	unsigned short ID, style, v;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Layout Object Header box",
	 name_buf);
    add_sub_item();

	ID = get2(s); s += 2;
	h = get4(s); s += 4;
	w = get4(s); s += 4;
	voff = get4(s); s += 4;
	hoff = get4(s); s += 4;
	style = (unsigned short)s[0]; ++s;

	if(style == 255) v = 4; else v = style;

	snprintf(item_name, MAX_ITEMBUF, "%s/ID(%hu)",name_buf,ID);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/style(%hu)'%s'",
     name_buf,style,lhdr_styles[v] );
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/hoff(%u) voff(%u)",
     name_buf,hoff,voff);
    add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/w(%u) h(%u)",
	 name_buf,w,h);
	add_sub_item();

#ifdef DEBUG_JPM
fprintf(stderr,"\tEXIT read_lhdr (s - box_end) ==> %ld\n",(long)(s - box_end));
#endif
	return box_end;
}/* read_lhdr() */

static unsigned char *read_lobj(unsigned short cur_id, unsigned short max_id,
	unsigned char *s, unsigned char *box_end, unsigned char *dend,
	const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep;
	short has_lhdr = 0, has_objc = 0;
    char name_buf[MAX_ITEMBUF];
	char iname[MAX_ITEMBUF+2];
	int pos = -1;

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Layout Object box", name_buf);
    add_sub_item();

	if(cur_id >= max_id) return box_end;

	while(s < box_end)
   {
	box_len = read_boxheader(s, dend, &hstep);

	if(memcmp(box_name, "lhdr", 4) == 0)
  {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] lhdr",name_buf,++pos);
	sup = tree->add(item_name);

	has_lhdr = 1;

	s = read_lhdr(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }  
    if(memcmp(box_name, "xml ", 4) == 0
    || memcmp(box_name, "XML ", 4) == 0)
  {
    Fl_Tree_Item *sup, *sub;
    char *buf = NULL;

	snprintf(iname, MAX_ITEMBUF, "%s/[%03d] xml", name_buf, ++pos);
    sup = tree->add(iname);

    snprintf(item_name, MAX_ITEMBUF, "%s/xml1", iname);
    sub = tree->add(item_name);

    s = read_xml(s + hstep, s + box_len, dend, &buf);

    sub->label(buf);

    sup->close();
    free(buf); --pos;
    continue;
  }
    if(memcmp(box_name, "uuid", 4) == 0)
  {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] uuid", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_uuid(s + hstep, s + box_len, dend, item_name);

    sup->close();
    continue;
  }
    if(memcmp(box_name, "uinf", 4) == 0)
  {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] uinf", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_uinf(s + hstep, s + box_len, dend, item_name);

    sup->close();
    continue;
  }

	if(memcmp(box_name, "objc", 4) == 0)
  {
	Fl_Tree_Item *sup;
	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] objc",name_buf,++pos);
	sup = tree->add(item_name);

	has_objc = 1;

	s = read_objc(s + hstep, s + box_len, dend, item_name);

	sup->close();
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

#ifdef DEBUG_JPM
fprintf(stderr,"read_lobj: UNKNOWN BOX FOUND name(%s) len(%lu)\n",
(char*)box_name,box_len);
#endif

    s += box_len;
   }
	if(has_objc == 0 || has_lhdr == 0)
   {
	fprintf(stderr,"%s:%d:\n\tFAILURE: objc(%d) or lhdr(%d) missing."
	 " STOP.\n",__FILE__,__LINE__,has_objc,has_lhdr);
	return dend;
   }


#ifdef DEBUG_JPM
fprintf(stderr,"\tEXIT read_lobj(s - box_end) ==> %ld\n",(long)(s - box_end));
#endif

	return box_end;

}/* read_lobj() */

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

    snprintf(item_name, MAX_ITEMBUF,"%s/[%2u] off(%lu) len(%u)",
     name_buf, i,(unsigned long)off,len);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF,"%s/       dr(%u)",name_buf,dr);
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
    snprintf(item_name, MAX_ITEMBUF, "%s/cref_rtyp(%d)",name_buf,rtyp);
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
	const char *name_src)
{
	size_t n;
	unsigned char *buf;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Label box", name_buf);
    add_sub_item();

	n = (size_t)(box_end - s); 
	buf = (unsigned char*)malloc(n+1); buf[n] = 0;

	if(buf)
   {
	memcpy(buf, s, n);

	snprintf(item_name, MAX_ITEMBUF, "%s/text(%s)",name_buf,buf);
	add_sub_item();

	free(buf);
   }
	return box_end;
}

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

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] flst",  name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_flst(s + hstep, s + box_len, dend, item_name);

    sup->close();
   }
    return box_end;
}//read_ftbl()

int read_jpm(unsigned char *src, unsigned char *parse_start,
	unsigned char *dend)
{
    uint64_t box_len;
    unsigned char *box_end, *s;
    unsigned int hstep;
    int pos = -1;
	short ftyp_found = 0, mhdr_found = 0, sdat_found = 0;

	jpm_start = src;
	s = parse_start;
	has_thumbnail_jp2h = has_thumbnail_jp2c = 0;

//--- HEAD

	while(s < dend)
   {
	box_len = read_boxheader(s, dend, &hstep);

	if( box_len == 0) return 0;

	if(memcmp(box_name, "ftyp", 4) == 0)//Mandatory
  {
	s += box_len;
	ftyp_found = 1;
	continue;
  }

    if(memcmp(box_name, "jp2h", 4) == 0)//Optional
  {
	if(ftyp_found == 0) return 0;

	has_thumbnail_jp2h = 1;
    s += box_len;
	continue;
  }

    if(memcmp(box_name, "jp2c", 4) == 0)//Mandatory if 'jp2' found
  {
    if(has_thumbnail_jp2h == 0) return 0;

	has_thumbnail_jp2c = 1;
    s += box_len;
    continue;
  }

	if(memcmp(box_name, "mhdr", 4) == 0)//Mandatory
  {
	s += box_len;
	mhdr_found = 1;
	
	break;
  }

   }//while(s < dend)

	if(ftyp_found == 0 || mhdr_found == 0)
   {
fprintf(stderr,"%s:%d:\n\tFAILURE: ftyp_found(%d) mhdr_found(%d).STOP.\n",
__FILE__,__LINE__,ftyp_found,mhdr_found);

	return 0;
   }
	if(has_thumbnail_jp2h == 1 && has_thumbnail_jp2c == 0)
   {
fprintf(stderr,"%s:%d:\n\tFAILURE: has_thumbnail_jp2h(%d) "
"has_thumbnail_jp2c(%d).STOP.\n",
__FILE__,__LINE__,has_thumbnail_jp2h,has_thumbnail_jp2c);

	return 0;

   }
	s = parse_start;

	box_len = read_boxheader(s, dend, &hstep);

    if(memcmp(box_name, "ftyp", 4) == 0)
   {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "[%03d] ftyp", ++pos);
    sup = tree->add(item_name);

    s = read_ftyp(s + hstep, s + box_len, item_name);

    sup->close();
	box_len = read_boxheader(s, dend, &hstep);
   }

    if(memcmp(box_name, "jp2h", 4) == 0)
   {
    Fl_Tree_Item *sup;

    has_thumbnail_jp2h = 1;
    thumbnail_jp2hpos = (s - jpm_start);
    thumbnail_jp2hstep = hstep;
    thumbnail_jp2hlen = box_len;

    snprintf(item_name, MAX_ITEMBUF, "[%03d] jp2h (doc. thumbnail)", ++pos);
    sup = tree->add(item_name);

    s = read_jp2h(s + hstep, s + box_len, dend, item_name);

    sup->close();
	box_len = read_boxheader(s, dend, &hstep);
   }
	if(memcmp(box_name, "jp2c", 4) == 0)
   {
    Fl_Tree_Item *sup;
    
    has_thumbnail_jp2c = 1;
    thumbnail_jp2cpos = (s - jpm_start);
    thumbnail_jp2cstep = hstep;
    thumbnail_jp2clen = box_len;
    
    snprintf(item_name, MAX_ITEMBUF, "[%03d] jp2c (doc. thumbnail)", ++pos);
    sup = tree->add(item_name);

    s = read_jp2c(s + hstep, s + box_len, dend, item_name);

    sup->close();
    box_len = read_boxheader(s, dend, &hstep);
   }

    if(memcmp(box_name, "mhdr", 4) == 0)
   {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "[%03d] mhdr", ++pos);
    sup = tree->add(item_name);

    s = read_mhdr(s + hstep, s + box_len, item_name);

    sup->close();
   }

//--- BODY

	while(s < dend)
   {
	box_len = read_boxheader(s, dend, &hstep);

    if(box_len == 0) { s = dend; break; }

    if((uint64_t)(dend - s) < box_len)
     box_end = dend;
    else
     box_end = s + (ptrdiff_t)box_len;


    if(memcmp(box_name, "dtbl", 4) == 0)
  { 
    Fl_Tree_Item *sup;
	char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] dtbl", ++pos);
    sup = tree->add(iname);

    s = read_dtbl(s + hstep, box_end, dend, iname);

	sup->close();
	continue;
  }

    if(memcmp(box_name, "jp2h", 4) == 0)
  {
    Fl_Tree_Item *sup;
	char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] jp2h", ++pos);
    sup = tree->add(iname);

    s = read_jp2h(s + hstep, box_end, dend, iname);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "jp2c", 4) == 0)
  {
    Fl_Tree_Item *sup;
    char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] jp2c", ++pos);
    sup = tree->add(iname);

	s = read_jp2c(s + hstep, box_end, dend, iname);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "pcol", 4) == 0)
  {
    Fl_Tree_Item *sup;
	char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] pcol", ++pos);
    sup = tree->add(iname);

	s = read_pcol(s + hstep, box_end, dend, iname, s - jpm_start, box_len);

	sup->close();
	continue;
  }

    if(memcmp(box_name, "uuid", 4) == 0)
  {
    Fl_Tree_Item *sup;
	char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] uuid", ++pos);
    sup = tree->add(iname);

    s = read_uuid(s + hstep, box_end, dend, iname);

    sup->close();
    continue;
  }

	if(memcmp(box_name, "uinf", 4) == 0)
  {
    Fl_Tree_Item *sup;
	char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] uinf", ++pos);
    sup = tree->add(iname);

	s = read_uinf(s + hstep, box_end, dend, iname);

	sup->close();
	continue;
  }
	if(memcmp(box_name, "page", 4) == 0)
  {
    Fl_Tree_Item *sup;
	char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] page", ++pos);
    sup = tree->add(iname);

	s = read_page(s + hstep, box_end, dend, iname, s - jpm_start, box_len);

	sup->close();
	continue;
  }

    if(memcmp(box_name, "ftbl", 4) == 0)
  {
    Fl_Tree_Item *sup;
	char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] ftbl", ++pos);
    sup = tree->add(iname);

    s = read_ftbl(s + hstep, box_end, dend, iname);

    sup->close();
    continue;
  }

	if(memcmp(box_name, "mdat", 4) == 0)
  {
    Fl_Tree_Item *sup;
	char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] mdat", ++pos);
    sup = tree->add(iname);

    s = read_mdat(s + hstep, box_end, dend, iname);

    sup->close();
    continue;
  }

    if(memcmp(box_name, "cref", 4) == 0)
  {
    Fl_Tree_Item *sup;
	char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] cref", ++pos);
    sup = tree->add(iname);

    s = read_cref(s + hstep, s + box_len, dend, iname);

    sup->close();
    continue;
  }

    if(memcmp(box_name, "htxb", 4) == 0)
  {
    Fl_Tree_Item *sup;
    char iname[MAX_ITEMBUF+2];
    
    snprintf(iname, MAX_ITEMBUF, "[%03d] htxb", ++pos);
    sup = tree->add(iname);

    s = read_htxb(s + hstep, s + box_len, dend, iname);
    
    sup->close();
    continue;
  } 
	if(memcmp(box_name, "sdat", 4) == 0)
  {
    Fl_Tree_Item *sup;
    char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] sdat", ++pos);
    sup = tree->add(iname);

	s = read_sdat(s + hstep, s + box_len, iname);

	sup->close();

	sdat_found = 1;
	continue;
  }
	if(memcmp(box_name, "sref", 4) == 0)
  {
    Fl_Tree_Item *sup;
    char iname[MAX_ITEMBUF+2];

	if(!sdat_found)
 {
	fprintf(stderr,"File not correct: sref found but no sdat found.\n");
	s += box_len;
	continue;
 }
    snprintf(iname, MAX_ITEMBUF, "[%03d] sref", ++pos);
    sup = tree->add(iname);

	s = read_sref(s + hstep, s + box_len, iname);

	sup->close();	
	continue;
  }
    if(memcmp(box_name, "xml ", 4) == 0
    || memcmp(box_name, "XML ", 4) == 0)
  {
    Fl_Tree_Item *sup, *sub;
    char *buf = NULL;
	char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] xml", ++pos);
    sup = tree->add(iname);

    snprintf(item_name, MAX_ITEMBUF, "%s/xml1", iname);
    sub = tree->add(item_name);

    s = read_xml(s + hstep, box_end, dend, &buf);

    sub->label(buf);

    sup->close();
    free(buf);
    continue;
  }
  {
	Fl_Tree_Item *sup;
	char iname[MAX_ITEMBUF+2];

    snprintf(iname, MAX_ITEMBUF, "[%03d] %s", ++pos, (char*)box_name);
    sup = tree->add(iname);

    snprintf(item_name, MAX_ITEMBUF, "%s/Not handled. Skipped.", iname);
    add_sub_item();

    sup->close();

  }


#ifdef DEBUG_JPM
fprintf(stderr,"read_jpm: UNKNOWN BOX FOUND name(%s) len(%lu)\n",
(char*)box_name,box_len);
#endif
	s = box_end;

   }

#ifdef DEBUG_JPM
fprintf(stderr,"EXIT with (dend - s) ==> %ld\n",(long)(dend-s));
#endif

	return 0;

}//read_jpm()
