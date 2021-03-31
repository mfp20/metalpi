#!/bin/bash

set -e

SCRIPTDIR="$(dirname "$0")"
HERE="$(cd "$SCRIPTDIR" && pwd)"
if [ -z "$1" ]; then
	PARALLEL="-j 2"
else
	PARALLEL="-j $1"
fi

echo
echo "*********************"
echo "* Building binutils *"
echo "*********************"
echo

rm -rf build/binutils
mkdir -p build/binutils
pushd build/binutils
../../binutils-vc4/configure --target=vc4-elf --prefix="$HERE"/prefix --disable-werror
make $PARALLEL
make $PARALLEL install
popd

echo
echo "************************"
echo "* Building stage 1 GCC *"
echo "************************"
echo

rm -rf build/gcc1
mkdir build/gcc1
pushd build/gcc1
../../gcc-vc4/configure --target=vc4-elf --prefix="$HERE"/prefix --disable-nls \
  --disable-ssp --enable-languages=c --without-headers --with-newlib \
  --disable-decimal-float --disable-threads --disable-libatomic \
  --disable-libitm --disable-libsanitizer --disable-libquadmath \
  --disable-lto --enable-sjlj-exceptions
make $PARALLEL
make $PARALLEL install
popd

echo
echo "*****************************"
echo "* Building Newlib C library *"
echo "*****************************"
echo

rm -rf build/newlib
mkdir build/newlib
pushd build/newlib
export PATH="$HERE"/prefix/bin:$PATH
../../newlib/configure --target=vc4-elf --prefix="$HERE"/prefix
make $PARALLEL
make $PARALLEL install
popd

echo
echo "************************"
echo "* Building stage 2 GCC *"
echo "************************"
echo

rm -rf build/gcc2
mkdir build/gcc2
pushd build/gcc2
../../gcc-vc4/configure --target=vc4-elf --prefix="$HERE"/prefix --disable-nls \
  --disable-ssp --enable-languages=c,c++ --disable-decimal-float \
  --enable-threads --disable-libatomic --disable-libsanitizer \
  --disable-libquadmath --disable-lto --enable-sjlj-exceptions --with-newlib
make $PARALLEL
make $PARALLEL install
popd

