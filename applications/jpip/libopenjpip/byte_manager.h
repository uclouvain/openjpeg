/*
 * $Id: byte_manager.h 44 2011-02-15 12:32:29Z kaori $
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

#ifndef   	BYTE_MANAGER_H_
# define   	BYTE_MANAGER_H_

/** 1Byte parameter type*/
typedef unsigned char Byte_t;

/** 2Byte parameter type*/
typedef unsigned short int Byte2_t;

/** 4Byte parameter type*/
typedef unsigned int Byte4_t;

/** 8Byte parameter type*/
typedef unsigned long long int Byte8_t;


/**
 * fetch bytes of data in file stream
 *
 * @param[in] fd     file discriptor
 * @param[in] offset start Byte position
 * @param[in] size   Byte length
 * @return           pointer to the fetched data
 */
Byte_t * fetch_bytes( int fd, long offset, int size);


/**
 * fetch a 1-byte Byte codes in file stream
 *
 * @param[in] fd     file discriptor
 * @param[in] offset start Byte position
 * @return           fetched codes
 */
Byte_t fetch_1byte( int fd, long offset);

/**
 * fetch a 2-byte big endian Byte codes in file stream
 *
 * @param[in] fd     file discriptor
 * @param[in] offset start Byte position
 * @return           fetched codes
 */
Byte2_t fetch_2bytebigendian( int fd, long offset);

/**
 * fetch a 4-byte big endian Byte codes in file stream
 *
 * @param[in] fd     file discriptor
 * @param[in] offset start Byte position
 * @return           fetched codes
 */
Byte4_t fetch_4bytebigendian( int fd, long offset);

/**
 * fetch a 8-byte big endian Byte codes in file stream
 *
 * @param[in] fd     file discriptor
 * @param[in] offset start Byte position
 * @return           fetched codes
 */
Byte8_t fetch_8bytebigendian( int fd, long offset);


/**
 * convert 2-byte big endian Byte codes to number
 *
 * @param[in] buf Byte codes
 * @return        resolved number
 */
Byte2_t big2( Byte_t *buf);

/**
 * convert 4-byte big endian Byte codes to number
 *
 * @param[in] buf Byte codes
 * @return        resolved number
 */
Byte4_t big4( Byte_t *buf);

/**
 * convert 8-byte big endian Byte codes to number
 *
 * @param[in] buf Byte codes
 * @return        resolved number
 */
Byte8_t big8( Byte_t *buf);

/**
 * modify 4Byte code in a codestream
 *  
 * @param[in]  code code value
 * @param[out] stream modifying codestream
 */
void modify_4Bytecode( Byte4_t code, Byte_t *stream);

/**
 * Get file size
 *
 * @param[in] fd file discriptor
 * @return       file size
 */
Byte8_t get_filesize( int fd);

#endif 	    /* !BYTE_MANAGER_H_ */
