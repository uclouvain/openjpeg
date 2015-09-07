#ifndef _FLIMAGE_MJ2_FILE_LANG_H_
#define _FLIMAGE_MJ2_FILE_LANG_H_

//static const char _s[]={""};
#ifdef WITH_ENGLISH
static const char MJ2_NO_FRAMES_FROM_s[]={"MJ2:Can not extract frames from"};
static const char MJ2_SHOW_FAILS_s[]=
 {"MJ2:Can not show file\n%s\n samples(%d) width(%d) height(%d)"};
#endif

#ifdef WITH_GERMAN
static const char MJ2_NO_FRAMES_FROM_s[]={"MJ2:kann nichts finden in"};
static const char MJ2_SHOW_FAILS_s[]=
 {"MJ2:kann Datei nicht zeigen f�r\n%s\n Bilder(%d) Weite(%d) H�he(%d)"};
#endif

#endif /* _FLIMAGE_MJ2_FILE_LANG_H_ */
