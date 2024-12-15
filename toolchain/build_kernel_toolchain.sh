#!/bin/bash

export BINUTILS_VERSION=2.43.1
export GCC_VERSION=14.2.0

export TARGET_NAME=$1
export TARGET=$TARGET_NAME-elf
export BASE_FOLDER=$(dirname $(realpath "$0"))/$TARGET_NAME/
export OUTPUT_FOLDER=$BASE_FOLDER/kernel/
export DOWNLOADS_FOLDER=$OUTPUT_FOLDER/src/

echo Building gcc and binutils for $TARGET_NAME in $OUTPUT_FOLDER

# create necessary folders
mkdir -p $BASE_FOLDER

# check for an already compiled toolchain
if [ -d $OUTPUT_FOLDER ]; then
    while true; do
        read -p "A kernel toolchain is already compiled. Do you want to delete it and recompile it? This action is irreversible! [y/n]" yn
        case $yn in
            [Yy]* ) rm -rf $OUTPUT_FOLDER; break;;
            [Nn]* ) exit;;
            * ) echo "Please answer yes or no.";;
        esac
    done
fi

mkdir -p $OUTPUT_FOLDER
mkdir -p $DOWNLOADS_FOLDER

# download tarballs
cd $DOWNLOADS_FOLDER
wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz

# expand sources
tar -xvf binutils-$BINUTILS_VERSION.tar.xz
tar -xvf gcc-$GCC_VERSION.tar.xz

# build binutils
cd binutils-$BINUTILS_VERSION
mkdir build
cd build
../configure --target=$TARGET --prefix="$OUTPUT_FOLDER" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

# build gcc
cd $DOWNLOADS_FOLDER/gcc-$GCC_VERSION
mkdir build
cd build
../configure --target=$TARGET --prefix="$OUTPUT_FOLDER" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j$(nproc)
make install-gcc