/*
 * Copyright (c) 2004, Yannick Verschueren
 * Copyright (c) 2004, Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
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

#include <stdio.h>
#include <stdlib.h>

#include "jpt.h"
#include "j2k.h"
#include "cio.h"


/*
 * Read the information contains in VBAS [JPP/JPT stream message header]
 * Store information (7 bits) in value
 *
 */
unsigned int jpt_read_VBAS_info(unsigned int value)
{
  unsigned char elmt;

  elmt = cio_read(1);
  while ((elmt >> 7) == 1) {
    value = (value << 7);
    value |= (elmt & 0x7f);
    elmt = cio_read(1);
  }
  value = (value << 7);
  value |= (elmt & 0x7f);

  return value;
}

/*
 * Initialize the value of the message header structure 
 *
 */
void jpt_init_Msg_Header(jpt_msg_header_struct_t * header)
{
  header->Id = 0;		/* In-class Identifier    */
  header->last_byte = 0;	/* Last byte information  */
  header->Class_Id = 0;		/* Class Identifier       */
  header->CSn_Id = 0;		/* CSn : index identifier */
  header->Msg_offset = 0;	/* Message offset         */
  header->Msg_length = 0;	/* Message length         */
  header->Layer_nb = 0;		/* Auxiliary for JPP case */
}

/*
 * Re-initialize the value of the message header structure
 *
 * Only parameters always present in message header
 *
 */
void jpt_reinit_Msg_Header(jpt_msg_header_struct_t * header)
{
  header->Id = 0;		/* In-class Identifier    */
  header->last_byte = 0;	/* Last byte information  */
  header->Msg_offset = 0;	/* Message offset         */
  header->Msg_length = 0;	/* Message length         */
}

/*
 * Read the message header for a JPP/JPT - stream
 *
 */
void jpt_read_Msg_Header(jpt_msg_header_struct_t * header)
{
  unsigned char elmt, Class = 0, CSn = 0;
  jpt_reinit_Msg_Header(header);

  /* ------------- */
  /* VBAS : Bin-ID */
  /* ------------- */
  elmt = cio_read(1);

  /* See for Class and CSn */
  switch ((elmt >> 5) & 0x03) {
  case 0:
    fprintf(stderr, "Forbidden value encounter in message header !!\n");
    break;
  case 1:
    Class = 0;
    CSn = 0;
    break;
  case 2:
    Class = 1;
    CSn = 0;
    break;
  case 3:
    Class = 1;
    CSn = 1;
    break;
  default:
    break;
  }

  /* see information on bits 'c' [p 10 : A.2.1 general, ISO/IEC FCD 15444-9] */
  if (((elmt >> 3) & 0x01) == 1)
    header->last_byte = 1;

  /* In-class identifier */
  header->Id |= (elmt & 0x0f);
  if ((elmt >> 7) == 1)
    header->Id = jpt_read_VBAS_info(header->Id);

  /* ------------ */
  /* VBAS : Class */
  /* ------------ */
  if (Class == 1) {
    header->Class_Id = 0;
    header->Class_Id = jpt_read_VBAS_info(header->Class_Id);
  }

  /* ---------- */
  /* VBAS : CSn */
  /* ---------- */
  if (CSn == 1) {
    header->CSn_Id = 0;
    header->CSn_Id = jpt_read_VBAS_info(header->CSn_Id);
  }

  /* ----------------- */
  /* VBAS : Msg_offset */
  /* ----------------- */
  header->Msg_offset = jpt_read_VBAS_info(header->Msg_offset);

  /* ----------------- */
  /* VBAS : Msg_length */
  /* ----------------- */
  header->Msg_length = jpt_read_VBAS_info(header->Msg_length);

  /* ---------- */
  /* VBAS : Aux */
  /* ---------- */
  if (header->CSn_Id == 1) {
    header->Layer_nb = 0;
    header->Layer_nb = jpt_read_VBAS_info(header->Layer_nb);
  }
}
