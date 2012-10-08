/*
 * $Id$
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

#include <stdlib.h>
#include "openjpip.h"
#include "jpip_parser.h"
#include "channel_manager.h"
#include "byte_manager.h"
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef SERVER
#include "auxtrans_manager.h"
#endif

#include <stdio.h>
#include "dec_clientmsg_handler.h"
#include "jpipstream_manager.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "jp2k_encoder.h"

#ifdef SERVER

server_record_t * init_JPIPserver( int tcp_auxport, int udp_auxport)
{
  server_record_t *record = (server_record_t *)opj_malloc( sizeof(server_record_t));
  
  record->sessionlist = gene_sessionlist();
  record->targetlist  = gene_targetlist();
  record->auxtrans = init_aux_transport( tcp_auxport, udp_auxport);
   
  return record;
}

void terminate_JPIPserver( server_record_t **rec)
{
  delete_sessionlist( &(*rec)->sessionlist);
  delete_targetlist( &(*rec)->targetlist); 
  close_aux_transport( (*rec)->auxtrans);
   
  opj_free( *rec);
}

QR_t * parse_querystring( const char *query_string)
{
  QR_t *qr;

  qr = (QR_t *)opj_malloc( sizeof(QR_t));
    
  qr->query = parse_query( query_string);
  qr->msgqueue = NULL;
  qr->channel = NULL;

  return qr;
}

bool process_JPIPrequest( server_record_t *rec, QR_t *qr)
{
  target_param_t *target = NULL;
  session_param_t *cursession = NULL;
  channel_param_t *curchannel = NULL;

  if( qr->query->target || qr->query->tid){
    if( !identify_target( *(qr->query), rec->targetlist, &target))
      return false;
  }

  if( qr->query->cid){
    if( !associate_channel( *(qr->query), rec->sessionlist, &cursession, &curchannel))
      return false;
    qr->channel = curchannel;
  }
  
  if( qr->query->cnew != non){
    if( !open_channel( *(qr->query), rec->sessionlist, rec->auxtrans, target, &cursession, &curchannel))
      return false;
    qr->channel = curchannel;
  }
  
  if( qr->query->cclose)
    if( !close_channel( *(qr->query), rec->sessionlist, &cursession, &curchannel))
      return false;
  
  if( (qr->query->fx > 0 && qr->query->fy > 0) || qr->query->box_type[0][0] != 0 || qr->query->len > 0)
    if( !gene_JPIPstream( *(qr->query), target, cursession, curchannel, &qr->msgqueue))
      return false;

  return true;
}

void add_EORmsg( int fd, QR_t *qr);

void send_responsedata( server_record_t *rec, QR_t *qr)
{
  int fd;
  const char tmpfname[] = "tmpjpipstream.jpp";
  Byte_t *jpipstream;
  Byte8_t len_of_jpipstream;

  if( (fd = open( tmpfname, O_RDWR|O_CREAT|O_EXCL, S_IRWXU)) == -1){
    fprintf( FCGI_stderr, "file open error %s", tmpfname);
    fprintf( FCGI_stdout, "Status: 503\r\n");
    fprintf( FCGI_stdout, "Reason: Implementation failed\r\n");
    return;
  }
  
  recons_stream_from_msgqueue( qr->msgqueue, fd);
  
  add_EORmsg( fd, qr); /* needed at least for tcp and udp */
  
  len_of_jpipstream = (Byte8_t)get_filesize( fd);
  jpipstream = fetch_bytes( fd, 0, len_of_jpipstream);
  
  close( fd);
  remove( tmpfname);

  fprintf( FCGI_stdout, "\r\n");

  if( len_of_jpipstream){
    
    if( qr->channel)
      if( qr->channel->aux == tcp || qr->channel->aux == udp){
	send_responsedata_on_aux( qr->channel->aux==tcp, rec->auxtrans, qr->channel->cid, jpipstream, len_of_jpipstream, 1000); /* 1KB per frame*/
	return;
      }
    
    if( fwrite( jpipstream, len_of_jpipstream, 1, FCGI_stdout) != 1)
      fprintf( FCGI_stderr, "Error: failed to write jpipstream\n");
  }

  opj_free( jpipstream);

  return;
}

void add_EORmsg( int fd, QR_t *qr)
{
  unsigned char EOR[3];

  if( qr->channel){
    EOR[0] = 0x00;   
    EOR[1] = is_allsent( *(qr->channel->cachemodel)) ? 0x01 : 0x02;
    EOR[2] = 0x00;
    if( write( fd, EOR, 3) != 3)
      fprintf( FCGI_stderr, "Error: failed to write EOR message\n");
  }
}

void end_QRprocess( server_record_t *rec, QR_t **qr)
{
  /* TODO: record client preferences if necessary*/
  (void)rec; /* unused */
  delete_query( &((*qr)->query));
  delete_msgqueue( &((*qr)->msgqueue));
  opj_free( *qr);
}


void local_log( bool query, bool messages, bool sessions, bool targets, QR_t *qr, server_record_t *rec)
{
  if( query)
    print_queryparam( *qr->query);

  if( messages)
    print_msgqueue( qr->msgqueue);

  if( sessions)
    print_allsession( rec->sessionlist);
  
  if( targets)
    print_alltarget( rec->targetlist);
}

#endif /*SERVER*/

#ifndef SERVER

dec_server_record_t * init_dec_server( int port)
{
  dec_server_record_t *record = (dec_server_record_t *)opj_malloc( sizeof(dec_server_record_t));

  record->cachelist = gene_cachelist();
  record->jpipstream = NULL;
  record->jpipstreamlen = 0;
  record->msgqueue = gene_msgqueue( true, NULL);
  record->listening_socket = open_listeningsocket( (uint16_t)port);

  return record;
}

void terminate_dec_server( dec_server_record_t **rec)
{
  delete_cachelist( &(*rec)->cachelist);  
  opj_free( (*rec)->jpipstream);
  
  if( (*rec)->msgqueue)
    delete_msgqueue( &((*rec)->msgqueue));

  if( close_socket( (*rec)->listening_socket) != 0)
    perror("close");
  
  opj_free( *rec);
}

client_t accept_connection( dec_server_record_t *rec)
{
  client_t client;
  
  client = accept_socket( rec->listening_socket);
  if( client == -1)
    fprintf( stderr, "error: failed to connect to client\n");
  
  return client;
}

bool handle_clientreq( client_t client, dec_server_record_t *rec)
{
  bool quit = false;
  msgtype_t msgtype = identify_clientmsg( client);
  
  switch( msgtype){
  case JPIPSTREAM:
    handle_JPIPstreamMSG( client, rec->cachelist, &rec->jpipstream, &rec->jpipstreamlen, rec->msgqueue);
    break;
      
  case PNMREQ:
    handle_PNMreqMSG( client, rec->jpipstream, rec->msgqueue, rec->cachelist);
    break;
    
  case XMLREQ:
    handle_XMLreqMSG( client, rec->jpipstream, rec->cachelist);
    break;

  case TIDREQ:
    handle_TIDreqMSG( client, rec->cachelist);
    break;
						
  case CIDREQ:
    handle_CIDreqMSG( client, rec->cachelist);
    break;

  case CIDDST:
    handle_dstCIDreqMSG( client, rec->cachelist);
    break;
    
  case SIZREQ:
    handle_SIZreqMSG( client, rec->jpipstream, rec->msgqueue, rec->cachelist);
    break;

  case JP2SAVE:
    handle_JP2saveMSG( client, rec->cachelist, rec->msgqueue, rec->jpipstream);
    break;

  case QUIT:
    quit = true;
    save_codestream( rec->jpipstream, rec->jpipstreamlen, "jpt");
    break;
  case MSGERROR:
    break;
  }

  fprintf( stderr, "\t end of the connection\n\n");
  if( close_socket(client) != 0){
    perror("close");
    return false;
  }

  if( quit)
    return false;

  return true;
}


jpip_dec_param_t * init_jpipdecoder( bool jp2)
{
  jpip_dec_param_t *dec;
  
  dec = (jpip_dec_param_t *)opj_calloc( 1, sizeof(jpip_dec_param_t));

  dec->msgqueue = gene_msgqueue( true, NULL);
  
  if( jp2)
    dec->metadatalist = gene_metadatalist();

  return dec;
}


bool fread_jpip( const char fname[], jpip_dec_param_t *dec)
{
  int infd;

  if(( infd = open( fname, O_RDONLY)) == -1){
    fprintf( stderr, "file %s not exist\n", fname);
    return false;
  }
  
  if(!(dec->jpiplen = (Byte8_t)get_filesize(infd)))
    return false;
  
  dec->jpipstream = (Byte_t *)opj_malloc( dec->jpiplen);

  if( read( infd, dec->jpipstream, dec->jpiplen) != (int)dec->jpiplen){
    fprintf( stderr, "file reading error\n");
    opj_free( dec->jpipstream);
    return false;
  }

  close(infd);

  return true;
}

void decode_jpip( jpip_dec_param_t *dec)
{
  parse_JPIPstream( dec->jpipstream, dec->jpiplen, 0, dec->msgqueue);

  if( dec->metadatalist){ /* JP2 encoding*/
    parse_metamsg( dec->msgqueue, dec->jpipstream, dec->jpiplen, dec->metadatalist);
    dec->ihdrbox = gene_ihdrbox( dec->metadatalist, dec->jpipstream);
    
    dec->jp2kstream = recons_jp2( dec->msgqueue, dec->jpipstream, dec->msgqueue->first->csn, &dec->jp2klen);
  }
  else /* J2k encoding  */
    /* Notice: arguments fw, fh need to be set for LRCP, PCRL, CPRL*/
    dec->jp2kstream = recons_j2k( dec->msgqueue, dec->jpipstream, dec->msgqueue->first->csn, 0, 0, &dec->jp2klen);  
}

bool fwrite_jp2k( const char fname[], jpip_dec_param_t *dec)
{
  int outfd;
  
#ifdef _WIN32
  if(( outfd = open( fname, O_WRONLY|O_CREAT, _S_IREAD | _S_IWRITE)) == -1){
#else
 if(( outfd = open( fname, O_WRONLY|O_CREAT, S_IRWXU|S_IRWXG)) == -1){
#endif
   fprintf( stderr, "file %s open error\n", fname);
   return false;
 }
  
 if( write( outfd, dec->jp2kstream, dec->jp2klen) != (int)dec->jp2klen)
   fprintf( stderr, "j2k file write error\n");

 close(outfd);

 return true;
}

void output_log( bool messages, bool metadata, bool ihdrbox, jpip_dec_param_t *dec)
{
  if( messages)
    print_msgqueue( dec->msgqueue);

  if( metadata)
    print_allmetadata( dec->metadatalist);

  if( ihdrbox){
    printf("W*H: %d*%d\n", dec->ihdrbox->height, dec->ihdrbox->width);
    printf("NC: %d, bpc: %d\n", dec->ihdrbox->nc, dec->ihdrbox->bpc);
  }
}

void destroy_jpipdecoder( jpip_dec_param_t **dec)
{
  opj_free( (*dec)->jpipstream);
  delete_msgqueue( &(*dec)->msgqueue);
  if( (*dec)->metadatalist){
    delete_metadatalist( &(*dec)->metadatalist);
    opj_free( (*dec)->ihdrbox);
  }

  opj_free( (*dec)->jp2kstream);
  opj_free( *dec);
}

index_t * get_index_from_JP2file( int fd)
{
  char *data;
 
  /* Check resource is a JP family file.*/
  if( lseek( fd, 0, SEEK_SET)==-1){
    fprintf( stderr, "Error: File broken (lseek error)\n");
    return NULL;
  }
  
  data = (char *)opj_malloc( 12); /* size of header*/
  if( read( fd, data, 12) != 12){
    opj_free( data);
    fprintf( stderr, "Error: File broken (read error)\n");
    return NULL;
  }
    
  if( *data || *(data + 1) || *(data + 2) ||
      *(data + 3) != 12 || strncmp (data + 4, "jP  \r\n\x87\n", 8)){
    opj_free( data);
    fprintf( stderr, "Error: No JPEG 2000 Signature box in this file\n");
    return NULL;
  }
  opj_free( data);
  
  return parse_jp2file( fd);
}

void destroy_index( index_t **idx)
{
  delete_index( idx);
}

void output_index( index_t *index)
{
  print_index( *index);
}

/* ---------------------------------------------------------------------- */
/* COMPRESSION FUNCTIONS*/
typedef struct opj_decompression
{
	/** Main header reading function handler*/
	opj_bool (*opj_read_header) (	struct opj_stream_private * cio,
									void * p_codec,
									opj_image_t **p_image,
									struct opj_event_mgr * p_manager);
	/** Decoding function */
	opj_bool (*opj_decode) (	void * p_codec,
								struct opj_stream_private *p_cio,
								opj_image_t *p_image,
								struct opj_event_mgr * p_manager);
	/** FIXME DOC */
	opj_bool (*opj_read_tile_header)(	void * p_codec,
										OPJ_UINT32 * p_tile_index,
										OPJ_UINT32* p_data_size,
										OPJ_INT32 * p_tile_x0, OPJ_INT32 * p_tile_y0,
										OPJ_INT32 * p_tile_x1, OPJ_INT32 * p_tile_y1,
										OPJ_UINT32 * p_nb_comps,
										opj_bool * p_should_go_on,
										struct opj_stream_private *p_cio,
										struct opj_event_mgr * p_manager);
	/** FIXME DOC */
	opj_bool (*opj_decode_tile_data)(	void * p_codec,
										OPJ_UINT32 p_tile_index,
										OPJ_BYTE * p_data,
										OPJ_UINT32 p_data_size,
										struct opj_stream_private *p_cio,
										struct opj_event_mgr * p_manager);
	/** Reading function used after codestream if necessary */
	opj_bool (* opj_end_decompress) (	void *p_codec,
										struct opj_stream_private *cio,
										struct opj_event_mgr * p_manager);
	/** Codec destroy function handler*/
	void (*opj_destroy) (void * p_codec);
	/** Setup decoder function handler */
	void (*opj_setup_decoder) (void * p_codec, opj_dparameters_t * p_param);
	/** Set decode area function handler */
	opj_bool (*opj_set_decode_area) (	void * p_codec,
										opj_image_t* p_image,
										OPJ_INT32 p_start_x, OPJ_INT32 p_end_x,
										OPJ_INT32 p_start_y, OPJ_INT32 p_end_y,
										struct opj_event_mgr * p_manager);

	/** Get tile function */
	opj_bool (*opj_get_decoded_tile) (	void *p_codec,
										opj_stream_private_t *p_cio,
										opj_image_t *p_image,
										struct opj_event_mgr * p_manager,
										OPJ_UINT32 tile_index);

	/** Set the decoded resolution factor */
	opj_bool (*opj_set_decoded_resolution_factor) ( void * p_codec, 
                                                    OPJ_UINT32 res_factor, 
                                                    opj_event_mgr_t * p_manager);

}opj_decompression_t;

typedef struct opj_compression
{
	opj_bool (* opj_start_compress) (	void *p_codec,
										struct opj_stream_private *cio,
										struct opj_image * p_image,
										struct opj_event_mgr * p_manager);

	opj_bool (* opj_encode) (	void * p_codec,
								struct opj_stream_private *p_cio,
								struct opj_event_mgr * p_manager);

	opj_bool (* opj_write_tile) (	void * p_codec,
									OPJ_UINT32 p_tile_index,
									OPJ_BYTE * p_data,
									OPJ_UINT32 p_data_size,
									struct opj_stream_private * p_cio,
									struct opj_event_mgr * p_manager);

	opj_bool (* opj_end_compress) (	void * p_codec,
									struct opj_stream_private *p_cio,
									struct opj_event_mgr * p_manager);

	void (* opj_destroy) (void * p_codec);

	void (*opj_setup_encoder) (	void * p_codec,
								opj_cparameters_t * p_param,
								struct opj_image * p_image,
								struct opj_event_mgr * p_manager);

}opj_compression_t;

typedef struct opj_codec_private
{
	/** FIXME DOC */
	union 
    {
        opj_decompression_t m_decompression;
        opj_compression_t m_compression;
    } m_codec_data;
    /** FIXME DOC*/
	void * m_codec;
	/** Event handler */
	opj_event_mgr_t m_event_mgr;
	/** Flag to indicate if the codec is used to decode or encode*/
	opj_bool is_decompressor;
	void (*opj_dump_codec) (void * p_codec, OPJ_INT32 info_flag, FILE* output_stream);
	opj_codestream_info_v2_t* (*opj_get_codec_info)(void* p_codec);
	opj_codestream_index_t* (*opj_get_codec_index)(void* p_codec);
}
opj_codec_private_t;

static opj_bool opj_jp2_write_jp(	opj_jp2_v2_t *jp2,
			    		    opj_stream_private_t *cio,
				    		opj_event_mgr_t * p_manager )
{
	/* 12 bytes will be read */
	unsigned char l_signature_data [12];

	/* preconditions */
	assert(cio != 00);
	assert(jp2 != 00);
	assert(p_manager != 00);

	/* write box length */
	opj_write_bytes(l_signature_data,12,4);
	/* writes box type */
	opj_write_bytes(l_signature_data+4,JP2_JP,4);
	/* writes magic number*/
	opj_write_bytes(l_signature_data+8,0x0d0a870a,4);
	
	if (opj_stream_write_data(cio,l_signature_data,12,p_manager) != 12) {
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}

static opj_bool opj_jp2_write_ftyp(opj_jp2_v2_t *jp2,
							opj_stream_private_t *cio,
							opj_event_mgr_t * p_manager )
{
	unsigned int i;
	unsigned int l_ftyp_size = 16 + 4 * jp2->numcl;
	unsigned char * l_ftyp_data, * l_current_data_ptr;
	opj_bool l_result;

	/* preconditions */
	assert(cio != 00);
	assert(jp2 != 00);
	assert(p_manager != 00);

	l_ftyp_data = (unsigned char *) opj_malloc(l_ftyp_size);
	
	if (l_ftyp_data == 00) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Not enough memory to handle ftyp data\n");
		return OPJ_FALSE;
	}

	memset(l_ftyp_data,0,l_ftyp_size);

	l_current_data_ptr = l_ftyp_data;

	opj_write_bytes(l_current_data_ptr, l_ftyp_size,4); /* box size */
	l_current_data_ptr += 4;

	opj_write_bytes(l_current_data_ptr, JP2_FTYP,4); /* FTYP */
	l_current_data_ptr += 4;

	opj_write_bytes(l_current_data_ptr, jp2->brand,4); /* BR */
	l_current_data_ptr += 4;

	opj_write_bytes(l_current_data_ptr, jp2->minversion,4); /* MinV */
	l_current_data_ptr += 4;

	for (i = 0; i < jp2->numcl; i++)  {
		opj_write_bytes(l_current_data_ptr, jp2->cl[i],4);	/* CL */
	}
	
	l_result = (opj_stream_write_data(cio,l_ftyp_data,l_ftyp_size,p_manager) == l_ftyp_size);
	if (! l_result)
	{
		opj_event_msg_v2(p_manager, EVT_ERROR, "Error while writing ftyp data to stream\n");
	}

	opj_free(l_ftyp_data);
	
	return l_result;
}

static opj_bool opj_jp2_default_validation (	opj_jp2_v2_t * jp2,
                                        opj_stream_private_t *cio,
                                        opj_event_mgr_t * p_manager
                                        )
{
	opj_bool l_is_valid = OPJ_TRUE;
	unsigned int i;

	/* preconditions */
	assert(jp2 != 00);
	assert(cio != 00);
	assert(p_manager != 00);

	/* JPEG2000 codec validation */
	/*TODO*/

	/* STATE checking */
	/* make sure the state is at 0 */
	l_is_valid &= (jp2->jp2_state == JP2_STATE_NONE);

	/* make sure not reading a jp2h ???? WEIRD */
	l_is_valid &= (jp2->jp2_img_state == JP2_IMG_STATE_NONE);

	/* POINTER validation */
	/* make sure a j2k codec is present */
	l_is_valid &= (jp2->j2k != 00);

	/* make sure a procedure list is present */
	l_is_valid &= (jp2->m_procedure_list != 00);

	/* make sure a validation list is present */
	l_is_valid &= (jp2->m_validation_list != 00);

	/* PARAMETER VALIDATION */
	/* number of components */
	l_is_valid &= (jp2->numcl > 0);
	/* width */
	l_is_valid &= (jp2->h > 0);
	/* height */
	l_is_valid &= (jp2->w > 0);
	/* precision */
	for (i = 0; i < jp2->numcomps; ++i)	{
		l_is_valid &= (jp2->comps[i].bpcc > 0);
	}

	/* METH */
	l_is_valid &= ((jp2->meth > 0) && (jp2->meth < 3));

	/* stream validation */
	/* back and forth is needed */
	l_is_valid &= opj_stream_has_seek(cio);

	return l_is_valid;
}

static void opj_jp2_setup_encoding_validation (opj_jp2_v2_t *jp2)
{
	/* preconditions */
	assert(jp2 != 00);

	opj_procedure_list_add_procedure(jp2->m_validation_list, (opj_procedure)opj_jp2_default_validation);
	/* DEVELOPER CORNER, add your custom validation procedure */
}

static opj_bool opj_jp2_skip_jp2c(	opj_jp2_v2_t *jp2,
					    	opj_stream_private_t *stream,
					    	opj_event_mgr_t * p_manager )
{
	/* preconditions */
	assert(jp2 != 00);
	assert(stream != 00);
	assert(p_manager != 00);

	jp2->j2k_codestream_offset = opj_stream_tell(stream);

	if (opj_stream_skip(stream,8,p_manager) != 8) {
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}

static opj_bool opj_jpip_skip_iptr(	opj_jp2_v2_t *jp2,
					    	opj_stream_private_t *stream,
					    	opj_event_mgr_t * p_manager )
{
	/* preconditions */
	assert(jp2 != 00);
	assert(stream != 00);
	assert(p_manager != 00);

	jp2->jpip_iptr_offset = opj_stream_tell(stream);

	if (opj_stream_skip(stream,24,p_manager) != 24) {
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}

void opj_jpip_setup_header_writing (opj_jp2_v2_t *jp2)
{
	/* preconditions */
	assert(jp2 != 00);

	opj_procedure_list_add_procedure(jp2->m_procedure_list,(opj_procedure)opj_jp2_write_jp );
	opj_procedure_list_add_procedure(jp2->m_procedure_list,(opj_procedure)opj_jp2_write_ftyp );
	opj_procedure_list_add_procedure(jp2->m_procedure_list,(opj_procedure)opj_jp2_write_jp2h );
	opj_procedure_list_add_procedure(jp2->m_procedure_list,(opj_procedure)opj_jpip_skip_iptr );
	opj_procedure_list_add_procedure(jp2->m_procedure_list,(opj_procedure)opj_jp2_skip_jp2c );

	/* DEVELOPER CORNER, insert your custom procedures */

}

static opj_bool opj_jp2_exec (  opj_jp2_v2_t * jp2,
                                opj_procedure_list_t * p_procedure_list,
                                opj_stream_private_t *stream,
                                opj_event_mgr_t * p_manager
                                )

{
	opj_bool (** l_procedure) (opj_jp2_v2_t * jp2, opj_stream_private_t *, opj_event_mgr_t *) = 00;
	opj_bool l_result = OPJ_TRUE;
	OPJ_UINT32 l_nb_proc, i;

	/* preconditions */
	assert(p_procedure_list != 00);
	assert(jp2 != 00);
	assert(stream != 00);
	assert(p_manager != 00);

	l_nb_proc = opj_procedure_list_get_nb_procedures(p_procedure_list);
	l_procedure = (opj_bool (**) (opj_jp2_v2_t * jp2, opj_stream_private_t *, opj_event_mgr_t *)) opj_procedure_list_get_first_procedure(p_procedure_list);

	for	(i=0;i<l_nb_proc;++i) {
		l_result = l_result && (*l_procedure) (jp2,stream,p_manager);
		++l_procedure;
	}

	/* and clear the procedure list at the end. */
	opj_procedure_list_clear(p_procedure_list);
	return l_result;
}

opj_bool opj_jpip_start_compress(opj_jp2_v2_t *jp2,
                                opj_stream_private_t *stream,
                                opj_image_t * p_image,
                                opj_event_mgr_t * p_manager
                                )
{
	/* preconditions */
	assert(jp2 != 00);
	assert(stream != 00);
	assert(p_manager != 00);

	/* customization of the validation */
	opj_jp2_setup_encoding_validation (jp2);

	/* validation of the parameters codec */
	if (! opj_jp2_exec(jp2,jp2->m_validation_list,stream,p_manager)) {
		return OPJ_FALSE;
	}

	/* customization of the encoding */
	opj_jpip_setup_header_writing(jp2);

	/* write header */
	if (! opj_jp2_exec (jp2,jp2->m_procedure_list,stream,p_manager)) {
		return OPJ_FALSE;
	}

	return opj_j2k_start_compress(jp2->j2k,stream,p_image,p_manager);
}

static opj_bool opj_jpip_write_iptr(opj_jp2_v2_t *jp2,
							opj_stream_private_t *cio,
							opj_event_mgr_t * p_manager )
{
	OPJ_OFF_T j2k_codestream_exit;
	OPJ_BYTE l_data_header [24];
	
	/* preconditions */
	assert(jp2 != 00);
	assert(cio != 00);
	assert(p_manager != 00);
	assert(opj_stream_has_seek(cio));
	
	j2k_codestream_exit = opj_stream_tell(cio);
	opj_write_bytes(l_data_header, 24, 4); /* size of iptr */
	opj_write_bytes(l_data_header + 4,JPIP_IPTR,4);									   /* IPTR */
#if 0
	opj_write_bytes(l_data_header + 4 + 4, 0, 8); /* offset */
	opj_write_bytes(l_data_header + 8 + 8, 0, 8); /* length */
#else
  opj_write_double(l_data_header + 4 + 4, 0); /* offset */
  opj_write_double(l_data_header + 8 + 8, 0); /* length */
#endif

	if (! opj_stream_seek(cio,jp2->jpip_iptr_offset,p_manager)) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
		return OPJ_FALSE;
	}
	
	if (opj_stream_write_data(cio,l_data_header,24,p_manager) != 24) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
		return OPJ_FALSE;
	}

	if (! opj_stream_seek(cio,j2k_codestream_exit,p_manager)) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}

static opj_bool opj_jpip_write_fidx(opj_jp2_v2_t *jp2,
							opj_stream_private_t *cio,
							opj_event_mgr_t * p_manager )
{
	OPJ_OFF_T j2k_codestream_exit;
	OPJ_BYTE l_data_header [24];
	
	/* preconditions */
	assert(jp2 != 00);
	assert(cio != 00);
	assert(p_manager != 00);
	assert(opj_stream_has_seek(cio));
	
	opj_write_bytes(l_data_header, 24, 4); /* size of iptr */
	opj_write_bytes(l_data_header + 4,JPIP_FIDX,4);									   /* IPTR */
  opj_write_double(l_data_header + 4 + 4, 0); /* offset */
  opj_write_double(l_data_header + 8 + 8, 0); /* length */

	if (opj_stream_write_data(cio,l_data_header,24,p_manager) != 24) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
		return OPJ_FALSE;
	}

	j2k_codestream_exit = opj_stream_tell(cio);
	if (! opj_stream_seek(cio,j2k_codestream_exit,p_manager)) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}

static opj_bool opj_jpip_write_cidx(opj_jp2_v2_t *jp2,
							opj_stream_private_t *cio,
							opj_event_mgr_t * p_manager )
{
	OPJ_OFF_T j2k_codestream_exit;
	OPJ_BYTE l_data_header [24];
	
	/* preconditions */
	assert(jp2 != 00);
	assert(cio != 00);
	assert(p_manager != 00);
	assert(opj_stream_has_seek(cio));
	
	j2k_codestream_exit = opj_stream_tell(cio);
	opj_write_bytes(l_data_header, 24, 4); /* size of iptr */
	opj_write_bytes(l_data_header + 4,JPIP_CIDX,4);									   /* IPTR */
#if 0
	opj_write_bytes(l_data_header + 4 + 4, 0, 8); /* offset */
	opj_write_bytes(l_data_header + 8 + 8, 0, 8); /* length */
#else
  opj_write_double(l_data_header + 4 + 4, 0); /* offset */
  opj_write_double(l_data_header + 8 + 8, 0); /* length */
#endif

	if (! opj_stream_seek(cio,j2k_codestream_exit,p_manager)) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
		return OPJ_FALSE;
	}
	
	if (opj_stream_write_data(cio,l_data_header,24,p_manager) != 24) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
		return OPJ_FALSE;
	}

	j2k_codestream_exit = opj_stream_tell(cio);
	if (! opj_stream_seek(cio,j2k_codestream_exit,p_manager)) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
		return OPJ_FALSE;
	}

	return OPJ_TRUE;
}

static void write_prxy_v2( int offset_jp2c, int length_jp2c, int offset_idx, int length_idx, opj_stream_private_t *cio,
              opj_event_mgr_t * p_manager )
{
  OPJ_BYTE l_data_header [8];
  int len, lenp;

#if 0
  lenp = cio_tell( cio);
  cio_skip( cio, 4);              /* L [at the end] */
  cio_write( cio, JPIP_PRXY, 4);  /* IPTR           */
#else
  lenp = opj_stream_tell(cio);
  opj_stream_skip(cio, 4, p_manager);         /* L [at the end] */
  opj_write_bytes(l_data_header,JPIP_PRXY,4); /* IPTR           */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
#endif
  
#if 0
  cio_write( cio, offset_jp2c, 8); /* OOFF           */
  cio_write( cio, length_jp2c, 4); /* OBH part 1     */
  cio_write( cio, JP2_JP2C, 4);    /* OBH part 2     */
#else
  opj_write_bytes( l_data_header, offset_jp2c, 8); /* OOFF           */
  opj_stream_write_data(cio,l_data_header,8,p_manager);
  opj_write_bytes( l_data_header, length_jp2c, 4); /* OBH part 1     */
  opj_write_bytes( l_data_header+4, JP2_JP2C, 4);  /* OBH part 2     */
  opj_stream_write_data(cio,l_data_header,8,p_manager);
#endif
  
#if 0
  cio_write( cio, 1,1);           /* NI             */
#else
  opj_write_bytes( l_data_header, 1, 1);/* NI             */
  opj_stream_write_data(cio,l_data_header,1,p_manager);
#endif

#if 0
  cio_write( cio, offset_idx, 8);  /* IOFF           */
  cio_write( cio, length_idx, 4);  /* IBH part 1     */
  cio_write( cio, JPIP_CIDX, 4);   /* IBH part 2     */
#else
  opj_write_bytes( l_data_header, offset_idx, 8);  /* IOFF           */
  opj_stream_write_data(cio,l_data_header,8,p_manager);
  opj_write_bytes( l_data_header, length_idx, 4);  /* IBH part 1     */
  opj_write_bytes( l_data_header+4, JPIP_CIDX, 4);   /* IBH part 2     */
  opj_stream_write_data(cio,l_data_header,8,p_manager);
#endif

#if 0
  len = cio_tell( cio)-lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);        /* L              */
  cio_seek( cio, lenp+len);
#else
  len = opj_stream_tell(cio)-lenp;
  opj_stream_skip(cio, lenp, p_manager);
  opj_write_bytes(l_data_header,len,4);/* L              */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
  opj_stream_seek(cio, lenp+len,p_manager);
#endif
}


static int write_fidx_v2( int offset_jp2c, int length_jp2c, int offset_idx, int length_idx, opj_stream_private_t *cio,
              opj_event_mgr_t * p_manager )
{  
  OPJ_BYTE l_data_header [4];
  int len, lenp;
  
#if 0
  lenp = cio_tell( cio);
  cio_skip( cio, 4);              /* L [at the end] */
  cio_write( cio, JPIP_FIDX, 4);  /* IPTR           */
#else
  lenp = opj_stream_tell(cio);
  opj_stream_skip(cio, 4, p_manager);
  opj_write_bytes(l_data_header,JPIP_FIDX,4); /* FIDX */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
#endif
  
  write_prxy_v2( offset_jp2c, length_jp2c, offset_idx, length_idx, cio,p_manager);

#if 0
  len = cio_tell( cio)-lenp;
  cio_seek( cio, lenp);
  cio_write( cio, len, 4);        /* L              */
  cio_seek( cio, lenp+len);  
#else
  len = opj_stream_tell(cio)-lenp;
  opj_stream_skip(cio, lenp, p_manager);
  opj_write_bytes(l_data_header,len,4);/* L              */
  opj_stream_write_data(cio,l_data_header,4,p_manager);
  opj_stream_seek(cio, lenp+len,p_manager);
#endif

  return len;
}

static opj_bool opj_jpip_write_jp2c(opj_jp2_v2_t *jp2,
							opj_stream_private_t *cio,
							opj_event_mgr_t * p_manager )
{
	OPJ_OFF_T j2k_codestream_exit;
	OPJ_BYTE l_data_header [8];
  OPJ_UINT32 len_jp2c;
  int len_cidx;
  int len_fidx;
  int pos_jp2c;
  int pos_fidx;
  int pos_cidx;
	
	/* preconditions */
	assert(jp2 != 00);
	assert(cio != 00);
	assert(p_manager != 00);
	assert(opj_stream_has_seek(cio));
	
	j2k_codestream_exit = opj_stream_tell(cio);
  len_jp2c = j2k_codestream_exit - jp2->j2k_codestream_offset;
  pos_jp2c = jp2->j2k_codestream_offset;
	opj_write_bytes(l_data_header, len_jp2c, 4); /* size of codestream */
	opj_write_bytes(l_data_header + 4,JP2_JP2C,4);									   /* JP2C */

	if (! opj_stream_seek(cio,jp2->j2k_codestream_offset,p_manager)) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
		return OPJ_FALSE;
	}
	
	if (opj_stream_write_data(cio,l_data_header,8,p_manager) != 8) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
		return OPJ_FALSE;
	}

	if (! opj_stream_seek(cio,j2k_codestream_exit,p_manager)) {
		opj_event_msg_v2(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
		return OPJ_FALSE;
	}

  /* CIDX */
  pos_cidx = opj_stream_tell( cio);
  len_cidx = write_cidx_v2( pos_jp2c+8, cio, jp2_get_cstr_info(jp2), len_jp2c-8);

  /* FIDX */
  pos_fidx = opj_stream_tell( cio);
  len_fidx = write_fidx_v2( pos_jp2c, len_jp2c, pos_cidx, len_cidx, cio, p_manager);

	return OPJ_TRUE;
}

static void opj_jp2_setup_end_header_writing (opj_jp2_v2_t *jp2)
{
	/* preconditions */
	assert(jp2 != 00);

	opj_procedure_list_add_procedure(jp2->m_procedure_list,(opj_procedure)opj_jpip_write_iptr );
	opj_procedure_list_add_procedure(jp2->m_procedure_list,(opj_procedure)opj_jpip_write_jp2c );
#if 0
  opj_procedure_list_add_procedure(jp2->m_procedure_list,(opj_procedure)opj_jpip_write_cidx );
  opj_procedure_list_add_procedure(jp2->m_procedure_list,(opj_procedure)opj_jpip_write_fidx );
#endif
	/* DEVELOPER CORNER, add your custom procedures */
}

opj_bool opj_jpip_end_compress(	opj_jp2_v2_t *jp2,
							    opj_stream_private_t *cio,
							    opj_event_mgr_t * p_manager
                                )
{
	/* preconditions */
	assert(jp2 != 00);
	assert(cio != 00);
	assert(p_manager != 00);

	/* customization of the end encoding */
	opj_jp2_setup_end_header_writing(jp2);

	if (! opj_j2k_end_compress(jp2->j2k,cio,p_manager)) {
		return OPJ_FALSE;
	}

	/* write header */
	return opj_jp2_exec(jp2,jp2->m_procedure_list,cio,p_manager);
}


opj_codec_t* OPJ_CALLCONV opj_jpip_create_compress(OPJ_CODEC_FORMAT p_format)
{
	opj_codec_private_t *l_codec = 00;

	l_codec = (opj_codec_private_t*)opj_calloc(1, sizeof(opj_codec_private_t));
	if (!l_codec) {
		return 00;
	}
	memset(l_codec, 0, sizeof(opj_codec_private_t));
	
	l_codec->is_decompressor = 0;

	switch(p_format) {
		case CODEC_JP2:
			/* get a JP2 decoder handle */
			l_codec->m_codec_data.m_compression.opj_encode = (opj_bool (*) (void *,
																			struct opj_stream_private *,
																			struct opj_event_mgr * )) opj_jp2_encode;

			l_codec->m_codec_data.m_compression.opj_end_compress = (opj_bool (*) (	void *,
																					struct opj_stream_private *,
																					struct opj_event_mgr *)) opj_jpip_end_compress;

			l_codec->m_codec_data.m_compression.opj_start_compress = (opj_bool (*) (void *,
																					struct opj_stream_private *,
																					struct opj_image * ,
																					struct opj_event_mgr *))  opj_jpip_start_compress;

			l_codec->m_codec_data.m_compression.opj_write_tile = (opj_bool (*) (void *,
																				OPJ_UINT32,
																				OPJ_BYTE*,
																				OPJ_UINT32,
																				struct opj_stream_private *,
																				struct opj_event_mgr *)) opj_jp2_write_tile;

			l_codec->m_codec_data.m_compression.opj_destroy = (void (*) (void *)) opj_jp2_destroy;

			l_codec->m_codec_data.m_compression.opj_setup_encoder = (void (*) (	void *,
																				opj_cparameters_t *,
																				struct opj_image *,
																				struct opj_event_mgr * )) opj_jp2_setup_encoder;

			l_codec->m_codec = opj_jp2_create(OPJ_FALSE);
			if (! l_codec->m_codec) {
				opj_free(l_codec);
				return 00;
			}

			break;

		case CODEC_UNKNOWN:
		case CODEC_JPT:
		default:
			opj_free(l_codec);
			return 00;
	}

	opj_set_default_event_handler(&(l_codec->m_event_mgr));
	return (opj_codec_t*) l_codec;
}

#endif /*SERVER*/
