#!/bin/bash

BUILD_DIR=/tmp/buildAllQtournament

#
#
#
# NO CHANGES BELOW THIS POINT !!!
#
#
#

source ./buildBasics.sh

#
# create the build dir
#
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# make sure we are in the right directory
ACTUAL_DIR=$(pwd)
if [ $ACTUAL_DIR != $BUILD_DIR ]; then
	echo Could not enter build directory!
	exit 1
fi

#
# remove any stale, old files
#
rm -rf dist *zip
mkdir dist Qt

#
# make sure the right libSloppy exists
#
SLOPPY_LIB="/usr/local/lib/libSloppy.so.$SLOPPY_VERSION"
if [ ! -f $SLOPPY_LIB ]; then
  echo
  echo !!! could not find $SLOPPY_LIB !!!
  echo
  exit
fi

#
# make sure the right SqliteOverlay exists
#
OVERLAY_LIB="/usr/local/lib/libSqliteOverlay.so.$DBOVERLAY_VERSION"
if [ ! -f $OVERLAY_LIB ]; then
  echo
  echo !!! could not find $OVERLAY_LIB !!!
  echo
  exit
fi

#
# Decide what to do with the report lib
#
REPORT_SEL=0
REPORT_LIB=Qt/SimpleReportGeneratorLib/release/libSimpleReportGenerator.so.$REPORTLIB_VERSION
if [ -f $REPORT_LIB ]; then
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
  rm -rf Qt/SimpleReportGeneratorLib

  wget $REPORTLIB_BASE_URL/$REPORTLIB_VERSION.zip
  unzip $REPORTLIB_VERSION.zip
  mv SimpleReportGeneratorLib-$REPORTLIB_VERSION SimpleReportGeneratorLib
  rm *zip
  mv SimpleReportGeneratorLib Qt
  cd Qt/SimpleReportGeneratorLib
  qmake CONFIG+="release" SimpleReportGenerator.pro
  make -j5
  mkdir release
  mv libSimpleReportGenerator.so* release
  cd ..
  cd ..
fi
if [ $REPORT_SEL -eq 1 ]; then
  echo "===> use existing"
fi

# At this point, either a fresh or an existing
# report must exist in the file system
if [ ! -f $REPORT_LIB ]; then
  echo
  echo "!!! No $REPORT_LIB in build dir !!!"
  echo
  exit 1
fi


#
# Decide what to do with the QTournament
#
TORNAMENT_SEL=0
TOURNAMENT_BIN=Qt/QTournament/QTournament
echo
echo
echo "############ QTournament ############"
echo
echo "Choose:"
echo "  0 = download and build version" $QTOURNAMENT_VERSION
echo "  1 = checkout and build MASTER"
if [ -f Qt/QTournament/.git/config ]; then
  echo "  2 = update git repo in build dir and re-build"
fi
if [ -f $TOURNAMENT_BIN ]; then
  echo "  3 = use existing QTournament binary"
fi
read -s -n 1 TORNAMENT_SEL

if [ $TORNAMENT_SEL != "0" ] && [ $TORNAMENT_SEL != "1" ] && [ $TORNAMENT_SEL != "2" ] && [ $TORNAMENT_SEL != "3" ]; then
  echo "error"
  exit
fi

if [ $TORNAMENT_SEL -eq 0 ]; then
  rm -rf Qt/QTournament

  wget $QTOURNAMENT_BASE_URL/$QTOURNAMENT_VERSION.zip
  unzip $QTOURNAMENT_VERSION.zip
  rm *zip
  mv QTournament-$QTOURNAMENT_VERSION QTournament
  mv QTournament Qt
  cd Qt/QTournament
fi

if [ $TORNAMENT_SEL -eq 1 ]; then
  rm -rf Qt/QTournament
  cd Qt

  git clone https://github.com/Foorgol/QTournament.git
  QTOURNAMENT_VERSION=master

  cd QTournament
fi

if [ $TORNAMENT_SEL -eq 2 ]; then
  cd Qt/QTournament

  git pull --all
  QTOURNAMENT_VERSION=git

  # delete old binary to force new linking
  rm QTournament
fi

if [ $TORNAMENT_SEL != "3" ]; then
  qmake CONFIG+=release QTournament.pro
  make -j5
  cd ..
  cd ..
fi

# At this point, a QTournament.exe must exist in the file system
if [ ! -f $TOURNAMENT_BIN ]; then
  echo
  echo "!!! No $TOURNAMENT_BIN in build dir !!!"
  echo
  exit 1
fi

echo
echo "!!! BUILD PROCESS FINISHED !!!"
echo

#
# pack everything
#
cp $SLOPPY_LIB dist/libSloppy.so.0
cp $OVERLAY_LIB dist/libSqliteOverlay.so.0
cp $REPORT_LIB dist/libSimpleReportGenerator.so.0
cp $TOURNAMENT_BIN dist/QTournament_bin
cd dist
cp ../Qt/QTournament/tournament_de.qm .
cp ../Qt/QTournament/LICENSE.TXT .
echo '#!/bin/bash' > QTournament
echo 'LD_LIBRARY_PATH=. ./QTournament_bin' >> QTournament
chmod u+x QTournament
zip QTournament-Linux-$QTOURNAMENT_VERSION.zip *

