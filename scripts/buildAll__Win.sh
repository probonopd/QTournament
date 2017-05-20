#!/bin/bash

# define where to build the files
BUILD_DIR=/tmp/buildQTournament

# the base path of the build tool binaries
MINGW64_BIN=/d/PortablePrograms/msys64/mingw64/bin

#
# No changes below this point!
#

source ./buildBasics.sh

PATH=$PATH:$MINGW64_BIN

export CXX=g++.exe
export CC=gcc.exe
CMAKE_GENERATOR=MSYS\ Makefiles
CMAKE_BIN=/d/PortablePrograms/msys64/mingw64/bin/cmake.exe

# create the build dir
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# make sure we are in the right directory
ACTUAL_DIR=$(pwd)
if [ $ACTUAL_DIR != $BUILD_DIR ]; then
	echo Could not enter build directory!
	exit 1
fi

rm -rf dist *zip
mkdir dist

#
# Decide what to do with libSloppy
#
echo "############ LIB SLOPPY ############"
echo
echo "Choose:"
echo "  0 = download, build and install"
echo "  1 = use binary in /usr/local/lib"
read -s -n 1 SLOPPY_SEL

if [ $SLOPPY_SEL != "0" ] && [ $SLOPPY_SEL != "1" ]; then
  echo "error"
  exit
fi

if [ $SLOPPY_SEL -eq 0 ]; then
  echo "===> Download, build and install!"
  rm -rf libSloppy

  wget $SLOPPY_BASE_URL/$SLOPPY_VERSION.zip -O libSloppy-$SLOPPY_VERSION.zip
  unzip libSloppy-$SLOPPY_VERSION.zip
  mv libSloppy-$SLOPPY_VERSION libSloppy
  cd libSloppy
  mkdir release
  cd release
  $CMAKE_BIN -DCMAKE_BUILD_TYPE=Release -G "$CMAKE_GENERATOR" ..
  make -j5
  make install
  cd ..
  cd ..
fi
if [ $SLOPPY_SEL -eq 1 ]; then
  echo "===> use existing"
fi

# At this point, either a fresh or an existing
# libSloppy must exist in the file system
if [ ! -f /usr/local/lib/libSloppy.dll ]; then
  echo
  echo "!!! No libSloppy.dll in /usr/local/lib !!!"
  echo
  exit 1
fi

#
# Decide what to do with SqliteOverlay
#
echo
echo
echo "############ SqliteOverlay ############"
echo
echo "Choose:"
echo "  0 = download, build and install"
echo "  1 = use binary in /usr/local/lib"
read -s -n 1 OVERLAY_SEL

if [ $OVERLAY_SEL != "0" ] && [ $OVERLAY_SEL != "1" ]; then
  echo "error"
  exit
fi

if [ $OVERLAY_SEL -eq 0 ]; then
  echo "===> Download, build and install!"
  rm -rf SqliteOverlay

  wget $DBOVERLAY_BASE_URL/$DBOVERLAY_VERSION.zip -O SqliteOverlay-$DBOVERLAY_VERSION.zip
  unzip SqliteOverlay-$DBOVERLAY_VERSION.zip
  mv SqliteOverlay-$DBOVERLAY_VERSION SqliteOverlay
  cd SqliteOverlay
  mkdir release
  cd release
  $CMAKE_BIN -DCMAKE_BUILD_TYPE=Release -G "$CMAKE_GENERATOR" ..
  make -j5
  make install
  cd ..
  cd ..
fi
if [ $OVERLAY_SEL -eq 1 ]; then
  echo "===> use existing"
fi

# At this point, either a fresh or an existing
# SqliteOverlayLib must exist in the file system
if [ ! -f /usr/local/lib/libSqliteOverlay.dll ]; then
  echo
  echo "!!! No libSqliteOverlay.dll in /usr/local/lib !!!"
  echo
  exit 1
fi

#
# Decide what to do with the report lib
#
REPORT_SEL=0
if [ -f Qt/qmake/SimpleReportGeneratorLib/release/SimpleReportGenerator0.dll ]; then
  echo
  echo
  echo "############ SimpleReportGenerator ############"
  echo
  echo "Choose:"
  echo "  0 = download and build"
  echo "  1 = use binary in temporary build dir"
  read -s -n 1 REPORT_SEL

  if [ $REPORT_SEL != "0" ] && [ $REPORT_SEL != "1" ]; then
    echo "error"
    exit
  fi
fi
if [ $REPORT_SEL -eq 0 ]; then
  echo "===> Download and build!"
  rm -rf Qt/qmake/SimpleReportGeneratorLib

  wget $REPORTLIB_BASE_URL/$REPORTLIB_VERSION.zip -O SimpleReportGeneratorLib-$REPORTLIB_VERSION.zip
  unzip SimpleReportGeneratorLib-$REPORTLIB_VERSION.zip
  mkdir -p Qt/qmake
  mv SimpleReportGeneratorLib-$REPORTLIB_VERSION Qt/qmake/SimpleReportGeneratorLib
  cd Qt/qmake/SimpleReportGeneratorLib
  qmake CONFIG+=release SimpleReportGenerator.pro
  make -j5
  cd ..
  cd ..
  cd ..
fi
if [ $REPORT_SEL -eq 1 ]; then
  echo "===> use existing"
fi

# At this point, either a fresh or an existing
# report must exist in the file system
if [ ! -f Qt/qmake/SimpleReportGeneratorLib/release/SimpleReportGenerator0.dll ]; then
  echo
  echo "!!! No SimpleReportGenerator0.dll in build dir !!!"
  echo
  exit 1
fi

#
# Decide what to do with the QTournament
#
TORNAMENT_SEL=0
echo
echo
echo "############ QTournament ############"
echo
echo "Choose:"
echo "  0 = download and build version" $QTOURNAMENT_VERSION
echo "  1 = checkout and build MASTER"
if [ -f Qt/qmake/QTournament/.git/config ]; then
  echo "  2 = update git repo in build dir and re-build"
fi
if [ -f Qt/qmake/QTournament/release/QTournament.exe ]; then
  echo "  4 = use existing QTournament.exe"
fi
read -s -n 1 TORNAMENT_SEL

if [ $TORNAMENT_SEL != "0" ] && [ $TORNAMENT_SEL != "1" ] && [ $TORNAMENT_SEL != "2" ] && [ $TORNAMENT_SEL != "4" ]; then
  echo "error"
  exit
fi

if [ $TORNAMENT_SEL -eq 0 ]; then
  rm -rf Qt/qmake/QTournament

  wget $QTOURNAMENT_BASE_URL/$QTOURNAMENT_VERSION.zip -O QTournament-$QTOURNAMENT_VERSION.zip
  unzip QTournament-$QTOURNAMENT_VERSION.zip
  mv QTournament-$QTOURNAMENT_VERSION Qt/qmake/QTournament
  cd Qt/qmake/QTournament
fi

if [ $TORNAMENT_SEL -eq 1 ]; then
  rm -rf Qt/qmake/QTournament
  cd Qt/qmake

  git clone https://github.com/Foorgol/QTournament.git
  QTOURNAMENT_VERSION=master

  cd QTournament
fi

if [ $TORNAMENT_SEL -eq 2 ]; then
  cd Qt/qmake/QTournament

  git pull --all
  QTOURNAMENT_VERSION=git
fi

if [ $TORNAMENT_SEL != "4" ]; then
  qmake CONFIG+=release QTournament.pro
  make -j5
  cd ..
  cd ..
  cd ..
fi

# At this point, a QTournament.exe must exist in the file system
if [ ! -f Qt/qmake/QTournament/release/QTournament.exe ]; then
  echo
  echo "!!! No QTournament.exe in build dir !!!"
  echo
  exit 1
fi

echo
echo "!!! BUILD PROCESS FINISHED !!!"
echo

# copy all generated files along with their dependencies
# to one folder
cp libSloppy/release/libSloppy.dll dist
cp SqliteOverlay/release/libSqliteOverlay.dll dist
cp Qt/qmake/SimpleReportGeneratorLib/release/SimpleReportGenerator0.dll dist
cp Qt/qmake/QTournament/release/QTournament.exe dist

cd dist

# dependencies
for f in libbz2-*.dll \
	libfreetype-*.dll \
	libgcc_s_seh-*.dll \
	libglib-*.dll \
	libgraphite*.dll \
	libharfbuzz-?.dll \
	libiconv-*.dll \
	libicudt??.dll \
	libicuin??.dll \
	libicuuc??.dll \
	libintl-*.dll \
	libpcre-*.dll \
	libpcre16-*.dll \
	libpng16-*.dll \
	libsqlite3-0.dll \
	libstdc++-*.dll \
	libwinpthread-*.dll \
        libboost_filesystem-mt.dll \
        libboost_system-mt.dll \
        libboost_log-mt.dll \
        libboost_log_setup-mt.dll \
        libboost_date_time-mt.dll \
        libboost_thread-mt.dll \
        libboost_regex-mt.dll \
        libboost_chrono-mt.dll \
        libboost_atomic-mt.dll \
        Qt5Core.dll \
	Qt5Gui.dll \
	Qt5PrintSupport.dll \
	Qt5Widgets.dll \
	libdl.dll \
	zlib*.dll; do

	cp $MINGW64_BIN/$f .
done

#plugins
mkdir platforms printsupport
cp $MINGW64_BIN/../share/qt5/plugins/platforms/qwindows.dll platforms
cp $MINGW64_BIN/../share/qt5/plugins/printsupport/windowsprintersupport.dll printsupport

#translation file(s)
cp ../Qt/qmake/QTournament/tournament_*qm .
cp $MINGW64_BIN/../share/qt5/translations/*_de.qm .

# License file
cp ../Qt/qmake/QTournament/LICENSE.TXT .

# zip everything
zip -r QTournament-Win-$QTOURNAMENT_VERSION.zip *

