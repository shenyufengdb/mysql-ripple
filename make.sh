#!/bin/bash
set -e
set -x

gcc --version

SOURCE_PATH=/mysql-ripple-master
BUILD_PATH=/soft/mysql/build
PROTOBUF_NAME=protobuf-3.9.0

build_version=${1:-"RELWITHDEBINFO"}
build_version=`echo $build_version | tr '[:upper:]' '[:lower:]'`
echo "Build version: ${build_version}"
[[ "$build_version" == "release" || "$build_version" == "minsizrel" ]] && \
  echo "Please use debug or relwithdebinfo" && exit 1

#prepare protobuf
mkdir -p $BUILD_PATH/3rd/protobuf
cd $BUILD_PATH/3rd/protobuf
PROTOBUF_NAME=protobuf-cpp-3.9.0
PROTOBUF_COMPRESSION_TYPE=tar.gz
if [ ! -d "${PROTOBUF_NAME}" ]; then
 if [ ! -f "${PROTOBUF_NAME}.${PROTOBUF_COMPRESSION_TYPE}" ]; then
   cp -rf $SOURCE_PATH/extra/protobuf/${PROTOBUF_NAME}.${PROTOBUF_COMPRESSION_TYPE} $BUILD_PATH/3rd/protobuf
 fi
 tar -xpf ${PROTOBUF_NAME}.${PROTOBUF_COMPRESSION_TYPE}
fi

#cmake
echo $SOURCE_PATH/binlogserver
mkdir -p $BUILD_PATH/binlogserver
cd $BUILD_PATH/binlogserver
rm -rf $BUILD_PATH/binlogserver/*
ls |grep -v "^debug$" |xargs rm -rf

CMAKE_OPT=" -DCMAKE_INSTALL_PREFIX=/debug/  -DCMAKE_BUILD_TYPE=${build_version} -DWITH_PROTOBUF=$BUILD_PATH/3rd/protobuf/${PROTOBUF_NAME}"

source /opt/rh/devtoolset-8/enable
/usr/bin/cmake3 $SOURCE_PATH $CMAKE_OPT

make -j$(getconf _NPROCESSORS_ONLN)