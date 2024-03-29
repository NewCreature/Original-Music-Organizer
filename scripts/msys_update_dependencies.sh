#!/bin/bash

function remake_dir() {
  rm -rf $1
  mkdir $1
}

if [ "$#" -ne 1 ]; then
  echo "Usage: msys_update_dependencies <path>"
  exit 1
fi

START_PATH=$(pwd)

mkdir -p $1
cd $1

# libbinio
if [ ! -d "libbinio" ];
then
  git clone https://github.com/NewCreature/libbinio.git
fi
cd libbinio
git pull
make -f makefile.msys
make -f makefile.msys install
cd ..

# adplug
if [ ! -d "adplug" ];
then
  git clone https://github.com/NewCreature/adplug.git
fi
cd adplug
git pull
make -f makefile.msys
make -f makefile.msys install
cd ..

# libgme
if [ ! -d "libgme" ];
then
  git clone https://github.com/NewCreature/libgme.git
fi
cd libgme
git pull
remake_dir _build
cd _build
cmake .. -DCMAKE_INSTALL_PREFIX=/mingw32 -G "Unix Makefiles"
make
make install
cd ..
cd ..

# mpg123
if [ ! -d "mpg123" ];
then
  git clone https://github.com/NewCreature/mpg123.git
fi
cd mpg123
git pull
./configure --enable-shared=no --enable-static=yes --with-cpu=generic --prefix=/mingw32
make clean
make
make install
cd ..

# return to original location
cd $START_PATH
