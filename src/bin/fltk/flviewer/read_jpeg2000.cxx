#include <config.h>
/*
 * author(s) and license
*/
/*
 * Part 1 JPEG 2000 Image Core Coding System (J2K, JP2)
 * 15444-1annexi.pdf: new JP2 file format
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define snprintf sprintf_s
#define strcasecmp _stricmp
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

//-----------------------------------
//WARNING: SHOW_PACKETS may need a very long time to be shown: 
//#define SHOW_PACKETS

#define TREE_LABEL_SIZE 11

unsigned char box_name[33];

//-----------------------------------
//#define DEBUG_MAIN
//#define READ_ALL_SOT_MARKERS
//#define PSOT_LENG_FROM_BUF_END
//#define DEBUG_DEFAULT_TEST
//#define DEBUG_BOXHEADER
//#define DEBUG_BOXHEADER8
//#define COLLECT_CURV
//#define DEBUG_CURV
//#define DEBUG_XYZ
//#define DEBUG_ICC_PROFILE
//#define DEBUG_DESC
//#define DEBUG_JP2C
//#define SHOW_COM
//#define QUANT_AS_STRING
//#define SHOW_SIZE_VALUE
//#define SHOW_PCLR_ENTRY
//#define DEBUG_JP2C_END
//#define SHOW_XML
//#define SHOW_UUID_DATA

//---------------- FORWARD ----------------

//read_ihdr(), read_bpcc()
static unsigned short nr_components;

//read_ppt()
static unsigned int ppt, ppt_store, ppt_len;

//read_ppm()
static unsigned int ppm_prev;
static unsigned int ppm_store;

//read_poc()
static unsigned char has_POC;
static unsigned int numpocs;

//read_siz()
static short extended_cap, Csiz;
static int one_tile_only;

//read_cod()
static short entropy_coder, use_sop_marker, use_eph_marker;
static short max_precincts, user_defined_precincts;

static unsigned short irreversible_transform, reversible_transform,
	transform;

//-------------- end FORWARD --------------

static jp2_color_t jp2_color;
static char item_name[MAX_ITEMBUF+2];

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

/*-----------------
 COM: Comment and extension marker
 COC: Coding style component marker
 COD: Coding style default maker
 EPH: End of packet marker
 EOI: End of image marker (EOC: end of codestream )
 PLM: Packet length, main header marker
 PLT: Packet length, tile-part header marker
 POD: Progression order change (POC), default marker
 PPM: Packed packet headers, main header marker
 PPT: Packed packet headers, tile-part header marker
 QCC: Quantization component marker
 QCD: Quantization default marker
 RGN: Region of interest marker
 SIZ: Size of image marker
 SOC: Start of image (codestream) marker
 SOP: Start of packet marker
 SOT: Start of tile marker
 TLM: Tile length marker


Part 2:

 DCO: Variable DC offset
 VMS: Visual masking
 DFS: Downsampling factor style
 ADS: Arbitrary decomposition style
 CBD: Component bit depth
 MCT: Multiple component transformation definition
 MCC: Multiple component collection
 MIC: Multiple component intermediate collection
 NLT: Non-linearity point transformation

 ARN: Arbitrary region of interest marker: obsolete?

Part 2, Part 10:
 ATK: Arbitrary transformation kernels

------------------*/
#define J2K_SOC 0xff4f
#define J2K_SIZ 0xff51
#define J2K_COD 0xff52
#define J2K_COC 0xff53
#define J2K_TLM 0xff55
#define J2K_PLM 0xff57
#define J2K_PLT 0xff58
#define J2K_QCD 0xff5c
#define J2K_QCC 0xff5d
#define J2K_RGN 0xff5e
#define J2K_POC 0xff5f
#define J2K_PPM 0xff60
#define J2K_PPT 0xff61
#define J2K_CRG 0xff63
#define J2K_COM 0xff64
#define J2K_SEC 0xff65
#define J2K_EPB 0xff66
#define J2K_ESD 0xff67
#define J2K_EPC 0xff68
#define J2K_RED 0xff69

/*------- Part 2 ---------*/
#define J2K_DCO 0xff70
#define J2K_VMS 0xff71
#define J2K_DFS 0xff72
#define J2K_ADS 0xff73
#define J2K_MCT 0xff74
#define J2K_MCC 0xff75
#define J2K_NLT 0xff76
#define J2K_MIC 0xff77
#define J2K_CBD 0xff78
#define J2K_ATK 0xff79
//ARN
/*----------------------*/

#define J2K_SOT 0xff90
#define J2K_SOP 0xff91
#define J2K_EPH 0xff92
#define J2K_SOD 0xff93

#define J2K_INSEC 0xff94
#define J2K_EOC 0xffd9


static const char *reg_text[]=
{
"    General use (binary values)",
"    General use (ISO 8859-1 (latin-1) values)",
"    Reserved use",
NULL
};

static const char *enumcs_text(int i)
{
	static const char *cs_text[]=
   {
	"Bi-level",
	"YCbCr(1)",
	"",
	"YCbCr(2)",
	"YCbCr(3)",
	
	"",
	"",
	"",
	"",
	"PhotoCD",
	
	"",
	"CMY",
	"CMYK",
	"YCCK",
	"CIELab",
	
	"Bi-level(2)",
	"sRGB",
	"greyscale",
	"sYCC",
	"CIEJab",
	
	"e-sRGB",
	"ROMM-RGB",
	"YPbPr(1125/60)",
	"YPbPr(1250/50)",
	"e-sYCC",
	
	NULL
   };

	if(i < 0 || i > 24) return "Unknown";
	return cs_text[i];

}//enumcs_text()

static void init()
{
	nr_components = 0;
	ppt = ppt_store = ppt_len = 0;
	ppm_prev = ppm_store = 0;
	has_POC = 0;
	numpocs = 0;
	extended_cap = 0;
	Csiz = 0;
	one_tile_only = 0;

	entropy_coder =  use_sop_marker =  use_eph_marker = 0;
	max_precincts = user_defined_precincts = 0;

	irreversible_transform = reversible_transform = transform = 0;
}

unsigned int get4(const unsigned char *b)
{
	return (b[0]<<24)|(b[1]<<16)|(b[2]<<8)|b[3];
}

unsigned int get2(const unsigned char *b)
{
	return (b[0]<<8)|b[1];
}

uint64_t get8(const unsigned char *b)
{
	uint64_t  v;
    unsigned int v2;
    v = (uint64_t)get4(b); v2 = get4(b+4); 
	return ((v<<32) | v2);
}

unsigned int get_size(const unsigned char *b, unsigned int size)
{
	unsigned int i, v = b[0];
#ifdef SHOW_SIZE_VALUE
fprintf(stderr,"\n=== get_size(%d) ===\n\n",size);
#endif
	for(i = 1; i < size; ++i)
   {
	v = (v<<8) | b[i];
   }
	return v;
}

static void apply_cdef(jp2_cdef_info_t *info, unsigned short n)
{
	unsigned short i, j, last;

	for(i = 0; i < n; ++i)
   {
	if(info[i].asoc != 65535 && info[i].typ != 65535) continue;

	last = n - 1;

	for(j = i; j < last; ++j)
  {
	info[j] = info[j+1];
  }
	--n;
   }
	FLViewer_set_max_components(n);
}

static void read_curv_type(unsigned char *tag)
{
#ifdef DEBUG_CURV
	unsigned char *start;
#endif
#ifdef COLLECT_CURV
    unsigned int i, 
	unsigned short v;
#endif
	unsigned int n;

    tag += 4;//reserved
	n = get4(tag); tag += 4;
#ifdef DEBUG_CURV
	start = tag;
fprintf(stderr,"  curv size[%u]\n",n);
#endif
	if(n == 1)
   {
#ifdef DEBUG_CURV
/* gamma */
	unsigned short v = get2(tag);

fprintf(stderr,"    curv[0]gamma(%u.%u)\n",(v>>8),v & 0xff);
#endif
	return;
   }


#ifdef COLLECT_CURV
    for(i = 0; i < n; ++i)
   {
    v = get2(tag); tag += 2;

fprintf(stderr,"    curv[%d] %u\n",i,v);
   }



#elif DEBUG_CURV
	i = 0;
	v = get2(tag); tag += 2;
fprintf(stderr,"    curv[%d] %u\n",i,v);
	v = get2(tag); tag += 2;
fprintf(stderr,"    curv[%d] %u\n",i+1,v);

	i = n/2; tag = start + i*2;
	v = get2(tag); tag += 2;
fprintf(stderr,"    curv[%d] %u\n",i,v);
	v = get2(tag); tag += 2;
fprintf(stderr,"    curv[%d] %u\n",i+1,v);

	i = n - 2; tag = start + i * 2;
	v = get2(tag); tag += 2;
fprintf(stderr,"    curv[%d] %u\n",i,v);
	v = get2(tag); tag += 2;
fprintf(stderr,"    curv[%d] %u\n",i+1,v);
#endif
}

static void read_XYZ_type(unsigned char *tag)
{
#ifdef DEBUG_XYZ
    unsigned int a,b,c;
#endif
	tag += 4;//reserved

#ifdef DEBUG_XYZ
    a = get4(tag); b =  get4(tag+4); c =  get4(tag+8);
fprintf(stderr,"  XYZ(%u.%u, %u.%u, %u.%u)",
a>>16,a & 0xff,a>>16,a & 0xff,a>>16,a & 0xff);
#endif
}

/* Is NOT ascii text, but UTF8 text */
static void read_desc_type(unsigned char *tag, unsigned int size)
{
#ifdef DEBUG_DESC
	unsigned int i;

fputs("    DESC(", stderr);	
	for(i = 0; i < size; ++i)
   {
     if(tag[i] == '/') fputs("\n/", stderr);
     else
     if( !iscntrl(tag[i])) fprintf(stderr,"%c",tag[i]);
   }
fputs(")", stderr);

#endif //DEBUG_DESC
}

static void add_sub_item()
{
	Fl_Tree_Item *sub = tree->add(item_name);
    sub->usericon(&L_documentpixmap);
}

static void read_icc_profile(unsigned char *s, const char *name_src)
{
	char sign[5], proclass[5];
	char cspace[5], pcs[5], platsign[5];
	unsigned char *version, *tag;
	unsigned char *profile_start;
	int profile_len, siz, j;
	unsigned cmmtype, da0, da1;
	unsigned int proflags, rendintent;
	unsigned int ciex,ciey,ciez;
	unsigned int off, nt, i;
	unsigned short year,month,day,hour,min,sec;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/ICC profile box",name_buf);
	add_sub_item();
	
	profile_start = s;
	profile_len = get4(s); s += 4;

	cmmtype = get4(s); s += 4;
	version = s; s += 4;
	proclass[4] = 0; memcpy(proclass, s, 4); s += 4;
	cspace[4] = 0; memcpy(cspace, s, 4); s += 4;
	pcs[4] = 0; memcpy(pcs, s, 4); s += 4;
	year = get2(s); s += 2; month = get2(s); s += 2; day = get2(s); s += 2;
	hour = get2(s); s += 2; min = get2(s); s += 2; sec = get2(s); s += 2;
	sign[4] = 0; memcpy(sign, s, 4); s += 4;
	platsign[4] = 0; memcpy(platsign, s, 4); s += 4;
	proflags = get4(s); s += 4;
	s += 4;/* device manufacturer */
	s += 4;/* device model */
	da0 = get4(s); s += 4; da1 = get4(s); s += 4;
	rendintent = get4(s); s += 4;
	ciex = get4(s); s += 4; ciey = get4(s); s += 4; ciez = get4(s); s += 4;
	s += 4;/* creator */
	s += 16;/* ID */

    snprintf(item_name, MAX_ITEMBUF, "%s/cmmtype(%#x)",name_buf,cmmtype);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/version(%u.%u.%u.0)",name_buf,
		version[0],version[1],version[2]);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/class(%s)",name_buf,proclass);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/color-space(%s)",name_buf,cspace);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/profile-connection-space(%s)",name_buf,pcs);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/date(%u.%u.%u) time(%u:%u:%u)",name_buf,
		day,month,year,hour,min,sec);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/signature(%s)",name_buf,sign);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/platform-signature(%s)",name_buf,platsign);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/profile-flags(%#x) [0] %u [1] %u [2:15] %u",
		name_buf,proflags,proflags & 1,proflags & 2, proflags & 0xfffc);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/device-attr(%u) white(%u)",name_buf,
		da0,(da1&1));
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/negative(%u) matte(%u) transparent(%u)",
		name_buf,(da1&2),(da1&4),(da1&8));
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/rendering-intent(%#x)",name_buf,
		rendintent);
	add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/CIE: x(%d.%u) y(%d.%u) z(%d.%u)",name_buf,
		(ciex>>16),ciex&0xffff,(ciey>>16),ciey&0xffff,(ciez>>16),
		ciez&0xffff);
	add_sub_item();

	s = profile_start + 128;
	nt = get4(s); s += 4;

	for(i = 0; i < nt; ++i)
   {
	memcpy(sign, s, 4); s += 4; 
	off = get4(s); s += 4; 
	siz = get4(s); s += 4;
	tag = profile_start + off;

#ifdef DEBUG_ICC_PROFILE
fprintf(stderr,"[%d] signature(%s) size[%d]\n",i,sign,siz);
#endif

    if(s + siz > profile_start + profile_len)
  {
    fprintf(stderr, "%s:%d:\n\t=== ICC Profile seems to be chopped."
	" RETURN. ===\n\n",__FILE__,__LINE__);
    return;
  }

	if(memcmp(sign, "cprt", 4) == 0)
  {
//#ifdef DEBUG_ICC
	char buf[32];
	int k = 0; 

	memset(buf, 0, 32);

	for(j = 4; j < siz; ++j) 
 {
	if(iscntrl(tag[j])) continue; 

	buf[k] = tag[j]; ++k;
	if(k == 31) break;
 }
    snprintf(item_name, MAX_ITEMBUF, "%s/%s",name_buf,buf);
	add_sub_item();
//#endif
  }
	else
	if(memcmp(sign, "desc", 4) == 0)
  {
	read_desc_type(tag + 4, siz - 4);
  }
	else
  {
	unsigned char buf[5];

	buf[4] = 0; memcpy(buf, tag, 4); tag += 4;

	if(memcmp(buf, "curv", 4) == 0)
	 read_curv_type(tag);
	else
	if(memcmp(buf, "XYZ ", 4) == 0)
	 read_XYZ_type(tag);
	else
	if(memcmp(buf, "desc", 4) == 0)
	 read_desc_type(tag, siz-4);
	else
 {
/*-----------
fprintf(stderr,"   %s\n", buf);
-------------*/
 }
  }

#ifdef DEBUG_ICC_PROFILE
fprintf(stderr,"\n");
#endif
   }
}/* read_icc_profile() */

uint64_t read_boxheader(unsigned char *box, unsigned char *dend,
	unsigned int *out_siz)
{
	uint64_t len;

    len = (uint64_t)get4(box);
    memcpy(box_name, box + 4, 4); box_name[4] = 0; *out_siz = 8; // 4 + 4

#ifdef DEBUG_BOXHEADER
	fprintf(stderr,"\tREAD[1]BOX_NAME(%s) BOX_LEN(%" PRIu64 ")\n",
		box_name,len);
#endif

    if(len == 0) return (uint64_t)(dend - box);//last box

	if(len > 1) return len;

	if(sizeof(uint64_t) == 8)
   {
	len = get8(box+8); *out_siz = 16; // 4 + 4 + 8
   }
	else
   {
	fprintf(stderr,"%s:%d:\n\tread_boxheader ==> can not read 8 bytes."
	" STOP.\n",__FILE__,__LINE__);

	len = 0; *out_siz = 16; // 4 + 4 + 8
   }
#ifdef DEBUG_BOXHEADER8
	fprintf(stderr,"\tREAD[2]               BOX_LEN(%" PRIu64 ")\n",
		len);
#endif

	return len;
}

static unsigned short read_siz(unsigned char *s, const char *name_src)
{
    unsigned int len, x,y,x0,y0,xt,yt, xt0,yt0;
    unsigned short r,c,i,xr,yr,prec,depth,sign;
    unsigned int tile_w, tile_h, image_w, image_h;
	int max_tiles;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Image and tile size marker",name_buf);
	add_sub_item();

    len = get2(s); s += 2;//Lsiz = 38 + 3 * Csiz

    r = get2(s); s += 2;//Rsiz
	snprintf(item_name, MAX_ITEMBUF, "%s/Rsiz(%d)",name_buf,r);
	add_sub_item();

    x = get4(s); s += 4;//Xsiz
	snprintf(item_name, MAX_ITEMBUF, "%s/Xsiz(%d)",name_buf,x);
	add_sub_item();

    y = get4(s); s += 4;//Ysiz
	snprintf(item_name, MAX_ITEMBUF, "%s/Ysiz(%d)",name_buf,y);
	add_sub_item();

    x0 = get4(s); s += 4;//XOsiz
	snprintf(item_name, MAX_ITEMBUF, "%s/X0siz(%d)",name_buf,x0);
	add_sub_item();

    y0 = get4(s); s += 4;//YOsiz
	snprintf(item_name, MAX_ITEMBUF, "%s/Y0siz(%d)",name_buf,y0);
	add_sub_item();

    xt = get4(s); s += 4;//XTsiz
	snprintf(item_name, MAX_ITEMBUF, "%s/XTsiz(%d)",name_buf,xt);
	add_sub_item();

    yt = get4(s); s += 4;//YTsiz
	snprintf(item_name, MAX_ITEMBUF, "%s/YTsiz(%d)",name_buf,yt);
	add_sub_item();

    xt0 = get4(s); s += 4;//XTOsiz
	snprintf(item_name, MAX_ITEMBUF, "%s/XT0siz(%d)",name_buf,xt0);
	add_sub_item();

    yt0 = get4(s); s += 4;//YTOsiz
	snprintf(item_name, MAX_ITEMBUF, "%s/YT0siz(%d)",name_buf,yt0);
	add_sub_item();

    c = get2(s); s += 2;//Csiz
	snprintf(item_name, MAX_ITEMBUF, "%s/Csiz(%d)",name_buf,c);
	add_sub_item();
	Csiz = c;

	FLViewer_set_max_components(c);

    image_w = x - x0; image_h = y - y0;
    tile_w = xt - xt0; tile_h = yt - yt0;

	if(tile_w <= 0 || tile_h <= 0)
   {
//This definitely is a bug.
	max_tiles = 0;	
   }
	else
   {
	int nr;

	max_tiles = image_w/tile_w;
	if(image_w%tile_w) ++max_tiles;

	nr = image_h/tile_h;
	if(image_h%tile_h) ++nr;

	max_tiles *= nr;
   }
	FLViewer_set_max_tiles(max_tiles);

    if(r >= 32768) extended_cap = 1; else extended_cap = 0;

	snprintf(item_name, MAX_ITEMBUF, "%s/Ext. Capabilities(%d)",name_buf,extended_cap);
	add_sub_item();

#ifdef DEBUG_SIZ
fprintf(stderr,"%s:%d:\n\tread_siz\n\tlen(%u)\n"
"\tcapabilities(%u)[extended: %d]\n\tx(%u : %u) y(%u : %u)"
"\n\txt(%u : %u) yt(%u : %u)\n\tIMAGE w(%d) h(%d) TILE w(%d) h(%d)\n"
"\tCsiz(%hu)\n",len,r, extended_cap, x0,x, y0,y,
xt0,xt, yt0,yt, image_w,image_h,tile_w,tile_h,Csiz);
#endif

    for(i = 0; i < c; ++i)
   {
    prec = s[0]; ++s;
    depth = (prec & 0x7f) + 1;
    sign = (prec & 0x80?1:0);//Ssiz
    xr = s[0]; ++s;//XRsiz
    yr = s[0]; ++s;//YRsiz

	snprintf(item_name, 
	 MAX_ITEMBUF, "%s/comp[%d] signed(%u) prec(%u) hsep(%u) vsep(%u)",
	 name_buf,i,sign,depth,xr,yr);
	add_sub_item();
   }
    one_tile_only = (image_w == tile_w && image_h == tile_h);
	snprintf(item_name, MAX_ITEMBUF, "%s/OneTileOnly(%d)",name_buf,one_tile_only);
	add_sub_item();

	ppm_store = 0; ppm_prev = 0;

    return len;
}//read_siz()

static unsigned short read_cod(unsigned char *s, const char *name_src)
{
    unsigned char *cur;
	const char *cs = "";
    unsigned short len, max_len, nr_layers;
    unsigned short Scod, prog_order, multi_comp_transform;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Coding style default marker",name_buf);
	add_sub_item();

    cur = s;
    max_len = get2(cur); cur += 2;
    Scod = cur[0]; ++cur;//Table A-13
    prog_order = cur[0]; ++cur;//Table A-16
    nr_layers = get2(cur); cur += 2;
    multi_comp_transform = cur[0]; ++cur;

	FLViewer_set_max_layers(nr_layers);

    entropy_coder = use_sop_marker = use_eph_marker = 0;

    if(Scod & 1) entropy_coder = 1;
    if(Scod & 2) use_sop_marker = 1;
    if(Scod & 4) use_eph_marker = 1;

	if(prog_order == 0) cs = "LRCP";
	else
	if(prog_order == 1) cs = "RLCP";
	else
	if(prog_order == 2) cs = "RPCL";
	else
	if(prog_order == 3) cs = "PCRL";
	else
	if(prog_order == 4) cs = "CPRL";
	else
	 cs = "Unknown";

	snprintf(item_name, MAX_ITEMBUF, "%s/prog_order(%hu)%s", name_buf,
	 prog_order, cs);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/nr_layers(%u)", name_buf,
	 nr_layers );
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/multi_comp_transform(%hu)", name_buf,
	 multi_comp_transform );
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/entropy_coder(%u)", name_buf,
	 entropy_coder );
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/use_sop_marker(%u)", name_buf, 
	 use_sop_marker);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/use_eph_marker(%u)", name_buf, 
	 use_eph_marker);
	add_sub_item();

    max_precincts = user_defined_precincts = 0;

    if(max_len < 13)
   {
     max_precincts = 1; // A.2

	snprintf(item_name, MAX_ITEMBUF, "%s/max_precincts(1)", name_buf);
	add_sub_item();
   }
    else
   {
	user_defined_precincts = 1;

	snprintf(item_name, MAX_ITEMBUF, "%s/user_defined_precincts(1)", name_buf);
	add_sub_item();
   }
   {
    unsigned int i;
    unsigned char code_block_width ,code_block_height,
    code_block_style, nr_decomp_levels;

/*--------------------------------------------------

                               A -- Progression order
                               B-- Number of layers
                               C -- Multiple component transformation
A   B   C | D E F G H Ii .. In D -- Number of decomposition levels
          |                    E -- Code-block width
  SGcod   |       SPcod        F -- Code-block height
                               G -- Code-block style
                               H -- Transformation
                               Ii through In -- Precinct size
----------------------------------------------------*/
/* SPcod:
*/
    nr_decomp_levels = cur[0] + 1; ++cur;
    code_block_width = cur[0]; ++cur;
    code_block_height = cur[0]; ++cur;
    code_block_style = cur[0]; ++cur;
    transform = cur[0]; ++cur;

	FLViewer_set_max_reduction((int)nr_decomp_levels);

	snprintf(item_name, MAX_ITEMBUF, "%s/nr_decomp_levels(%hu)", name_buf, 
	 nr_decomp_levels);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/code_block_width(%d)", name_buf, 
	 code_block_width);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/code_block_height(%u)", name_buf, 
	 code_block_height);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/code_block_style(%d)", name_buf, 
	 code_block_style);
	add_sub_item();

    irreversible_transform = reversible_transform = 0;
    if(transform == 0)
  {
    irreversible_transform = 1;

	snprintf(item_name, MAX_ITEMBUF, "%s/transformation(0) (9-7 irreversible)", 
	 name_buf);
	add_sub_item();
  }
    else
    if(transform == 1)
  {
    reversible_transform = 1;

	snprintf(item_name, MAX_ITEMBUF, "%s/transformation(1) (5-3 reversible)", 
	 name_buf);
	add_sub_item();
  }
	else
  {

	snprintf(item_name, MAX_ITEMBUF, "%s/transformation(%hu)", name_buf,
	 transform);
	add_sub_item();
  }

    if(max_len == 12)
  {
    unsigned short j;
        for(j = 0; j < nr_decomp_levels; ++j)
       {
		snprintf(item_name, MAX_ITEMBUF, "%s/[%d] precinct w(%hu) h(%hu)", name_buf, 
		 j,15,15);
		add_sub_item();
       }
    return max_len;
  }
    len = 12;

    i = 0;
    while(len < max_len)
  {
    len += nr_decomp_levels;

    if((Scod & 1))// Table A-21
 {
    unsigned short j, v;

        for(j = 0; j < nr_decomp_levels; ++j)
       {
        v = cur[0]; ++cur;

		snprintf(item_name, MAX_ITEMBUF, "%s/[%d] precinct w(%hu) h(%hu)", name_buf,
		 j,(v&0xf), (v>>4));
		add_sub_item();
       }
 }
    else
 {
    unsigned short j;
        for(j = 0; j < nr_decomp_levels; ++j)
       {
		snprintf(item_name, MAX_ITEMBUF, "%s/[%d] precinct w(%hu) h(%hu)", name_buf,
		 j,15,15);
		add_sub_item();
       }
 }
    ++i;
  }
   }

    return max_len;

}// read_cod()

static unsigned short read_coc(unsigned char *s, const char *name_src)
{
	unsigned short len, Scoc;
	unsigned short A, B, C, D, E;
    char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Coding style component marker",name_buf);
	add_sub_item();

	len = get2(s); s += 2;

	Scoc = s[0]; ++s;//csty
	snprintf(item_name, MAX_ITEMBUF, "%s/Scoc(%hu)", name_buf,Scoc);
	add_sub_item();

	A = s[0] + 1; ++s;
	snprintf(item_name, MAX_ITEMBUF, "%s/nr_decomp_levels(%hu)", name_buf,A);
	add_sub_item();

	B = s[0] + 2; ++s;
	snprintf(item_name, MAX_ITEMBUF, "%s/code-block-width(%hu)", name_buf,B);
	add_sub_item();

	C = s[0] + 2; ++s;
	snprintf(item_name, MAX_ITEMBUF, "%s/code-block-height(%hu)", name_buf,C);
	add_sub_item();

	D = s[0]; ++s;
	snprintf(item_name, MAX_ITEMBUF, "%s/code-block-style(%hu)", name_buf,D);
	add_sub_item();

	E = s[0]; ++s;
	snprintf(item_name, MAX_ITEMBUF, "%s/transformation(%hu)", name_buf,E);
	add_sub_item();

	if(Scoc == 0)
   {
	unsigned short i;

	for(i = 0; i < A; ++i)
  {
    snprintf(item_name, MAX_ITEMBUF, "%s/Precinct[%d] Width(15) Height(15)",
     name_buf,i );
	add_sub_item();
  }
   }
	else
	if(Scoc & 1)
   {
	unsigned short i, K;

	for(i = 0; i < A; ++i)
  {
	K = s[0]; ++s;

    snprintf(item_name, MAX_ITEMBUF, "%s/Precinct[%d] Width(%hu) Height(%hu)", 
	 name_buf,i,(K & 0xf), (K >> 4) );
	add_sub_item();
  }
   }
	return len;

}//read_coc()

static unsigned short read_rgn(unsigned char *s, const char *name_src)
{
    unsigned short len, Crgn, Srgn, SPrgn;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Region-of-interest marker",name_buf);
	add_sub_item();

	len = get2(s); s += 2;
	Crgn = get_size(s, (Csiz < 257?1:2) );
	snprintf(item_name, MAX_ITEMBUF, "%s/Component Index(%hu)",name_buf,Crgn);
	add_sub_item();

	Srgn = s[0]; ++s;
    snprintf(item_name, MAX_ITEMBUF, "%s/ROI style(%hu)",name_buf,Srgn);
	add_sub_item();

	SPrgn = s[0]; ++s;
    snprintf(item_name, MAX_ITEMBUF, "%s/Implicit ROI shift(%hu)",name_buf,SPrgn);
	add_sub_item();

	return len;

}//read_rgn()

static unsigned short read_plm(unsigned char *s, const char *name_src)
{   
    unsigned short len;
	short siz;
    char name_buf[MAX_ITEMBUF];
    
    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Packet length marker",name_buf);
	add_sub_item();

	len = get2(s); s += 2;
	siz = (short)len;
	++s; //Zplm

	siz -= 3;
	while(siz > 0)
   {
	unsigned int i, Nplm, packet_len = 0;
	unsigned short v;

	Nplm = get4(s); s += 4; siz -= 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/Index of marker segment(%u)",name_buf,Nplm);
	add_sub_item();

	for(i = 0; i < Nplm; ++i)
  {
	v = s[0]; ++s; --siz;
	packet_len = (packet_len << 7) + v;

//	if((v & 0x80) == 0) packet_len = 0; //New packet

    snprintf(item_name, MAX_ITEMBUF, "%s/Packet[%d] length(%u)",name_buf,i,packet_len);
	add_sub_item();

	if((v & 0x80) == 0) packet_len = 0; //New packet
  }
   }
	return len;

}//read_plm()

static unsigned short read_qcc(unsigned char *s, const char *name_src)
{
    unsigned short len, Cqcc, Sqcc;
	unsigned short header_size, num_bands;
	unsigned short qntsty;
//	unsigned short numgbits;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Quantization component marker",name_buf);
	add_sub_item();

    len = get2(s); s += 2;
	header_size = len - 2 - 1;

	if(Csiz < 257)
   {
	Cqcc = s[0]; ++s;
   }
	else
   {
	Cqcc = get2(s); s += 2;
   }

	Sqcc = s[0]; ++s;

	qntsty = Sqcc & 0x1f; //numgbits = Sqcc >> 5;

	if(qntsty == 1)
   {
	num_bands = 1;
   }
	else
	if(qntsty == 0)
   {
	num_bands = header_size;
   }
	else
   {
	num_bands = header_size/2;
   }
	snprintf(item_name, MAX_ITEMBUF, "%s/Cqcc(%d) Sqcc(%d) numbands(%d)",
	 name_buf, Cqcc, Sqcc, num_bands);
	add_sub_item();

	if(qntsty == 0)
   {
	unsigned short i, v;

	for(i = 0; i < num_bands; ++i)
  {
	v = s[0]; ++s;

    snprintf(item_name, MAX_ITEMBUF, "%s/Band[%d] Exponent(%hu) Mantissa(%hu)",
     name_buf,i,(v>>3), 0);
	add_sub_item();
  }
   }
	else
   {
	unsigned short i, v;

	for(i = 0; i < num_bands; ++i)
  {
	v = get2(s); s += 2;

    snprintf(item_name, MAX_ITEMBUF, "%s/Band[%d] Exponent(%hu) Mantissa(%hu)",
     name_buf,i,(v>>11), (v & 0x7ff));
	add_sub_item();
  }
   }
	return len;
}//read_qcc()

static unsigned short read_poc(unsigned char *s, const char *name_src)
{
	unsigned short len, numpchgs, i, old_poc;
	unsigned short RSpoc, CSpoc;
	unsigned short LYEpoc, REpoc, CEpoc;
	unsigned short Ppoc; 
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Progression order change marker",name_buf);
	add_sub_item();

	len = get2(s); s += 2;

	old_poc = has_POC ? numpocs + 1 : 0;
	has_POC = 1;

	numpchgs = (len - 2) / (5 + 2 * (Csiz < 257 ? 1 : 2));

	if( numpchgs >= 32 ) numpchgs = 0;

	for(i = old_poc; i < numpchgs + old_poc; ++i)
   {
	RSpoc = s[0]; ++s;
	snprintf(item_name, MAX_ITEMBUF, "%s/poc[%i] RSpoc(%hu)",name_buf,i,RSpoc);
	add_sub_item();

	if(Csiz < 257)
  {
	CSpoc = s[0]; ++s;	
  }
	else
  {
	CSpoc = get2(s); s += 2;	
  }
	snprintf(item_name, MAX_ITEMBUF, "%s/poc[%i] CSpoc(%hu)",name_buf,i,CSpoc);
	add_sub_item();

	LYEpoc = get2(s); s += 2;
	snprintf(item_name, MAX_ITEMBUF, "%s/poc[%i] LYEpoc(%hu)",name_buf,i,LYEpoc);
	add_sub_item();

	REpoc = s[0]; ++s;
	snprintf(item_name, MAX_ITEMBUF, "%s/poc[%i] REpoc(%hu)",name_buf,i,REpoc);
	add_sub_item();

	if(Csiz < 257)
  {
	CEpoc = s[0]; ++s;
  }
	else
  {
	CEpoc = get2(s); s += 2;
  }
	snprintf(item_name, MAX_ITEMBUF, "%s/poc[%i] CEpoc(%hu)",name_buf,i,CEpoc);
	add_sub_item();

	Ppoc = s[0]; ++s;
	snprintf(item_name, MAX_ITEMBUF, "%s/poc[%i] Ppoc(%hu)",name_buf,i,Ppoc);
	add_sub_item();
   }
	numpocs = numpchgs + old_poc - 1;

	return len;

}//read_poc()

static unsigned short read_tlm(unsigned char *s, const char *name_src)
{
	unsigned short len;
	unsigned short Ztlm, Stlm;
	unsigned int ST, SP, dSP, i, tile_tlm, Ttlm, Ptlm;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Tile-part lengths marker",name_buf);
    add_sub_item();

	len = get2(s); s += 2;
	Ztlm = s[0]; ++s;
    snprintf(item_name, MAX_ITEMBUF, "%s/Ztlm(%hu)",name_buf,Ztlm);
    add_sub_item();

	Stlm = s[0]; ++s;
    snprintf(item_name, MAX_ITEMBUF, "%s/Stlm(%hu)",name_buf,Stlm);
    add_sub_item();

	ST = ((Stlm >> 4) & 0x01) + ((Stlm >> 4) & 0x02);
	SP = (Stlm >> 6) & 0x01;
	dSP = (SP ? 4 : 2);
	tile_tlm = (len - 4) / ((SP + 1) * 2 + ST);

	for(i = 0; i < tile_tlm; ++i)
   {
	Ttlm = get_size(s, ST); s += ST;
	Ptlm = get_size(s, dSP); s += dSP;

    snprintf(item_name, MAX_ITEMBUF, "%s/Tlm[%i] Ttlm(%u) Ptlm(%u)",name_buf,i,Ttlm,Ptlm);
    add_sub_item();
   }

	return len;	

}//read_tlm()

unsigned char *read_resc(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
	char name_buf[MAX_ITEMBUF];
	unsigned short vcn, vcd, hcn, hcd, vce, hce;

	strcpy(name_buf, (char*)name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Capture resolution box",name_buf);
    add_sub_item();

	vcn = get2(s); s += 2;
	vcd = get2(s); s += 2;
	hcn = get2(s); s += 2;
	hcd = get2(s); s += 2;
	vce = s[0]; ++s;
	hce = s[0]; ++s;

	snprintf(item_name, MAX_ITEMBUF, "%s/vcn(%hu) vcd(%hu)", name_buf,vcn,vcd);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/hcn(%hu) hcd(%hu)", name_buf,hcn,hcd);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/vce(%hu) hce(%hu)", name_buf,vce,hce);
	add_sub_item();

	return s;
}//read_resc()

unsigned char *read_resd(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
	char name_buf[MAX_ITEMBUF];
	unsigned short vdn, vdd, hdn, hdd, vde, hde;

	strcpy(name_buf, (char*)name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Default display resolution box",name_buf);
    add_sub_item();

	vdn = get2(s); s += 2;
	vdd = get2(s); s += 2;
	hdn = get2(s); s += 2;
	hdd = get2(s); s += 2;
	vde = s[0]; ++s;
	hde = s[0]; ++s;

	snprintf(item_name, MAX_ITEMBUF, "%s/vdn(%hu) vdd(%hu)", name_buf,vdn,vdd);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/hdn(%hu) hdd(%hu)", name_buf,hdn,hdd);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/vde(%hu) hde(%hu)", name_buf,vde,hde);
	add_sub_item();

	return s;
}//read_resd()

unsigned char *read_res(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const unsigned char *name_src)
{
	char name_buf[MAX_ITEMBUF];
	uint64_t box_len;
	unsigned int hstep;

	strcpy(name_buf, (char*)name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Resolution box",name_buf);
    add_sub_item();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	snprintf(item_name, MAX_ITEMBUF, "%s/%s", name_buf,(char*)box_name);

	if(memcmp(box_name, "resc", 4) == 0)
   {
	Fl_Tree_Item *sup = tree->add(item_name);

	s = read_resc(s + hstep, s + box_len, item_name);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	snprintf(item_name, MAX_ITEMBUF, "%s/%s", name_buf,(char*)box_name);
   }
	if(memcmp(box_name, "resd", 4) == 0)
   {
	Fl_Tree_Item *sup = tree->add(item_name);

	s = read_resd(s + hstep, s + box_len, item_name);

	sup->close();
   }
	return s;
}

unsigned char *read_ftyp(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
	int i, n;
	char name_buf[MAX_ITEMBUF];
	char buf[5], brand[5];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/File type box", name_buf);
    add_sub_item();

	brand[4] = 0; buf[4] = 0;
	memcpy(brand, s, 4); s += 4;
//	minv = get4(s); 
	s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/brand(%s)", name_buf,(char*)brand);
	add_sub_item();

	n = (int)((box_end - s)/4);

	for(i = 0; i < n; ++i)
   {
	memcpy(buf, s, 4); s += 4;

	if(*buf == 0) memset(buf, ' ', 4);

	snprintf(item_name, MAX_ITEMBUF, "%s/CL(%s)", name_buf,(char*)buf);
	add_sub_item();
   }
	return s;
}

unsigned char *read_ihdr(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
	char name_buf[MAX_ITEMBUF];
	int w, h;
	unsigned short nc, bpc, c, unkc, ipr, depth, sign;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Image header box",name_buf);
    add_sub_item();

	h = get4(s); s += 4;
	w = get4(s); s += 4;
	nc = get2(s); s += 2;
	bpc = s[0]; ++s; 
	depth = (bpc & 0x7f) + 1; 
	sign = (bpc & 0x80)?1:0;
	c = s[0]; ++s;
	unkc = s[0]; ++s;
	ipr = s[0]; ++s;

#ifdef DEBUG_IHDR
fprintf(stderr,"read_ihdr\n\tw(%u) h(%u) nc(%u) bpc(%u)\n\tsigned(%u) "
"depth(%u)\n\tcompress(%u) unknown_c(%u) ipr(%u)\n",w, h, 
nc,bpc, sign,depth,c, unkc, ipr);
#endif

	snprintf(item_name, MAX_ITEMBUF, "%s/w(%d)", name_buf, w);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/h(%d)", name_buf, h);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/numcomps(%d)", name_buf, nc);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/bpc(%d)", name_buf, bpc);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/depth(%d)", name_buf, depth);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/signed(%d)", name_buf, sign);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/compress(%d)", name_buf, c);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/unkc(%d)", name_buf, unkc);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/ipr(%d)", name_buf, ipr);
	add_sub_item();
	
	nr_components = nc;

	return s;
}/* read_ihdr() */

unsigned char *read_bpcc(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
	unsigned char *v;
	char name_buf[MAX_ITEMBUF];
    unsigned short i;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Bits per component box",name_buf);
    add_sub_item();

	v = s;

    for(i = 0; i < nr_components; ++i)
   {
	snprintf(item_name, MAX_ITEMBUF, "%s/[%d]bpcc(%d)", name_buf,
	 i,v[0]);
	add_sub_item();

	++v;
   }
    return (s + nr_components);
}

unsigned char *read_colr(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
	char name_buf[MAX_ITEMBUF];
	unsigned int meth, prec, approx, enumcs = 0;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Colour specification box",name_buf);
    add_sub_item();

	meth = s[0]; ++s;
	prec = s[0]; ++s;
	approx = s[0]; ++s;

	snprintf(item_name, MAX_ITEMBUF, "%s/meth(%d)",name_buf,meth);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/prec(%d)",name_buf,prec);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/approx(%d)",name_buf,approx);
	add_sub_item();

	if(meth == 1)/* enumerated colorspace */
   {
	const char *cs = "";

	enumcs = get4(s); s += 4;
	cs = enumcs_text(enumcs);

	snprintf(item_name, MAX_ITEMBUF, "%s/enumcs(%d: %s)",
	 name_buf,enumcs,cs);
	add_sub_item();

	if(enumcs == 14) //CIELab 
  {
	s = box_end;
  }
   }
	else /* Profile */
   {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "%s/ICC",name_buf);
	sup = tree->add(item_name);

	read_icc_profile(s, item_name);

	sup->close();
	s = box_end;//skip ICC Profile
   }
	assert(box_end == s);

	if( !jp2_color.has_colr)
   {
	jp2_color.meth = meth;
	jp2_color.precedence = prec;
	jp2_color.approx = approx;
	jp2_color.enumcs = enumcs;
	jp2_color.has_colr = 1;
   }

	return s;
}//read_colr()

unsigned char *read_cdef(unsigned char *s, unsigned char *box_end,
	const char *name_src)
{
	jp2_cdef_info_t *cdef_info;
	char name_buf[MAX_ITEMBUF];
	unsigned short i, n, c, typ, asoc;
//
// Part 1, I.5.3.6: 'There shall be at most one Channel Definition box
//                   inside a JP2 Header box.'
//
	if(jp2_color.has_cdef)
   {
	return box_end;
   }
	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Channel definition box",name_buf);
    add_sub_item();

	n = get2(s); s += 2;

	cdef_info = (jp2_cdef_info_t*)malloc(n * sizeof(jp2_cdef_info_t));
	if(cdef_info == NULL) return box_end;

	jp2_color.has_cdef = 1;

    for(i = 0; i < n; ++i)
   {
    c = get2(s); s += 2;
    typ = get2(s); s += 2;
    asoc = get2(s); s += 2;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%d] c(%d) typ(%d) asoc(%d)", name_buf,
	 i, c, typ, asoc);
	add_sub_item();

	cdef_info[i].cn = c; cdef_info[i].typ = typ; cdef_info[i].asoc = asoc;
   }

	apply_cdef(cdef_info, n);

	free(cdef_info);

	return s;

}//read_cdef()

unsigned char *read_pclr(unsigned char *s, unsigned char *box_end,
	unsigned int *out_channels, const char *name_src)
{
	char name_buf[MAX_ITEMBUF];
    unsigned char *prec;
    unsigned int *entries;
    int size;
    unsigned short nr_entries, nr_channels;
    unsigned short i, j;
	int count;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Palette box",name_buf);
    add_sub_item();

    nr_entries = get2(s); s += 2; /* NE */
    nr_channels = s[0]; ++s; /* NPC */
	count = 11;//8+3

	*out_channels = nr_channels;

    i = nr_channels * nr_entries;
    entries = (unsigned int*)calloc(i, sizeof(unsigned int));
	if(entries == NULL) return box_end;

    prec = (unsigned char*)calloc(nr_channels, 1);
	if(prec == NULL)
   {
	free(entries);
	return box_end;
   }

    for(i = 0; i < nr_channels; ++i)
   {
    prec[i] = s[0]; ++s; /* Bi */
	++count;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%d] signed(%u) depth(%u)",
	 name_buf, i, (prec[i]&0x80?1:0), (prec[i]&0x7f) + 1 );
	add_sub_item();
   }

    for(j = 0; j < nr_entries; ++j)
   {
#ifdef SHOW_PCLR_ENTRY
fprintf(stderr,"   entry[%03u]",j);
#endif
    for(i = 0; i < nr_channels; ++i)
  {
    size = ((prec[i]&0x7f) + 1 + 7)/8;

    entries[j*nr_channels + i] = get_size(s, size); s += size; /* Cji */
	count += size;

#ifdef SHOW_PCLR_ENTRY
fprintf(stderr,"%u ", entries[j*nr_channels + i]);
#endif
  }
#ifdef SHOW_PCLR_ENTRY
fputs("\n",stderr);
#endif
   }
	free(prec); free(entries);

	return box_end;
}/* read_pclr() */

unsigned char *read_cmap(unsigned char *s, unsigned char *box_end,
	unsigned int nr_channels, const char *name_src)
{
	char name_buf[MAX_ITEMBUF];
    unsigned short i, cmp;
    unsigned char mtyp, pcol;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Component mapping box",name_buf);
    add_sub_item();

    for(i = 0; i < nr_channels; ++i)
   {
    cmp = get2(s); s += 2; 
    mtyp = s[0]; ++s; 
    pcol = s[0]; ++s;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%d] cmp(%u) mtyp(%u) pcol(%u)",
		name_buf, i,cmp,mtyp,pcol);
	add_sub_item();
   }

	return s;
}/* read_cmap() */

unsigned char *read_jp2h(unsigned char *s, unsigned char *box_end, 
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep, nr_channels = 0;
	int pos = -1;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/JP2 header box",name_buf);
    add_sub_item();

	while(s < box_end)
   {
	++pos;
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] %s",name_buf,pos,
	 (char*)box_name);

	if(memcmp(box_name, "ihdr", 4) == 0)
  {
	Fl_Tree_Item *sup;
	sup = tree->add(item_name);

	s = read_ihdr(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }

    if(memcmp(box_name, "bpcc", 4) == 0)
  {
	Fl_Tree_Item *sup;
	sup = tree->add(item_name);

    s = read_bpcc(s + hstep, s + box_len, item_name);

	sup->close();
    continue;
  }

	if(memcmp(box_name, "colr", 4) == 0)
  {
	Fl_Tree_Item *sup;
	sup = tree->add(item_name);

	s = read_colr(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  } 
	if(memcmp(box_name, "cdef", 4) == 0)
  {
	Fl_Tree_Item *sup;
	sup = tree->add(item_name);

	s = read_cdef(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "pclr", 4) == 0)
  {
	Fl_Tree_Item *sup;
	sup = tree->add(item_name);

	s = read_pclr(s + hstep, s + box_len, &nr_channels, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "cmap", 4) == 0)
  {
	Fl_Tree_Item *sup;
	sup = tree->add(item_name);

	s = read_cmap(s + hstep, s + box_len, nr_channels, item_name);

	sup->close();
	continue;
  }
	if(memcmp(box_name, "res ", 4) == 0)
  {
	Fl_Tree_Item *sup;
	sup = tree->add(item_name);

	s = read_res(s + hstep, s + box_len, dend, (unsigned char*)item_name);

	sup->close();
	continue;
  }
	if(memcmp(box_name, "xml ", 4) == 0
	|| memcmp(box_name, "XML ", 4) == 0)
  {
    Fl_Tree_Item *sup, *sub;
    char *buf = NULL;
	char iname[MAX_ITEMBUF];

	snprintf(iname, MAX_ITEMBUF, "[%03d] XML", ++pos);
	sup = tree->add(iname);

	snprintf(item_name, MAX_ITEMBUF, "%s/xml1", iname);
	sub = tree->add(item_name);

    s = read_xml(s + hstep, s + box_len, dend, &buf);

    sub->label(buf);

	sup->close();
    free(buf); --pos;
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

	s += box_len;
   }

	return box_end;
}//read_jp2h()

unsigned short read_plt(unsigned char *s, const char *name_src)
{
	unsigned short len, Zplt;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, 
	 "%s/Packet length, tile-part header marker", name_buf);
    add_sub_item();

	len = get2(s); s += 2; 
	Zplt = s[0]; ++s; 

	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/Zplt(%hu) len(%hu)", name_buf,Zplt,len);
	add_sub_item();

	return len;
}// read_plt()

static unsigned short read_ppm(unsigned char *s, const char *name_src)
{
    unsigned short max_len, len, Zppm;
	unsigned int Nppm, i, j;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Packed packet headers marker (Main)", name_buf);
    add_sub_item();

    max_len = get2(s); s += 2;
	len = max_len - 3;
	Zppm = s[0]; ++s;
    snprintf(item_name, MAX_ITEMBUF, "%s/Zppm(%hu)", name_buf,Zppm);
    add_sub_item();

	while(len > 0)
   {
	if(ppm_prev)
  { 
	Nppm = ppm_prev;
  }
	else
  {
	Nppm = get4(s); s += 4;

    snprintf(item_name, MAX_ITEMBUF, "%s/Nppm(%u)", name_buf,Nppm);
    add_sub_item();
  }
	j = ppm_store;

	for(i = Nppm; i > 0; --i)
  {
	++s; ++j;

	if(--len == 0) break;
  }
	ppm_prev = i - 1;
	ppm_store = j;
  
   }//while(len > 0)

	return max_len;
}//read_ppm()

static unsigned short read_ppt(unsigned char *s, const char *name_src)
{
    unsigned short len, Zppt;
	unsigned int i, j;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Packed packet headers marker (Tile part)", name_buf);
    add_sub_item();

    len = get2(s); s += 2;
	Zppt = s[0]; ++s;
    snprintf(item_name, MAX_ITEMBUF, "%s/Zppt(%hu)", name_buf,Zppt);
    add_sub_item();

	ppt = 1;

	if(Zppt == 0)//First marker
   {
	ppt_store = 0; ppt_len = len - 3;
   }
	else
   {
	ppt_len = len - 3 + ppt_store;
   }
	j = ppt_store;
	for(i = len - 3; i > 0; --i)
   {
	++s;
	++j;
   }
	ppt_store = j;

    return len;
}//read_ppt()

#ifdef SHOW_PACKETS

static unsigned short read_sop(unsigned char *s, const char *name_src)
{
    unsigned short len, Nsop;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Start of packet marker", name_buf);
    add_sub_item();

    len = get2(s); s += 2;

	Nsop = get2(s); s += 2;
    snprintf(item_name, MAX_ITEMBUF, "%s/Packet sequence number(%hu)", name_buf,Nsop);
    add_sub_item();

    return len;
}//read_sop()

#endif //SHOW_PACKETS

static unsigned short read_crg(unsigned char *s, const char *name_src)
{
    unsigned short len, i, v;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Component registration marker", name_buf);
    add_sub_item();

    len = get2(s); s += 2;
	for(i = 0; i < Csiz; ++i)
   {
	v = get2(s); s += 2;
    snprintf(item_name, MAX_ITEMBUF, "%s/Xcrg[%d] %hu", name_buf,i,v);
    add_sub_item();
   }
	for(i = 0; i < Csiz; ++i)
   {
	v = get2(s); s += 2;
    snprintf(item_name, MAX_ITEMBUF, "%s/YXcrg[%d] %hu", name_buf,i,v);
    add_sub_item();
   }
    return len;
}//read_crg()

static unsigned short read_com(unsigned char *s, const char *name_src)
{
    Fl_Tree_Item *sub;
	char *txt;
    unsigned short len;
	unsigned int r, i, n;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Comment marker", name_buf);
    add_sub_item();

	len = get2(s); s += 2;
	r = get2(s); s += 2;
	n = len - 4;
	txt = (char*)malloc(n + 8);
	if(txt == NULL) return len;

	memset(txt, 0, n+8);
	strcpy(txt, "    ");
	memcpy(txt+4, s, n);

	if(r > 1) i = 2; else i = r;

    snprintf(item_name, MAX_ITEMBUF, "%s/com1", name_buf);
    sub = tree->add(item_name);
	sub->label(reg_text[i]);

    snprintf(item_name, MAX_ITEMBUF, "%s/com2", name_buf);
    sub = tree->add(item_name);
	sub->label(txt);

#ifdef SHOW_COM
fprintf(stderr,"%s:%d:\n\tCOM\n\tR[%d]%s\n\tT(%s)\n",__FILE__,__LINE__,
i,reg_text[i],txt);
#endif
	free(txt);

    return len;
}//read_com()

static unsigned char *read_sot(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned char *start;
	unsigned short tile_nr;
#ifdef READ_ALL_SOT_MARKERS
	unsigned short len;
#endif
	uint64_t Psot;
	unsigned char TPsot, TNsot;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Start of tile-part marker",name_buf);
	add_sub_item();

	start = s;
#ifdef READ_ALL_SOT_MARKERS
	len = get2(s); s += 2;
#else
	s += 2;
#endif
	tile_nr = get2(s); s += 2;

    snprintf(item_name, MAX_ITEMBUF,"%s/TileNr(%d)",name_buf,tile_nr);
    add_sub_item();

	Psot = (uint64_t)get4(s); s += 4;
	TPsot = s[0]; ++s;
	TNsot = s[0]; ++s;

#ifdef PSOT_LENG_FROM_BUF_END
	if(Psot == 0) 
  { 
	Psot = (uint64_t)(dend - s);

    snprintf(item_name, MAX_ITEMBUF,"%s/Psot(%" PRIu64 ")",name_buf,Psot);
    add_sub_item();
/*---------------
fprintf(stderr,"%s:%d:\n\tbuf_end(%p) - s(%p) ==> Psot(%lu)\n"
"\tbox_end(%p) - s(%p)  ==> Psot(%lu)\n",__FILE__,__LINE__,
(void*)dend,(void*)s,Psot,
(void*)box_end,(void*)s,(uint64_t)(box_end - s) );
---------------*/
  }//EOC, EOI
#else /* not PSOT_LENG_FROM_BUF_END */
	if(Psot == 0) 
  { 
	Psot = (uint64_t)(box_end - s);

    snprintf(item_name, MAX_ITEMBUF,"%s/Psot(%" PRIu64 ")",name_buf,Psot);
    add_sub_item();
/*------------------
fprintf(stderr,"%s:%d:\n\tbuf_end(%p) - s(%p) ==> Psot(%lu):not used\n"
"\tbox_end(%p) - s(%p) ==> Psot(%lu)\n",__FILE__,__LINE__,
(void*)dend,(void*)s,(uint64_t)(dend - s),
(void*)box_end,(void*)s,Psot);
------------------*/
  }
#endif /* PSOT_LENG_FROM_BUF_END */

#ifdef READ_ALL_SOT_MARKERS
	s = start + len;
#else
	s = start + Psot - 2;
#endif

    snprintf(item_name, MAX_ITEMBUF,"%s/TPsot(%d)",name_buf, TPsot);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF,"%s/TNsot(%d)",name_buf, TNsot);
    add_sub_item();

	return s;
}//read_sot()

static unsigned short read_qcd(unsigned char *s, const char *name_src)
{
#ifdef QUANT_AS_STRING
	const char *cs = "";
#endif
	unsigned short max_len, i;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Quantization default marker",name_buf);
    add_sub_item();

	max_len = get2(s); s += 2;

	for(i = 0; i < Csiz; ++i)
   {
	unsigned short sqcd, v, bits;

	sqcd = s[0]; ++s;
	v = (sqcd & 0x1f); bits = (sqcd>>5);

#ifdef QUANT_AS_STRING
	if(v == 0) cs = "no";//No quantization
	else
	if(v == 1) cs = "implicit";//Scalar derived
	else
	if(v == 2) cs = "explicit";//Scalar expounded 
	else
	 cs = "unknown";

	snprintf(item_name, MAX_ITEMBUF, "%s/Sqcd[%d] quant(%s) bits(%hu)", name_buf,
		i,cs,bits);
#else
    snprintf(item_name, MAX_ITEMBUF, "%s/Sqcd[%d] q(%hu) b(%hu)", name_buf,
        i,v,bits);
#endif

	add_sub_item();
   }

	return max_len;
}//read_qcd();

static unsigned short read_epb(unsigned char *s, const char *name_src)
{
	unsigned int ldp, p;
	unsigned short len, v;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	len = get2(s); s += 2;

    snprintf(item_name, MAX_ITEMBUF, "%s/Error protection block", name_buf);
    add_sub_item();

	v = s[0]; ++s;
	snprintf(item_name, MAX_ITEMBUF, "%s/style(%hu)", name_buf, v);
	add_sub_item();

	ldp = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/length_of_data(%u)", name_buf, ldp);
	add_sub_item();

	p = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/parameters(%u)", name_buf, p);
	add_sub_item();



	return len;
}//read_epb()

static unsigned short read_epc(unsigned char *s, const char *name_src)
{
	unsigned int cl;
	unsigned short len, crc, epc;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	len = get2(s); s += 2;

    snprintf(item_name, MAX_ITEMBUF, "%s/Error protection capacity", name_buf);
    add_sub_item();

	crc = get2(s); s += 2;
    snprintf(item_name, MAX_ITEMBUF, "%s/Parity crc(%hu)", name_buf, crc);
    add_sub_item();

	cl = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/codestrem_len(%u)", name_buf, cl);
    add_sub_item();

	epc = s[0]; ++s;
    snprintf(item_name, MAX_ITEMBUF, "%s/parameter(%hu)", name_buf, epc);
    add_sub_item();

	return len;

}//read_epc()

static unsigned short read_esd(unsigned char *s, const char *name_src)
{
	unsigned short len, v;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	len = get2(s); s += 2;

    snprintf(item_name, MAX_ITEMBUF, "%s/Error sensitivity descriptor", name_buf);
    add_sub_item();

	if(Csiz < 257)
   {
	v = s[0]; ++s;
   }
	else
   {
	v = get2(s); s += 2;
   }
    snprintf(item_name, MAX_ITEMBUF, "%s/component(%hu)", name_buf, v);
    add_sub_item();

	v = s[0]; ++s;
    snprintf(item_name, MAX_ITEMBUF, "%s/usage(%hu)", name_buf, v);
    add_sub_item();

	return len;

}//read_esd()

static unsigned short read_red(unsigned char *s, const char *name_src)
{
	unsigned short len, v;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
	len = get2(s); s += 2;

    snprintf(item_name, MAX_ITEMBUF, "%s/Residual errors descriptor", name_buf);
    add_sub_item();

    v = s[0]; ++s;
    snprintf(item_name, MAX_ITEMBUF, "%s/usage(%hu)", name_buf, v);
    add_sub_item();

	return len;

}//read_red()


unsigned char *test_marker(unsigned char *s, unsigned char *box_end,
	unsigned int type)
{
	unsigned int len;
	unsigned char prefix, suffix;

	suffix = type & 0xff; 
	s += 2;

/* 0x30 : 0x3f ==> reserved, no parameters */
	if(suffix >= 0x30 && suffix <= 0x3f) return s;

	len = get2(s); s += len;
	if(s >= box_end) 
   {
	fprintf(stderr,"\t1:MARKER %#x is unknown.\n",type);
	return s;//box_end;
   }	
	type = get2(s); 

	if(s + 2 >= box_end) 
   {
	fprintf(stderr,"\t2:MARKER %#x is unknown. STOP.\n",type);
	return box_end;
   }
	prefix = (type>>8) & 0xff; suffix = type & 0xff;

	if(prefix == 0xff) return s;

	fprintf(stderr,"\t3:NEXT MARKER %#x is unknown. STOP.\n",type);
	return box_end;
}

unsigned char *read_jp2c(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned char *stop;
#ifdef DEBUG_JP2C
	unsigned char *start0 = s;
#endif
	unsigned int type, next_type, len;
	int pos = -1;
	int eoc_found = 0, sod_found = 0;
	char name_buf[MAX_ITEMBUF];
	char iname[MAX_ITEMBUF+2];

	strcpy(name_buf, name_src);
	snprintf(item_name, MAX_ITEMBUF, "%s/Contiguous codestream box",name_buf);
	add_sub_item();


	while(s < box_end)
   {
	type = get2(s);
	++pos;

#ifdef DEBUG_JP2C
fprintf(stderr,"[%03d] marker(%#x) position(%ld)\n",pos,type,(long)(s-start0));
#endif
	s += 2; len = 0;

	switch(type)
  {
    case J2K_SOC:
	   {
		Fl_Tree_Item *sup;

		snprintf(iname, MAX_ITEMBUF,"%s/[%03d] SOC",name_buf,pos);
		sup = tree->add(iname);

		snprintf(item_name, 
		 MAX_ITEMBUF, "%s/Start of codestream marker", iname);
		add_sub_item();

		sup->close();
		continue;
	   }

    case J2K_SOT:
	   {
		Fl_Tree_Item *sup;

		snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] SOT", name_buf,pos);
		sup = tree->add(item_name);

		s = read_sot(s, box_end, dend, item_name);

		sup->close();
		continue;
	   }

    case J2K_SOD:
	   {
		Fl_Tree_Item *sup;

        snprintf(iname, MAX_ITEMBUF,"%s/[%03d] SOD",name_buf,pos);
		sup = tree->add(iname);

		snprintf(item_name, MAX_ITEMBUF, "%s/Start of data marker",iname);
        add_sub_item();

		sup->close();
		sod_found = 1;

		next_type = get2(s);

		if(next_type == J2K_SOT || next_type == J2K_SOP) 
	  {
		continue;
	  }
		s = dend - 2;
		continue;
	   }

    case J2K_EOC:
	   {
		Fl_Tree_Item *sup;

        snprintf(iname, MAX_ITEMBUF,"%s/[%03d] EOC",name_buf,pos);
		sup = tree->add(iname);

		snprintf(item_name, MAX_ITEMBUF,"%s/End of codestream marker",iname);
        add_sub_item();

		sup->close();
		eoc_found = 1;
		continue;
	   }

    case J2K_SIZ:
	   {
		Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF,"%s/[%03d] SIZ",name_buf,pos);
        sup = tree->add(item_name);

		s += read_siz(s, item_name);

		sup->close();
		continue;
	   }

    case J2K_COD:
	   {
		Fl_Tree_Item *sup;

		snprintf(item_name, MAX_ITEMBUF,"%s/[%03d] COD", name_buf,pos);
		sup = tree->add(item_name);

		s += read_cod(s, item_name);

		sup->close();
		continue;
	   }

    case J2K_COC:
	   {
		Fl_Tree_Item *sup;

		snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] COC", name_buf,pos);
		sup = tree->add(item_name);
		
		s += read_coc(s, item_name);

		sup->close();

		continue;
	   }
    case J2K_RGN:
	   {
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] RGN", name_buf,pos);
        sup = tree->add(item_name); 

		s += read_rgn(s, item_name);

		sup->close();
		continue;
	   }

    case J2K_QCD:
	   {
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF,"%s/[%03d] QCD", name_buf,pos);
        sup = tree->add(item_name);

        s += read_qcd(s, item_name);

        sup->close();
		continue;
	   }

    case J2K_QCC:
	   {
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] QCC", name_buf,pos);
        sup = tree->add(item_name); 

		s += read_qcc(s, item_name);

		sup->close();
		continue;
	   }

    case J2K_POC://POD
	   {
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] POC", name_buf,pos);
        sup = tree->add(item_name); 

		s += read_poc(s, item_name);

		sup->close();
		continue;
	   }

    case J2K_TLM:
	   {
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] COC", name_buf,pos);
        sup = tree->add(item_name); 

		s += read_tlm(s, item_name);

		sup->close();
		continue;
	   }

    case J2K_PLM:
	   {
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] PLM", name_buf,pos);
        sup = tree->add(item_name); 

		s += read_plm(s, item_name);

		sup->close();
		continue;
	   }

    case J2K_PLT:
	   {
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] PLT", name_buf,pos);
        sup = tree->add(item_name); 

		s += read_plt(s, item_name);

		sup->close();
		continue;
	   }

    case J2K_PPM:
	   {
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] PPM", name_buf,pos);
        sup = tree->add(item_name);

		s += read_ppm(s, item_name);

		sup->close();
		continue;
	   }

    case J2K_PPT:
	   {
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] PPT", name_buf,pos);
        sup = tree->add(item_name);

		s += read_ppt(s, item_name);

		sup->close();
		continue;
	   }

    case J2K_SOP:
	   {
#ifdef SHOW_PACKETS
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] SOP", name_buf,pos);
        sup = tree->add(item_name);

		s += read_sop(s, item_name);

		sup->close();
#endif
		stop = box_end - 2; 
		next_type = 0;
		while(s < stop)
	   {
		if(s[0] == 0xff)
	  {
		next_type = get2(s);

		if(next_type == J2K_EPH) break;
	  }
		++s;
	   }
        continue;
	   }
		
    case J2K_EPH:
	   {
#ifdef SHOW_PACKETS
        Fl_Tree_Item *sup;

        snprintf(iname, MAX_ITEMBUF, "%s/[%03d] EPH", name_buf,pos);
        sup = tree->add(iname);

	    snprintf(item_name, MAX_ITEMBUF, "%s/End of packet marker", iname);
	    add_sub_item();

		sup->close();
#endif
		stop = box_end - 2; next_type = 0;
		while(s < stop)
	   {
		if(s[0] == 0xff)
	  {
		next_type = get2(s);
		if(next_type == J2K_SOP || next_type == J2K_SOT) break;
	  }
		++s;
	   }
        continue;
	   }

    case J2K_CRG:
	   {
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] CRG", name_buf,pos);
        sup = tree->add(item_name);

		s += read_crg(s, item_name);

		sup->close();
		continue;
	   }

    case J2K_COM://CME
	   {
        Fl_Tree_Item *sup;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] COM", name_buf,pos);
        sup = tree->add(item_name);

		s += read_com(s, item_name);

		sup->close();
		continue;
	   }

	case J2K_EPC://JPWL
	   {
		Fl_Tree_Item *sup;

		snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] EPC", name_buf,pos);
		sup = tree->add(item_name);

		s += read_epc(s, item_name);

		sup->close();
		continue;
	   }

	case J2K_EPB://JPWL
	   {
		Fl_Tree_Item *sup;

		snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] EPB", name_buf,pos);
		sup = tree->add(item_name);

		s += read_epb(s, item_name);

		sup->close();
		continue;
	   }

	case J2K_ESD://JPWL
	   {
		Fl_Tree_Item *sup;

		snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] ESD", name_buf,pos);
		sup = tree->add(item_name);

		s += read_esd(s, item_name);

		sup->close();
		continue;
	   }

	case J2K_RED://JPWL
	   {
		Fl_Tree_Item *sup;

		snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] RED", name_buf,pos);
		sup = tree->add(item_name);

		s += read_red(s, item_name);

		sup->close();
		continue;
	   }

	case J2K_SEC://JPSEC
	   {
		Fl_Tree_Item *item;

		snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] SEC", name_buf,pos);
		item = tree->add(item_name);
		item->usericon(&L_documentpixmap);

		len = get2(s); s += len;

		continue;
	   }

	case J2K_INSEC://JPSEC
	   {
		Fl_Tree_Item *item;

		snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] INSEC", name_buf,pos);
		item = tree->add(item_name);
		item->usericon(&L_documentpixmap);

		len = get2(s); s += len;

		continue;
	   }
/*---------- Part 2, Extensions ---------------*/
    case J2K_DCO:
      {
        Fl_Tree_Item *item;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] dco", name_buf,pos);
        item = tree->add(item_name);
        item->usericon(&L_documentpixmap);

        len = get2(s); s += len;

        continue;
       }

    case J2K_VMS:
      {
        Fl_Tree_Item *item;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] vms", name_buf,pos);
        item = tree->add(item_name);
        item->usericon(&L_documentpixmap);

        len = get2(s); s += len;

        continue;
       }

    case J2K_DFS:
      {
        Fl_Tree_Item *item;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] dfs", name_buf,pos);
        item = tree->add(item_name);
        item->usericon(&L_documentpixmap);

        len = get2(s); s += len;

        continue;
       }

    case J2K_ADS:
      {
        Fl_Tree_Item *item;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] ads", name_buf,pos);
        item = tree->add(item_name);
        item->usericon(&L_documentpixmap);

        len = get2(s); s += len;

        continue;
       }

    case J2K_MCT:
      {
        Fl_Tree_Item *item;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] mct", name_buf,pos);
        item = tree->add(item_name);
        item->usericon(&L_documentpixmap);

        len = get2(s); s += len;

        continue;
       }

    case J2K_MCC:
      {
        Fl_Tree_Item *item;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] mcc", name_buf,pos);
        item = tree->add(item_name);
        item->usericon(&L_documentpixmap);

        len = get2(s); s += len;

        continue;
       }

    case J2K_NLT:
      {
        Fl_Tree_Item *item;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] nlt", name_buf,pos);
        item = tree->add(item_name);
        item->usericon(&L_documentpixmap);

        len = get2(s); s += len;

        continue;
       }

    case J2K_CBD:
      {
        Fl_Tree_Item *item;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] cbd", name_buf,pos);
        item = tree->add(item_name);
        item->usericon(&L_documentpixmap);

        len = get2(s); s += len;

        continue;
       }

    case J2K_ATK://jp3d
      {
        Fl_Tree_Item *item;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] atk", name_buf,pos);
        item = tree->add(item_name);
        item->usericon(&L_documentpixmap);

        len = get2(s); s += len;

        continue;
       }

    case J2K_MIC:
      {
        Fl_Tree_Item *item;

        snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] mic", name_buf,pos);
        item = tree->add(item_name);
        item->usericon(&L_documentpixmap);

        len = get2(s); s += len;

        continue;
       }

/*--------------- end Part 2 ----------------*/

	default:
      {
        Fl_Tree_Item *item;

        snprintf(iname, MAX_ITEMBUF, "%s/[%03d] Unknown marker 0x%x", 
			name_buf,pos,type);
        item = tree->add(iname);
        item->usericon(&L_documentpixmap);

#ifdef DEBUG_DEFAULT_TEST
fprintf(stderr,"test_marker:box_end(%p) - s(%p) ==> %ld\n",
(void*)box_end,(void*)s,(long)(box_end - s));
#endif
		s = test_marker(s, box_end, type);
	   }
		break;
  }
   }

	if(sod_found && ! eoc_found)
   {
	Fl_Tree_Item *sup;

	snprintf(iname, MAX_ITEMBUF,"%s/[%03d] EOC",name_buf,pos);
	sup = tree->add(iname);

	snprintf(item_name, MAX_ITEMBUF,"%s/End of codestream marker",iname);
	add_sub_item();

	sup->close();
   }

	if((box_end - s) == 0) return box_end;
 
#ifdef DEBUG_JP2C_END
fprintf(stderr,"\n%s:%d:END OF READ_JP2C\n"
"\ts(%p) - box_end(%p) ==> %d\n",__FILE__,__LINE__,
(void*)s,(void*)box_end, (int)(s - box_end) );


fprintf(stderr,"\tBOX_END[-1:-7] %02x,%02x,%02x,%02x,%02x,%02x,%02x\n",
box_end[-1],box_end[-2],box_end[-3],box_end[-4],
box_end[-5],box_end[-6],box_end[-7]);

fprintf(stderr,"\tDEND[-1:-7] %02x,%02x,%02x,%02x,%02x,%02x,%02x\n",
dend[-1],dend[-2],dend[-3],dend[-4],dend[-5],dend[-6],dend[-7]);
#endif

	if(s <= box_end)
   {
#ifdef DEBUG_JP2C_END
	fprintf(stderr,"EXIT read_jp2c\n\tbox_end - s ==> %lu\n",
	 (unsigned long)(box_end - s));
	fprintf(stderr,"\tbox_end(%p)\n\tbuf_end(%p)\n",(void*)box_end,
	 (void*)dend);
#endif
	return box_end;
   }
#ifdef DEBUG_JP2C_END
	fprintf(stderr,"EXIT read_jp2c\n\tdend(%p) - s(%p) ==> %lu\n",
	 (void*)dend,(void*)s,(unsigned long)(dend - s));
	fprintf(stderr,"\tWARNING: s(%p) > box_end(%p) ==> %lu\n",
	 (void*)s,(void*)box_end,(unsigned long)(s - box_end));
#endif
	return box_end;

}// read_jp2c()

unsigned char *read_jp2i(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
    snprintf(item_name, MAX_ITEMBUF, "%s/Intellectual property box", name_src);
    add_sub_item();

	return box_end;
}//read_jp2i()

unsigned char *read_xml(unsigned char *s, unsigned char *box_end,
	 unsigned char *dend, char **out_buf)
{
    char *b;
	size_t len;

	assert(box_end <= dend);

    len = box_end - s;
    b = (char*)calloc(1, len+8);
	if(b == NULL) return box_end;

//FIXME	memcpy(b, (char*)s, len); b[len] = 0;
	fl_utf8toa((char*)s, len, b, len);
#ifdef SHOW_XML
fprintf(stderr,"\n%s:%d:\n\tXML(%s)\n\n",__FILE__,__LINE__,b);
#endif
//FIXME to short output
	b[32] = 0;
	*out_buf = b;

	return box_end;
}//read_xml()

unsigned char *read_url(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src)
{
    unsigned int flag, src_len, dst_len;
    unsigned short vers;
    char name_buf[MAX_ITEMBUF];
    char dst[2048];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Data Entry URL box",name_buf);
    add_sub_item();

    vers = (unsigned short)s[0]; ++s;
    flag = get_size(s, 3); s += 3;
    src_len = (unsigned int)strlen((char*)s);
    dst_len = fl_utf8toa((char*)s, src_len, dst, 2047);

    snprintf(item_name, MAX_ITEMBUF,"%s/DE vers(%hu) flag(%u) url[%u](%s)",
     name_buf, vers,flag,dst_len,dst);
    add_sub_item();

    return box_end;
}

unsigned char *read_ulst(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src)
{
    char name_buf[MAX_ITEMBUF];
    unsigned char buf[13];
    unsigned int i, n, id;

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/UUID List box",name_buf);

    buf[12] = 0;
    n = get2(s); s += 2;

    for(i = 0; i < n; ++i)
   {
    id = get4(s); s += 4; memcpy(buf, s, 12); s += 12;
    snprintf(item_name, MAX_ITEMBUF, "%s/[%u]ID(%u%s)",name_buf,i,id,buf);
    add_sub_item();
   }
    return box_end;
}

unsigned char *read_uuid(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned int len;
	char name_buf[MAX_ITEMBUF];
	char data[2048];
	char dst[2048];
	
    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, 
	 "%s/Universal unique identifier box", name_buf);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, 
	"%s/ID(%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x)",
	name_buf,
	s[0], s[1], s[2], s[3],
	s[4], s[5],
	s[6], s[7],
	s[8], s[9],
	s[10], s[11], s[12], s[13], s[14], s[15] );
	s += 16;
    add_sub_item();

	memset(data, 0, 2048);
	memset(dst, 0, 2048);

	len = (unsigned int)(box_end - s);
	if(len > 2047) len = 2047;
	memcpy(data, s, len); data[len] = 0;
	fl_utf8toa((char*)data, len, dst, len); 
#ifdef SHOW_UUID_DATA
	dst[len] = 0;
fprintf(stderr,"%s:%d:\n\tDATA(%s)\n",__FILE__,__LINE__,dst);
#else
	dst[32] = 0;
#endif
	snprintf(item_name, MAX_ITEMBUF, "%s/data(%s)",name_buf,dst);
	add_sub_item();

	return box_end;
}

unsigned char *read_uinf(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
    uint64_t box_len;
    unsigned int hstep;
    char name_buf[MAX_ITEMBUF];
    int pos = -1;

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/UUID Info box",name_buf);
    add_sub_item();

    box_len = read_boxheader(s, dend, &hstep);

    if(memcmp(box_name, "ulst", 4) == 0)
   {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] uinf", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_ulst(s + hstep, s + box_len, dend, item_name);

    sup->close();

    box_len = read_boxheader(s, dend, &hstep);
   }

    if(memcmp(box_name, "url ", 4) == 0)
   {
    Fl_Tree_Item *sup;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%03d] url", name_buf, ++pos);
    sup = tree->add(item_name);

    s = read_url(s + hstep, s + box_len, dend, item_name);

    sup->close();
   }

    return box_end;
}

static void test_ftyp(FTYPInfo *info, unsigned char *s,
    unsigned char *box_end)
{
    int i, n;
    unsigned char buf[5];

    buf[4] = 0;
    memcpy(buf, s, 4); s += 4;

#ifdef DEBUG_TEST_FTYP
fprintf(stderr,"%s:%d:\n\tFTYP BRAND(%s)\n",__FILE__,__LINE__,(char*)buf);
#endif

    if(strcasecmp((char*)buf, "jp2 ") == 0)
     info->brand_jp2 = 1;
    else
    if(strcasecmp((char*)buf, "jpx ") == 0)
     info->brand_jpx = 1;
    else
    if(strcasecmp((char*)buf, "jpm ") == 0)
     info->brand_jpm = 1;

    info->minv = get4(s); s += 4;

    n = (box_end - s)/4;

    for(i = 0; i < n; ++i)
   {
    memcpy(buf, s, 4); s += 4;
#ifdef DEBUG_TEST_FTYP
fprintf(stderr,"%s:%d:\n\tFTYP CL[%d]%s\n",__FILE__,__LINE__,i,(char*)buf);
#endif

    if(strcasecmp((char*)buf, "jp2 ") == 0)
     info->compat_jp2 = 1;
    else
    if(strcasecmp((char*)buf, "jp21") == 0)
     info->compat_jp21 = 1;
    else
    if(strcasecmp((char*)buf, "jpx ") == 0)
     info->compat_jpx = 1;
    else
    if(strcasecmp((char*)buf, "jpxb") == 0)
     info->compat_jpxb = 1;
    else
    if(strcasecmp((char*)buf, "jpm ") == 0)
     info->compat_jpm = 1;

   }
}/* test_ftyp() */

int JPEG2000_test_ftyp(const char *fname, unsigned char *s, uint64_t size,
    FTYPInfo *info)
{
    unsigned char *dend;
    const char *cs;
    uint64_t box_len;
    unsigned int hstep, found = 0;

    memset(info, 0, sizeof(struct ftyp_info));

    dend = s + size;

    if(memcmp(s, JP2_RFC3745_MAGIC, 12) == 0)
   {
    info->magic_len = 12;
   }
    else
    if(memcmp(s, JP2_MAGIC, 4) == 0)
   {
    info->magic_len = 4;
   }
    else
    if(memcmp(s, J2K_CODESTREAM_MAGIC, 4) == 0)
   {
    info->is_j2k = 1;
    info->decod_format = J2K_CFMT;

    return 1;
   }
    else
     return 0;

    s += info->magic_len;

    while(s < dend)
   {
    box_len = read_boxheader(s, dend, &hstep);

    if( box_len == 0) break;

    if(memcmp(box_name, "ftyp", 4) == 0)
  {
    test_ftyp(info, s + hstep, s + box_len);

    found = 1;
    break;
  }
    s += box_len;

   }//while(

    if(found == 0)
   {
    fprintf(stderr,"\nJPX_read_ftyp: no ftyp box found. STOP.\n\n");
    return 0;
   }

    if((cs = strrchr(fname, '.')) != NULL)
   {
    ++cs;

    if(strncasecmp(cs, "jpt", 3) == 0)
  {
    info->decod_format = JPT_CFMT;
    info->is_jpt = 1;

    return 1;
  }
   }
    info->decod_format = JP2_CFMT;

    if(info->brand_jpx)
   {
    info->is_jpx = 1;

    return 1;
   }

    if(info->brand_jp2)
   {
    info->is_jp2 = 1;

    return 1;
   }

    if(info->brand_jpm)
   {
    info->is_jpm = 1;

    return 1;
   }
    return 0;
}//JPEG2000_test_ftyp()

int JPEG2000_build_tree(const char *read_idf)
{
	unsigned char *src, *s, *box_end, *dend;
	const char *basename;
	uint64_t box_len;
#ifdef HAVE_FSEEKO
    off_t src_len;
#elif HAVE__FSEEKI64
    int64_t src_len;
#else
    long src_len;
#endif
	unsigned int hstep;
	int pos = -1;
	unsigned short is_codestream = 0, is_mj2 = 0, is_jpx = 0, is_jpm = 0;
	FILE *reader;
	char sup_name[MAX_ITEMBUF+2];

	if((reader = fopen(read_idf, "rb")) == NULL) return 0;

	(void)fread(box_name, 1, 12, reader);
	rewind(reader);

	if(memcmp(box_name, JP2_RFC3745_MAGIC, 12) == 0)
   {
	box_len = (uint64_t)12;
	strcpy((char*)box_name, "jP  ");
   }
	else
	if(memcmp(box_name, JP2_MAGIC, 4) == 0)
   {
	box_len = (uint64_t)4;
	strcpy((char*)box_name, "jP  ");
   }
	else
	if(memcmp(box_name, J2K_CODESTREAM_MAGIC, 4) == 0)
   {
	box_len = (uint64_t)0; *box_name = 0;
	is_codestream = 1;
   }
	else
   { 
	fprintf(stderr, "NOT A JPEG2000 FILE: %s\n",read_idf);
	fclose(reader);
	return 0;
   }
#ifdef _WIN32
	basename = strrchr(read_idf, '\\');
#else
	basename = strrchr(read_idf, '/');
#endif
	if(basename) ++basename; else basename = read_idf;

	if(tree != NULL)
   {
	tree->clear();
   }
	box_name[4] = 0; 

#ifdef HAVE_FSEEKO
    fseeko(reader, 0, SEEK_END);
#elif HAVE__FSEEKI64
    _fseeki64(reader, 0, SEEK_END);
#else
    fseek(reader, 0, SEEK_END);
#endif

#ifdef HAVE_FSEEKO
	src_len = ftello(reader);
#elif HAVE__FSEEKI64
	src_len = _ftelli64(reader);
#else
	src_len = ftell(reader);
#endif

#ifdef HAVE_FSEEKO
    fseeko(reader, 0, SEEK_SET);
#elif HAVE__FSEEKI64
    _fseeki64(reader, 0, SEEK_SET);
#else
    fseek(reader, 0, SEEK_SET);
#endif

	src = (unsigned char*) malloc(src_len+4);

	if(src == NULL)
   {
	fclose(reader);
	return 0;
   }
	(void)fread(src, 1, src_len, reader);

	fclose(reader);
	reader = NULL;

	init();

	memset(&jp2_color, 0, sizeof(jp2_color_t));

	s = src + (ptrdiff_t)box_len; 
	dend = src + (ptrdiff_t)src_len;
	memset(src+(ptrdiff_t)src_len,'1',4);

	if(is_codestream)
   {
    Fl_Tree_Item *sup;

    snprintf(sup_name, MAX_ITEMBUF, "[%03d] jp2c", ++pos);
    sup = tree->add(sup_name);

//-------------- J2K -----------------
	s = read_jp2c(s, dend, dend, sup_name);
//-------------- J2K -----------------
	sup->close();
   }
	else
   {
    box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) { s = dend; goto end_mark; }

    box_end = s + (ptrdiff_t)box_len;

	if(box_end >= dend) { s = dend; goto end_mark; }

    if(memcmp(box_name, "ftyp", 4) == 0)
  {
	unsigned char brand[5];

    brand[4] = 0;
    memcpy(brand, s+hstep, 4);

	if(memcmp(brand, "mjp2", 4) == 0)
	 is_mj2 = 1;
	else
	if(memcmp(brand, "jpx ", 4) == 0)
	 is_jpx = 1;
	else
	if(memcmp(brand, "jpm ", 4) == 0)
	 is_jpm = 1;
  }
	if(is_mj2)
  {
//------------------------
	read_mj2(s, dend);
//------------------------
	goto end_mark;
  }

	if(is_jpx)
  {
//------------------------
	read_jpx(src, s, dend);
//------------------------
	goto end_mark;
  }

	if(is_jpm)
  {
//------------------------
	read_jpm(src, s, dend);
//------------------------
	goto end_mark;
  }
//------- JP2, JPM -------
	while(s < dend)
  {
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) { s = dend; break; }

	if((uint64_t)(dend - s) < box_len)
	 box_end = dend;
	else
	 box_end = s + (ptrdiff_t)box_len;

	if(memcmp(box_name, "ftyp", 4) == 0)
 {
	Fl_Tree_Item *sup;

	snprintf(sup_name, MAX_ITEMBUF, "[%03d] ftyp", ++pos);
	sup = tree->add(sup_name);

	s = read_ftyp(s + hstep, s + box_len, sup_name);

	sup->close();
	continue;
 }

	if(memcmp(box_name, "jp2h", 4) == 0)
 {
	Fl_Tree_Item *sup;

	snprintf(sup_name, MAX_ITEMBUF, "[%03d] jp2h", ++pos);
	sup = tree->add(sup_name);

	s = read_jp2h(s + hstep, s + box_len, dend, sup_name);

	sup->close();
	continue;
 }

	if(memcmp(box_name, "jp2c", 4) == 0)
 {
	Fl_Tree_Item *sup;

	snprintf(sup_name, MAX_ITEMBUF, "[%03d] jp2c", ++pos);
	sup = tree->add(sup_name);
/*----------------------------------------------------------*/
	s = read_jp2c(s + hstep, s + box_len, dend, sup_name);
/*----------------------------------------------------------*/
	assert(s == box_end);

	sup->close();
	continue;
 }

	if(memcmp(box_name, "jp2i", 4) == 0)
 {
	Fl_Tree_Item *sup;

	snprintf(sup_name, MAX_ITEMBUF, "[%03d] jp2i", ++pos);
	sup = tree->add(sup_name);

	s = read_jp2i(s + hstep, s + box_len, dend, sup_name);

	sup->close();
	continue;
 }

	if(memcmp(box_name, "xml ", 4) == 0
	|| memcmp(box_name, "XML ", 4) == 0)
 {
	Fl_Tree_Item *sup, *sub;
	char *buf = NULL;

	snprintf(sup_name, MAX_ITEMBUF, "[%03d] XML", ++pos);
	sup = tree->add(sup_name);

	snprintf(sup_name, MAX_ITEMBUF, "%s/xml1", sup_name);
	sub = tree->add(sup_name);

	s = read_xml(s + hstep, s + box_len, dend, &buf);

	sub->label(buf);

	sup->close();

	free(buf);
	continue;
 }

	if(memcmp(box_name, "uuid", 4) == 0)
 {
	Fl_Tree_Item *sup;

	snprintf(sup_name, MAX_ITEMBUF, "[%03d] uuid", ++pos);
	sup = tree->add(sup_name);

	s = read_uuid(s + hstep, s + box_len, dend, sup_name);

	sup->close();
	continue;
 }

	if(memcmp(box_name, "uinf", 4) == 0)
 {
	Fl_Tree_Item *sup;

	snprintf(sup_name, MAX_ITEMBUF, "[%03d] uinf", ++pos);
	sup = tree->add(sup_name);

	s = read_uinf(s + hstep, s + box_len, dend, sup_name);

	sup->close();
	continue;
 }
 {
	 Fl_Tree_Item *sup;

	snprintf(sup_name, MAX_ITEMBUF, "[%03d] %s", ++pos, (char*)box_name);
	sup = tree->add(sup_name);

    snprintf(item_name, MAX_ITEMBUF, "%s/Not handled. Skipped.", sup_name);
    add_sub_item();

	sup->close();

 }
#ifdef PRINTF_UNKNOWN_BOX
fprintf(stderr,"%s:%d:\n\tTRACE Unknown Box name(%s) len(%ld)\n"
"\tbox_end - s(%ld) dend - s(%ld)\n",__FILE__,__LINE__,
(char*)box_name,box_len,(box_end - s),(dend - s) );
#endif
	s = box_end;

  }//while(
 
   }
end_mark:

	free(src);
	snprintf(sup_name, MAX_ITEMBUF, "%s (%" PRId64 " Byte)", basename, 
		(int64_t)src_len);
	tree->root_label(sup_name);
	tree->labelsize(TREE_LABEL_SIZE);
	tree->redraw();

	return 1;

}// JPEG2000_build_tree()
