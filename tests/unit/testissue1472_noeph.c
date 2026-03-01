#include <stdio.h>
#include <string.h>

#include "openjpeg.h"

typedef struct {
    const OPJ_BYTE* data;
    OPJ_SIZE_T data_len;
    OPJ_SIZE_T offset;
} test_mem_stream_t;

static const OPJ_BYTE issue1472_noeph[] = {
    0xff, 0x4f, 0xff, 0x51, 0x00, 0x2c, 0x00, 0x02, 0x04, 0x00, 0x00, 0x64,
    0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x09,
    0x00, 0x00, 0xfc, 0x0b, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x07, 0x04, 0x01, 0x07, 0x42, 0x01,
    0xff, 0x52, 0x00, 0x0e, 0x03, 0x04, 0xff, 0x01, 0x00, 0x01, 0x04, 0x04,
    0x00, 0x01, 0x00, 0x22, 0xff, 0x5c, 0x00, 0x07, 0x40, 0x40, 0x48, 0x48,
    0x50, 0xff, 0x64, 0x00, 0x2d, 0x00, 0x01, 0x43, 0x72, 0x65, 0x61, 0x74,
    0x6f, 0x72, 0x3a, 0x20, 0x41, 0x56, 0x2d, 0x4a, 0x32, 0x4b, 0x20, 0x28,
    0x01, 0x00, 0x80, 0x56, 0x61, 0x74, 0x6f, 0x72, 0x3a, 0x20, 0x41, 0x56,
    0x2d, 0x4a, 0x32, 0x4b, 0x20, 0x28, 0x63, 0x29, 0x20, 0x69, 0x6f, 0x74,
    0xff, 0x90, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01,
    0xff, 0x93, 0xff, 0xff, 0x4f, 0xff, 0x51
};

static void test_error_callback(const char *msg, void *user_data)
{
    (void)msg;
    (void)user_data;
}

static void test_warning_callback(const char *msg, void *user_data)
{
    (void)msg;
    (void)user_data;
}

static void test_info_callback(const char *msg, void *user_data)
{
    (void)msg;
    (void)user_data;
}

static OPJ_SIZE_T test_read_callback(void* p_buffer, OPJ_SIZE_T p_nb_bytes,
                                     void *p_user_data)
{
    test_mem_stream_t* mem_stream = (test_mem_stream_t*)p_user_data;
    OPJ_SIZE_T bytes_to_read = p_nb_bytes;

    if (mem_stream->offset >= mem_stream->data_len || p_nb_bytes == 0U) {
        return (OPJ_SIZE_T)-1;
    }

    if (mem_stream->offset + p_nb_bytes > mem_stream->data_len) {
        bytes_to_read = mem_stream->data_len - mem_stream->offset;
    }

    memcpy(p_buffer, mem_stream->data + mem_stream->offset, bytes_to_read);
    mem_stream->offset += bytes_to_read;
    return bytes_to_read;
}

static OPJ_OFF_T test_skip_callback(OPJ_OFF_T p_nb_bytes, void *p_user_data)
{
    test_mem_stream_t* mem_stream = (test_mem_stream_t*)p_user_data;

    mem_stream->offset += (OPJ_SIZE_T)p_nb_bytes;
    return p_nb_bytes;
}

static OPJ_BOOL test_seek_callback(OPJ_OFF_T p_nb_bytes, void * p_user_data)
{
    test_mem_stream_t* mem_stream = (test_mem_stream_t*)p_user_data;

    mem_stream->offset = (OPJ_SIZE_T)p_nb_bytes;
    return OPJ_TRUE;
}

int main(void)
{
    test_mem_stream_t mem_stream;
    opj_stream_t *stream = NULL;
    opj_codec_t *codec = NULL;
    opj_image_t *image = NULL;
    opj_dparameters_t parameters;

    mem_stream.data = issue1472_noeph;
    mem_stream.data_len = sizeof(issue1472_noeph);
    mem_stream.offset = 0U;

    stream = opj_stream_create(1024, OPJ_TRUE);
    if (stream == NULL) {
        fprintf(stderr, "Failed to create input stream\n");
        return 1;
    }

    opj_stream_set_user_data(stream, &mem_stream, NULL);
    opj_stream_set_user_data_length(stream, sizeof(issue1472_noeph));
    opj_stream_set_read_function(stream, test_read_callback);
    opj_stream_set_skip_function(stream, test_skip_callback);
    opj_stream_set_seek_function(stream, test_seek_callback);

    codec = opj_create_decompress(OPJ_CODEC_J2K);
    if (codec == NULL) {
        fprintf(stderr, "Failed to create decoder\n");
        opj_stream_destroy(stream);
        return 1;
    }

    opj_set_info_handler(codec, test_info_callback, NULL);
    opj_set_warning_handler(codec, test_warning_callback, NULL);
    opj_set_error_handler(codec, test_error_callback, NULL);

    opj_set_default_decoder_parameters(&parameters);
    if (!opj_setup_decoder(codec, &parameters)) {
        fprintf(stderr, "Failed to setup decoder\n");
        opj_destroy_codec(codec);
        opj_stream_destroy(stream);
        return 1;
    }

    if (!opj_read_header(stream, codec, &image)) {
        fprintf(stderr, "Expected the malformed codestream to reach packet decoding\n");
        opj_destroy_codec(codec);
        opj_stream_destroy(stream);
        opj_image_destroy(image);
        return 1;
    }

    if (opj_decode(codec, stream, image)) {
        fprintf(stderr, "Malformed codestream unexpectedly decoded successfully\n");
        opj_destroy_codec(codec);
        opj_stream_destroy(stream);
        opj_image_destroy(image);
        return 1;
    }

    opj_destroy_codec(codec);
    opj_stream_destroy(stream);
    opj_image_destroy(image);
    return 0;
}
