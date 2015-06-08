#ifndef _FLIMAGE_OPJ_FILE_LANG_H_
#define _FLIMAGE_OPJ_FILE_LANG_H_

//static const char _s[]={""};
#ifdef WITH_ENGLISH
static const char J2K_DECODE_FAILS_s[]={"Got no J2K image for"};
static const char JP2_DECODE_FAILS_s[]={"Got no JP2 image for"};
static const char JPT_DECODE_FAILS_s[]={"Got no JPT image for"};
static const char UNKNOWN_FILE_TYPE_s[]={"Can not decode file"};
static const char FSEEK_FAILS_s[]={"FSEEK fails for file\n%s\nwith %s"};
static const char FTELL_FAILS_s[]={"FTELL fails for file\n%s\nwith %s"};
static const char INCORRECT_FILE_EXT_s[]=
 {"The extension of the file\n%s\nis incorrect.\nFOUND '%s'. SHOULD BE %s"};
static const char FILENAME_TOO_LONG_s[]={"filename too long. Ignored."};
static const char FILESIZE_TOO_LONG_s[]={"filesize too long. Ignored."};
static const char DST_DID_NOT_OPEN_s[]={"Destination %s\n\tdid not open"};
static const char WRITE_JP2_FAILS_s[]={"Writing JP2 file fails"};
static const char GOT_NO_IMAGE_s[]={"opj_image_create() failed"};
static const char WRONG_DST_EXT_s[]=
 {"Destination file %s\nhas wrong extension"};
static const char INCORRECT_FILE_EXT_s[]=
 {"The extension of the file\n%s\nis incorrect.\nFOUND '%s'. SHOULD BE %s"};

#endif

#ifdef WITH_GERMAN
static const char J2K_DECODE_FAILS_s[]={"J2K Dekodierungsfehler für"};
static const char JP2_DECODE_FAILS_s[]={"JP2 Dekodierungsfehler für"};
static const char JPT_DECODE_FAILS_s[]={"JPT Dekodierungsfehler für"};
static const char UNKNOWN_FILE_TYPE_s[]={"Kann diese Datei nicht dekodieren"};
static const char FSEEK_FAILS_s[]={"FSEEK scheitert für Datei\n%s\nmit %s"};
static const char FTELL_FAILS_s[]={"FTELL scheitert für Datei\n%s\nmit %s"};
static const char INCORRECT_FILE_EXT_s[]=
 {"Die Endung der Datei\n%s\nist falsch.\nGEFUNDEN '%s'. SOLLTE SEIN %s"};
static const char FILENAME_TOO_LONG_s[]={"Datei-Name zu lang. Ignoriert."};
static const char FILESIZE_TOO_LONG_s[]={"Datei ist zu groß. Ignoriert."};
static const char DST_DID_NOT_OPEN_s[]={"Ziel-Datei %s\n\töffnet nicht"};
static const char WRITE_JP2_FAILS_s[]={"JP2-Datei läßt sich nicht schreiben"};
static const char GOT_NO_IMAGE_s[]={"opj_image_create() scheitert"};
static const char WRONG_DST_EXT_s[]=
 {"Ziel-Datei %s\nhat falsche Extension"};
static const char INCORRECT_FILE_EXT_s[]=
 {"Die Endung der Datei\n%s\nist falsch.\nGEFUNDEN '%s'. SOLLTE SEIN %s"};

#endif

#endif /* _FLIMAGE_OPJ_FILE_LANG_H_ */
