#!/bin/bash

if [ ! $PFM_ABE_DEV ]; then

    export PFM_ABE_DEV=${1:-"/usr/local"}

fi

export PFM_BIN=$PFM_ABE_DEV/bin
export PFM_LIB=$PFM_ABE_DEV/lib
export PFM_INCLUDE=$PFM_ABE_DEV/include


CHECK_QT=`echo $QTDIR | grep "qt-3"`
if [ $CHECK_QT ] || [ ! $QTDIR ]; then
    QTDIST=`ls ../../FOSS_libraries/qt-*.tar.gz | cut -d- -f5 | cut -dt -f1 | cut -d. --complement -f4`
    QT_TOP=Trolltech/Qt-$QTDIST
    QTDIR=$PFM_ABE_DEV/$QT_TOP
fi


#  Check for major version >= 5 so that we can add the "widgets" field to QT

QT_MAJOR_VERSION=`echo $QTDIR | sed -e 's/^.*Qt-//' | cut -d. -f1`
if [ $QT_MAJOR_VERSION -ge 5 ];then
    WIDGETS="widgets"
else
    WIDGETS=""
fi


SYS=`uname -s`


if [ $SYS = "Linux" ]; then
    DEFS="NVLinux _LARGEFILE64_SOURCE _XOPEN_SOURCE"
    export LD_LIBRARY_PATH=$PFM_LIB:$QTDIR/lib:$LD_LIBRARY_PATH


#   Check for a static build

    if [ -a $QTDIR/lib/libQtCore.a ]; then
        CONF=staticlib
        DEFS="NVLinux _LARGEFILE64_SOURCE _XOPEN_SOURCE PFM_STATIC"
    fi
else


#   Had to make this a static library on Windoze due to my inability to make a shared version of 
#   the feature library.  Google for __declspec and MINGW to see what's happening.

    DEFS="XML_LIBRARY WIN32 NVWIN3X _LARGEFILE64_SOURCE PFM_STATIC"
    CONF="staticlib exceptions"
    export QMAKESPEC=win32-g++
fi


# This is the only way I can keep lasdefinitions.hpp from barfing warnings all over my builds.

LASLIB_BS="-fno-strict-aliasing"


# Building the Makefile using qmake and adding extra includes, defines, and libs


rm -f qrc_icons.cpp nvutility.pro Makefile


# Get the version number from the nvutility_version.h file.

UVERSION=`grep "nvutility library V" nvutility_version.h | cut -d- -f2 | cut -dV -f2`


$QTDIR/bin/qmake -project -t lib -o nvutility.tmp
cat >nvutility.pro <<EOF
contains(QT_CONFIG, opengl): QT += opengl
QT += $WIDGETS
INCLUDEPATH += $PFM_INCLUDE
DEFINES += $DEFS
QMAKE_CXXFLAGS_EXCEPTIONS_ON = -fexceptions
VERSION = $UVERSION
CONFIG += $CONF
CONFIG += console
QMAKE_CXXFLAGS += $LASLIB_BS
EOF

cat nvutility.tmp >>nvutility.pro
rm nvutility.tmp


$QTDIR/bin/qmake -o Makefile



if [ $SYS = "Linux" ]; then
    make
    if [ $? != 0 ];then
        exit -1
    fi
    chmod 755 libnvutility*
    mv libnvutility* $PFM_LIB
else
    if [ ! $WINMAKE ]; then
        WINMAKE=release
    fi
    if [ $? != 0 ];then
        exit -1
    fi
    make $WINMAKE
    cp $WINMAKE/libnvutility.a $PFM_LIB
    rm $WINMAKE/libnvutility.a
fi


chmod 644 *.h
chmod 644 *.hpp

cp *.h $PFM_INCLUDE
cp *.hpp $PFM_INCLUDE


#  Run doxygen to create the programmer documentation (this is only done from the top-level mk script).

if [ "$BUILD_DOXYGEN" = "YES" ];then

    echo "###################################################"
    echo "Running doxygen for utility library"
    echo "###################################################"

    rm -rf ../../Documentation/APIs_and_Programs/NVUTILITY_API_Documentation
    mkdir NVUTILITY_API_Documentation
    doxygen 2>&1 >/dev/null
    mv NVUTILITY_API_Documentation ../../Documentation/APIs_and_Programs
fi


# Get rid of the Makefile so there is no confusion.  It will be generated again the next time we build.

rm Makefile
