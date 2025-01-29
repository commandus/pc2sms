#!/bin/sh
PROTO=pc2sms.proto
DEST=gen
CPP_PLUGIN=`which grpc_cpp_plugin`
protoc -I . --cpp_out=$DEST $PROTO
protoc -I . --grpc_out=$DEST --plugin=protoc-gen-grpc=$CPP_PLUGIN $PROTO
exit 0

