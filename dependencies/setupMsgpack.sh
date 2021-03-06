#!/bin/bash
# Shell script for installing msgpack-c dependency automatically

basedir=$(pwd)

name=msgpack-c
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

set -e

# Download and verify
./download-dependencies.py ${name}

source setHopsanBuildPaths.sh

# Create build dir and enter it
mkdir -p $builddir
cd $builddir

# Generate makefiles
cmake -Wno-dev -DCMAKE_INSTALL_PREFIX=${installdir} ${codedir}

# Build and install
cmake --build . --parallel $(getconf _NPROCESSORS_ONLN)
cmake --build . --target install

# Return to basedir
cd $basedir
echo "setupMsgpack.sh done!"
