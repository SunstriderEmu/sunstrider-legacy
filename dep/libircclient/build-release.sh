#!/bin/sh

# Export the source code

PACKAGE=libircclient
BINARYFILE="libircclient.dll"

FILE_VERSION="src/params.h"

# Get current version
VERSION_MAJOR=`sed -n 's/^\#define\s\+LIBIRC_VERSION_HIGH\s\+\([0-9]\+\)/\1/p' $FILE_VERSION`
VERSION_MINOR=`sed -n 's/^\#define\s\+LIBIRC_VERSION_LOW\s\+\([0-9]\+\)/\1/p' $FILE_VERSION`
CURRENTVER="$VERSION_MAJOR.$VERSION_MINOR"

BUILDDIR="build"
RELEASEDIR="release/$CURRENTVER"
PKGDIR="$PACKAGE-$CURRENTVER"

if [ -d "$BUILDDIR" ]; then
	rm -rf "$BUILDDIR"
fi

if [ ! -d "$RELEASEDIR" ]; then
	mkdir -p $RELEASEDIR
fi

if [ -d "$PKGDIR" ]; then
	rm -rf "$PKGDIR"
fi

# Store the source code
svn export . "$PKGDIR/" || exit 1

# Source package
tar zcf "$RELEASEDIR/$PKGDIR.tar.gz" $PKGDIR/ || exit 1
rm -rf $PKGDIR/*

# win32
svn export . "$BUILDDIR/" || exit 1
export PATH=$PATH:/usr/toolchains/windows-x86-mingw-qtsdl/bin/
(cd $BUILDDIR && ./configure --enable-shared --host=i686-pc-mingw32 && make)  || exit 1

BINDIR="$PKGDIR/bin"
mkdir -p $BINDIR
cp $BUILDDIR/src/libircclient.dll $BINDIR/ || exit 1
cp $BUILDDIR/src/libircclient.lib $BINDIR/  || exit 1
mkdir "$PKGDIR/include"
cp include/*.h "$PKGDIR/include"  || exit 1
mkdir "$PKGDIR/examples"
cp examples/*.c* $PKGDIR/examples  || exit 1

zip -r $RELEASEDIR/$PKGDIR-win32-dll.zip $PKGDIR || exit 1

# And the SSL version
(cd $BUILDDIR && make distclean && ./configure --enable-shared --host=i686-pc-mingw32 --enable-openssl && make clean && make)  || exit 1
cp $BUILDDIR/src/libircclient.dll $BINDIR/  || exit 1
cp $BUILDDIR/src/libircclient.lib $BINDIR/  || exit 1

zip -r $RELEASEDIR/$PKGDIR-win32-dll-openssl.zip $PKGDIR || exit 1

rm -rf "$BUILDDIR" "$PKGDIR"
echo "Done! Version $CURRENTVER released!"
