#!/bin/bash -eux 

#
# THIS SCRIPT SHALL BE RUN IN THE DOCKER CONTAINER
# THAT WILL BE USED FOR BUILDING THE APPIMAGE!!
#

apt-get -y update
apt-get -y upgrade
apt-get install -y g++-8 make cmake libsodium23 libsodium-dev git wget libboost-all-dev sqlite3 libsqlite3-dev qt5-default fuse

# for some weird reason, debian doesn't link
# the actuall compiler "g++-8" to the standard name "g++"
#
# that causes qmake to fail and setting QMAKE_CXX doesn't help
ln -s /usr/bin/g++-8 /usr/bin/g++

