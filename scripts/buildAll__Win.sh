#!/bin/bash

# define where to build the files
BUILD_DIR=/tmp/buildQTournament

# the base path of the build tool binaries
MINGW64_BIN=/d/PortablePrograms/msys64/mingw64/bin

# define the source code versions to be used
DBOVERLAY_VERSION=0.2.0
REPORTLIB_VERSION=0.3.0
QTOURNAMENT_VERSION=master

#
# No changes below this point!
#
PATH=$PATH:$MINGW64_BIN

export CXX=g++.exe
export CC=gcc.exe
CMAKE_GENERATOR=MSYS\ Makefiles
CMAKE_BIN=/d/PortablePrograms/msys64/mingw64/bin/cmake.exe

# Guthub URLs for the code download
DBOVERLAY_BASEURL=https://github.com/Foorgol/SqliteOverlay/archive
REPORTLIB_BASEURL=https://github.com/Foorgol/SimpleReportGeneratorLib/archive
QTOURNAMENT_BASEURL=https://github.com/Foorgol/QTournament/archive

# create the build dir
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# make sure we are in the right directory
ACTUAL_DIR=$(pwd)
if [ $ACTUAL_DIR != $BUILD_DIR ]; then
	echo Could not enter build directory!
	exit 1
fi

rm -rf dist

# clean the build tree by removing old build files
rm -rf SqliteOverlay Qt dist *zip

# download sources
wget $DBOVERLAY_BASEURL/$DBOVERLAY_VERSION.zip -O SqliteOverlay-$DBOVERLAY_VERSION.zip
wget $REPORTLIB_BASEURL/$REPORTLIB_VERSION.zip -O SimpleReportGeneratorLib-$REPORTLIB_VERSION.zip
wget $QTOURNAMENT_BASEURL/$QTOURNAMENT_VERSION.zip -O QTournament-$QTOURNAMENT_VERSION.zip

# Unzip and build the database overlay
unzip SqliteOverlay-$DBOVERLAY_VERSION.zip
mv SqliteOverlay-$DBOVERLAY_VERSION SqliteOverlay
cd SqliteOverlay
mkdir release
cd release
$CMAKE_BIN -DCMAKE_BUILD_TYPE=Release -G "$CMAKE_GENERATOR" ..
make
cd ..
cd ..

# Unzip and build the report generator
unzip SimpleReportGeneratorLib-$REPORTLIB_VERSION.zip
mkdir -p Qt/qmake
mv SimpleReportGeneratorLib-$REPORTLIB_VERSION Qt/qmake/SimpleReportGeneratorLib
cd Qt/qmake/SimpleReportGeneratorLib
qmake CONFIG+=release SimpleReportGenerator.pro
make
cd ..
cd ..
cd ..

# Unzip and build QTournament
unzip QTournament-$QTOURNAMENT_VERSION.zip
mv QTournament-$QTOURNAMENT_VERSION Qt/qmake/QTournament
cd Qt/qmake/QTournament
qmake CONFIG+=release QTournament.pro
make
cd ..
cd ..
cd ..

# copy all generated files along with their dependencies
# to one folder
mkdir dist
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
	Qt5Core.dll \
	Qt5Gui.dll \
	Qt5PrintSupport.dll \
	Qt5Widgets.dll \
	zlib*.dll; do

	cp $MINGW64_BIN/$f .
done

#plugins
mkdir platforms printsupport
cp $MINGW64_BIN/../share/qt5/plugins/platforms/qwindows.dll platforms
cp $MINGW64_BIN/../share/qt5/plugins/printsupport/windowsprintersupport.dll printsupport

#translation file(s)
cp ../Qt/qmake/QTournament/tournament_*qm .

# zip everything
zip QTournament-$QTOURNAMENT_VERSION.zip *

