/*
* Copyright (c) 2016, Aaron Boxer
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/
#ifndef __MEM_STREAM_H
#define __MEM_STREAM_H

opj_stream_t*  opj_create_buffer_stream(OPJ_BYTE *buf,
										OPJ_SIZE_T len,
										OPJ_BOOL p_is_read_stream);

opj_stream_t* opj_create_mapped_file_read_stream(const char *fname);

#endif