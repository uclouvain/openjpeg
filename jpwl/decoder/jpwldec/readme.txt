List of parameters for the coder JPEG 2000 :

Date : June the 25th, 2003
Author : Yannick Verschueren
Contact : verschueren@tele.ucl.ac.be 

- the option -help displays the readme.txt file on screen

- The markers COD and QCD are writed both of two in the main_header and never appear in the tile_header.  The markers in the main header are : SOC SIZ COD QCD COM.

- This coder can encode mega image, a test was made on a 24000x24000 pixels color image.  You need enough disk space memory (twice the original) to encode the image. (i.e. for a 1.5 Gb image you need a minimum of 3Gb of disk memory) 

REMARKS :
---------

* the value of rate enter in the code line is the compression factor !
exemple :

-r 20,10,1 means quality 1 : compress 20x, quality 2 : compress 10x and quality 3 : compress 1x = lossless  

* The number of resolution can be modified by the program in view to respect profile-0 conditions (Taubman, Marcelin (2002), "JPEG2000, image compression fundamentals, standards and practice", p700)

By default :
------------

 * lossless
 * 1 tile
 * size of precinct 2^15 x 2^15 (means 1 precinct)
 * size of code-block 64 x 64
 * Number of resolution : 6
 * No SOP marker in the codestream
 * No EPH marker in the codestream
 * No sub-sampling in x and y direction
 * No mode switch activated
 * progression order : LRCP
 * No index file
 * No ROI upshifted
 * No offset of the origin of the image
 * No offset of the origin of the tiles
 * Reversible DWT 5-3

Parameters :
------------

-i             : source file  (-i source.pnm also *.pgm, *.ppm) "required"

-o             : destination file (-o dest.j2k) "required"

-r             : different rates (-r 20,10,5) "optional" 

-n             : Number of resolution (-n 3) "optional"

-b             : size of code block (-b 32,32) "optional"

-c             : size of precinct (-c 128,128) "optional"

-t             : size of tile (-t 512,512) "optional"

-p             : progression order (-p LRCP) [LRCP, RLCP, RPCL, PCRL, CPRL] "optional"

-s             : subsampling factor (-s 2,2) [-s X,Y] "optional"

-SOP           : write SOP marker before each packet "optional"

-EPH           : write EPH marker after each header packet "optional"

-M             : mode switch (-M 3) [1= BYPASS(LAZY) 2=RESET 4=RESTART(TERMALL) 8=VSC 16=ERTERM(SEGTERM) 32=SEGMARK(SEGSYM)]  "optional"
                    for several mode switch you have to add the value of each mode you want
                    ex : RESTART(4) + RESET(2) + SEGMARK(32) = -M 38 

-x             : Create an index file *.Idx (-x index_name.Idx) "optional"

-ROI:c=%d,U=%d : quantization indices upshifted for component c=%d [%d = 0,1,2] 
                 with a value of U=%d [0 <= %d <= 37] (i.e. -ROI:c=0,U=25) "optional"

-d             : offset of the origin of the image (-d 150,300) "optional"

-T             : offset of the origin of the tiles (-T 100,75) "optional"

-I             : Use the irreversible DWT 9-7 (-I) "optional"

IMPORTANT : 
-----------

* subsampling bigger than 2 can produce error

The index file respect the structure below :
---------------------------------------------

Image_height Image_width
progression order
Tiles_size_X Tiles_size_Y
Components_nb
Layers_nb
decomposition_levels
Precincts_size_X Precincts_size_Y
Main_header_end_position
Codestream_size
Tile0 start_pos end_Theader end_pos
Tile1  "         "           "
...
TileN  "         "           "
Tpacket_0 Tile layer res. comp. prec. start_pos end_pos
...
Tpacket_M "    "     "    "     "     "         "
