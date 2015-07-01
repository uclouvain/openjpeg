**Restoring indexfile in opj_compress.c and opj_decompress.c**

This patch allows once more to get an indexfile with

  opj_compress -i INFILE -o OUTFILE -x INDEXFILE

  opj_decompress -i INFILE -o OUTFILE -x INDEXFILE

