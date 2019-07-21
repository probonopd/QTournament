#!/bin/bash

#
# DEPENDENCIES
#
SLOPPY_VER=0.3.1
DBLIB_VER=0.4.1
REPORTLIB_VER=0.3.0
QTOURNAMENT_VER=0.6.0


#
# DEFS
#
TRANSFER_DIR=/tmp/QTournament_AppImage
TOOLS_DIR="$TRANSFER_DIR"/tools
APP_DIR="$TRANSFER_DIR"/AppDir
SRC_DIR="$TRANSFER_DIR"/src
BUILD_CONTAINER=qtournament_appimage_buildenv
BUILD_DIR="$TRANSFER_DIR"/build

REPORTLIB_BASE_URL=https://github.com/Foorgol/SimpleReportGeneratorLib/archive
DBOVERLAY_BASE_URL=https://github.com/Foorgol/SqliteOverlay/archive
QTOURNAMENT_BASE_URL=https://github.com/Foorgol/QTournament/archive
SLOPPY_BASE_URL=https://github.com/Foorgol/libSloppy/archive


SLOPPY_ARCHIVE=$SLOPPY_VER.zip
DBLIB_ARCHIVE=$DBLIB_VER.zip
BUILD_SCRIPT=buildLibsAndApp.sh

RAW_APPIMAGE="$TRANSFER_DIR"/QTournament-x86_64.AppImage


#
# FUNCTIONS
#
prep_transfer_dir()
{
  if [ -d "$TRANSFER_DIR" ]; then
    rm -rf "$TRANSFER_DIR"
  fi
  if [ $? -ne 0 ]; then
    echo
    echo
    echo "*********************************************"
    echo
    echo "Error deleting the transfer dir / build dir !"
    echo
    echo "*********************************************"
    exit 1
  fi
  
  mkdir -p "$TRANSFER_DIR"
  mkdir -p "$TOOLS_DIR"
  mkdir -p "$APP_DIR"
  mkdir -p "$SRC_DIR"
  mkdir -p "$BUILD_DIR"
}

populate_transfer_dir()
{
  pushd "$SRC_DIR"
  
  mkdir download
  
  # libSloppy
  cd download
  wget $SLOPPY_BASE_URL/$SLOPPY_ARCHIVE
  unzip $SLOPPY_ARCHIVE
  rm $SLOPPY_ARCHIVE
  mv libSloppy-$SLOPPY_VER libSloppy
  mv libSloppy ..
  cd ..
  
  # SqliteOverlay
  cd download
  wget $DBOVERLAY_BASE_URL/$DBLIB_ARCHIVE
  unzip $DBLIB_ARCHIVE
  rm $DBLIB_ARCHIVE
  mv SqliteOverlay-$DBLIB_VER SqliteOverlay
  mv SqliteOverlay ..
  cd ..
  
  # Report Generator lib
  cd download
  wget $REPORTLIB_BASE_URL/$REPORTLIB_VER.zip
  unzip $REPORTLIB_VER.zip
  rm $REPORTLIB_VER.zip
  mv SimpleReportGeneratorLib-$REPORTLIB_VER SimpleReportGeneratorLib
  mv SimpleReportGeneratorLib ..
  cd ..
  
  # QTournament itself
  cd download
  wget $QTOURNAMENT_BASE_URL/$QTOURNAMENT_VER.zip
  unzip $QTOURNAMENT_VER.zip
  rm $QTOURNAMENT_VER.zip
  mv QTournament-$QTOURNAMENT_VER tnmt
  mv tnmt ..
  cd ..
  
  popd
  
  # the build script
  cp $BUILD_SCRIPT $TRANSFER_DIR
  
  # other resources
  cp qtournament.desktop $TRANSFER_DIR
  cp dummyIcon.svg $TRANSFER_DIR/qtournament.svg
}


download_linuxdeploy ()
{
  pushd "$TOOLS_DIR"
  if [ ! -e linuxdeploy-x86_64.AppImage ]; then
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
  fi

  chmod u+x *AppImage
  popd
}

prep_build_container()
{
  # does the image for the build environment exist?
  REFRESH_BUILD_CONTAINER=y
  docker images | grep $BUILD_CONTAINER > /dev/null
  if [ $? -eq 0 ]; then
    echo
    echo "Do you want to update the QTournament Build Environment container?"
    echo "(Enter = yes, y = yes, n = no, Ctrl-C = cancel)"
    echo -n "---> " ; read REFRESH_BUILD_CONTAINER

    if [ -z $REFRESH_BUILD_CONTAINER ]; then
      REFRESH_BUILD_CONTAINER=y
    fi
  fi

  if [ $REFRESH_BUILD_CONTAINER = "y" ]; then
    docker build -t $BUILD_CONTAINER -f Dockerfile_BuildEnv .
  fi
}

compile_everything()
{
  docker run -ti \
    --privileged --cap-add=ALL \
    -v $TRANSFER_DIR:/tmp/transfer \
    "$BUILD_CONTAINER" \
    /tmp/transfer/$BUILD_SCRIPT
  docker system prune
}

save_binary()
{
  # the target name for the AppImage file
  TARGET_NAME="QTournament-Linux-"$QTOURNAMENT_VER".AppImage"

  #
  # generate a unique name for a backup
  # and backup possibly existing AppImages
  # for the same software version
  #
  if [ -e $TARGET_NAME ]; then
    BACKUP_NAME="_"$TARGET_NAME
    
    while [ -e $BACKUP_NAME ]; do
      BACKUP_NAME="_"$BACKUP_NAME
    done
    
    mv $TARGET_NAME $BACKUP_NAME
  fi

  # copy the file over from the transfer directory
  cp "$RAW_APPIMAGE" $TARGET_NAME
  
  echo
  echo
  echo
  echo "#"
  echo "#"
  echo "##### AppImage stored to "$(pwd)"/"$TARGET_NAME
  echo "#"
  echo "#"
  echo
}


#
# MAIN
#

if [ $0 != "./buildAppImage.sh" ]; then
  echo
  echo "Please execute this script directly"
  echo "from its directory in the source tree!"
  echo
  exit 1
fi

prep_build_container

prep_transfer_dir
populate_transfer_dir
download_linuxdeploy

compile_everything

# check success
if [ ! -e $RAW_APPIMAGE ]; then
  echo
  echo
  echo
  echo "#"
  echo "#"
  echo "##### For some wierd reason, the build process for the AppImage FAILED!"
  echo "#"
  echo "#"
  echo
  exit 1
fi

save_binary

echo
echo "Do you want to force-delete the temporary directory"
echo "that contains the generated build files?"
echo "(y = yes, Enter = no, n = no, Ctrl-C = cancel)"
echo -n "---> " ; read CLEAR_TRANSFER_DIR

if [ -z $CLEAR_TRANSFER_DIR ]; then
  CLEAR_TRANSFER_DIR=n
fi

if [ $CLEAR_TRANSFER_DIR = "y" ]; then
  echo
  echo "Please provide SUDO password for deleting "$TRANSFER_DIR
  sudo rm -rf $TRANSFER_DIR
fi

