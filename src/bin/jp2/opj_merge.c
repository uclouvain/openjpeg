#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "openjpeg.h"
#include "opj_getopt.h"
#include "format_defs.h"
#ifdef _WIN32
#define strcasecmp _stricmp
#endif

/**
 * Prints usage information for opj_merge.
 */
void merge_help_display(void);

void merge_help_display(void)
{
    fprintf(stdout,
            "\nThis is the opj_merge utility from the OpenJPEG project.\n"
            "It provides limited support for creating a jpx file with\n"
            "references to multiple local jp2 files.\n"
            "It has been compiled against openjp2 library v%s.\n\n", opj_version());
    fprintf(stdout,
            "At this time, this utility can only insert references into jpx files.\n"
            "It does not include all jp2 codestreams. All jp2 files being\n"
            "merged must have the same resolution and color format.\n\n");
    fprintf(stdout, "Parameters:\n");
    fprintf(stdout, "-----------\n");
    fprintf(stdout, "Required Parameters (except with -h):\n");
    fprintf(stdout, "-i <file>\n");
    fprintf(stdout, "    Input file, may be passed multiple times to\n");
    fprintf(stdout, "    indicate all jp2 files that will be merged.\n");
    fprintf(stdout, "-o <output file>\n");
    fprintf(stdout, "    Output file (accepted extensions are jpx).\n");
    fprintf(stdout, "-h\n");
    fprintf(stdout, "    Display this help information.\n");
}

static int get_file_format(char *filename)
{
    unsigned int i;
    static const char *extension[] = {
        "pgx", "pnm", "pgm", "ppm", "pbm", "pam", "bmp", "tif", "tiff", "raw", "yuv", "rawl", "tga", "png", "j2k", "jp2", "j2c", "jpc", "jpx"
    };
    static const int format[] = {
        PGX_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, BMP_DFMT, TIF_DFMT, TIF_DFMT, RAW_DFMT, RAW_DFMT, RAWL_DFMT, TGA_DFMT, PNG_DFMT, J2K_CFMT, JP2_CFMT, J2K_CFMT, J2K_CFMT, JPX_CFMT
    };
    char * ext = strrchr(filename, '.');
    if (ext == NULL) {
        return -1;
    }
    ext++;
    for (i = 0; i < sizeof(format) / sizeof(*format); i++) {
        if (strcasecmp(ext, extension[i]) == 0) {
            return format[i];
        }
    }
    return -1;
}

/**
 * Parse incoming command line arguments and store the
 * result in args
 * @param[in] argc Program argc
 * @param[in] argv Program argv
 * @param[out] outfile ptr to const char* which will get the output file.
 * @param[inout] input_file_list array instance, should be length argc to ensure it's big enough.
 * @param[out] infile_count Number of input files given.
 */
static void parse_cmdline(int argc,
                         char** argv,
                         char** outfile,
                         const char** input_file_list,
                         OPJ_UINT32* infile_count)
{
    const char optlist[] = "i:o:h";
    OPJ_UINT32 num_files = 0;
    int c;
    while ((c = opj_getopt(argc, argv, optlist)) != -1) {
        switch (c) {
            case 'i': // Input file
                // Store the input file in the array
                input_file_list[num_files++] = opj_optarg;
                break;
            case 'o':
                *outfile = opj_optarg;
                if (get_file_format(*outfile) != JPX_CFMT) {
                    fprintf(stderr, "Unknown output file %s [output must be *.jpx]\n", *outfile);
                    exit(1);
                }
                break;
            case 'h':
                merge_help_display();
                exit(0);
        }
    }

    *infile_count = num_files;

    if (num_files == 0) {
        fprintf(stderr, "No input files given!\n");
        exit(1);
    }

    if (*outfile == NULL) {
        fprintf(stderr, "No output file given!\n");
        exit(1);
    }

    // Null terminate the list of input files.
    input_file_list[num_files] = NULL;
}

int main(int argc, char **argv) {
    OPJ_UINT64 i = 0;
    /** Create jpx encoder */
    opj_codec_t* codec = opj_create_compress(OPJ_CODEC_JPX);
    /* set encoding parameters to default values */
    opj_cparameters_t parameters;
    /** Output file */
    opj_stream_t *outfile = NULL;
    /** Creation status */
    OPJ_BOOL bSuccess = OPJ_FALSE;
    OPJ_UINT32 file_count;
    const char** input_files = calloc(argc, sizeof(const char*));
    char* output_file;
    /** Program return code */
    int ret = 1;
    if (!input_files) {
        fprintf(stderr, "Failed to allocate memory for input file list\n");
        exit(1);
    }

    parse_cmdline(argc, argv, &output_file, input_files, &file_count);
    printf("Merging\n");
    for(i = 0; i < file_count; i++) {
        printf("  - %s\n", input_files[i]);
    }
    printf("Into %s\n", output_file);

    if (!codec) {
        fprintf(stderr, "Failed to initialize the jpx codec.\n");
        return ret;
    }

    opj_set_default_encoder_parameters(&parameters);

    // Creating references to other jpx files doesn't require image data.
    // so pass NULL for the image parameter and hope for the best.
    if (! opj_setup_encoder(codec, &parameters, OPJ_NO_IMAGE_DATA)) {
        fprintf(stderr, "Failed to setup encoder: opj_setup_encoder\n");
        goto fin;
    }

    // Use extra options to specify the list of files to be merged into the jpx file.
    {
        if (!opj_encoder_set_extra_options(codec, input_files)) {
            fprintf(stderr, "Failed to set list of jp2 files to include: opj_encoder_set_extra_options\n");
            goto fin;
        }
    }

    // /* open a byte stream for writing */
    outfile = opj_stream_create_default_file_stream(output_file, OPJ_FALSE);
    if (!outfile) {
        fprintf(stderr, "Failed to allocate memory for the output file");
        goto fin;
    }

    bSuccess = opj_start_compress(codec, OPJ_NO_IMAGE_DATA, outfile);
    if (!bSuccess) {
        fprintf(stderr, "Failed to create the jpx image: opj_start_compress\n");
        goto fin;
    }

    bSuccess = bSuccess && opj_encode(codec, outfile);
    if (!bSuccess) {
        fprintf(stderr, "Failed to encode the image: opj_encode\n");
    }

    bSuccess = bSuccess && opj_end_compress(codec, outfile);
    if (!bSuccess) {
        fprintf(stderr, "Failed to encode the image: opj_end_compress\n");
        goto fin;
    }

    ret = 0;

fin:
    if (codec) {
        opj_destroy_codec(codec);
    }
    if (outfile) {
        opj_stream_destroy(outfile);
    }
    free(input_files);
    return ret;
}