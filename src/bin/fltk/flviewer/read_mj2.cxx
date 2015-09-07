#include <config.h>
/*
 * author(s) and license
*/
/*
 * Part 3 Motion JPEG 2000 (MJ2)
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
#endif /* _WIN32 */

#include <FL/Fl.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Pixmap.H>
#include <FL/fl_utf8.h>

#include "opj_inttypes.h"
#include "opj_stdint.h"

#include "viewerdefs.hh"
#include "tree.hh"

//-----------------------------
track_t *Tracks;
//-----------------------------
/*------------------------------*/

//#define MAX_ITEMBUF 256
static char item_name[MAX_ITEMBUF+2];

/*------------------------------*/

static unsigned char handler_type[5];
static char has_vmhd, has_smhd, has_hmhd;

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

static sample_t *new_track_samples(int max_samples)
{
	sample_t *samples;
	int i;

	assert(Tracks);
	i = Tracks[0].nr_tracks;
#ifdef DEBUG_MJ2
fprintf(stderr,"%s:%d:\n\tENTER tracks nr(%d) max(%d) samples nr(%d)\n",
__FILE__,__LINE__,i,Tracks[0].max_tracks, max_samples);
#endif

	samples = (sample_t*)calloc(1, (max_samples+1)*sizeof(sample_t));

	if(samples == NULL)
   {
	fprintf(stderr,"read_mj2.cxx:%d:\n\tmemory out\n",__LINE__);
	exit(1);
   }
	Tracks[i].samples = samples;
	Tracks[i].max_samples = max_samples;

	Tracks[0].nr_tracks += 1;
#ifdef DEBUG_MJ2
fprintf(stderr,"%s:%d:\n\tEXIT nr_tracks(%d)\n",
__FILE__,__LINE__,i+1);
#endif

	return samples;
}

#define LOG(s) fprintf(stderr,"%s:%d:\n\t%s\n",__FILE__,__LINE__,s)

static void add_sub_item()
{
    Fl_Tree_Item *sub = tree->add(item_name);
    sub->usericon(&L_documentpixmap);
}

static unsigned char *read_cprt(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src, uint64_t len)
{
	unsigned int vf;
	unsigned short lang;
	char name_buf[MAX_ITEMBUF];
	char *txt;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Copyright box", name_buf);
	add_sub_item();

	vf = get4(s); s += 4; len -= 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(0x%x)", name_buf,
        (vf>>24) & 0xff,(vf & 0x00ffffff));
	add_sub_item();

	lang = get2(s); s += 2; len -= 2;
   {
    char a, b, c;
    
    if(lang)
  {
    c = (lang>>1)&0x37; b = (lang>>6)&0x37; a = (lang>>11)&0x37;
    snprintf(item_name, MAX_ITEMBUF, "%s/language(%c%c%c)",name_buf, a, b, c);
  }
    else
    snprintf(item_name, MAX_ITEMBUF, "%s/language(0)",name_buf);

	add_sub_item();
   }
	txt = (char*)malloc(len + 1);

	if(txt != NULL)
   {
	if(memcmp(s, "\xfe\xff", 2) == 0)
  {
//UTF16BE FIXME
	memcpy(txt, s, len);
  }
	else
  {
//UTF8
	fl_utf8toa((char*)s, (unsigned int)len, txt, (unsigned int)len);
  }
    snprintf(item_name, MAX_ITEMBUF, "%s/notice(%s)", name_buf,txt);
	add_sub_item();

	free(txt); s += len;
   }

	return box_end;

}//read_cprt()
//
// Container: Movie Box (`moov') or Track Box (`trak')
//
static unsigned char *read_udta(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
    uint64_t box_len;
    unsigned int hstep;
	int pos = -1;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/User data box", name_buf);
    add_sub_item();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	while(memcmp(box_name, "cprt", 4) == 0)//zero or more
   {
	Fl_Tree_Item *sup;
    char sup_buf[MAX_ITEMBUF];

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] cprt", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_cprt(s + hstep, s + box_len, dend, sup_buf, box_len);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }

	return box_end;
}//read_udta()

static unsigned char *read_tfhd(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src, int len)
{
	unsigned int vf, tID;
	int tf_flags;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Track fragment header box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4; 
	tf_flags = (vf & 0x00ffffff);
    tID = get4(s); s += 4;
    
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(0x%x)", name_buf,
        (vf>>24) & 0xff,tf_flags);
    add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/track-ID(%u)", name_buf,tID);
    add_sub_item();

	if(len > 7)
  {
	uint64_t off;

	if(sizeof(uint64_t) == 4) return dend;

	off = get8(s); s += 8; len -= 8;

	snprintf(item_name, MAX_ITEMBUF, "%s/base-data-offset(%" PRIu64 ")",
		name_buf, off);
    add_sub_item();
  }
	if(len > 3)
  {
	unsigned int index = get4(s); s += 4; len -= 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/sample-description-index(%u)", name_buf,index);
    add_sub_item();
  }

	if(len > 3)
  {
	unsigned int dur = get4(s); s += 4; len -= 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/default-sample-duration(%u)", name_buf,dur);
    add_sub_item();
  }

	if(len > 3)
  {
	unsigned int size = get4(s); s += 4; len -= 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/default-sample-size(%u)", name_buf,size);
    add_sub_item();
  }

	if(len > 3)
  {
	unsigned int flags = get4(s); s += 4; len -= 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/default-sample-flags(%u)", name_buf,flags);
    add_sub_item();
  }

//fprintf(stderr,"%s:%d:\n\tEXIT tfhd len(%d)\n",__FILE__,__LINE__,len);
//--------------------------------
	return box_end;

}//read_tfhd()

static unsigned char *read_trun(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned int sc, f, i, vf;
	int off, tr_flags;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Track fragment run box", name_buf);
    add_sub_item();

	vf = get4(s); s += 4;
	tr_flags = (vf & 0x00ffffff);
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) tr_flags(0x%x)", name_buf,
        (vf>>24) & 0xff,tr_flags);
    add_sub_item();

	sc = get4(s); s += 4;
	off = get4(s); s += 4;
	f = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/sample-count(%u)", name_buf,sc);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/data-offset(%d)", name_buf,off);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/first-sample-flags(%u)", name_buf,f);
	add_sub_item();

	for(i = 0; i < sc; ++i)
   {
	unsigned int dur, size, f, off;

	dur = get4(s); s += 4;
	size = get4(s); s += 4;
	f = get4(s); s += 4;
	off = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/Sample[%u] duration(%u) size(%u)"
	 " flags(%u) offset(%u)", name_buf,i,dur,size,f,off);
	add_sub_item();
   }
	return box_end;
}//read_trun()

static unsigned char *read_traf(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src)
{   
    Fl_Tree_Item *sup;
	uint64_t box_len;
    unsigned int hstep;
	int len;
	int pos = -1;
    char name_buf[MAX_ITEMBUF];
    char sup_buf[MAX_ITEMBUF];
	
    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Track fragment box", name_buf);
    add_sub_item();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "tfhd", 4) != 0)
   {

LOG("tfhd missing. STOP.");

	return dend;
   }
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%3d] tfhd", name_buf, ++pos);
	sup = tree->add(sup_buf);

	len = (int)(box_len - hstep - 8);

	s = read_tfhd(s + hstep, s + box_len, dend, sup_buf, len);

	sup->close();

	if(s >= dend) return dend;

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "trun", 4) == 0)
   {
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%3d] trun", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_trun(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	if(s >= dend) return dend;
   }

    return box_end;
}//read_traf()

static unsigned char *read_mfhd(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned int vf, n;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Movie fragment header box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
    n = get4(s); s += 4;
    
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(%d)", item_name,
        (vf>>24) & 0xff,(vf & 0x00ffffff));
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/sequence-number(%u)", item_name,n);
    add_sub_item();

	return box_end;
}//read_mfhd()

static unsigned char *read_moof(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	Fl_Tree_Item *sup;
	uint64_t box_len;
	unsigned int hstep;
	int pos = -1;
	char name_buf[MAX_ITEMBUF];
	char sup_buf[MAX_ITEMBUF];

	strcpy(name_buf, (char*)name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Movie fragment box", name_buf);
    add_sub_item();
    
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "mfhd", 4) != 0)
   {

LOG("mfhd missing. STOP.");

	return dend;
   }
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] mfhd", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_mfhd(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	if(s >= dend) return dend;

	for(;;)
   {
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "traf", 4) != 0) break;

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%d] traf", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_traf(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	if(s >= dend) return dend;

   }/* for() */
	
	return box_end;

}//read_moof()

static unsigned char *read_tfra(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned int vf, tID, v, i;
	unsigned short version, traf_num, trun_num, sample_num;
	char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, 
	 "%s/Track fragment random access box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
	version = (vf>>24) & 0xff;
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%hu) flags(%d)", name_buf,
        version,(vf & 0x00ffffff));
    add_sub_item();

	tID = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/track_ID(%u)", name_buf, tID);
    add_sub_item();

	v = get4(s); s += 4;//reserved 26 Bit
	traf_num = (((v>>26)&0x3) + 1) * 8;
	trun_num = (((v>>28)&0x3) + 1) * 8;
	sample_num = (((v>>30)&0x3) + 1) * 8;

	v = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/Number of entries(%u)", name_buf, v);
    add_sub_item();
    
	for(i = 0; i < v; ++i)
   {
	if(version == 1)
  {
	uint64_t t, o;

	if(sizeof(uint64_t) == 4) return dend;

	t = get8(s); s += 8;
	o = get8(s); s += 8;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%hu] time(%" PRIu64 ")",
		name_buf, i,t);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/[%hu] moof_offset(%" PRIu64 ")",
		name_buf, i,o);
    add_sub_item();
  }
	else
  {
	unsigned int t, o;

	t = get4(s); s += 4;
	o = get4(s); s += 4;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%hu] time(%u)", name_buf, i,t);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/[%hu] moof_offset(%u)", name_buf, i,o);
    add_sub_item();
  }
    snprintf(item_name, MAX_ITEMBUF, "%s/[%hu] traf_num(%hu)", 
	 name_buf,i,traf_num);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/[%hu] trun_num(%hu)", 
	 name_buf,i,trun_num);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/[%hu] sample_num(%hu)", 
	 name_buf,i,sample_num);
    add_sub_item();
   }//for(i

	return box_end;
}//read_tfra()

static unsigned char *read_mfro(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src)
{
	unsigned int vf, v;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, 
	 "%s/Movie fragment random access box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(%d)", name_buf,
        (vf>>24) & 0xff,(vf & 0x00ffffff));
    add_sub_item();

	v = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/size(%u)", name_buf, v);
    add_sub_item();

    return box_end;
}//read_mfro()

static unsigned char *read_mfra(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep;
	int pos = -1;
	char name_buf[MAX_ITEMBUF];
	char sup_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Movie fragment random access box",
	 name_buf);
    add_sub_item();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "tfra", 4) == 0)//option, > 1
   {
	Fl_Tree_Item *sup;
	
    snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] tfra", name_buf, ++pos);
    sup = tree->add(sup_buf);

	s = read_tfra(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }

	if(memcmp(box_name, "mfro", 4) == 0)//mandatory, 1
   {
	Fl_Tree_Item *sup;
	
    snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] mfro", name_buf, ++pos);
    sup = tree->add(sup_buf);

	s = read_mfro(s + hstep, s + box_len, dend, sup_buf);

	sup->close();
   }
	else
   {

LOG("mfro missing. STOP.");

	s = dend;
   }
	return s;
}//read_mfra()

static unsigned char *read_vmhd(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	const char *cs = "";
	unsigned int vf;
	unsigned short gmode, opcolor[3];
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Video media header box", name_buf);
    add_sub_item();

	vf = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(%d)", name_buf,
        (vf>>24) & 0xff,(vf & 0x00ffffff));
    add_sub_item();

	gmode = get2(s); s += 2;

	if(gmode == 0) cs = "copy";
	else
	if(gmode == 0x24) cs = "transparent";
	else
	if(gmode == 0x100) cs = "alpha";
	else
	if(gmode == 0x101) cs = "whitealpha";
	else
	if(gmode == 0x102) cs = "blackalpha";

	snprintf(item_name, MAX_ITEMBUF, "%s/graphicsmode(%s)", name_buf, cs);
    add_sub_item();

	opcolor[0] = get2(s); s += 2;
	opcolor[1] = get2(s); s += 2;
	opcolor[2] = get2(s); s += 2;

	snprintf(item_name, MAX_ITEMBUF, "%s/opcolor(0x%x 0x%x 0x%x)", name_buf, 
		opcolor[0],opcolor[1],opcolor[2]);
    add_sub_item();

	return box_end;
}/* read_vmhd() */

static unsigned char *read_smhd(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
    char name_buf[MAX_ITEMBUF];	
	unsigned int vf;
	int bal;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Sound media header box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(%d)", name_buf,
        (vf>>24) & 0xff,(vf & 0x00ffffff));
    add_sub_item();

	bal = get2(s); s += 2;
    snprintf(item_name, MAX_ITEMBUF, "%s/balance(%u)", name_buf, bal);
    add_sub_item();

	s += 2;//reserved

	return s;
}

static unsigned char *read_hmhd(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
    char name_buf[MAX_ITEMBUF];
	unsigned int vf, maxbit, avgbit, slideavgbit;
	unsigned short maxpdu, avgpdu;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Hint media header box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(%d)", name_buf,
        (vf>>24) & 0xff,(vf & 0x00ffffff));
    add_sub_item();

	maxpdu = get2(s); s += 2;
    snprintf(item_name, MAX_ITEMBUF, "%s/max-pdu(%u)", name_buf, maxpdu);
    add_sub_item();

	avgpdu = get2(s); s += 2;
    snprintf(item_name, MAX_ITEMBUF, "%s/average-pdu(%u)", name_buf, avgpdu);
    add_sub_item();

	maxbit = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/max-bitrate(%u)", name_buf, maxbit);
    add_sub_item();

	avgbit = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/average-bitrate(%u)", 
	 name_buf, avgbit);
    add_sub_item();

	slideavgbit = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/sliding-average-bitrate(%u)",
	 name_buf,slideavgbit);
    add_sub_item();

	return box_end;
}

static unsigned char *read_dref(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	Fl_Tree_Item *sub;
    uint64_t box_len;
    unsigned int hstep;
    unsigned int vers, en, i;
    unsigned short flag;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Data reference box", name_buf);
    add_sub_item();
    
	vers = s[0]; ++s;
	flag = get_size(s, 3); s += 3;
	en = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/version(%hu) flag(%u)", 
	 name_buf, vers,flag);
	add_sub_item();
	
	snprintf(item_name, MAX_ITEMBUF, "%s/Entries(%d)", name_buf, en);
	add_sub_item();
	++en;

	for(i = 1; i < en; ++i)
   {
	unsigned char *eend, *ename;
	char ebuf[MAX_ITEMBUF*2];

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	eend = s + box_len;
	s += hstep;

	if(memcmp(box_name, "url ", 4) == 0)
  {
	vers = s[0]; ++s;
	flag = get_size(s, 3); s += 3;

	snprintf(ebuf, MAX_ITEMBUF, 
	 "%s/[%d] url: version(%hu) flag(%u)", name_buf, i, vers, flag);
    sub = tree->add(ebuf);
	sub->usericon(&L_documentpixmap);
  }
	else
	if(memcmp(box_name, "urn ", 4) == 0)
  {
	vers = s[0]; ++s;
	flag = get_size(s, 3); s += 3;
	ename = s;

	snprintf(ebuf, MAX_ITEMBUF,
	 "%s/[%d] urn: version(%hu) flag(%u) ename(%s)", 
	 name_buf, i, vers, flag, ename);
    sub = tree->add(ebuf);
	sub->usericon(&L_documentpixmap);
  }
	else
  {
	fprintf(stderr,"[%u] wrong entry name(%s)\n",i,box_name);
  }
	s = eend;

   }

	return s;
}//read_dref()

static unsigned char *read_dinf(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
    Fl_Tree_Item *sup;
	uint64_t box_len;
	unsigned int hstep;
	int pos = -1;
    char name_buf[MAX_ITEMBUF];
    char sup_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Data information box", name_buf);
    add_sub_item();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "dref", 4) != 0)
   {

LOG("dref missing. STOP.");

	return dend;
   }
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] dref", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_dref(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	return s;

}/* read_dinf() */

static unsigned char *read_stts(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src)
{
    unsigned int vf, en, i, sc;
    int sd;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Time-to-sample box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
    en = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/version(%u) flag(%u)",name_buf,
	 (vf>>24)&0xff, vf&0xffffff);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, "%s/entry-count(%u)",name_buf,en);
	add_sub_item();

    for(i = 0; i < en; ++i)
  {
    sc = get4(s); s += 4;
    sd = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/[%u] sample-count(%u) sample-delta(%d)", name_buf,i,sc,sd);
	add_sub_item();
  }
//mj2.c : mj2_tts_decompact(tk);

	return box_end;
}//read_stts()

static unsigned char *read_stsc(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
    unsigned int vf, en, i;
    unsigned int fc, spc, sdi;
	char name_buf[MAX_ITEMBUF];

	strcpy((char*)name_buf, (char*)name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Sample-to-chunk data box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
    en = get4(s); s += 4;

    snprintf(item_name, MAX_ITEMBUF, "%s/version(%u) flag(%u)",name_buf,
     (vf>>24)&0xff, vf&0xffffff);
    add_sub_item();
    
    snprintf(item_name, MAX_ITEMBUF, "%s/entry-count(%u)",name_buf,en);
    add_sub_item();
    
	++en;

    for(i = 1; i < en; ++i)
  { 
    fc = get4(s); s += 4;
    spc = get4(s); s += 4;
    sdi = get4(s); s += 4;
    
	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/[%u] first-chunk(%u) samples-per-chunk(%u)"
	 " sample-index(%u)",name_buf, i, fc,spc,sdi);
    add_sub_item();
  }
	return box_end;
}//read_stsc()

static unsigned char *read_stsz(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	sample_t *TrackSamples;
    unsigned int vf, en,sample_size;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Sample sizes box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
    sample_size = get4(s); s += 4;
    en = get4(s); s += 4;

    snprintf(item_name, MAX_ITEMBUF, "%s/version(%u) flag(%u)",name_buf,
     (vf>>24)&0xff, vf&0xffffff);
    add_sub_item();
#ifdef USE_LONG_NAME
	snprintf(item_name, MAX_ITEMBUF, "%s/sample-size(%u) sample-count(%u)",
#else
    snprintf(item_name, MAX_ITEMBUF, "%s/siz(%u) cnt(%u)",
#endif
	 name_buf,sample_size,en);
    add_sub_item();

	++en; // 1:en

	TrackSamples = new_track_samples(en);
	
    if(sample_size == 0)
   {
    unsigned int i, esize;

    for(i = 1; i < en; ++i)
  {
    esize = get4(s); s += 4;

	TrackSamples[i].size = esize;
#ifdef USE_LONG_NAME
	snprintf(item_name, MAX_ITEMBUF, "%s/[%04d] entry-size(%u)",
#else
    snprintf(item_name, MAX_ITEMBUF, "%s/[%04d] size(%u)",
#endif
     name_buf,i,esize);
    add_sub_item();
  }
   }
	else // all tracks have same size
   {
	unsigned int i;

	for(i = 1; i < en; ++i)
  {
	TrackSamples[i].size = sample_size;
#ifdef USE_LONG_NAME
	snprintf(item_name, MAX_ITEMBUF, "%s/[%04d] entry-size(%u)",
#else
    snprintf(item_name, MAX_ITEMBUF, "%s/[%04d] size(%u)",
#endif
     name_buf,i,sample_size);
    add_sub_item();
    
  }
   }
	return box_end;
}//read_stsz()

static unsigned char *read_stz2(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	sample_t *TrackSamples;
    unsigned int vf, en, v, i;
	unsigned short bits, val;
	char name_buf[MAX_ITEMBUF];

    strcpy((char*)name_buf, (char*)name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Compact sample sizes box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
	v = get4(s); s += 4;
	bits = (v>>24)&0xff;//24 Bit reserved, 8 Bit field-size
	en = get4(s); s += 4;

	if(bits != 4 && bits != 8 && bits != 16)
   {
	fprintf(stderr,"%s:%d:\n\tstz2 has wrong bits size(%hu). STOP.\n",
		__FILE__,__LINE__,bits);

	return dend;
   }
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%u) flag(%u)",name_buf,
     (vf>>24)&0xff, vf&0xffffff);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/field-size(%hu) sample-count(%u)",
     name_buf,bits,en);
    add_sub_item();

	++en;
	TrackSamples = new_track_samples(en);

	for(i = 1; i < en; ++i)
   {
	if(bits == 4)
  {
	val = (s[0]<<4) |( s[1]&0x0f); s += 2;
  }
	else
	if(bits == 8)
  {
	val = s[0]; ++s;
  }
	else
//	if(bits == 16)
  {
	val = get2(s); s += 2;
  }
    snprintf(item_name, MAX_ITEMBUF, "%s/[%04d] size(%hu)",
     name_buf,i,val);
    add_sub_item();

	TrackSamples[i].size = val;
   }//for(i

	return box_end;
}//read_stz2()

static unsigned char *read_stco(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	sample_t *TrackSamples;
    unsigned int vf, en, co, i;
	int nr_tracks;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Chunk offset box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
    en = get4(s); s += 4;
	
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%u) flag(%u)",name_buf,
     (vf>>24)&0xff, vf&0xffffff);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/entry-count(%u)",name_buf,en);
    add_sub_item();

	assert(Tracks);
	nr_tracks = Tracks[0].nr_tracks;
	TrackSamples = Tracks[nr_tracks-1].samples;
	assert(TrackSamples);

	++en;
    for(i = 1; i < en; ++i)
   { 
    co = get4(s); s += 4;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%04d] offset(%u)",
	 name_buf,i,co);
    add_sub_item();

	TrackSamples[i].pos = (int64_t)co;
   }

	return box_end;
}//read_stco()

static unsigned char *read_co64(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	sample_t *TrackSamples;
	uint64_t co;
    unsigned int vf, en, i;
	int nr_tracks;
    char name_buf[MAX_ITEMBUF];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Chunk offset box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
	en = get4(s); s += 4;

    snprintf(item_name, MAX_ITEMBUF, "%s/version(%u) flag(%u)",name_buf,
     (vf>>24)&0xff, vf&0xffffff);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/entry-count(%u)",name_buf,en);
    add_sub_item();

    assert(Tracks);
    nr_tracks = Tracks[0].nr_tracks;
    TrackSamples = Tracks[nr_tracks-1].samples;
    assert(TrackSamples);

    ++en;

    for(i = 1; i < en; ++i)
   {
    co = get8(s); s += 8;

	TrackSamples[i].pos = (int64_t)co;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%04d] offset(%" PRIu64 ")",
     name_buf,i,co);
    add_sub_item();
   }
	return box_end;
}//read_co64()

static unsigned char *read_fiel(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned short fcount, forder;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Field Coding box", name_buf);
    add_sub_item();

    fcount = (unsigned short)s[0]; ++s;
    forder = (unsigned short)s[0]; ++s;

	snprintf(item_name, MAX_ITEMBUF,
	 "%s/fieldcount(%hu)",name_buf, fcount);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF,
	 "%s/fieldorder(%hu)",name_buf, forder);
	add_sub_item();

	return box_end;
}

static unsigned char *read_jp2p(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src, uint64_t box_len)
{
    unsigned int i, n, vf;
    unsigned char brand[5];
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/MJP2 profile box", name_buf);
    add_sub_item();

    vf = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%u) flag(%u)",name_buf,
     (vf>>24)&0xff, vf&0xffffff);
    add_sub_item();

    n = (unsigned int)((box_len - 12)/4); brand[4] = 0;
    snprintf(item_name, MAX_ITEMBUF, "%s/entry-count(%u)",name_buf,n);
    add_sub_item();

    for(i = 0; i < n; ++i)
   {
    memcpy(brand, s, 4); s += 4;

	if(*brand == 0) memset(brand,' ', 4);

	snprintf(item_name, MAX_ITEMBUF, "%s/CL(%s)", name_buf,(char*)brand);
    add_sub_item();
   }
	return box_end;
}

static unsigned char *read_mjp2(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep, vf;
	int hres, vres;
	unsigned short w, h, nlen, depth;
	int pos = -1;
    char name_buf[MAX_ITEMBUF];
    char sup_buf[MAX_ITEMBUF];
	char name32[33];

    strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/MJP2 sample entry box", name_buf);
    add_sub_item();

	vf = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(%d)", name_buf,
        (vf>>24) & 0xff,(vf & 0xffffff));
    add_sub_item();

	s += 4; // FIXME: not clear Part 3, 6.2.16.1
	s += 2;//predef
	s += 2;//res
	s += 12;//predef

	w = get2(s); s += 2;
	snprintf(item_name, MAX_ITEMBUF, "%s/width(%hu)",name_buf,w);
	add_sub_item();

	h = get2(s); s += 2;
    snprintf(item_name, MAX_ITEMBUF, "%s/height(%hu)",name_buf,h);
	add_sub_item();

	hres = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/hres(%hi.%hi)",name_buf,
		(hres>>16),(hres&0xffff));
	add_sub_item();

	vres = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/vres(%hi.%hi)",name_buf,
		(vres>>16),(vres&0xffff));
	add_sub_item();
	
	s += 4;//res
	s += 2;//predef
	nlen = s[0];
	memcpy(name32, s+1, nlen); name32[nlen] = 0;
    snprintf(item_name, MAX_ITEMBUF, "%s/name[%hu](%s)",name_buf,nlen,name32);
	add_sub_item();

	s += 32;
	depth = get2(s); s += 2;
    snprintf(item_name, MAX_ITEMBUF, "%s/depth(%hu)",name_buf,depth);
	add_sub_item();

	s += 2;//predef

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

//JP2HeaderBox required:
	if(memcmp(box_name, "jp2h", 4) == 0)
   {
	Fl_Tree_Item *sup;

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] jp2h", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_jp2h(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	if(s >= dend) return s;

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }

//FieldCodingBox optional:
	if(memcmp(box_name, "fiel", 4) == 0)
   {
	Fl_Tree_Item *sup;

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] fiel", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_fiel(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }

//MJP2ProfileBox optional:
	if(memcmp(box_name, "jp2p", 4) == 0)
   {
	Fl_Tree_Item *sup;

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] jp2p", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_jp2p(s + hstep, s + box_len, dend, sup_buf, box_len);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }

//MJP2PrefixBox optional:
	if(memcmp(box_name, "jp2x", 4) == 0)
   {
	Fl_Tree_Item *sup;

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] jp2x", name_buf, ++pos);
	sup = tree->add(sup_buf);

	snprintf(item_name, MAX_ITEMBUF, "%s/data[%" PRIu64 "]", sup_buf,
	 (box_len-8));
	add_sub_item();

	sup->close();

	s += box_len;

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }

//MJP2SubSamplingBox optional:
	if(memcmp(box_name, "jsub", 4) == 0)
   {
	Fl_Tree_Item *sup;

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] jsub", name_buf, ++pos);
	sup = tree->add(sup_buf);
	
	s += hstep;

	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/hsub(%hu) vsub(%hu)", sup_buf, s[0],s[1]);
    add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/hoff(%hu) voff(%hu)", sup_buf, s[2],s[3]);
    add_sub_item();

	sup->close();

	s += 4;

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }

//MJP2OriginalFormatBox optional:
	if(memcmp(box_name, "orfo", 4) == 0)
   {
	Fl_Tree_Item *sup;

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] orfo", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s += hstep;

	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/original-fieldcount(%hu)", sup_buf, (unsigned short)s[0]);
	add_sub_item();

	snprintf(item_name, MAX_ITEMBUF, 
	 "%s/original-fieldorder(%hu)", sup_buf, (unsigned short)s[1]);
	add_sub_item();
	
	s += 2;

	sup->close();
   }

	return box_end;

}//read_mjp2(

static unsigned char *read_stsd(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int  hstep, vf, en, i;
	int pos;
	char name_buf[MAX_ITEMBUF];
	char sup_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Sample description box", name_buf);
    add_sub_item();
    
	vf = get4(s); s += 4;
	en = get4(s); s += 4;

    snprintf(item_name, MAX_ITEMBUF, "%s/version(%u) flag(%u)",name_buf,
     (vf>>24)&0xff, vf&0xffffff);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/entry-count(%u)",name_buf,en);
    add_sub_item();
    
	++en; pos = 0;

	for(i = 1; i < en; ++i)
   {
	unsigned char *bend;

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	bend = s + box_len;

#ifdef DEBUG_MJ2
fprintf(stderr,"%s:%d:handler_type(%s) <==> box_name(%s)\n",__FILE__,__LINE__,
handler_type,box_name);
#endif

	if(memcmp(handler_type, "soun", 4) == 0)
  {

fprintf(stderr,"%s:%d:\n\tAUDIO TRACK IGNORED\n",__FILE__,__LINE__);

	s = bend;
	continue;
  }

	if(memcmp(handler_type, "vide", 4) == 0)
  {
	Fl_Tree_Item *sup;

	if(memcmp(box_name, "mjp2", 4) != 0)//mj2.c : mj2_read_smj2()
 {

fprintf(stderr,"[%d]'vide' has wrong box name:(%s)\n",i,box_name);

	s = bend;
	continue;
 }

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%3d] mjp2", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_mjp2(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	if(s != bend) return dend;

	s = bend;

	continue;	
  }	/* vide */

	if(memcmp(handler_type, "hint", 4) == 0)
  {
	fprintf(stderr,"%s:%d:\n\tHINT TRACK IGNORED\n",__FILE__,__LINE__);
	s = bend;
	continue;
  }
	s = bend;
   }/* for(i) */

	return s;

}//read_stsd()

static unsigned char *read_stbl(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	Fl_Tree_Item *sup;
	uint64_t box_len;
	unsigned int hstep;
	int pos = -1;
	char name_buf[MAX_ITEMBUF];
	char sup_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Sample table box", name_buf);
    add_sub_item();
    
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "stsd", 4) != 0)
   {

LOG("stsd missing. STOP.");

	return dend;
   }
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] stsd", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_stsd(s + hstep, s + box_len, dend, sup_buf);

	sup->close();
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "stts", 4) != 0)
   {

LOG("stts missing. STOP.");

	return dend;
   }
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] stts", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_stts(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "stsc", 4) != 0)
   {

LOG("stsc missing. STOP.");

	return dend;
   }
    snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] stsc", name_buf, ++pos);
    sup = tree->add(sup_buf);

    s = read_stsc(s + hstep, s + box_len, dend, sup_buf);
    sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "stsz", 4) == 0)
   {
    snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] stsz", name_buf, ++pos);
    sup = tree->add(sup_buf);

    s = read_stsz(s + hstep, s + box_len, dend, sup_buf);

    sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }
	else
	if(memcmp(box_name, "stz2", 4) == 0)
   {
    snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] stz2", name_buf, ++pos);
    sup = tree->add(sup_buf);

    s = read_stz2(s + hstep, s + box_len, dend, sup_buf);

    sup->close();

	if(s == dend) return s;

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }
	else
   {

LOG("stsz/stz2 missing. STOP.");

	return dend;

   }

	if(memcmp(box_name, "stco", 4) == 0)
   {
    snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] stco", name_buf, ++pos);
    sup = tree->add(sup_buf);

    s = read_stco(s + hstep, s + box_len, dend, sup_buf);

    sup->close();
   }
	else
	if(memcmp(box_name, "co64", 4) == 0)
   {
	if(sizeof(uint64_t) == 4) return dend;

    snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] co64", name_buf, ++pos);
    sup = tree->add(sup_buf);

    s = read_co64(s + hstep, s + box_len, dend, sup_buf);

    sup->close();
   }
	else
   {

LOG("stco/co64 missing. STOP.");

	return dend;
   }
	return box_end;
}/* read_stbl() */

static unsigned char *read_elst(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	char name_buf[MAX_ITEMBUF];
	unsigned int vf, en,i;
	unsigned short version, irate, frate;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Edit list box", name_buf);
    add_sub_item(); 

	vf = get4(s); s += 4;
	version = (vf>>24) & 0xff;
	en = get4(s); s += 4;

    snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(0x%x)", name_buf,
		version, (vf & 0xffffff) );
    add_sub_item();
    
    snprintf(item_name, MAX_ITEMBUF, "%s/entry-count(%u)", name_buf,en);
    add_sub_item(); 

	for(i = 0; i < en; ++i)
   {
	if(version == 1)
  {
    uint64_t d, t;

	if(sizeof(uint64_t) == 4) return dend;

    d = get8(s); s += 8;
    t = get8(s); s += 8;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%u] segment-duration(%" PRIu64 ")",
		name_buf,i,d);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/[%u] media-time(%" PRIu64 ")",
		name_buf,i,t);
    add_sub_item();
  }
    else
  {
    unsigned int d, t;

    d = get4(s); s += 4;
    t = get4(s); s += 4;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%u] segment-duration(%u)", 
	 name_buf,i,d);
    add_sub_item();

    snprintf(item_name, MAX_ITEMBUF, "%s/[%u] media-time(%d)", name_buf,i,t);
    add_sub_item();
  }
	irate = get2(s); s += 2;
	frate = get2(s); s += 2;

    snprintf(item_name, MAX_ITEMBUF, "%s/[%u] media-rate(%hu.%hu)", name_buf,
		i,irate,frate);
    add_sub_item();
   }//for(i

	return box_end;
}//read_elst()

static unsigned char *read_mdhd(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	unsigned int vf;
	unsigned short lang, version;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Media header box", name_buf);
    add_sub_item();
    
	vf = get4(s); s += 4;
	version = (vf>>24)&0xff;
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%hu) flags(%u)", name_buf, 
		version,(vf&0xffffff));
    add_sub_item();

	if(version == 0)
   {
	unsigned int v;

    v = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/creation-time(%u)", name_buf, v);
    add_sub_item();

	v = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/modification-time(%u)", name_buf, v);
    add_sub_item();

	v = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/timescale(%u)",name_buf, v);
    add_sub_item();

	v = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/duration(%u)",name_buf, v);
    add_sub_item();
   }
	else
   {
    uint64_t v;

	if(sizeof(uint64_t) == 4) return dend;
 
    v = get8(s); s += 8;
    snprintf(item_name, MAX_ITEMBUF, "%s/creation-time(%" PRIu64 ")", 
	 name_buf, v);
    add_sub_item();
    
    v = get8(s); s += 8;
    snprintf(item_name, MAX_ITEMBUF, "%s/modification-time(%" PRIu64 ")",
		name_buf, v);
    add_sub_item();
    
    v = get8(s); s += 8;
    snprintf(item_name, MAX_ITEMBUF, "%s/timescale(%" PRIu64 ")",name_buf, v);
    add_sub_item();

    v = get8(s); s += 8;
    snprintf(item_name, MAX_ITEMBUF, "%s/duration(%" PRIu64 ")",name_buf, v);
    add_sub_item();
   }

	lang = get2(s); s += 2;
   {
	char a, b, c;

	if(lang)
  {
	c = (lang>>1)&0x37; b = (lang>>6)&0x37; a = (lang>>11)&0x37;
	snprintf(item_name, MAX_ITEMBUF, "%s/language(%c%c%c)",name_buf, a, b, c);
  }
	else
	snprintf(item_name, MAX_ITEMBUF, "%s/language(0)",name_buf);

    add_sub_item();
   }
	s += 2;//pre-defined

	return box_end;
}//read_mdhd()

static unsigned char *read_hdlr(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	char *hname;
	unsigned int vf;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Handler reference box", name_buf);
    add_sub_item();

	vf = get4(s); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(%d)", name_buf,
        (vf>>24) & 0xff,(vf & 0x00ffffff));
    add_sub_item();

	s += 4;//pre-defined

	handler_type[4] = 0; memcpy(handler_type, s, 4); s += 4;
    snprintf(item_name, MAX_ITEMBUF, "%s/handler-type(%s)", 
	 name_buf, (char*)handler_type);
    add_sub_item();
	
	s += 12;//reserved

	hname = strdup((char*)s); s += strlen(hname) + 1;

	if(hname != NULL)
   {
	snprintf(item_name, MAX_ITEMBUF, "%s/name(%s)", name_buf, hname);
	add_sub_item();

	free(hname);
   }
	return box_end;
}//read_hdlr()

static unsigned char *read_minf(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	Fl_Tree_Item *sup;
	uint64_t box_len;
	unsigned int hstep;
	int pos = -1;
	char name_buf[MAX_ITEMBUF];
	char sup_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Media information box", name_buf);
    add_sub_item();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	has_vmhd = has_smhd = has_hmhd = 0;

	if(memcmp(box_name, "vmhd", 4) == 0)
   {
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] vmhd", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_vmhd(s + hstep, s + box_len, dend, sup_buf);

	sup->close();
	has_vmhd = 1;
   }
	else
	if(memcmp(box_name, "smhd", 4) == 0)
   {
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] smhd", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_smhd(s + hstep, s + box_len, dend, sup_buf);

	sup->close();
	has_smhd = 1;
   }
	else
	if(memcmp(box_name, "hmhd", 4) == 0)
   {
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] hmhd", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_hmhd(s + hstep, s + box_len, dend, sup_buf);

	sup->close();
	has_hmhd = 1;
   }
	else
   {

LOG("neither vmhd nor smhd nor hmhd found. STOP.");

	return dend;
   }
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "dinf", 4) != 0)
   {

LOG("dinf missing. STOP.");

	return dend;
   }

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] dinf", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_dinf(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "stbl", 4) != 0)
   {

LOG("stbl missing. STOP.");

	return dend;
   }

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] stbl", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_stbl(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	return s;
}

static unsigned char *read_mdia(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	Fl_Tree_Item *sup;
	uint64_t box_len;
	unsigned int hstep;
	char name_buf[MAX_ITEMBUF];
	char sup_buf[MAX_ITEMBUF];
	int pos = -1;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Media box", name_buf);
    add_sub_item();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "mdhd", 4) != 0)
   {
LOG("mdhd missing. STOP.");

	return dend;
   }
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] mdhd", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_mdhd(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "hdlr", 4) != 0)
   {

LOG("hdrl missing. STOP.");

	return dend;
   }

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] hdlr", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_hdlr(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "minf", 4) != 0)
   {
LOG("minf missing. STOP.");

	return dend;
   }
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] minf", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_minf(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	return s;

}/* read_mdia() */

static unsigned char *read_tkhd(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src)

{
    unsigned int vf, val;
    unsigned int ct, mt, curID, dur, max_w, max_h;
    short layer, vol, i;
    char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Track header box", name_buf);
    add_sub_item();
    
	vf = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(%d)", name_buf, 
		(vf>>24) & 0xff,(vf & 0x00ffffff));
	add_sub_item();
	
    ct = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/creation-time(%u)", name_buf, ct);
	add_sub_item();

    mt = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/modification-time(%u)", name_buf, mt);
	add_sub_item();

    curID = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/track-ID(%u)", name_buf, curID);
	add_sub_item();

    s += 4;//reserved

    dur = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/duration(%u)", name_buf, dur);
	add_sub_item();

	s += 8;//reserved

    layer = get2(s); s += 2;
	snprintf(item_name, MAX_ITEMBUF, "%s/layer(%d)", name_buf, layer);
	add_sub_item();

	s += 2;//pre-defined

	vol = get2(s); s += 2;
	snprintf(item_name, MAX_ITEMBUF, "%s/volume(%d.%d)", name_buf, 
	 (vol>>8), (vol & 0xff));
	add_sub_item();

	s += 2;//reserved

	for(i = 0; i < 9; ++i)
   {
	val = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/matrix[%d](%d)",name_buf, i,val);
	add_sub_item();
   }

	max_w = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/width(%u.%u)", name_buf, 
		(max_w>>16),(max_w & 0xffff));
	add_sub_item();

	max_h = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/height(%u.%u)", name_buf, 
		(max_h>>16),(max_h & 0xffff));
	add_sub_item();

	if(Tracks == NULL)
   {
	Tracks = (track_t*)calloc(1, 4 * sizeof(track_t));

	if(Tracks == NULL)
  {
	fprintf(stderr,"read_mj2.cxx:%d:\n\tmemory out\n",__LINE__);
	exit(1);
  }
	Tracks[0].max_tracks = 4;
   }
	i = curID - 1;
	assert(i < Tracks[0].max_tracks);
	Tracks[i].duration = dur;
	Tracks[i].width = (unsigned int)(max_w>>16);
	Tracks[i].height = (unsigned int)(max_h>>16);

	return box_end;

}//read_tkhd()

static unsigned char *read_trak(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	Fl_Tree_Item *sup;
	uint64_t box_len;
	unsigned int hstep;
	char name_buf[MAX_ITEMBUF];
	char sup_buf[MAX_ITEMBUF];
	int pos = -1;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Track box", name_buf);
    add_sub_item();
	
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "tkhd", 4) != 0)
   {
LOG("tkhd missing. STOP.");

	return dend;
   }
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] tkhd", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_tkhd(s + hstep, s + box_len, dend, sup_buf);
 
	sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "tref", 4) == 0)
   {
	unsigned char *boxend = s + box_len;
	int val;

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] tref", name_buf, ++pos);
	sup = tree->add(sup_buf);

    val = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/reference-type(%d)", 
	 sup_buf, val);
	add_sub_item();

	sup->close();

	s = boxend;

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }

	if(memcmp(box_name, "edts", 4) == 0)
   {
	Fl_Tree_Item *edts_sub;
	unsigned char *boxend = s + box_len;

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] edts",name_buf, ++pos);
	edts_sub = tree->add(sup_buf);

	s = boxend;

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "elst", 4) == 0)
  {
	Fl_Tree_Item *elst_sub;
	unsigned char *boxend = s + box_len;
	char sub_buf[MAX_ITEMBUF];

	snprintf(sub_buf, MAX_ITEMBUF, "%s/[%03d] elst", sup_buf,++pos);
	elst_sub = tree->add(sub_buf);

	s = read_elst(s, box_end, dend, sub_buf);

	elst_sub->close();

	s = boxend;
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
  }
	edts_sub->close();
   }

	if(memcmp(box_name, "mdia", 4) != 0)
   {

LOG("mdia missing. STOP.");

	return dend;
   }
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] mdia", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_mdia(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	return s;
}/* read_trak() */

static unsigned char *read_mvex(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	uint64_t box_len;
	unsigned int hstep;
	unsigned int vf, tID, di, dur, size, flags;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Movie extends box", name_buf);
    add_sub_item();
	
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "mehd", 4) == 0)
   {
fputs("--- FOUND mehd ---\n", stderr);
	s += box_len;

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }
	if(memcmp(box_name, "trex", 4) != 0)
   {

LOG("trex missing. STOP.");

	return dend;
   }
	s += hstep;
	vf = get4(s); s += 4;
	tID = get4(s); s += 4;
	di = get4(s); s += 4;
	dur = get4(s); s += 4;
	size = get4(s); s += 4;
	flags = get4(s); s += 4;

fprintf(stderr,"trex\n\t"
"vf(%hi,%hi) trackID(%u) index(%u) dur(%u) size(%u) flags(%u)\n",
(vf>>16),(vf&0xffff),tID,di,dur,size,flags);

	return box_end;
}//read_mvex()

static unsigned char *read_mvhd(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src)
{
	unsigned int vf, ct, mt, ts, dur, rate, nextID, val;
	short vol, i;
	char name_buf[MAX_ITEMBUF];

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Movie header box", name_buf);
    add_sub_item();
    
	vf = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/version(%d) flags(%d)",name_buf, 
		(vf>>24) & 0xff,(vf & 0x00ffffff));
	add_sub_item();

	ct = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/creation-time(%u)",name_buf, ct);
	add_sub_item();

	mt = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/modification-time(%u)",name_buf, mt);
	add_sub_item();

	ts = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/timescale(%u)",name_buf, ts);
	add_sub_item();

	dur = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/duration(%u)",name_buf, dur);
	add_sub_item();

	rate = get4(s); s += 4;
	snprintf(item_name, MAX_ITEMBUF, "%s/rate(%u.%u)",name_buf, 
		(rate>>16),(rate & 0xffff));
	add_sub_item();

	vol = get2(s); s += 2;
	snprintf(item_name, MAX_ITEMBUF, "%s/volume(%u.%u)",name_buf, 
		(vol>>8),(vol & 0xff));
	add_sub_item();

	s += 10;//reserved(2) + reserved(8)

	for(i = 0; i < 9; ++i)
   {
	val = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/matrix[%d](%d)",name_buf, i,val);
	add_sub_item();
   }

	s += 24; //uint32  predefined[6]
	nextID = get4(s); s += 4;

	snprintf(item_name, MAX_ITEMBUF, "%s/next-track-ID(%u)",name_buf,nextID);
	add_sub_item();

	if(Tracks == NULL)
   {
    Tracks = (track_t*)calloc(1, nextID * sizeof(track_t));

	if(Tracks == NULL)
  {
	fprintf(stderr,"read_mj2.cxx:%d:\n\tmemory out\n",__LINE__);
	exit(1);
  }
	Tracks[0].max_tracks = nextID;
   }
    Tracks[0].duration = dur;

	return box_end;

}//read_mvhd()

static unsigned char *read_moov(unsigned char *s, unsigned char *box_end,
	unsigned char *dend, const char *name_src)
{
	Fl_Tree_Item *sup;
	uint64_t box_len;
	unsigned int hstep;
	char name_buf[MAX_ITEMBUF];
	char sup_buf[MAX_ITEMBUF];
	int pos = -1;

	strcpy(name_buf, name_src);
    snprintf(item_name, MAX_ITEMBUF, "%s/Movie box", name_buf);
    add_sub_item();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "mvhd", 4) != 0) 
   {

LOG("mvhd missing. STOP.");

	return dend;
   }

	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] mvhd", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_mvhd(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "udta", 4) == 0)//optional
   {
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] udta", name_buf,++pos);
	sup = tree->add(sup_buf);

	s = read_udta(s + hstep, s + box_len, dend, sup_buf);

	sup->close();
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
   }

	if(memcmp(box_name, "trak", 4) != 0) 
   {

LOG("trak missing. STOP.");

	return dend;
   }

	for(;;)
   {
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] trak", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_trak(s + hstep, s + box_len, dend, sup_buf);

	sup->close();

	if(s >= dend) break;

	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

	if(memcmp(box_name, "mvex", 4) == 0)
  {
	snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] mvex", name_buf, ++pos);
	sup = tree->add(sup_buf);

	s = read_mvex(s + hstep, s + box_len, dend, sup_buf);

	sup->close();
  }
	box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;

    if(memcmp(box_name, "udta", 4) == 0)//optional
  {
    snprintf(sup_buf, MAX_ITEMBUF, "%s/[%03d] udta", name_buf, ++pos);
    sup = tree->add(sup_buf);

    s = read_udta(s + hstep, s + box_len, dend, sup_buf);

    sup->close();
    box_len = read_boxheader(s, dend, &hstep);

	if(box_len == 0) return dend;
  }

	if(memcmp(box_name, "trak", 4) != 0) break;

   }
	return s;
}/* read_moov() */

int read_mj2(unsigned char *s, unsigned char *dend)
{
	unsigned char *box_end;
	uint64_t box_len;
	unsigned int hstep, nr_moov;
	int pos = -1;

	Tracks = NULL;
	nr_moov = 0;

	while(s < dend)
   {
	++pos;
	box_len = read_boxheader(s, dend, &hstep);

	if( box_len == 0) { s = dend; break; }

    if((uint64_t)(dend - s) < box_len)
     box_end = dend;
    else
     box_end = s + (ptrdiff_t)box_len;

	if(memcmp(box_name, "ftyp", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] ftyp", pos);
	sup = tree->add(item_name);

	s = read_ftyp(s + hstep, s + box_len, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "moov", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] moov", pos);
	sup = tree->add(item_name);

	s = read_moov(s + hstep, s + box_len, dend, item_name);

	sup->close();
	++nr_moov;
	continue;
  }

	if(memcmp(box_name, "moof", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] moof", pos);
	sup = tree->add(item_name);

	s = read_moof(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "mfra", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] mfra", pos);
	sup = tree->add(item_name);

	s = read_mfra(s + hstep, s + box_len, dend, item_name);

	sup->close();
	continue;
  }

	if(memcmp(box_name, "mdat", 4) == 0)
  {
	Fl_Tree_Item *sup;

	snprintf(item_name, MAX_ITEMBUF, "[%03d] mdat", pos);
	sup = tree->add(item_name);

	snprintf(item_name, MAX_ITEMBUF, "[%03d] mdat/Media data box", pos);
	add_sub_item();

	sup->close();

	s = box_end;
	continue;
  }

	if((memcmp(box_name, "free", 4) == 0) 
	|| (memcmp(box_name, "skip", 4) == 0))
  {
	Fl_Tree_Item *sup;

	sup = tree->add((char*)box_name);

	snprintf(item_name, MAX_ITEMBUF, "%s/%" PRIu64 " Byte skipped", 
		box_name, box_len);
	add_sub_item();

	sup->close();	
	s = box_end;
	continue;
  }
  {
	snprintf(item_name, MAX_ITEMBUF, "Unknown box %s", (char*)box_name);
	add_sub_item();
  }
	s = box_end;
   }

   {
	int64_t delta = (dend-s);

	if(delta != 0)
  {
	fprintf(stderr,"EXIT read_mj2 with delta  ==> %" PRId64 "\n",delta );
  }
   }

	return 1;
}/* read_mj2() */
