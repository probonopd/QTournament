#!/bin/bash

BUILD_DIR=/tmp/buildAllQtournament

DBOVERLAY_VERSION=0.2.0

REPORTLIB_VERSION=0.3.0

QTOURNAMENT_VERSION=0.4.0-RC1

#
#
#
# NO CHANGES BELOW THIS POINT !!!
#
#
#
REPORTLIB_BASE_URL=https://github.com/Foorgol/SimpleReportGeneratorLib/archive
DBOVERLAY_BASE_URL=https://github.com/Foorgol/SqliteOverlay/archive
QTOURNAMENT_BASE_URL=https://github.com/Foorgol/QTournament/archive

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
rm -rf SqliteOverlay SimpleReportGeneratorLib Qt dist

#
# download and build SQLiteOverlayLib
#
wget $DBOVERLAY_BASE_URL/$DBOVERLAY_VERSION.zip
unzip $DBOVERLAY_VERSION.zip
mv SqliteOverlay-$DBOVERLAY_VERSION SqliteOverlay
rm *zip
cd SqliteOverlay
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ..
cd ..

#
# download and build the SimpleReportGeneratorLib
#
wget $REPORTLIB_BASE_URL/$REPORTLIB_VERSION.zip
unzip $REPORTLIB_VERSION.zip
mv SimpleReportGeneratorLib-$REPORTLIB_VERSION SimpleReportGeneratorLib
rm *zip
mkdir Qt
mv SimpleReportGeneratorLib Qt
cd Qt/SimpleReportGeneratorLib
qmake CONFIG+="release" SimpleReportGenerator.pro 
make
mkdir release
mv libSimpleReportGenerator.so* release
cd ..

#
# download and build QTournament
#
wget $QTOURNAMENT_BASE_URL/$QTOURNAMENT_VERSION.zip
unzip $QTOURNAMENT_VERSION.zip
rm *zip
mv QTournament-$QTOURNAMENT_VERSION QTournament
cd QTournament
qmake CONFIG+="release" QTournament.pro
make
cd ..
cd ..

#
# pack everything
#
mkdir dist
cd dist
cp ../SqliteOverlay/release/libSqliteOverlay.so.0.2.0 libSqliteOverlay.so.0
cp ../Qt/SimpleReportGeneratorLib/release/libSimpleReportGenerator.so.0.3.0 libSimpleReportGenerator.so.0
cp ../Qt/QTournament/QTournament QTournament_bin
cp ../Qt/QTournament/tournament_de.qm .
echo '#!/bin/bash' > QTournament
echo 'LD_LIBRARY_PATH=. ./QTournament_bin' >> QTournament
chmod u+x QTournament
zip QTournament-$QTOURNAMENT_VERSION.zip *

