#ifndef _FLVIEWER_PAPER_SIZES_HH_
#define _FLVIEWER_PAPER_SIZES_HH_

#define PSIZE_POS 1

static struct papersize
{
	const char *name;
	double paper_w, paper_h; /* in mm */
	const char *media_s;
} paper_sizes[] = {
{"A3", 297, 420, "A3"},
{"A4", 210, 297, "A4"},
{"A5", 148, 210, "A5"},
{"A6", 105, 148, "BBox"},
{"9x13 cm", 88.9, 127, "BBox"}, /* 3.5x5 inch */
{"10x15 cm", 101.6, 152.4, "BBox"}, /* 4x6 inch */
{"PHOTO4.5x6", 114.3, 152.4, "BBox"}, /* digital cameras */
{"13x18 cm", 127, 177.8, "BBox"}, /* 5x7 inches */
{"20x25 cm", 203.2, 254, "BBox"}, /* 8x10 inches */
{"LETTER", 215.9, 297.4, "letter"}, /* 8.5x11 inches */
{"LEDGER", 279.4, 431.8, "ledger"}, /* 11x17 inches */
{"LEGAL", 215.9, 355.6, "legal"}, /* 8.5x14 inches */
/*--
{""},
{""},
{""},
--*/
{NULL, 0, 0, NULL}
};

#ifdef INFO_ONLY
/*----------------------
DOTS       MM
media_w = paper_w * (72./25.4);
media_h = paper_h * (72./25.4);

ISO 216 sizes
(mm × mm) A Series
A0  841 × 1189
A1  594 × 841
A2  420 × 594
A3  297 × 420
A4  210 × 297
A5  148 × 210
A6  105 × 148
A7  74 × 105
A8  52 × 74
A9  37 × 52
A10 26 × 37

ISO 216 sizes
(mm × mm) B Series
B0 	1000 × 1414
B1 	707 × 1000
B2 	500 × 707
B3 	353 × 500
B4 	250 × 353
B5 	176 × 250
B6 	125 × 176
B7 	88 × 125
B8 	62 × 88
B9 	44 × 62
B10 31 × 44


269 sizes
(mm × mm) C Series
C0    917 × 1297
C1    648 × 917
C2    458 × 648
C3    324 × 458
C4    229 × 324
C5    162 × 229
C6    114 × 162
C7/6  81 × 162
C7    81 × 114
C8    57 × 81
C9    40 × 57
C10   28 × 40
DLE   110 × 220



----------------------*/
#endif /* INFO_ONLY */


#endif /* _FLVIEWER_PAPER_SIZES_HH_ */
