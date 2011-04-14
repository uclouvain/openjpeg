/*
 * $Id: mainpage.h 47 2011-02-17 16:57:49Z kaori $
 *
 * Copyright (c) 2002-2011, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2011, Professor Benoit Macq
 * Copyright (c) 2010-2011, Kaori Hagihara
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*! \mainpage OpenJPIP v1.0 Documentation
 *
 * \section intro Introduction
 * This manual documents the low-level OpenJPIP C API.\n
 * OpenJPIP software is an implementation of JPEG 2000 Part9: Interactivity tools, APIs and protocols (JPIP).\n
 * ( For more info about JPIP, check the website: http://www.jpeg.org/jpeg2000/j2kpart9.html)\n
 *
 * This whole documents covers the following six programs.\n
 * - opj_server.c     JPIP server supporting HTTP connection and JPT-stream
 * - opj_dec_server.c Server to decode JPT-stream and communicate locally with JPIP client, which is coded in java
 * - addXMLinJP2.c  To Embed metadata into JP2 file
 * - jpt_to_jp2.c   To Convert JPT-stream to JP2
 * - jpt_to_j2k.c   To Convert JPT-stream to J2K
 * - test_index.c   To test index code format of a JP2 file
 *
 * \section license License
 * This software is released under the BSD license, anybody can use or modify the library, even for commercial applications.\n
 * The only restriction is to retain the copyright in the sources or the binaries documentation.\n
 * Neither the author, nor the university accept any responsibility for any kind of error or data loss which may occur during usage.
 *
 *
 * \section reqlibs Required libraries
 *  - OpenJPEG library
 *  - FastCGI development kit (C libraries) at server (http://www.fastcgi.com)
 *
 *  We tested this software with a virtual server running on the same Linux machine as the clients.
 *
 *
 * \section compilenotes Compiling Notes
 * When you are making opj_server, set anything (e.g. yes) to the parameter jpipserver to define itself in the Makefile, which enables to make it in server mode.\n
 * Otherwise do not define (or do not set to) the parameter jpipserver.\n
 * Be sure that any object files and library file libopenjpip.a are not reused to compile in the two different mode (server mode and non server mode).\n
 * In other words, do make clean before making new targets which are in different modes as previous make.\n
 *
 * 
 * \section sysarchtect System Architecture
 * JPIP protocol is implimented between the server program opj_server and the client java program opj_viewer.\n
 * The opj_server parses JPIP requests and sends corresponding JPT-stream.
 * The opj_viewer is an image viewer with GUI to publish JPIP requests and receive JPT-stream.\n
 * JPT-stream is unreadable for the opj_viewer by itself, and the local server program opj_dec_server handles the decoding and caching of JPT-stream.\n
 * Typical requests and replies among JPIP server (opj_server), JPIP client (opj_viewer), and Image decoding Server (opj_dec_server) is presented below.\n
 * The JPIP client requests the JPIP server JPT-stream, which is directly sent to the Image decoding Server, and which provides the image in PGM or PPM format to the JPIP client.\n
 * JPIP client can read PGM and PPM images natively.\n
 * Before connecting to the JPIP server, the JPIP client checks local cache data of the requesting image with the image decoding server.
 * If its cache exists, the image decoding server provides ChannelID (CID), which identifies the image and its cache model on the JPIP server, and the whole system can continue the session using the CID.
 *
 *  \image html jpip_protocol.png "Message Sequence Chart of OpenJPIP impementation"
 *
 * \author Kaori Hagihara UCL/SST/ICTM/ELEN
 */
