#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openjpeg.h"

typedef enum {
    TEST_DECODE_HEADER_FAILURE = -1,
    TEST_DECODE_FAILURE = 0,
    TEST_DECODE_SUCCESS = 1
} test_decode_result_t;

#define ISSUE1472_SCOD_OFFSET 52
#define TEST_CSTY_SOP 0x02

#define TEST_TMP_FILENAME "testissue1472_noeph_tmp.bin"

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

static void test_quiet_callback(const char *msg, void *user_data)
{
    (void)msg;
    (void)user_data;
}

static test_decode_result_t test_decode_codestream(const OPJ_BYTE *data,
        OPJ_SIZE_T data_len, OPJ_CODEC_FORMAT codec_format)
{
    opj_stream_t *stream = NULL;
    opj_codec_t *codec = NULL;
    opj_image_t *image = NULL;
    opj_dparameters_t parameters;
    FILE *fp;
    test_decode_result_t result = TEST_DECODE_HEADER_FAILURE;

    fp = fopen(TEST_TMP_FILENAME, "wb");
    if (fp == NULL) {
        return TEST_DECODE_HEADER_FAILURE;
    }
    if (fwrite(data, 1U, data_len, fp) != data_len) {
        fclose(fp);
        remove(TEST_TMP_FILENAME);
        return TEST_DECODE_HEADER_FAILURE;
    }
    fclose(fp);

    stream = opj_stream_create_default_file_stream(TEST_TMP_FILENAME, OPJ_TRUE);
    if (stream == NULL) {
        remove(TEST_TMP_FILENAME);
        return TEST_DECODE_HEADER_FAILURE;
    }

    codec = opj_create_decompress(codec_format);
    if (codec == NULL) {
        opj_stream_destroy(stream);
        remove(TEST_TMP_FILENAME);
        return TEST_DECODE_HEADER_FAILURE;
    }

    opj_set_info_handler(codec, test_quiet_callback, NULL);
    opj_set_warning_handler(codec, test_quiet_callback, NULL);
    opj_set_error_handler(codec, test_quiet_callback, NULL);

    opj_set_default_decoder_parameters(&parameters);
    if (!opj_setup_decoder(codec, &parameters)) {
        goto cleanup;
    }

    if (!opj_read_header(stream, codec, &image)) {
        goto cleanup;
    }

    result = opj_decode(codec, stream, image) ? TEST_DECODE_SUCCESS :
             TEST_DECODE_FAILURE;

cleanup:
    opj_destroy_codec(codec);
    opj_stream_destroy(stream);
    opj_image_destroy(image);
    remove(TEST_TMP_FILENAME);
    return result;
}

static OPJ_BYTE *test_read_file(const char *filename, OPJ_SIZE_T *data_len)
{
    FILE *fp = fopen(filename, "rb");
    long file_len;
    OPJ_BYTE *data;

    *data_len = 0U;
    if (fp == NULL) {
        return NULL;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }

    file_len = ftell(fp);
    if (file_len <= 0) {
        fclose(fp);
        return NULL;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }

    data = (OPJ_BYTE *)malloc((size_t)file_len);
    if (data == NULL) {
        fclose(fp);
        return NULL;
    }

    if (fread(data, 1U, (size_t)file_len, fp) != (size_t)file_len) {
        free(data);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    *data_len = (OPJ_SIZE_T)file_len;
    return data;
}

/* Set the SOP bit in the Scod byte of the first COD marker of the
 * codestream. The search is anchored at the SOC marker so that incidental
 * 0xff 0x52 byte pairs in JP2 wrapper boxes cannot be patched by mistake. */
static OPJ_BOOL test_enable_sop_csty(OPJ_BYTE *data, OPJ_SIZE_T data_len)
{
    OPJ_SIZE_T offset;
    OPJ_BOOL soc_found = OPJ_FALSE;

    for (offset = 0U; offset + 4U < data_len; ++offset) {
        if (!soc_found) {
            if (data[offset] == 0xffU && data[offset + 1U] == 0x4fU) {
                soc_found = OPJ_TRUE;
            }
            continue;
        }
        if (data[offset] == 0xffU && data[offset + 1U] == 0x52U) {
            data[offset + 4U] |= TEST_CSTY_SOP;
            return OPJ_TRUE;
        }
    }

    return OPJ_FALSE;
}

static int test_sop_optional_tolerated_eof_fixture(const char *data_root,
        const char *relative_filename)
{
    char filename[4096];
    OPJ_SIZE_T data_len;
    OPJ_BYTE *data;
    test_decode_result_t result;

    if (snprintf(filename, sizeof(filename), "%s%s", data_root,
                 relative_filename) >= (int)sizeof(filename)) {
        fprintf(stderr, "Test fixture path is too long\n");
        return 1;
    }

    data = test_read_file(filename, &data_len);
    if (data == NULL) {
        fprintf(stderr, "Unable to read test fixture %s\n", filename);
        return 1;
    }

    if (!test_enable_sop_csty(data, data_len)) {
        fprintf(stderr, "Unable to find COD Scod byte in %s\n", filename);
        free(data);
        return 1;
    }

    result = test_decode_codestream(data, data_len, OPJ_CODEC_JP2);
    free(data);

    if (result != TEST_DECODE_SUCCESS) {
        fprintf(stderr, "SOP-bit/no-SOP tolerated EOF fixture failed to decode: %s\n",
                filename);
        return 1;
    }

    return 0;
}

static int test_sop_optional_tolerated_eof(const char *data_root)
{
    static const char *fixtures[] = {
        "/input/nonregression/Marrin.jp2",
        "/input/nonregression/dwt_interleave_h.gsr105.jp2"
    };
    size_t i;

    if (data_root == NULL || strstr(data_root, "OPJ_DATA_ROOT-NOTFOUND") != NULL) {
        return 0;
    }

    for (i = 0U; i < sizeof(fixtures) / sizeof(fixtures[0]); ++i) {
        if (test_sop_optional_tolerated_eof_fixture(data_root, fixtures[i]) != 0) {
            return 1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    /* The embedded codestream carries Scod = 0x03 (PRT+SOP) at offset 52;
     * the other entries derive the PRT-only and PRT+SOP+EPH variants. */
    static const struct {
        OPJ_BYTE scod;
        const char *description;
    } malformed_variants[] = {
        { 0x03, "no-EPH (PRT+SOP)" },
        { 0x01, "PRT-only" },
        { 0x07, "EPH-required (PRT+SOP+EPH)" }
    };
    OPJ_BYTE codestream[sizeof(issue1472_noeph)];
    size_t i;

    for (i = 0U; i < sizeof(malformed_variants) / sizeof(malformed_variants[0]);
            ++i) {
        test_decode_result_t result;

        memcpy(codestream, issue1472_noeph, sizeof(issue1472_noeph));
        codestream[ISSUE1472_SCOD_OFFSET] = malformed_variants[i].scod;

        result = test_decode_codestream(codestream, sizeof(codestream),
                                        OPJ_CODEC_J2K);
        if (result != TEST_DECODE_FAILURE) {
            fprintf(stderr,
                    "%s malformed codestream unexpectedly avoided decode failure\n",
                    malformed_variants[i].description);
            return 1;
        }
    }

    return test_sop_optional_tolerated_eof(argc > 1 ? argv[1] : NULL);
}
