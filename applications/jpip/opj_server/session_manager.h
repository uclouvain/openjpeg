/*
 * $Id: session_manager.h 53 2011-05-09 16:55:39Z kaori $
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

#ifndef   	SESSION_MANAGER_H_
# define   	SESSION_MANAGER_H_

#include "bool.h"
#include "channel_manager.h"
#include "target_manager.h"

//! Session parameters
typedef struct session_param{
  channellist_param_t *channellist; //!< channel list pointer
  targetlist_param_t  *targetlist;  //!< target  list pointer
  struct session_param *next;       //!< pointer to the next session
} session_param_t;

//! Session list parameters
typedef struct sessionlist_param{
  session_param_t *first; //!< first session pointer of the list
  session_param_t *last;  //!< last  session pointer of the list
} sessionlist_param_t;


/**
 * generate a session list
 *
 * @return pointer to the generated session list
 */
sessionlist_param_t * gene_sessionlist();


/**
 * generate a session under the sesion list
 *
 * @param[in] sessionlist session list to insert the new session
 * @return                pointer to the generated session
 */
session_param_t * gene_session( sessionlist_param_t *sessionlist);

/**
 * search a channel and its belonging session by channel ID
 *
 * @param[in]     cid           channel identifier
 * @param[in]     sessionlist   session list pointer
 * @param[in,out] foundsession  address of the found session pointer
 * @param[in,out] foundchannel  address of the found channel pointer
 * @return                      if the channel is found (true) or not (false)
 */
bool search_session_and_channel( char cid[], 
				 sessionlist_param_t *sessionlist, 
				 session_param_t **foundsession, 
				 channel_param_t **foundchannel);

/**
 * insert a target into a session
 *
 * @param[in] session session pointer
 * @param[in] target target pointer
 */
void insert_target_into_session( session_param_t *session, target_param_t *target);


/**
 * delete a session
 *
 * @param[in] session     address of the session pointer
 * @param[in] sessionlist session list pointer
 * @return                    if succeeded (true) or failed (false)
 */
bool delete_session( session_param_t **session, sessionlist_param_t *sessionlist);


/**
 * delete session list
 *
 * @param[in,out] sessionlist address of the session list pointer
 */
void delete_sessionlist( sessionlist_param_t **sessionlist);

/**
 * print all sessions
 *
 * @param[in] sessionlist session list pointer
 */
void print_allsession( sessionlist_param_t *sessionlist);


#endif 	    /* !SESSION_MANAGER_H_ */
