#ifndef _FLIMAGE_FILE_LANG_H_
#define _FLIMAGE_FILE_LANG_H_
//static const char _s[]={""};
#ifdef WITH_ENGLISH
static const char CANNOT_USE_FILE_s[]= {"Can not use file"};
static const char NO_DRIVER_FOUND_s[]= {"NO DRIVER FOUND"};
static const char FILE_NOT_FOUND_s[]= {"File not found"};
static const char CANNOT_LOAD_FILE_s[]= {"Can not load file"};
static const char GIF_INVALID_CMAP_s[]={"GIF colormap invalid"};
static const char GIF_FILE_CORRUPT_s[]={"GIF file corrupt: value %d > %d"};
static const char FILESIZE_TOO_LONG_s[]={"filesize too large. Ignored."};
static const char NO_FILE_TO_PRINT_s[]={"No file to print."};
static const char FILE_SIZE_ZERO_s[]={"File length is zero"};
static const char USE_SYSTEM_COMMAND_TO_PRINT_s[]=
 {"Please use the system command(s)\nto print the file:"};
static const char TILE_s[]={"Tile:"};
static const char OF_s[]={" of "};
static const char RESOLUTION_s[]={"Resolution:"};
static const char RELOAD_s[]={"Reload"};
static const char REDUCTION_s[]={"Reduction:"};
static const char NO_FILE_TO_SAVE_s[]={"No file to save"};
static const char SAVE_AS_PNG_s[]={"Save As PNG"};
static const char ENTER_PNG_TO_SAVE_s[]=
 {"Enter a name to save a PNG file:"};
static const char FILE_EXISTS_s[]=
 {"File already exists.\n    Continue?"};
static const char SAVE_AS_OPJ_s[]={"Save As JP2/J2K"};
static const char ENTER_OPJ_TO_SAVE_s[]=
 {"Enter a name to save a JP2/J2K file:"};
static const char MISSING_PNG_EXT_s[]={"File extension not '.png'. Goon?"};
static const char DICOM_FAILED_s[]={"DICOM_load\n  read_idf(%s)\nfailed."};
static const char AREA_s[]={"Area:"};
static const char FILENAME_TOO_LONG_s[]={"filename too long. Ignored."};
#endif

#ifdef WITH_GERMAN
static const char GIF_INVALID_CMAP_s[]={"GIF Farbtafel ungültig"};
static const char GIF_FILE_CORRUPT_s[]={"GIF Datei beschädig: value %d > %d"};
static const char CANNOT_USE_FILE_s[]= {"Kann Datei nicht verwenden"};
static const char NO_DRIVER_FOUND_s[]= {"KEIN TREIBER GEFUNDEN"};
static const char FILE_NOT_FOUND_s[]= {"Datei nicht gefunden"};
static const char CANNOT_LOAD_FILE_s[]= {"Kann Datei nicht laden"};
static const char FILESIZE_TOO_LONG_s[]={"Datei ist zu groß. Ignoriert."};
static const char NO_FILE_TO_PRINT_s[]={"Keine druckbare Datei gefunden."};
static const char FILE_SIZE_ZERO_s[]={"Datei-Länge ist 0"};
static const char USE_SYSTEM_COMMAND_TO_PRINT_s[]=
 {"Bitte System-Befehl(e) benutzen,\num folgende Datei zu drucken:"};
static const char TILE_s[]={"Kachel:"};
static const char OF_s[]={" von "};
static const char RESOLUTION_s[]={"Verkleinerung:"};
static const char RELOAD_s[]={"Neu laden"};
static const char REDUCTION_s[]={"Verkleinerung:"};
static const char NO_FILE_TO_SAVE_s[]={"Keine Datei zum Sichern gefunden"};
static const char SAVE_AS_PNG_s[]={"Sichern als PNG"};
static const char ENTER_PNG_TO_SAVE_s[]=
 {"Einen Name eingeben für eine PNG Datei:"};
static const char FILE_EXISTS_s[]=
 {"Datei besteht bereits.\n    Weiter?"};
static const char SAVE_AS_OPJ_s[]={"Sichern als JP2/J2K"};
static const char ENTER_OPJ_TO_SAVE_s[]=
 {"Einen Name eingeben für eine JP2/J2K Datei:"};
static const char MISSING_PNG_EXT_s[]={"Datei-Endung nicht '.png'. Weiter?"};
static const char DICOM_FAILED_s[]={"DICOM_load\n  Datei(%s)\nunbrauchbar."};
static const char AREA_s[]={"Fläche:"};
static const char FILENAME_TOO_LONG_s[]={"Datei-Name zu lang. Ignoriert."};
#endif

#endif /* _FLIMAGE_FILE_LANG_H_ */
