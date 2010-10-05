MAJOR = 1
MINOR = 4
BUILD = 0

JP3D_MAJOR = 1
JP3D_MINOR = 3
JP3D_BUILD = 0

prefix=/usr/local
CC = gcc

#Set this to yes if you want to compile/install shared libs.
ENABLE_SHARED = no
#
#Set to yes if you BOTH have the library AND the header
#Set to no if a file is missing or you hate it.
#Either lcms or lcms2 : not both
#SHOULD BE IN SYNC WITH opj_config.h
WITH_LCMS1 = no
WITH_LCMS2 = no
WITH_PNG = no
WITH_TIFF = no

#Set to yes if you have doxygen installed
#Set to no if doxygen is missing.
HAS_DOXYGEN = yes

#Check whether these paths are correct; change them appropiatly.
LCMS1_INCLUDE = -I/usr/include
LCMS2_INCLUDE = -I/usr/include
PNG_INCLUDE = -I/usr/include
TIFF_INCLUDE = -I/usr/include

LCMS1_LIB = -L/usr/lib -llcms -lm
LCMS2_LIB = -L/usr/lib -llcms2 -lm
PNG_LIB = -L/usr/lib -lpng -lz -lm
TIFF_LIB = -L/usr/lib -ltiff -lm
