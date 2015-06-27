#ifndef _FLVIEWER_TREE_HH_
#define _FLVIEWER_TREE_HH_

#include <FL/Fl_Tree.H>

#define MAX_ITEMBUF 254

class UserTree : public Fl_Tree
{
public:
    UserTree(int xx, int yy, int ww, int hh)
    : Fl_Tree(xx, yy, ww, hh, NULL)
    { }
    virtual int handle(int event);
};

extern UserTree *tree;

extern unsigned char box_name[33];

typedef struct jp2_cdef_info
{
    unsigned short cn, typ, asoc;
} jp2_cdef_info_t;

typedef struct jp2_cdef
{
    jp2_cdef_info_t *info;
    unsigned short n;
} jp2_cdef_t;

typedef struct jp2_cmap_comp
{
    unsigned short cmp;
    unsigned char mtyp, pcol;
} jp2_cmap_comp_t;

typedef struct jp2_pclr
{
    unsigned int *entries;
    unsigned char *channel_sign;
    unsigned char *channel_size;
    jp2_cmap_comp_t *cmap;
    unsigned short nr_entries;
    unsigned char nr_channels;
} jp2_pclr_t;

typedef struct jp2_color
{
    unsigned char *icc_profile_buf;
    unsigned int icc_profile_len;

    jp2_cdef_t *jp2_cdef;
    jp2_pclr_t *jp2_pclr;
    unsigned short has_colr, has_cdef;
	unsigned int enumcs, meth, precedence, approx;
} jp2_color_t;

typedef struct sample
{
    int64_t pos;
    unsigned int size;
} sample_t;

typedef struct track
{
	sample_t *samples;
	int max_samples;
	int nr_tracks;
	int max_tracks;

	unsigned int duration;
	unsigned int width;
	unsigned int height;
} track_t;

extern track_t *Tracks;

typedef struct ftyp_info
{
    unsigned int magic_len;
    int decod_format;
    unsigned int minv;
    unsigned char is_j2k, is_jp2, is_jpx, is_jpm, is_jpt;

    unsigned char brand_jp2, brand_jpx, brand_jpm;

    unsigned char compat_jp2, compat_jp21;

    unsigned char compat_jpx, compat_jpxb;

    unsigned char compat_jpm;

}FTYPInfo;

//--- PROTOTYPES

extern int JPEG2000_test_ftyp(const char *fname, unsigned char *s,
	uint64_t size, FTYPInfo *info);
extern int JPEG2000_build_tree(const char *read_idf);

extern int read_mj2(unsigned char *parse_start, unsigned char *dend);
extern int read_jpx(unsigned char *src, unsigned char *parse_start,
	unsigned char *dend);
extern int read_jpm(unsigned char *src, unsigned char *parse_start,
	unsigned char *dend);

extern uint64_t read_boxheader(unsigned char *box, unsigned char *dend,
    unsigned int *out_siz);

extern uint64_t get8(const unsigned char *b);
extern unsigned int get4(const unsigned char *b);
extern unsigned int get2(const unsigned char *b);
extern unsigned int get_size(const unsigned char *b, unsigned int size);

extern unsigned char *read_resc(unsigned char *s, unsigned char *box_end,
    const unsigned char *name_src);
extern unsigned char *read_resd(unsigned char *s, unsigned char *box_end,
    const unsigned char *name_src);
extern unsigned char *read_res(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const unsigned char *name_src);

extern unsigned char *read_ftyp(unsigned char *s, unsigned char *box_end,
    const char *name_src);
extern unsigned char *read_jp2h(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src);

extern unsigned char *read_ihdr(unsigned char *s, unsigned char *box_end,
	const char *name_src);
extern unsigned char *read_bpcc(unsigned char *s, unsigned char *box_end,
    const char *name_src);

extern unsigned char *read_pclr(unsigned char *s, unsigned char *box_end,
    unsigned int *out_channels, const char *name_src);
extern unsigned char *read_cmap(unsigned char *s, unsigned char *box_end,
    unsigned int nr_channels, const char *name_src);
extern unsigned char *read_colr(unsigned char *s, unsigned char *box_end,
    const char *name_src);

extern unsigned char *read_cdef(unsigned char *s, unsigned char *box_end,
    const char *name_src);
extern unsigned char *read_xml(unsigned char *s, unsigned char *box_end,
     unsigned char *dend, char **out_buf);

extern unsigned char *read_uuid(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src);
extern unsigned char *read_uinf(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src);
extern unsigned char *read_url(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src);

extern unsigned char *read_ulst(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src);


extern unsigned char *read_jp2c(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src);
extern unsigned char *read_jp2i(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src);
extern unsigned char *read_dtbl(unsigned char *s, unsigned char *box_end,
    unsigned char *dend, const char *name_src);


extern void FLViewer_set_max_mj2_samples(unsigned int n);
extern void FLViewer_set_max_components(unsigned int n);
extern void FLViewer_set_max_layers(unsigned int n);

extern int FLViewer_max_mj2_samples();
extern void FLViewer_set_max_mj2_tracks(unsigned int n);

extern int FLViewer_max_layers();
extern int FLViewer_layers();

extern int FLViewer_max_components();
extern int FLViewer_component();

extern void FLViewer_set_max_tiles(int mx_tiles);
extern void FLViewer_set_max_reduction(int mx_reduct);

extern unsigned char *OPENJPEG_mj2_decode(const char *read_idf,
    unsigned char *src, uint64_t size, int *out_selected);


#endif /* _FLVIEWER_TREE_HH_ */
