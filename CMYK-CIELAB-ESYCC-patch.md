####Library patch for CMYK, CIELAB, eSYCC images

1. opj_decompress.c

   calls the new functions defined in color.c; see 3.

2. color.h

   declares the new functions defined in color.c; see 3.

3. color.c

   defines the new functions:

   **color_apply_conversion(opj_image_t*)**: 

   uses icc_profile_buf with icc_profile_len == 0.
   This allows entensions without changing the structures of opj_image_t:
   the structure change should have been done before but has not been done.
   There has not been introduced a color_space for CIELAB, etc.
   The first extension is with CIELab.

   **color_cmyk_to_rgb(opj_image_t*)**

   **color_esycc_to_rgb(opj_image_t*)**

4. jp2.c

   collects data for CIELab
   sets the color_space for EYCC and CMYK

5. CMakeLists.txt

   With THIRDPARTY the libraries are static. LINUX needs '-fPIC' or fails,
   if static libraries without '-fPIC' AND dynamic libraries are linked:
   LINUX reports an error.

Her Highness KDU shows CIELab and CMYK images.

OPENJPEG now can opj_decompress (and flviewer can show) CIELAB, CMYK and
sEYCC images.


winfried
