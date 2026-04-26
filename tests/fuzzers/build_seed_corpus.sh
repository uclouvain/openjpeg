#!/bin/bash

set -e

if [ "$OUT" == "" ]; then
    echo "OUT env var not defined"
    exit 1
fi

mkdir -p "$OUT"

SRC_DIR=$(dirname $0)/../..
ISSUE1472_SEED="$SRC_DIR/data/input/nonregression/issue1472-bigloop.j2k"
ISSUE1472_TMP_DIR=""

cleanup()
{
    if [ "$ISSUE1472_TMP_DIR" != "" ]; then
        rm -rf "$ISSUE1472_TMP_DIR"
    fi
}

trap cleanup EXIT

cd $SRC_DIR/data/input/conformance
rm -f $OUT/opj_decompress_fuzzer_seed_corpus.zip
zip $OUT/opj_decompress_fuzzer_seed_corpus.zip *.jp2 *.j2k
cd $OLDPWD

if [ -f "$ISSUE1472_SEED" ]; then
    ISSUE1472_TMP_DIR=$(mktemp -d "$OUT/issue1472-seeds.XXXXXX")
    ISSUE1472_SCOD=$(od -An -tx1 -j 52 -N 1 "$ISSUE1472_SEED" | tr -d ' \n')

    if [ "$ISSUE1472_SCOD" != "07" ]; then
        echo "Unexpected Scod byte in $ISSUE1472_SEED: 0x$ISSUE1472_SCOD"
        exit 1
    fi

    cp "$ISSUE1472_SEED" "$ISSUE1472_TMP_DIR/issue1472-bigloop.j2k"
    cp "$ISSUE1472_SEED" "$ISSUE1472_TMP_DIR/issue1472-bigloop-noeph.j2k"
    cp "$ISSUE1472_SEED" "$ISSUE1472_TMP_DIR/issue1472-bigloop-prt-only.j2k"

    # Byte 52 is the COD Scod value in the upstream issue1472 seed.
    printf '\003' | dd of="$ISSUE1472_TMP_DIR/issue1472-bigloop-noeph.j2k" \
        bs=1 seek=52 conv=notrunc
    printf '\001' | dd of="$ISSUE1472_TMP_DIR/issue1472-bigloop-prt-only.j2k" \
        bs=1 seek=52 conv=notrunc

    cd "$ISSUE1472_TMP_DIR"
    zip "$OUT/opj_decompress_fuzzer_seed_corpus.zip" issue1472-bigloop*.j2k
    cd "$OLDPWD"
fi

cd $SRC_DIR/data/input/nonregression/htj2k
zip $OUT/opj_decompress_fuzzer_seed_corpus.zip *.j2k *.jhc *.jph
cd $OLDPWD
