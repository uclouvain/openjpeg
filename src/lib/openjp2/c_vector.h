//
// Created by caesar kekxv on 2020/4/17.
//

#ifndef CLANGTOOLS_VECTOR_H
#define CLANGTOOLS_VECTOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct c_vector;
typedef struct c_vector c_vector;

/**
 * init
 * @param cVector
 */
int c_vector_init(c_vector **cVector);

/**
 * get c_vector size
 * @param cVector
 * @return
 */
size_t c_vector_size(c_vector *cVector);

/**
 * resize c_vector
 * @param cVector
 */
static int c_vector_resize(c_vector *cVector, size_t count);

/**
 * push back c_vector
 * @param cVector
 * @param data
 * @param offset
 * @param count
 */
int c_vector_push_back(c_vector *cVector, void *data, size_t offset, size_t count);

/**
 * push zero to c_vector
 * @param cVector
 * @return
 */
int c_vector_push_back_zero(c_vector *cVector);

/**
 * set data
 * @param cVector
 * @param index
 * @param data
 * @param offset
 * @param count
 * @return
 */
size_t c_vector_set(c_vector *cVector, size_t index, void *data, size_t offset, size_t count);

/**
 * insert data
 * @param cVector
 * @param index
 * @param data
 * @param offset
 * @param count
 * @return
 */
size_t c_vector_insert(c_vector *cVector, size_t index, void *data, size_t offset, size_t count);

/**
 * get c_vector data
 * @param cVector
 * @param offset
 * @return
 */
void *c_vector_get(c_vector *cVector, size_t offset);

/**
 * get c_vector data
 * @param cVector
 * @return
 */
void *c_vector_data(c_vector *cVector);

/**
 * c_vector delete
 * @param cVector
 * @param offset
 * @param count
 */
size_t c_vector_delete(c_vector *cVector, size_t offset, size_t count);

/**
 * free c_vector
 * @param cVector
 */
void c_vector_free(c_vector **cVector);

int c_vector_seek(c_vector *cVector, size_t offset, int whence);
int c_vector_seekg(size_t offset,c_vector *cVector);
int c_vector_skip(size_t offset,c_vector *cVector);

size_t c_vector_read(void *p_buffer, size_t p_nb_bytes, c_vector *v);

size_t c_vector_write(void *p_buffer, size_t p_nb_bytes, c_vector *v);


#endif //CLANGTOOLS_VECTOR_H
