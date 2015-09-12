#!/bin/bash

# This script executes the install step when running under travis-ci

# Set-up some error handling
set -o nounset   ## set -u : exit the script if you try to use an uninitialised variable
set -o errexit   ## set -e : exit the script if any statement returns a non-true return value
set -o pipefail  ## Fail on error in pipe

function exit_handler ()
{
	local exit_code="$?"
	
	test ${exit_code} == 0 && return;

	echo -e "\nInstall failed !!!\nLast command at line ${BASH_LINENO}: ${BASH_COMMAND}";
	exit "${exit_code}"
}
trap exit_handler EXIT
trap exit ERR 

# travis-ci doesn't dump cmake version in system info, let's print it 
cmake --version

OPJ_SOURCE_DIR=$(cd $(dirname $0)/../.. && pwd)

# We need test data
if [ "${TRAVIS_BRANCH:-}" == "" ]; then
	TRAVIS_BRANCH=$(git -C ${OPJ_SOURCE_DIR} branch | grep '*' | tr -d '*[[:blank:]]') #default to same branch as we're setting up
fi
OPJ_DATA_HAS_BRANCH=$(git ls-remote --heads git://github.com/uclouvain/openjpeg-data.git ${TRAVIS_BRANCH} | wc -l)
if [ ${OPJ_DATA_HAS_BRANCH} -ne 0 ]; then
	OPJ_DATA_BRANCH=${TRAVIS_BRANCH}
else
	OPJ_DATA_BRANCH=master #default to master
fi
echo "Cloning openjpeg-data from ${OPJ_DATA_BRANCH} branch"
git clone --depth=1 --branch=${OPJ_DATA_BRANCH} git://github.com/uclouvain/openjpeg-data.git data

# We need jpylyzer for the test suite
echo "Retrieving jpylyzer"
wget -qO - https://github.com/openpreserve/jpylyzer/archive/1.14.2.tar.gz | tar -xz
mv jpylyzer-1.14.2 jpylyzer
chmod +x jpylyzer/jpylyzer/jpylyzer.py

# When OPJ_NONCOMMERCIAL=1, kakadu trial binaries are used for testing. Here's the copyright notice from kakadu:
# Copyright is owned by NewSouth Innovations Pty Limited, commercial arm of the UNSW Australia in Sydney.
# You are free to trial these executables and even to re-distribute them, 
# so long as such use or re-distribution is accompanied with this copyright notice and is not for commercial gain.
# Note: Binaries can only be used for non-commercial purposes.
if [ "${OPJ_NONCOMMERCIAL:-}" == "1" ]; then
	if [ "${TRAVIS_OS_NAME:-}" == "linux" ] || uname -s | grep -i Linux &> /dev/null; then
		echo "Retrieving Kakadu"
		wget -q http://kakadusoftware.com/wp-content/uploads/2014/06/KDU77_Demo_Apps_for_Linux-x86-64_150710.zip
		cmake -E tar -xf KDU77_Demo_Apps_for_Linux-x86-64_150710.zip
		mv KDU77_Demo_Apps_for_Linux-x86-64_150710 kdu
	fi
fi
