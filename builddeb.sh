#!/bin/bash

echo "When cross-compiling, do not forget to set the environment variables CROSS_COMPILE, SYSROOT and CMAKE_PKG_CONFIG_PATH"

rm -rf build/
mkdir -p build/package/src/
git archive -o update.tgz HEAD
mv update.tgz build/package/src/
cd build/package/src/
tar -xvf update.tgz
JOBS=$((2*$(nproc)))
DEB_BUILD_OPTIONS=nocheck dpkg-buildpackage -j$JOBS -d -uc -us -aarm64 --target-arch arm64
