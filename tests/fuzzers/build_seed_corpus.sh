#!/bin/bash

set -e

if [ "$OUT" == "" ]; then
    echo "OUT env var not defined"
    exit 1
fi

SRC_DIR=$(dirname $0)/../..
FUZZER_DIR=$(dirname $0)

echo "Building decoder seed corpus..."

# Build main decoder corpus from test data
cd $SRC_DIR/data/input/conformance
rm -f $OUT/opj_decompress_fuzzer_seed_corpus.zip
zip $OUT/opj_decompress_fuzzer_seed_corpus.zip *.jp2 *.j2k 2>/dev/null || true
cd $OLDPWD

# Add HTJ2K test files
if [ -d "$SRC_DIR/data/input/nonregression/htj2k" ]; then
    cd $SRC_DIR/data/input/nonregression/htj2k
    zip $OUT/opj_decompress_fuzzer_seed_corpus.zip *.j2k *.jhc *.jph 2>/dev/null || true
    cd $OLDPWD
fi

# Create corpus copies for format-specific fuzzers
echo "Creating corpus for format-specific fuzzers..."
cp $OUT/opj_decompress_fuzzer_seed_corpus.zip $OUT/opj_decompress_fuzzer_J2K_seed_corpus.zip 2>/dev/null || true
cp $OUT/opj_decompress_fuzzer_seed_corpus.zip $OUT/opj_decompress_fuzzer_JP2_seed_corpus.zip 2>/dev/null || true

# Create corpus for tile fuzzer (uses same test files)
cp $OUT/opj_decompress_fuzzer_seed_corpus.zip $OUT/opj_tile_fuzzer_seed_corpus.zip 2>/dev/null || true

# Create corpus for decode area fuzzer
cp $OUT/opj_decompress_fuzzer_seed_corpus.zip $OUT/opj_decode_area_fuzzer_seed_corpus.zip 2>/dev/null || true

# Create corpus for components fuzzer
cp $OUT/opj_decompress_fuzzer_seed_corpus.zip $OUT/opj_components_fuzzer_seed_corpus.zip 2>/dev/null || true

echo "Building encoder seed corpus..."

# Build encoder seed corpus from synthetic test inputs
if [ -d "$FUZZER_DIR/corpus/opj_compress_fuzzer" ]; then
    cd "$FUZZER_DIR/corpus/opj_compress_fuzzer"
    zip -j "$OUT/opj_compress_fuzzer_seed_corpus.zip" * 2>/dev/null || true
    cd - > /dev/null
fi

# Build roundtrip seed corpus (same as encoder)
if [ -f "$OUT/opj_compress_fuzzer_seed_corpus.zip" ]; then
    cp "$OUT/opj_compress_fuzzer_seed_corpus.zip" "$OUT/opj_roundtrip_fuzzer_seed_corpus.zip"
fi

echo "Seed corpus build complete!"
