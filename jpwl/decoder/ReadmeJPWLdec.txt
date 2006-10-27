Version ****JPWLcorrect****

This version realizes a JPWL decoder based on OpenJPEG library.
Realized decoder accepts an input JPWL codestream, corrects errors sgnalling eventual
residual errors and finally outputs the corrected JPWL codestream. This codestream shall 
be used by a JPEG2000 Part-1 decoder to decode the image.

How to use decoder from prompt line:

"jpwl_correct input.j2k o.bmp"

where,

	- "input.j2k" is the input corrupted JPWL codestream
	- "o.bmp" this file is necessary as parameter but the decoder doesn't create
           that as output.

Decoder create output file "output.j2c", that is the corrected JPWL codestream.

Note that .j2c is the extension accepted by "kakadu" decoder.

