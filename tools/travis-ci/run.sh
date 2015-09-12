#!/bin/bash

# This script executes the script step when running under travis-ci

# Set-up some bash options
set -o nounset   ## set -u : exit the script if you try to use an uninitialised variable
set -o errexit   ## set -e : exit the script if any statement returns a non-true return value
set -o pipefail  ## Fail on error in pipe

# Set-up some variables
if [ "${OPJ_CI_BUILD_CONFIGURATION:-}" == "" ]; then
	export OPJ_CI_BUILD_CONFIGURATION=Release #default
fi
OPJ_SOURCE_DIR=$(cd $(dirname $0)/../.. && pwd)

if [ "${OPJ_DO_SUBMIT:-}" == "" ]; then
	OPJ_DO_SUBMIT=0 # Do not flood cdash by default
fi
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
		echo "Compiler not supported: ${CC}"; exit 1
	fi
else
	echo "OS not supported: ${TRAVIS_OS_NAME}"; exit 1
fi

if [ "${OPJ_CI_ARCH:-}" == "" ]; then
	echo "Guessing build architecture"
	MACHINE_ARCH=$(uname -m)
	if [ "${MACHINE_ARCH}" == "x86_64" ]; then
		export OPJ_CI_ARCH=x86_64
	fi
	echo "${OPJ_CI_ARCH}"
fi

if [ "${TRAVIS_BRANCH:-}" == "" ]; then
	echo "Guessing branch"
	TRAVIS_BRANCH=$(git -C ${OPJ_SOURCE_DIR} branch | grep '*' | tr -d '*[[:blank:]]') #default to master
fi

OPJ_BUILDNAME=${OPJ_OS_NAME}-${OPJ_CC_VERSION}-${OPJ_CI_ARCH}-${TRAVIS_BRANCH}
if [ "${TRAVIS_PULL_REQUEST:-}" != "false" ] && [ "${TRAVIS_PULL_REQUEST:-}" != "" ]; then
	OPJ_BUILDNAME=${OPJ_BUILDNAME}-pr${TRAVIS_PULL_REQUEST}
fi
OPJ_BUILDNAME=${OPJ_BUILDNAME}-${OPJ_CI_BUILD_CONFIGURATION}-3rdP

if [ "${OPJ_NONCOMMERCIAL:-}" == "1" ] && [ "${OPJ_CI_SKIP_TESTS:-}" != "1" ] && [ -d kdu ]; then
	echo "
Testing will use Kakadu trial binaries. Here's the copyright notice from kakadu:
Copyright is owned by NewSouth Innovations Pty Limited, commercial arm of the UNSW Australia in Sydney.
You are free to trial these executables and even to re-distribute them,
so long as such use or re-distribution is accompanied with this copyright notice and is not for commercial gain.
Note: Binaries can only be used for non-commercial purposes.
"
fi

set -x
# This will print configuration
# travis-ci doesn't dump cmake version in system info, let's print it 
cmake --version

export OPJ_SITE=${OPJ_SITE}
export OPJ_BUILDNAME=${OPJ_BUILDNAME}
export OPJ_SOURCE_DIR=${OPJ_SOURCE_DIR}
export OPJ_BUILD_CONFIGURATION=${OPJ_CI_BUILD_CONFIGURATION}
export OPJ_DO_SUBMIT=${OPJ_DO_SUBMIT}

ctest -S ${OPJ_SOURCE_DIR}/tools/ctest_scripts/travis-ci.cmake -V
