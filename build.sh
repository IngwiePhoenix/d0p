#!/bin/bash

set -x

# Building d0p
cd $(dirname "$0")
ME=$(pwd)

[ ! -d $ME/lib ] && mkdir $ME/lib
if [ "$CXX" == "" ]; then
	echo "--> Using 'g++' as the CXX compiler, hopefuly its in your path!"
	CXX=g++
fi
if [ "$CC" == "" ]; then
        echo "--> Using 'gcc' as the CC compiler, hopefuly its in your path!"
        CC=gcc
fi

# First, we want to create some global build functions
build() {
	[ -f "./Makefile" ] && make distclean
	./configure --prefix=$ME $* 
	[ $? != 0 ] && exit $?
	make
	make install
}

buildIt() {
	# Now, build xz first, as we want to use its liblzma for compressing stuff
	if [ ! -f "$ME/lib/liblzma.a" ]; then
		cd $ME/xz*
		build --enable-small \
			--disable-scripts \
			--enable-silent-rules \
			--enable-static \
			--disable-shared \
			--disable-xz \
			--disable-lzmadec \
			--disable-xzdec \
			--disable-lzmainfo \
			--disable-lzma-links
	fi

	if [ ! -f "$ME/lib/libarchive.a" ]; then
		cd $ME/libarchive*
		build --disable-bsdtar \
			--disable-bsdcpio \
			--enable-static \
			--disable-shared \
			--enable-silent-rules
	fi

	# Build libraries
	# Start with yaml-cpp
	if [ ! -f "$ME/lib/libyaml-cpp.a" ]; then
		cd $ME/yaml-cpp
		$CXX -I$ME/include -c *.cpp
		ar cr libyaml-cpp.a *.o
	fi

	# Now base64
	if [ ! -f "$ME/lib/libbase64.a" ]; then
		cd $ME/base64
		$CXX -I$ME/include/base64 -c *.cpp
		ar cr libbase64.a *.o
	fi

	# Collect selfmade libs
	find $ME -name "*.a" | while read a; do
		cp "$a" $ME/lib
	done

	# Build d0p
	cd $ME/d0p
	$CXX -I$ME/include/d0p -I$ME/include \
		-L$ME/lib \
		-lz -lbz2 -lxml2 -llzma -lbase64 -larchive \
		d0p.cpp -o d0p
}

cleanIt() {
	find $ME -maxdepth 2 -type f -name "Makefile" | while read mkf; do
		make -f "$mkf" uninstall
		make -f "$mkf" clean
		make -f "$mkf" distclean
	done
	rm -rvf $ME/lib $ME/bin $ME/share $ME/include/lzma* $ME/include/archive*
}

case "$1" in
	clean) cleanIt; exit;;
	*)     buildIt; exit;;
esac
