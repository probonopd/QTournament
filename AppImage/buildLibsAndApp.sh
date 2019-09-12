#!/bin/bash


#
# THIS SCRIPT SHALL BE RUN IN THE DOCKER CONTAINER
# THAT WILL BE USED FOR BUILDING THE APPIMAGE!!
#

export CXX=gcc-8


TRANSFER_DIR=/tmp/transfer
APP_DIR="$TRANSFER_DIR"/AppDir
SRC_DIR="$TRANSFER_DIR"/src
BUILD_DIR="$TRANSFER_DIR"/build
TOOLS_DIR="$TRANSFER_DIR"/tools
LDEPLOY="$TOOLS_DIR"/linuxdeploy-x86_64.AppImage

build_LibSloppy()
{
  pushd "$BUILD_DIR"
  
  mkdir sloppy
  cd sloppy
  cmake -DCMAKE_BUILD_TYPE=Release "$SRC_DIR"/libSloppy
  make -j$(nproc) install
  
  popd
}

build_DbLib()
{
  pushd "$BUILD_DIR"
  
  mkdir sqliteoverlay
  cd sqliteoverlay
  cmake -DCMAKE_BUILD_TYPE=Release "$SRC_DIR"/SqliteOverlay
  make -j$(nproc) install
  
  popd
}

build_RepGenLib()
{
  pushd "$BUILD_DIR"
  
  mkdir repgenlib
  cd repgenlib
  qmake CONFIG+="release" "$SRC_DIR"/SimpleReportGeneratorLib/SimpleReportGenerator.pro
  make -j$(nproc) install
  
  popd
}

build_QTournament()
{
  # Special hack:
  # Modify the .pro-file to search for the report generator lib
  # in standard lib dirs as well
  pushd "$SRC_DIR"/tnmt
  sed -e 's/ -lSloppy/ -lSloppy -lSimpleReportGenerator/' QTournament.pro > patched.pro
  popd

  #
  # the normal build
  #  
  pushd "$BUILD_DIR"
  
  mkdir tnmt
  cd tnmt
  qmake CONFIG+="release c++1z" "$SRC_DIR"/tnmt/patched.pro
  make -j$(nproc)
  
  popd
  
  # Remove the patched project file
  pushd "$SRC_DIR"/tnmt
  rm patched.pro
  popd
}

deploy()
{
  pushd "$TRANSFER_DIR"
  LD_LIBRARY_PATH=/usr/local/lib \
  $LDEPLOY -e "$BUILD_DIR"/tnmt/QTournament \
           -d "$TRANSFER_DIR"/qtournament.desktop \
           -i "$TRANSFER_DIR"/qtournament.svg \
           -l /usr/lib/x86_64-linux-gnu/libssl.so.1.1 \
           -l /usr/lib/x86_64-linux-gnu/libsodium.so \
           --appdir $APP_DIR \
           --plugin qt
           
  # manually include translation files
  # and store them uglyly directly in /usr/bin
  cp "$SRC_DIR"/tnmt/*qm $APP_DIR/usr/bin
  
  # now build the final AppImage
  $LDEPLOY --appdir $APP_DIR --output appimage
}

#
# MAIN
#
build_LibSloppy
build_DbLib
build_RepGenLib
build_QTournament
deploy

echo "### PAUSE -- press Enter to continue and stop the docker build container"
read
