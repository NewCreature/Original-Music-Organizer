#!/bin/bash

function merge_libs() {
  lipo -create $1/$3 $2/$3 -output $1/$3
  rm -f $2/$3
}

function remake_dir() {
  rm -rf $1
  mkdir $1
}

if [ "$#" -ne 1 ]; then
  echo "Usage: macos_update_dependencies <path>"
  exit 1
fi

START_PATH=$(pwd)
X86_SDK=MacOSX10.13.sdk
ARM_SDK=MacOSX11.sdk

mkdir -p $1
cd $1

# libbinio
SDK_PATH=/Library/Developer/CommandLineTools/SDKs/$X86_SDK
if [ ! -d "libbinio" ];
then
  git clone https://github.com/NewCreature/libbinio.git
fi
cd libbinio
git pull
make -f makefile.macos universal
sudo make -f makefile.macos install
cd ..

# adplug
SDK_PATH=/Library/Developer/CommandLineTools/SDKs/$X86_SDK
if [ ! -d "adplug" ];
then
  git clone https://github.com/NewCreature/adplug.git
fi
cd adplug
git pull
make -f makefile.macos universal
sudo make -f makefile.macos install
cd ..

# libgme
SDK_PATH=/Library/Developer/CommandLineTools/SDKs/$X86_SDK
if [ ! -d "libgme" ];
then
  git clone https://github.com/NewCreature/libgme.git
fi
cd libgme
git pull
remake_dir _build_x86
cd _build_x86
cmake .. -DCMAKE_OSX_SYSROOT=$SDK_PATH -DCMAKE_OSX_ARCHITECTURES=i386\;x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET=10.6
make
cd ..
SDK_PATH=/Library/Developer/CommandLineTools/SDKs/$ARM_SDK
remake_dir _build_arm
cd _build_arm
cmake .. -DCMAKE_OSX_SYSROOT=$SDK_PATH -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
make
merge_libs gme ../_build_x86/gme libgme.a
sudo make install
cd ..
cd ..

# mpg123
if [ ! -d "mpg123" ];
then
  git clone https://github.com/NewCreature/mpg123.git
fi
cd mpg123
git pull
./configure CFLAGS="-arch i386 -arch x86_64 -isysroot /Library/Developer/CommandLineTools/SDKs/$X86_SDK -mmacos-version-min=10.6" --enable-shared=no --enable-static=yes --with-cpu=generic
make
mv src/libmpg123/.libs/libmpg123.a libmpg123.a
make clean
echo ---------------------------------
echo ---------------------------------
./configure CFLAGS="-arch arm64 -isysroot /Library/Developer/CommandLineTools/SDKs/$ARM_SDK -mmacos-version-min=11.0" --enable-shared=no --enable-static=yes --with-cpu=generic --host=`uname -m`-apple-darwin
make
merge_libs src/libmpg123/.libs . libmpg123.a
sudo cp src/libmpg123/mpg123.h /usr/local/include
sudo cp src/libmpg123/.libs/libmpg123.a /usr/local/libs
cd ..

# return to original location
cd $START_PATH
