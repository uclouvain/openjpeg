/*
* Copyright (c) 2016, Aaron Boxer
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/
#ifndef __VECTOR_H
#define __VECTOR_H

/*
Vector - a dynamic array.

*/

typedef struct opj_vec{
	OPJ_INT32 size;     /* current size of vec */
	OPJ_INT32 capacity;  /* maximum size of vec */
	void* *data;		/* array of void* pointers */ 
	OPJ_BOOL owns_data;
} opj_vec_t;

/*
Initialize vector
*/

OPJ_BOOL opj_vec_init(opj_vec_t *vec, OPJ_BOOL owns_data);

OPJ_BOOL opj_vec_init_with_capacity(opj_vec_t *vec, OPJ_INT32 capacity, OPJ_BOOL owns_data);

/*
Add a value to the end of the vector
*/
OPJ_BOOL opj_vec_push_back(opj_vec_t *vec, void* value);

/*
Set a value at specified index. If index is greater then the size of the vector,
all intervening indices will be initialized to NULL
*/
OPJ_BOOL opj_vec_set(opj_vec_t *vec, OPJ_INT32 index, void* value);

/*
Get value at specified index
*/
void* opj_vec_get(opj_vec_t *vec, OPJ_INT32 index);

/*
Get value at end of vector
*/
void* opj_vec_back(opj_vec_t *vec);

/*
Clean up vector resources. Does NOT free vector itself
*/
void opj_vec_cleanup(opj_vec_t *vec);

/*
Clean up vector resources and free vector itself
*/
void opj_vec_destroy(opj_vec_t *vec);

#endif