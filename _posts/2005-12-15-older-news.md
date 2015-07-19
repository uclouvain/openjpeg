---
layout: post
title: Older news
---
{% include JB/setup %}

## 15 December 2005: OpenJPEG 1.0 has been released

Thanks to Hervé Drolon from the [Freeimage project](http://freeimage.sourceforge.net/), major enhancements have been brought to the OpenJPEG library. They are shortly summarized in the following list. As you will see, this new version is completely "iso-functional" in comparison with the previous one... Nevertheless, deep changes have been made giving the openjpeg library enough robustness to release version 1.0. The new library structure, function names, ... are largely inspired from the work that has been done for the JPEG library.

We hope that this robust and stable version of the OpenJPEG library will help you even more when building your JPEG 2000 applications !

Major changes :

  * User interface : the UI is now independent from the library code. In order to use the library, in addition to the binary of the library itself, you only need one header file : openjpeg.h. The interface is actually much inspired from the LibJPEG library. A DLL target will be added soon to the .lib/.so/.a targets already present.
  * Multi-threading : one of the major enhancements of this version : the library can know be used in multi-thread applications.
  * Error handling : the error handling system has been completely rebuilt and uses now an event managing system with callbacks. It manages errors, warnings and debug infos (like processing time, ...). Moreover, this system is easily extensible to other kind of events.
  * Parameters : parameters are sent to the codec through new strcutures called opj\_cparameters\_t et opj\_dparameters\_t. Adding new parameters is now very easy using these structures.
  * Documentation : the library is now documented with the Javadoc Standard. An html version of the documentation generated with Doxygen will soon be online.

## 1 september 2005: Forum has been opened

A forum has been opened where questions and comments can be posted about the library. Feel free to contribute !

## 27 april 2005: Version 0.97

Most of the the new features of version 0.97 are related to MJ2. It now includes a metadata reader from a Motion JPEG 2000 video file that generates an XML file with a representation of that content. This part of the OpenJPEG codec has been developped by Glenn Pearson . Thanks for his contributions ! This version also includes optimisations of the J2K and MJ2 codecs. Check the documentation for more info.

## 26 january 2005 : "layering" option added to the decoder

A '-l' option has been added to the decoder. This option allows user to specify the maximum number of layers to be decoded. In addition to this, the command line for the decoder (j2k\_to\_image) has been updated.

## 14 december 2004 : Version 0.96

This version integrated also updates for j2k, jp2 and mj2 file formats.

## 13 december 2004: Modularity increased with MJ2 files

The modularity of the MJ2 codec has been increased. Two tools (MJ2\_Wrapper and MJ2\_Extractor) are independant from OpenJPEG and enable you to wrap JPEG 2000 codestream into a MJ2 file, and to extract codestreams from MJ2 files. Besides these two new tools, the MJ2 codec compresses YUV frames to an MJ2 file, and decompresses them using the OpenJPEG codec.

## 20 July 2004 : Version 0.95

The main update of the version 0.95 is the Class-1 Profile-1 compliance of the decoder. We are working hard to make the whole codec full part-1 compliant as soon as possible (some tests are still to be made on the encoder). This version 0.95 integrates also recent updates like jp2 or mj2 file formats support

## 15 July 2004 : MJ2 files encoding and decoding

You can now create and decode Motion JPEG 2000 files with OpenJPEG. The compression from YUV files can be achieved using all standard OpenJPEG coding options applied to the J2K codestreams. You can use standard MJ2 boxes, or define your own. The decompression of MJ2 files to YUV is straightforward. For the moment, this new option is available only in the CVS package (**not** in the version 0.9 package).

## 25 June 2004 : JP2-format encoding and decoding

JP2 file-format is now managed by OpenJPEG. Just specify a .jp2 extension in place of the .j2k extension. At the encoder, you may specify the content of the JP2 boxes, or use the standard ones created by default. At the decoder, a JP2-structure is created and can be used afterwards. For the moment, this new option is available only in the CVS package (**not** in the version 0.9 package).

## 3 May 2004 : Version 0.9

Version 0.9 has been released. Several new features are available :

  * Option '-reduce n' added to the decoder. The decoder stops the inverse wavelet transform at the right resolution define through the option '-reduce n'. The value entered means that the dimension of the output image will be 2n times smaller than the original one.
  * New information inside index. In addition to the information already present in the index structure (byterange, ...), the distortion reduction brought by each packet has been added. This can be useful when browsing very large images through a network (JPIP protocol) or when evaluating the importance of each packet for error resilience. The structure of the index is fully described in the documentation.
  * Option '-q n' added to the encoder. This option enables the user to specifiy a psnr to which he/she wants the image to be compressed, in place of a compression ratio. The encoder will then give the smallest codestream reaching this quality level. The options '-q' and '-r' cannot be used together of course.