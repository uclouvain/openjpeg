/** CPB **/

OPJ_BOOL mem_stream_resize(mem_stream_t * mem_stream)
{
    size_t cursize = mem_stream->mem_cursize;
    OPJ_BYTE *old_data = mem_stream->mem_data;

    /* Allocate twice the amount of memory currently used */
    OPJ_BYTE *new_data = opj_malloc((cursize * 2) * sizeof(OPJ_BYTE));

    if (! new_data) {
        return OPJ_FALSE;  /* resize failed */
    }

    memcpy(new_data, old_data, cursize);
    mem_stream->mem_data = new_data;
    mem_stream->mem_cursize = cursize * 2;
    opj_free(old_data);

    return OPJ_TRUE;
}

OPJ_SIZE_T mem_stream_read(void * p_buffer, OPJ_SIZE_T p_nb_bytes,
                           void * p_user_data)
{
    size_t available;
    size_t l_nb_bytes_to_read;
    mem_stream_t *l_dest = 00;

    l_dest = (mem_stream_t *) p_user_data;

    available = l_dest->mem_cursize - l_dest->mem_curidx;
    if (! available) {
        return (OPJ_SIZE_T) - 1;
    }

    l_nb_bytes_to_read = (available < p_nb_bytes) ? available : p_nb_bytes;
    memcpy(p_buffer, &(l_dest->mem_data[l_dest->mem_curidx]), l_nb_bytes_to_read);
    l_dest->mem_curidx += l_nb_bytes_to_read;

    return l_nb_bytes_to_read;
}

OPJ_SIZE_T mem_stream_write(void * p_buffer, OPJ_SIZE_T p_nb_bytes,
                            void * p_user_data)
{
    mem_stream_t *l_dest = 00;
    l_dest = (mem_stream_t *) p_user_data;

    /* Ensure there is enough space to hold p_nb_bytes more bytes of data */
    while ((l_dest->mem_cursize - l_dest->mem_curidx) < p_nb_bytes) {
        if (!l_dest->mem_resize_fn(l_dest)) {
            return (OPJ_SIZE_T) - 1;
        }
    }

    memcpy((void *) & (l_dest->mem_data[l_dest->mem_curidx]),
           p_buffer, p_nb_bytes);
    l_dest->mem_curidx += p_nb_bytes;

    return (OPJ_SIZE_T) p_nb_bytes;
}

OPJ_OFF_T mem_stream_skip(OPJ_OFF_T p_nb_bytes, void * p_user_data)
{
    /* Position user memory pointer to p_nb_bytes bytes from CURRENT */
    mem_stream_t *l_mem_stream = 00;
    l_mem_stream = (mem_stream_t *) p_user_data;

    if (p_nb_bytes >= 0) {
        OPJ_UINT64 new_pos = l_mem_stream->mem_curidx + (OPJ_UINT64) p_nb_bytes;
        if (new_pos > l_mem_stream->mem_cursize) {
            /* Not enough bytes to move to SEEK position */
            return (OPJ_OFF_T) - 1;
        }

        l_mem_stream->mem_curidx = new_pos;
    } else {
        /* p_nb_bytes is negative, ensure it's absolute value is less than curidx */
        OPJ_OFF_T new_pos = (OPJ_OFF_T)l_mem_stream->mem_curidx + p_nb_bytes;
        if (new_pos < 0) {
            /* Can't move past [0] */
            return (OPJ_OFF_T) - 1;
        }

        l_mem_stream->mem_curidx = (OPJ_UINT64)new_pos;
    }
    return p_nb_bytes;
}

OPJ_BOOL mem_stream_seek(OPJ_OFF_T p_position, void * p_user_data)
{
    /* Position user memory pointer to p_position bytes from START */
    /* Return OPJ_TRUE if successful to recreate negation of fseek() results */
    mem_stream_t *l_mem_stream = 00;
    l_mem_stream = (mem_stream_t *) p_user_data;

    if (p_position < 0) {
        /* Can't move to negative index position */
        return OPJ_FALSE;
    }
    if (l_mem_stream->mem_cursize < (OPJ_UINT64)p_position) {
        /* Not enough bytes to move to SKIP position */
        return OPJ_FALSE;
    }

    l_mem_stream->mem_curidx = (OPJ_UINT64)p_position;
    return OPJ_TRUE; /* Success */
}

OPJ_BOOL mem_stream_free(void * p_user_data)
{
    mem_stream_t *l_mem_stream = 00;
    l_mem_stream = (mem_stream_t *) p_user_data;

    opj_free(l_mem_stream->mem_data);
    opj_free(l_mem_stream);

    return (l_mem_stream == NULL);
}
/** CPB END **/
