#!/bin/bash
#
# sudo apt-get update
# sudo apt-get install build-essential autoconf libtool pkg-config cmake gcc clang
# git clone https://github.com/grpc/grpc.git
# git clone -b v1.24.3 https://github.com/grpc/grpc.git
# cd grpc
# git submodule update --init
# make
# sudo make install
# sudo cp ./bins/opt/protobuf/protoc /usr/local/sbin
#
# TODO It does not work! Clone repository, zipped version cannot build third parties
#
GRPC_VERSION=1.24.3
if [ ! -d grpc-${GRPC_VERSION} ]; then
  if [ ! -f grpc-${GRPC_VERSION} ]; then
    wget --no-check-certificate -O grpc-${GRPC_VERSION}.tgz https://github.com/grpc/grpc/archive/v${GRPC_VERSION}.tar.gz
  fi
  tar xzf grpc-${GRPC_VERSION}.tgz
fi;
cd grpc-${GRPC_VERSION}
git submodule update --init
make
sudo make install
protoc --version

exit 0
