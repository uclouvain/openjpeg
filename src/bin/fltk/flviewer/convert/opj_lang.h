#ifndef _FLIMAGE_OPJ_FILE_LANG_H_
#define _FLIMAGE_OPJ_FILE_LANG_H_

//static const char _s[]={""};
#ifdef WITH_ENGLISH
static const char JPEG2000_DECODE_FAILS_s[]={"Got no JPEG2000 image for"};
static const char DST_DID_NOT_OPEN_s[]={"Destination %s\n\tdid not open"};
static const char WRITE_JPEG2000_FAILS_s[]={"Writing JPEG2000 file fails"};
static const char GOT_NO_IMAGE_s[]={"opj_image_create() failed"};
static const char WRONG_DST_EXT_s[]=
 {"Destination file %s\nhas wrong extension"};
static const char WRITE_JP2_FAILS_s[]={"Writing JP2 file fails"};
static const char JP2_DECODE_FAILS_s[]={"Got no JP2 image for"};

#endif

#ifdef WITH_GERMAN
static const char JPEG2000_DECODE_FAILS_s[]={"JPEG2000 Dekodierungsfehler f�r"};
static const char DST_DID_NOT_OPEN_s[]={"Ziel-Datei %s\n\t�ffnet nicht"};
static const char WRITE_JPEG2000_FAILS_s[]=
 {"JPEG2000-Datei l��t sich nicht schreiben"};
static const char GOT_NO_IMAGE_s[]={"opj_image_create() scheitert"};
static const char WRONG_DST_EXT_s[]=
 {"Ziel-Datei %s\nhat falsche Extension"};
static const char WRITE_JP2_FAILS_s[]={"JP2-Datei l��t sich nicht schreiben"};
static const char JP2_DECODE_FAILS_s[]={"JP2 Dekodierungsfehler f�r"};

#endif

#endif /* _FLIMAGE_OPJ_FILE_LANG_H_ */