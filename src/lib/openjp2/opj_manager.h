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
#ifndef __OPJ_MANAGER_H
#define __OPJ_MANAGER_H
/**
@file opj_manager.h
*/

/**
 * OpenJPEG Manager opaque structure
 *
 * This object is responsible for memory allocations & events.
 * When no manager is used, events can still be retrieved for an opj_codec_t object using opj_set_info_handler, opj_set_warning_handler and opj_set_error_handler.
 * The use of a manager & those functions are mutually exclusive.
 *
 */
struct opj_manager
{
    /** callback user context that will be passed back each time an allocation callback is called. */
    void* context;

    /** malloc callback. */
    opj_malloc_callback malloc_callback;

    /** calloc callback. */
    opj_calloc_callback calloc_callback;

    /** realloc callback. */
    opj_realloc_callback realloc_callback;

    /** free callback. */
    opj_free_callback free_callback;

    /** aligned malloc callback. */
    opj_aligned_malloc_callback aligned_malloc_callback;

    /** aligned free callback. */
    opj_aligned_free_callback aligned_free_callback;

    /** event manager. */
    opj_event_mgr_t event_mgr;
};

/**
 * Get the global manager.
 *
 * @return Global manager
 */
opj_manager_t opj_manager_get_global_manager(void);


static INLINE void* opj_manager_malloc(opj_manager_t manager, OPJ_SIZE_T size)
{
    return manager->malloc_callback(size, manager->context);
}
static INLINE void* opj_manager_calloc(opj_manager_t manager, OPJ_SIZE_T num, OPJ_SIZE_T size)
{
    return manager->calloc_callback(num, size, manager->context);
}
static INLINE void* opj_manager_realloc(opj_manager_t manager, void* ptr, OPJ_SIZE_T size)
{
    return manager->realloc_callback(ptr, size, manager->context);
}
static INLINE void opj_manager_free(opj_manager_t manager, void* ptr)
{
    manager->free_callback(ptr, manager->context);
}
static INLINE void* opj_manager_aligned_malloc(opj_manager_t manager, OPJ_SIZE_T size, OPJ_SIZE_T alignment)
{
    return manager->aligned_malloc_callback(size, alignment, manager->context);
}
static INLINE void opj_manager_aligned_free(opj_manager_t manager, void* ptr)
{
    manager->aligned_free_callback(ptr, manager->context);
}

#endif /* __OPJ_MANAGER_H */

