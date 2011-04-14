/*
 * $Id: target_manager.h 44 2011-02-15 12:32:29Z kaori $
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

#ifndef   	TARGET_MANAGER_H_
# define   	TARGET_MANAGER_H_

#include "index_manager.h"

//! maximum length of target name
#define MAX_LENOFTARGET 128

//! target parameters
typedef struct target_param{
  char filename[MAX_LENOFTARGET]; //!< file name
  int fd;                         //!< file descriptor
  int csn;                        //!< codestream number
  index_param_t *codeidx;         //!< index information of codestream
  struct target_param *next;      //!< pointer to the next target
} target_param_t;


//! Target list parameters
typedef struct targetlist_param{
  target_param_t *first; //!< first target pointer of the list
  target_param_t *last;  //!< last  target pointer of the list
} targetlist_param_t;



/**
 * generate a target list
 *
 * @return pointer to the generated target list
 */
targetlist_param_t * gene_targetlist();


/**
 * generate a target
 *
 * @param[in] targetname target file name
 * @return               pointer to the generated target
 */
target_param_t * gene_target( char *targetname);


/**
 * delete a target
 *
 * @param[in,out] target address of the deleting target pointer
 */
void delete_target( target_param_t **target);


/**
 * delete a target in list
 *
 * @param[in,out] target     address of the deleting target pointer
 * @param[in] targetlist target list pointer
 */
void delete_target_in_list( target_param_t **target, targetlist_param_t *targetlist);


/**
 * delete target list
 *
 * @param[in,out] targetlist address of the target list pointer
 */
void delete_targetlist(targetlist_param_t **targetlist);

/**
 * print all target parameters
 *
 * @param[in] targetlist target list pointer
 */
void print_alltarget( targetlist_param_t *targetlist);


/**
 * search a target by filename
 *
 * @param[in] targetname target filename
 * @param[in] targetlist target list pointer
 * @return               found target pointer
 */
target_param_t * search_target( char targetname[], targetlist_param_t *targetlist);

#endif 	    /* !TARGET_MANAGER_H_ */

