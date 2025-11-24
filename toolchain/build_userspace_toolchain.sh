#!/bin/bash

export TARGET=$1-pc-gaos
export BASE_FOLDER=$(dirname $(realpath "$0"))/$1/userspace
export OUTPUT_FOLDER=$BASE_FOLDER/
export DOWNLOADS_FOLDER=$OUTPUT_FOLDER/src/

echo Building gcc and binutils for $TARGET in $OUTPUT_FOLDER

# create necessary folders
mkdir -p $BASE_FOLDER

# check for an already compiled toolchain
if [ -d $OUTPUT_FOLDER ]; then
    while true; do
        read -p "A userspace toolchain is already compiled. Do you want to delete it and recompile it? This action is irreversible! [y/n]" yn
        case $yn in
            [Yy]* ) rm -rf $OUTPUT_FOLDER; break;;
            [Nn]* ) exit;;
            * ) echo "Please answer y(es) or n(o).";;
        esac
    done
fi

mkdir -p $OUTPUT_FOLDER
mkdir -p $DOWNLOADS_FOLDER

# download sources
cd $DOWNLOADS_FOLDER
git clone https://github.com/Moldytzu/gcc-gaos.git --depth=1
git clone https://github.com/Moldytzu/binutils-gaos.git --depth=1

# build binutils
cd binutils-gaos
mkdir build
cd build
../configure --target=$TARGET --prefix="$OUTPUT_FOLDER" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

# build gcc
cd $DOWNLOADS_FOLDER/gcc-gaos
mkdir build
cd build
../configure --target=$TARGET --prefix="$OUTPUT_FOLDER" --disable-nls --enable-languages=c --without-headers --disable-shared --disable-multilib --disable-bootstrap
make all-gcc -j$(nproc)
make install-gcc