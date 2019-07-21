#!/bin/bash -eux 

#
# THIS SCRIPT SHALL BE RUN IN THE DOCKER CONTAINER
# THAT WILL BE USED FOR BUILDING THE APPIMAGE!!
#

#yum -y install epel-release
apt-get -y update
apt-get -y upgrade
apt-get install -y g++ make cmake libsodium18 libsodium-dev git wget libboost-all-dev sqlite3 libsqlite3-dev qt5-default fuse
#yum -y install qt5-qtbase-devel gcc gcc-c++
