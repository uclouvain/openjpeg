/*
* Copyright (c) 2016, Aaron Boxer
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/

#include "opj_includes.h"


#ifdef _WIN32
#include <windows.h>
#else /* _WIN32 */

#include <errno.h>

#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>
# include <unistd.h>
# include <sys/mman.h>

#endif

#include <fcntl.h>


#ifdef _WIN32
typedef void* opj_handle_t;
#else
typedef OPJ_INT32 opj_handle_t;
#endif


typedef struct opj_buf_info
{
	OPJ_BYTE *buf;
	OPJ_OFF_T off;
	OPJ_SIZE_T len;
	opj_handle_t fd;		// for file mapping
} opj_buf_info_t;

static void opj_free_buffer_info(void* user_data) {
	if (user_data)
		opj_free(user_data);
}

static OPJ_SIZE_T opj_zero_copy_read_from_buffer(void ** p_buffer,
	OPJ_SIZE_T p_nb_bytes,
	opj_buf_info_t* p_source_buffer)
{
	OPJ_SIZE_T l_nb_read = 0;

	if (((OPJ_SIZE_T)p_source_buffer->off + p_nb_bytes) < p_source_buffer->len) {
		l_nb_read = p_nb_bytes;
	}

	*p_buffer = p_source_buffer->buf + p_source_buffer->off;
	p_source_buffer->off += (OPJ_OFF_T)l_nb_read;

	return l_nb_read ? l_nb_read : ((OPJ_SIZE_T)-1);
}

static OPJ_SIZE_T opj_read_from_buffer(void * p_buffer,
	OPJ_SIZE_T p_nb_bytes,
	opj_buf_info_t* p_source_buffer)
{
	OPJ_SIZE_T l_nb_read;

	if ((OPJ_SIZE_T)p_source_buffer->off + p_nb_bytes < p_source_buffer->len) {
		l_nb_read = p_nb_bytes;
	}
	else {
		l_nb_read = (p_source_buffer->len - (OPJ_SIZE_T)p_source_buffer->off);
	}
	memcpy(p_buffer, p_source_buffer->buf + p_source_buffer->off, l_nb_read);
	p_source_buffer->off += (OPJ_OFF_T)l_nb_read;

	return l_nb_read ? l_nb_read : ((OPJ_SIZE_T)-1);
}

static OPJ_SIZE_T opj_write_from_buffer(void * p_buffer,
	OPJ_SIZE_T p_nb_bytes,
	opj_buf_info_t* p_source_buffer)
{
	memcpy(p_source_buffer->buf + (OPJ_SIZE_T)p_source_buffer->off, p_buffer, p_nb_bytes);
	p_source_buffer->off += (OPJ_OFF_T)p_nb_bytes;
	p_source_buffer->len += p_nb_bytes;

	return p_nb_bytes;
}

static OPJ_OFF_T opj_skip_from_buffer(OPJ_OFF_T p_nb_bytes,
	opj_buf_info_t * p_source_buffer)
{
	if (p_source_buffer->off + p_nb_bytes <  (OPJ_OFF_T)p_source_buffer->len) {
		p_source_buffer->off += p_nb_bytes;
	}
	else {
		p_source_buffer->off = (OPJ_OFF_T)p_source_buffer->len;
	}
	return p_nb_bytes;
}

static OPJ_BOOL opj_seek_from_buffer(OPJ_OFF_T p_nb_bytes,
	opj_buf_info_t * p_source_buffer)
{
	if (p_nb_bytes <  (OPJ_OFF_T)p_source_buffer->len) {
		p_source_buffer->off = p_nb_bytes;
	}
	else {
		p_source_buffer->off = (OPJ_OFF_T)p_source_buffer->len;
	}
	return OPJ_TRUE;
}


static void opj_set_up_buffer_stream(opj_stream_t* l_stream, OPJ_SIZE_T len, OPJ_BOOL p_is_read_stream) {
	opj_stream_set_user_data_length(l_stream, len);

	if (p_is_read_stream) {
		opj_stream_set_read_function(l_stream, (opj_stream_read_fn)opj_read_from_buffer);
		opj_stream_set_zero_copy_read_function(l_stream, (opj_stream_zero_copy_read_fn)opj_zero_copy_read_from_buffer);
	}
	else
		opj_stream_set_write_function(l_stream, (opj_stream_write_fn)opj_write_from_buffer);
	opj_stream_set_skip_function(l_stream, (opj_stream_skip_fn)opj_skip_from_buffer);
	opj_stream_set_seek_function(l_stream, (opj_stream_seek_fn)opj_seek_from_buffer);
}

opj_stream_t*  opj_create_buffer_stream(OPJ_BYTE *buf,
										OPJ_SIZE_T len,
										OPJ_BOOL p_is_read_stream)
{
	opj_stream_t* l_stream;
	opj_buf_info_t* p_source_buffer = NULL;

	if (!buf || !len)
		return NULL;

	p_source_buffer = (opj_buf_info_t*)opj_malloc(sizeof(opj_buf_info_t));
	if (!p_source_buffer)
		return NULL;

	l_stream = opj_stream_create(0, p_is_read_stream);
	if (!l_stream) {
		opj_free(p_source_buffer);
		return NULL;

	}

	memset(p_source_buffer, 0, sizeof(opj_buf_info_t));
	p_source_buffer->buf = buf;
	p_source_buffer->off = 0;
	p_source_buffer->len = len;

	opj_stream_set_user_data(l_stream, p_source_buffer, opj_free_buffer_info);
	opj_set_up_buffer_stream(l_stream, p_source_buffer->len, p_is_read_stream);
	return l_stream;
}





OPJ_INT32 opj_get_file_open_mode(const char* mode)
{
	OPJ_INT32 m = -1;
	switch (mode[0]) {
	case 'r':
		m = O_RDONLY;
		if (mode[1] == '+')
			m = O_RDWR;
		break;
	case 'w':
	case 'a':
		m = O_RDWR | O_CREAT;
		if (mode[0] == 'w')
			m |= O_TRUNC;
		break;
	default:
		break;
	}
	return m;
}


#ifdef _WIN32

static OPJ_UINT64  opj_size_proc(opj_handle_t fd)
{
	ULARGE_INTEGER m;
	m.LowPart = GetFileSize(fd, &m.HighPart);
	return(m.QuadPart);
}


static void* opj_map(opj_handle_t fd, OPJ_SIZE_T len) {
	void* ptr = NULL;
	HANDLE hMapFile = NULL;

	if (!fd || !len)
		return NULL;

	/* Passing in 0 for the maximum file size indicates that we
	would like to create a file mapping object for the full file size */
	hMapFile = CreateFileMapping(fd, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hMapFile == NULL) {
		return NULL;
	}
	ptr = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
	CloseHandle(hMapFile);
	return ptr;
}

static OPJ_INT32 opj_unmap(void* ptr, OPJ_SIZE_T len) {
	OPJ_INT32 rc = -1;
	(void)len;
	if (ptr)
		rc = UnmapViewOfFile(ptr) ? 0 : -1;
	return rc;
}

static opj_handle_t opj_open_fd(const char* fname, const char* mode) {
	void*	fd = NULL;
	OPJ_INT32 m = -1;
	DWORD			dwMode = 0;

	if (!fname)
		return (opj_handle_t)-1;


	m = opj_get_file_open_mode(mode);
	switch (m) {
	case O_RDONLY:
		dwMode = OPEN_EXISTING;
		break;
	case O_RDWR:
		dwMode = OPEN_ALWAYS;
		break;
	case O_RDWR | O_CREAT:
		dwMode = OPEN_ALWAYS;
		break;
	case O_RDWR | O_TRUNC:
		dwMode = CREATE_ALWAYS;
		break;
	case O_RDWR | O_CREAT | O_TRUNC:
		dwMode = CREATE_ALWAYS;
		break;
	default:
		return NULL;
	}

	fd = (opj_handle_t)CreateFileA(fname,
		(m == O_RDONLY) ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, dwMode,
		(m == O_RDONLY) ? FILE_ATTRIBUTE_READONLY : FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (fd == INVALID_HANDLE_VALUE) {
		return (opj_handle_t)-1;
	}
	return fd;
}

static OPJ_INT32 opj_close_fd(opj_handle_t fd) {
	OPJ_INT32 rc = -1;
	if (fd) {
		rc = CloseHandle(fd) ? 0 : -1;
	}
	return rc;
}

#else

static OPJ_UINT64 opj_size_proc(opj_handle_t fd)
{
	struct stat sb;
	if (!fd)
		return 0;

	if (fstat(fd, &sb)<0)
		return(0);
	else
		return((OPJ_UINT64)sb.st_size);
}

static void* opj_map(opj_handle_t fd, OPJ_SIZE_T len) {
	void* ptr = NULL;
	OPJ_UINT64		size64 = 0;

	if (!fd)
		return NULL;

	size64 = opj_size_proc(fd);
	ptr = (void*)mmap(0, (size_t)size64, PROT_READ, MAP_SHARED, fd, 0);
	return ptr == (void*)-1 ? NULL : ptr;
}

static OPJ_INT32 opj_unmap(void* ptr, OPJ_SIZE_T len) {
	if (ptr)
		munmap(ptr, len);
	return 0;
}

static opj_handle_t opj_open_fd(const char* fname, const char* mode) {
	opj_handle_t	fd = 0;
	OPJ_INT32 m = -1;
	if (!fname) {
		return (opj_handle_t)-1;
	}
	m = opj_get_file_open_mode(mode);
	fd = open(fname, m, 0666);
	if (fd < 0) {
#ifdef DEBUG_ERRNO
		if (errno > 0 && strerror(errno) != NULL) {
			printf("%s: %s", fname, strerror(errno));
		}
		else {
			printf("%s: Cannot open", fname);
		}
#endif
		return (opj_handle_t)-1;
	}
	return fd;
}

static OPJ_INT32 opj_close_fd(opj_handle_t fd) {
	if (!fd)
		return 0;
	return  (close(fd));
}

#endif



static void opj_mem_map_free(void* user_data) {
	if (user_data) {
		opj_buf_info_t* buffer_info = (opj_buf_info_t*)user_data;
		opj_unmap(buffer_info->buf, buffer_info->len);
		opj_close_fd(buffer_info->fd);
		opj_free(buffer_info);
	}
}

/*
Currently, only read streams are supported for memory mapped files.
*/
opj_stream_t* opj_create_mapped_file_read_stream(const char *fname)
{
	opj_stream_t*	l_stream = NULL;
	opj_buf_info_t* buffer_info = NULL;
	void*			mapped_view = NULL;
	OPJ_BOOL p_is_read_stream = OPJ_TRUE;

	opj_handle_t	fd = opj_open_fd(fname, p_is_read_stream ? "r" : "w");
	if (fd == (opj_handle_t)-1)
		return NULL;

	buffer_info = (opj_buf_info_t*)opj_malloc(sizeof(opj_buf_info_t));
	memset(buffer_info, 0, sizeof(opj_buf_info_t));
	buffer_info->fd = fd;
	buffer_info->len = (OPJ_SIZE_T)opj_size_proc(fd);

	l_stream = opj_stream_create(0, p_is_read_stream);
	if (!l_stream) {
		opj_mem_map_free(buffer_info);
		return NULL;
	}

	mapped_view = opj_map(fd, buffer_info->len);
	if (!mapped_view) {
		opj_stream_destroy(l_stream);
		opj_mem_map_free(buffer_info);
		return NULL;
	}

	buffer_info->buf = mapped_view;
	buffer_info->off = 0;

	opj_stream_set_user_data(l_stream, buffer_info, (opj_stream_free_user_data_fn)opj_mem_map_free);
	opj_set_up_buffer_stream(l_stream, buffer_info->len, p_is_read_stream);


	return l_stream;
}
