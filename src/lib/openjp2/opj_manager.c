/*
 * The copyright in this software is being made available under the 2-clauses 
 * BSD License, included below. This software may be subject to other third 
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2014, Matthieu Darbois
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

#include "opj_includes.h"
#include "opj_malloc.h"

/**
 * Default malloc function
 * @param  size        Size of the memory block, in bytes.
 * @param  client_data Client object where will be return the event message
 * @return returns     a pointer to the memory block allocated by the function.
 * */
static void* opj_default_malloc(OPJ_SIZE_T size, void *client_data);

/**
 * Default calloc function
 * @param  num         Number of elements to allocate.
 * @param  size        Size of an element, in bytes.
 * @param  client_data Client object where will be return the event message
 * @return returns     a pointer to the memory block allocated by the function.
 * */
static void* opj_default_calloc(OPJ_SIZE_T num, OPJ_SIZE_T size, void *client_data);

/**
 * Default realloc function
 * @param ptr         Pointer to a memory block previously allocated with opj_malloc_callback, opj_calloc_callback or opj_realloc_callback.
 * @param size        New size for the memory block, in bytes.
 * @param client_data Client object where will be return the event message
 * @return returns    a pointer to the reallocated memory block, which may be either the same as ptr or a new location.
 * */
static void* opj_default_realloc(void* ptr, OPJ_SIZE_T size, void *client_data);

/**
 * Default free function
 * @param  ptr         Pointer to a memory block previously allocated with opj_malloc_callback, opj_calloc_callback or opj_realloc_callback.
 * @param  client_data Client object where will be return the event message
 * @return returns     a pointer to the memory block allocated by the function.
 * */
static void opj_default_free(void* ptr, void *client_data);

/**
 * Default aligned_malloc function
 * @param  size        Size of the memory block, in bytes.
 * @param  alignment   The value of alignment shall be a multiple of sizeof( void *), that is also a power of two.
 * @param  client_data Client object where will be return the event message
 * @return returns     a pointer to the memory block allocated by the function.
 * */
static void* opj_default_aligned_malloc(OPJ_SIZE_T size, OPJ_SIZE_T alignment, void *client_data);

/**
 * Default aligned_free function
 * @param  ptr         Pointer to a memory block previously allocated with opj_aligned_malloc_callback.
 * @param  client_data Client object where will be return the event message
 * @return returns     a pointer to the memory block allocated by the function.
 * */
static void opj_default_aligned_free(void* ptr, void *client_data);

/**
 * Default manager that can be used when no manager is available.
 *
 * */
static const struct opj_manager opj_global_manager =
{
    NULL,
    opj_default_malloc,
    opj_default_calloc,
    opj_default_realloc,
    opj_default_free,
    opj_default_aligned_malloc,
    opj_default_aligned_free,
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};


OPJ_API opj_manager_t OPJ_CALLCONV opj_manager_create(
    void*                       context,
    opj_malloc_callback         malloc_callback,
    opj_calloc_callback         calloc_callback,
    opj_realloc_callback        realloc_callback,
    opj_free_callback           free_callback,
    opj_aligned_malloc_callback aligned_malloc_callback,
    opj_aligned_free_callback   aligned_free_callback
    )
{
    opj_manager_t l_manager = NULL;

    /* Check parameters */
    if (
        (malloc_callback == NULL) ||
        (calloc_callback == NULL) ||
        (realloc_callback == NULL) ||
        (free_callback == NULL) ||
        (aligned_malloc_callback == NULL) ||
        (aligned_free_callback == NULL)
    ) {
        return NULL;
    }

    l_manager = malloc_callback(sizeof(struct opj_manager), context);
    if (l_manager == NULL) {
        return NULL;
    }
    l_manager->context = context;
    l_manager->malloc_callback = malloc_callback;
    l_manager->calloc_callback = calloc_callback;
    l_manager->realloc_callback = realloc_callback;
    l_manager->free_callback = free_callback;
    l_manager->aligned_malloc_callback = aligned_malloc_callback;
    l_manager->aligned_free_callback = aligned_free_callback;

    /* NULL message handler are valid (& more efficient than default handler from opj_set_default_event_handler) */
    memset(&(l_manager->event_mgr), 0, sizeof(opj_event_mgr_t));

    return l_manager;
}


OPJ_API opj_manager_t OPJ_CALLCONV opj_manager_create_default(void)
{
    return opj_manager_create(
        NULL,
        opj_default_malloc,
        opj_default_calloc,
        opj_default_realloc,
        opj_default_free,
        opj_default_aligned_malloc,
        opj_default_aligned_free
    );
}

OPJ_API void OPJ_CALLCONV opj_manager_destroy(opj_manager_t manager)
{
    opj_free_callback l_callback = NULL;
    void* l_context = NULL;
    if (manager != NULL) {
        l_callback = manager->free_callback;
        l_context = manager->context;
        memset(manager, 0, sizeof(struct opj_manager));
        l_callback(manager, l_context);
    }
}

OPJ_API OPJ_BOOL OPJ_CALLCONV opj_manager_set_info_handler(opj_manager_t manager,
                                                           opj_msg_callback p_callback,
                                                           void * p_user_data)
{
    if(manager == NULL){
        return OPJ_FALSE;
    }

    manager->event_mgr.info_handler = p_callback;
    manager->event_mgr.m_info_data = p_user_data;
    return OPJ_TRUE;
}

OPJ_API OPJ_BOOL OPJ_CALLCONV opj_manager_set_warning_handler(opj_manager_t manager,
                                                              opj_msg_callback p_callback,
                                                              void * p_user_data)
{
	if(manager == NULL){
		return OPJ_FALSE;
	}
	
	manager->event_mgr.warning_handler = p_callback;
	manager->event_mgr.m_warning_data = p_user_data;
	return OPJ_TRUE;
}

OPJ_API OPJ_BOOL OPJ_CALLCONV opj_manager_set_error_handler(opj_manager_t manager,
                                                            opj_msg_callback p_callback,
                                                            void * p_user_data)
{
    if(manager == NULL){
        return OPJ_FALSE;
    }
    
    manager->event_mgr.error_handler = p_callback;
    manager->event_mgr.m_error_data = p_user_data;
    return OPJ_TRUE;
}

opj_manager_t opj_manager_get_global_manager(void)
{
    return (opj_manager_t)&opj_global_manager; /* const is dropped */
}

static void* opj_default_malloc(OPJ_SIZE_T size, void *client_data)
{
    OPJ_ARG_NOT_USED(client_data);
    return opj_malloc(size);
}
static void* opj_default_calloc(OPJ_SIZE_T num, OPJ_SIZE_T size, void *client_data)
{
    OPJ_ARG_NOT_USED(client_data);
    return opj_calloc(num, size);
}
static void* opj_default_realloc(void* ptr, OPJ_SIZE_T size, void *client_data)
{
    OPJ_ARG_NOT_USED(client_data);
    return opj_realloc(ptr, size);
}
static void opj_default_free(void* ptr, void *client_data)
{
    OPJ_ARG_NOT_USED(client_data);
    opj_free(ptr);
}
static void* opj_default_aligned_malloc(OPJ_SIZE_T size, OPJ_SIZE_T alignment, void *client_data)
{
    OPJ_ARG_NOT_USED(client_data);
    return opj_aligned_malloc(size, alignment);
}
static void opj_default_aligned_free(void* ptr, void *client_data)
{
    OPJ_ARG_NOT_USED(client_data);
    opj_aligned_free(ptr);
}

