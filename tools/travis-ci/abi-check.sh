#!/bin/bash

# This script executes the abi-check step when running under travis-ci (in run step)

# Set-up some bash options
set -o nounset   ## set -u : exit the script if you try to use an uninitialised variable
set -o errexit   ## set -e : exit the script if any statement returns a non-true return value
set -o pipefail  ## Fail on error in pipe
set -o xtrace    ## set -x : Print a trace of simple commands and their arguments after they are expanded and before they are executed.

# Exit if not ABI check
if [ "${OPJ_CI_ABI_CHECK:-}" != "1" ]; then
	exit 0
fi

OPJ_SOURCE_DIR=$(cd $(dirname $0)/../.. && pwd)

if [ ! -d ${HOME}/abi-check ]; then
	mkdir ${HOME}/abi-check
fi

cd ${HOME}/abi-check

if [ ! -f ${HOME}/abi-check/.restored ]; then
	# Clean all if .restored is not present
	touch not.empty
	rm -rf ./*
	# Let's get tools not available with apt
	mkdir tools
	wget -qO - https://tools.ietf.org/tools/rfcdiff/rfcdiff-1.42.tgz | tar -xz
	mv rfcdiff-1.42 ${PWD}/tools/rfcdiff
	wget -qO - https://github.com/lvc/installer/archive/0.2.tar.gz | tar -xz
	mkdir ${PWD}/tools/abi-tracker
	make -C installer-0.2 install prefix=${PWD}/tools/abi-tracker target=abi-tracker
	mkdir tracker
fi

cd tracker

# Check ABI
export PATH=${PWD}/../tools/rfcdiff:${PWD}/../tools/abi-tracker/bin:$PATH
sed -e "s/@OPJ_SOURCE_DIR@/${OPJ_SOURCE_DIR//\//\\/}/g" ${OPJ_SOURCE_DIR}/tools/abi-tracker/openjpeg.json > openjpeg.json
abi-monitor -get   openjpeg.json
abi-monitor -build openjpeg.json
abi-tracker -build openjpeg.json
