#!/bin/bash

# This script executes the script step when running under travis-ci

# Set-up some bash options
set -o nounset   ## set -u : exit the script if you try to use an uninitialised variable
set -o errexit   ## set -e : exit the script if any statement returns a non-true return value
set -o pipefail  ## Fail on error in pipe

# Set-up some variables
OPJ_SOURCE_DIR=$(cd $(dirname $0)/../.. && pwd)

OPJ_DO_SUBMIT=0 # Do not flood cdash
if [ "${TRAVIS_REPO_SLUG:-}" != "" ]; then
	OPJ_OWNER=$(echo "${TRAVIS_REPO_SLUG}" | sed 's/\(^.*\)\/.*/\1/')
	OPJ_SITE="${OPJ_OWNER}.travis-ci.org"
	if [ "${OPJ_OWNER}" == "uclouvain" ]; then
		OPJ_DO_SUBMIT=1
	fi
else
	OPJ_SITE="$(hostname)"
fi

if [ "${TRAVIS_OS_NAME:-}" == "" ]; then
  # Let's guess OS for testing purposes
	echo "Guessing OS"
	if uname -s | grep -i Darwin &> /dev/null; then
		TRAVIS_OS_NAME=osx
	elif uname -s | grep -i Linux &> /dev/null; then
		TRAVIS_OS_NAME=linux
		if [ "${CC:-}" == "" ]; then
			# default to gcc
			export CC=gcc
		fi
	else
		echo "Failed to guess OS"; exit 1
	fi
	echo "${TRAVIS_OS_NAME}"
fi

if [ "${TRAVIS_OS_NAME}" == "osx" ]; then
	OPJ_OS_NAME=$(sw_vers -productName | tr -d ' ')$(sw_vers -productVersion | sed 's/\([^0-9]*\.[0-9]*\).*/\1/')
	OPJ_CC_VERSION=$(xcodebuild -version | grep -i xcode)
	OPJ_CC_VERSION=xcode${OPJ_CC_VERSION:6}
elif [ "${TRAVIS_OS_NAME}" == "linux" ]; then
	OPJ_OS_NAME=linux
	if which lsb_release > /dev/null; then
		OPJ_OS_NAME=$(lsb_release -si)$(lsb_release -sr | sed 's/\([^0-9]*\.[0-9]*\).*/\1/')
	fi
	if [ "${CC}" == "gcc" ]; then
		OPJ_CC_VERSION=gcc$(${CC} --version | head -1 | sed 's/.*\ \([0-9.]*[0-9]\)/\1/')
	elif [ "${CC}" == "clang" ]; then
		OPJ_CC_VERSION=clang$(${CC} --version | grep version | sed 's/.*version \([^0-9.]*[0-9.]*\).*/\1/')
	else
		echo "Compiler not supported: ${CC}"
	fi
else
	echo "OS not supported: ${TRAVIS_OS_NAME}"
fi

if [ "${TRAVIS_BRANCH:-}" == "" ]; then
	echo "Guessing branch"
	TRAVIS_BRANCH=$(git -C ${OPJ_SOURCE_DIR} branch | grep '*' | tr -d '*[[:blank:]]') #default to master
fi

OPJ_BUILDNAME=${OPJ_OS_NAME}-${OPJ_CC_VERSION}-${TRAVIS_BRANCH}
if [ "${TRAVIS_PULL_REQUEST:-}" != "false" ] && [ "${TRAVIS_PULL_REQUEST:-}" != "" ]; then
	OPJ_BUILDNAME=${OPJ_BUILDNAME}-pr${TRAVIS_PULL_REQUEST}
fi
OPJ_BUILDNAME=${OPJ_BUILDNAME}-Release-3rdP

if [ "${OPJ_NONCOMMERCIAL:-}" == "1" ] && [ -d kdu ]; then
	echo "
Testing will use Kakadu trial binaries. Here's the copyright notice from kakadu:
Copyright is owned by NewSouth Innovations Pty Limited, commercial arm of the UNSW Australia in Sydney.
You are free to trial these executables and even to re-distribute them,
so long as such use or re-distribution is accompanied with this copyright notice and is not for commercial gain.
Note: Binaries can only be used for non-commercial purposes.
"
fi

set -x
if [ "${OPJ_NONCOMMERCIAL:-}" == "1" ] && [ -d kdu ]; then
	if [ "${TRAVIS_OS_NAME}" == "linux" ]; then
		if [ "${LD_LIBRARY_PATH:-}" == "" ]; then
			export LD_LIBRARY_PATH=${PWD}/kdu
		else
			export LD_LIBRARY_PATH=${PWD}/kdu:${LD_LIBRARY_PATH}
		fi
	fi
	export PATH=${PWD}/kdu:${PATH}
fi

mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_CODEC=ON -DBUILD_THIRDPARTY=ON -DBUILD_TESTING=ON -DOPJ_DATA_ROOT=${PWD}/../data -DJPYLYZER_EXECUTABLE=${PWD}/../jpylyzer/jpylyzer/jpylyzer.py -DSITE=${OPJ_SITE} -DBUILDNAME=${OPJ_BUILDNAME} ${OPJ_SOURCE_DIR}
ctest -D ExperimentalStart
ctest -D ExperimentalBuild -V
ctest -D ExperimentalTest -j2 || true
ctest -D ExperimentalSubmit || true
