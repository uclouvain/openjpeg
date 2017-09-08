# OpenJPEG NEWS

More details in the [Changelog](https://github.com/uclouvain/openjpeg/blob/master/CHANGELOG.md)

## OpenJPEG 2.2.0

No API/ABI break compared to v2.1.2 but additional symbols for multithreading support (hence the MINOR version bump).

### Codebase improvements

* Memory consumption reduction at decoding side [\#968](https://github.com/uclouvain/openjpeg/pull/968)
* Multi-threading support at decoding side [\#786](https://github.com/uclouvain/openjpeg/pull/786)
* Tier-1 speed optimizations (encoder and decoder) [\#945](https://github.com/uclouvain/openjpeg/pull/945)
* Tier-1 decoder further optimization [\#783](https://github.com/uclouvain/openjpeg/pull/783)
* Inverse 5x3 DWT speed optimization: single-pass lifting and SSE2/AVX2 implementation [\#957](https://github.com/uclouvain/openjpeg/pull/957)
* Fixed a bug that prevented OpenJPEG to compress losslessly in some situations [\#949](https://github.com/uclouvain/openjpeg/pull/949)
* Fixed BYPASS/LAZY, RESTART/TERMALL and PTERM mode switches
* Many other bug fixes (including security fixes)

### Maintenance improvements

* Benchmarking scripts to automatically compare the speed of latest OpenJPEG build with latest release and/or Kakadu binaries [\#917](https://github.com/uclouvain/openjpeg/pull/917)
* CPU and RAM usage profiling scripts [\#918](https://github.com/uclouvain/openjpeg/pull/918)
* Codebase reformatting (with astyle) and scripts to automatically check that new commits comply with formatting guidelines [\#919](https://github.com/uclouvain/openjpeg/pull/919)
* Register OpenJPEG at Google OSS Fuzz initiative, so as to automatically have OpenJPEG tested against Google fuzzer [\#965](https://github.com/uclouvain/openjpeg/issues/965)

## OpenJPEG 2.1.2

* Bug fixes (including security fixes)
* No API/ABI break compared to v2.1.1

## OpenJPEG 2.1.1

* Huge amount of critical bugfixes
* Speed improvements
* No API/ABI break compared to v2.1

## OpenJPEG 2.1.0

### New Features

    * Digital Cinema profiles have been fixed and updated
	* New option to disable MCT if needed
    * extended RAW support: it is now possible to input raw images
	  with subsampled color components (422, 420, etc)
    * New way to deal with profiles
	  
### API/ABI modifications
(see [here](http://www.openjpeg.org/abi-check/timeline/openjpeg/) for details)

    * Removed deprecated functions 
	    * opj_stream_create_default_file_stream(FILE*,...)
        * opj_stream_create_file_stream(FILE*,...)
        * opj_stream_set_user_data (opj_stream_t* p_stream, void * p_data)
	* Added 
        * opj_stream_create_default_file_stream(char*,...)
        * opj_stream_create_file_stream(char*,...)
        * opj_stream_destroy(opj_stream_t*)
        * opj_stream_set_user_data (opj_stream_t* p_stream, void * p_data, 
            ... opj_stream_free_user_data_fn p_function)
        * JPEG 2000 profiles and Part-2 extensions defined through '#define'
    * Changed
        * 'alpha' field added to 'opj_image_comp' structure
        * 'OPJ_CLRSPC_EYCC' added to enum COLOR_SPACE
        * 'OPJ_CLRSPC_CMYK' added to enum COLOR_SPACE
        * 'OPJ_CODEC_JPP' and 'OPJ_CODEC_JPX' added to CODEC_FORMAT
          (not yet used in use)
        * 'max_cs_size' and 'rsiz' fields added to opj_cparameters_t
    
### Misc

    * OpenJPEG is now officially conformant with JPEG 2000 Part-1
	  and will soon become official reference software at the 
	  JPEG committee.
	* Huge amount of bug fixes. See CHANGES for details.


## OpenJPEG 2.0.0

### New Features

    * streaming capabilities
    * merge JP3D

### API modifications
(see [here](http://www.openjpeg.org/abi-check/timeline/openjpeg/) for details)

    * Use a 64bits capable API
    
### Misc

    * removed autotools build system
    * folders hierarchies reorganisation
    * Huge amount of bug fixes. See CHANGES for details.
