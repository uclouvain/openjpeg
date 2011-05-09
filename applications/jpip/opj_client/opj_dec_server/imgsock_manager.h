/*
 * $Id: imgsock_manager.h 53 2011-05-09 16:55:39Z kaori $
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

#ifndef   	IMGSOCK_MANAGER_H_
# define   	IMGSOCK_MANAGER_H_

#include "bool.h"
#include "byte_manager.h"

/**
 * open listening socket
 *
 * @return file descriptor for the new socket
 */
int open_listeningsocket();

#define NUM_OF_MSGTYPES 7
typedef enum eMSGTYPE{ JPTSTREAM, PNMREQ, XMLREQ, CIDREQ, CIDDST, JP2SAVE, QUIT, ERROR} msgtype_t;


/**
 * indeitify client message type
 *
 * @param [in] connected_socket file descriptor of the connected socket
 * @return                      message type
 */
msgtype_t identify_clientmsg( int connected_socket);

/**
 * receive JPT-stream from client
 *
 * @param [in]  connected_socket file descriptor of the connected socket
 * @param [out] target           received target file name (if not received, null string)
 * @param [out] cid              received channel identifier (if not received, null string)
 * @param [out] streamlen        length of the received codestream
 * @return                          codestream
 */
Byte_t * receive_JPTstream( int connected_socket, char *target, char *cid, int *streamlen);

/**
 * send PGM/PPM image stream to the client
 *
 * @param [in]  connected_socket file descriptor of the connected socket
 * @param [in]  pnmstream        PGM/PPM image codestream
 * @param [in]  width            width  of the image
 * @param [in]  height           height of the image
 * @param [in]  numofcomp        number of components of the image
 * @param [in]  maxval           maximum value of the image (only 255 supported)
 */
void send_PNMstream( int connected_socket, Byte_t *pnmstream, unsigned int width, unsigned int height, unsigned int numofcomp, Byte_t maxval);

/**
 * send XML data stream to the client
 *
 * @param [in]  connected_socket file descriptor of the connected socket
 * @param [in]  xmlstream        xml data stream
 * @param [in]  length           legnth of the xml data stream
 */
void send_XMLstream( int connected_socket, Byte_t *xmlstream, int length);

/**
 * send CID data stream to the client
 *
 * @param [in]  connected_socket file descriptor of the connected socket
 * @param [in]  cid              cid string
 * @param [in]  cidlen           legnth of the cid string
 */
void send_CIDstream( int connected_socket, char *cid, int cidlen);

/**
 * send response signal to the client
 *
 * @param [in]  connected_socket file descriptor of the connected socket
 * @param [in]  succeed          whether if the requested process succeeded
 */
void response_signal( int connected_socket, bool succeed);

/**
 * read a string line (ending with '\n') from client
 *
 * @param [in]  connected_socket file descriptor of the connected socket
 * @param [out] buf              string to be stored
 * @return                       red size
 */

int read_line(int connected_socket, char *buf);
#endif /* !IMGSOCK_MANAGER_H_ */

/*! \file
 * PROTOCOL specification to communicate with opj_dec_server
 *
 *\section sec1 JPT-stream
 * Cache JPT-stream in server
 *
 * client -> server: JPT-stream\\n version 1.0\\n (optional for cid registration: targetnamestring\\n cidstring\\n) bytelengthvalue\\n data \n
 * server -> client: 1 or 0 (of 1Byte response signal)
 * 
 *\section sec2 PNM request
 * Get decoded PGM/PPM image
 *
 * client -> server: PNM request\\n cidstring\\n fw\\n fh\\n \n
 * server -> client: P6 or P5 (2Byte) width (2Byte Big endian) height (2Byte Big endian) maxval (1Byte) data
 *
 *\section sec3 XML request
 * Get XML data
 *
 * client -> server: XML request\\n \n
 * server -> client: XML (3Byte) length (2Byte Big endian) data
 *
 *\section sec4 CID request
 * Get Channel ID of identical target image
 *
 * client -> server: CID request\\n targetname\\n \n
 * server -> client: CID (3Byte) length (1Byte) ciddata
 *
 *\section sec5 CID destroy
 * Close Channel ID
 *
 * client -> server: CID destroy\\n ciddata \n
 * server -> client: 1 or 0 (of 1Byte response signal)
 *
 *\section sec6 JP2 save
 * Save in JP2 file format
 *
 * client -> server: JP2 save\\n ciddata \n
 * server -> client: 1 or 0 (of 1Byte response signal)
 *
 *\section sec7 QUIT
 * Quit the opj_dec_server program
 *
 * client -> server: quit or QUIT
 */
