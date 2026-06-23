#!/bin/bash

set -e

if [ "$SRC" == "" ]; then
    echo "SRC env var not defined"
    exit 1
fi

if [ "$OUT" == "" ]; then
    echo "OUT env var not defined"
    exit 1
fi

if [ "$CXX" == "" ]; then
    echo "CXX env var not defined"
    exit 1
fi

SRC_DIR=$(dirname $0)/../..
FUZZER_DIR=$(dirname $0)

build_fuzzer()
{
    fuzzerName=$1
    sourceFilename=$2
    shift
    shift
    echo "Building fuzzer $fuzzerName"
    $CXX $CXXFLAGS -std=c++11 -I$SRC_DIR/src/lib/openjp2 -I$SRC_DIR/build/src/lib/openjp2 \
        $sourceFilename $* -o $OUT/$fuzzerName \
        $LIB_FUZZING_ENGINE $SRC_DIR/build/bin/libopenjp2.a -lm -lpthread
}

# Build all fuzzers
echo "Building fuzzers..."
fuzzerFiles=$FUZZER_DIR/*.cpp
for F in $fuzzerFiles; do
    fuzzerName=$(basename $F .cpp)
    build_fuzzer $fuzzerName $F
done

echo "Copying dictionaries..."

# Copy JPEG 2000 dictionary for all fuzzers
if [ -f "$FUZZER_DIR/opj_decompress_fuzzer.dict" ]; then
    cp "$FUZZER_DIR/opj_decompress_fuzzer.dict" "$OUT/"

    # Decoder fuzzers use the dictionary
    for fuzzer in opj_decompress_fuzzer_J2K opj_decompress_fuzzer_JP2 \
                  opj_tile_fuzzer opj_decode_area_fuzzer opj_components_fuzzer \
                  opj_dump_info_fuzzer; do
        cp "$OUT/opj_decompress_fuzzer.dict" "$OUT/${fuzzer}.dict" 2>/dev/null || true
    done

    # Round-trip and encoder fuzzers can also benefit from the dictionary
    for fuzzer in opj_roundtrip_fuzzer opj_compress_fuzzer opj_tiled_encoder_fuzzer \
                  opj_mct_fuzzer opj_encoder_options_fuzzer opj_subsampled_fuzzer; do
        cp "$OUT/opj_decompress_fuzzer.dict" "$OUT/${fuzzer}.dict" 2>/dev/null || true
    done
fi

echo "Copying options files..."

# Copy options files
for optFile in $FUZZER_DIR/*.options; do
    if [ -f "$optFile" ]; then
        cp "$optFile" "$OUT/"
    fi
done

echo "Building seed corpora..."

# Build encoder seed corpus if it exists
if [ -d "$FUZZER_DIR/corpus/opj_compress_fuzzer" ]; then
    cd "$FUZZER_DIR/corpus/opj_compress_fuzzer"
    zip -j "$OUT/opj_compress_fuzzer_seed_corpus.zip" * 2>/dev/null || true
    cd - > /dev/null

    # All encoder fuzzers can use the same seed corpus
    for fuzzer in opj_roundtrip_fuzzer opj_tiled_encoder_fuzzer opj_mct_fuzzer \
                  opj_encoder_options_fuzzer opj_subsampled_fuzzer; do
        cp "$OUT/opj_compress_fuzzer_seed_corpus.zip" "$OUT/${fuzzer}_seed_corpus.zip" 2>/dev/null || true
    done
fi

echo "Fuzzer build complete! Built $(ls -1 $OUT/*_fuzzer 2>/dev/null | wc -l) fuzzers."
