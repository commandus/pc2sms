#!/bin/sh
# protoc -I . --cpp_out=gen pc2sms.proto
# protoc -I . --grpc_out=gen --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin pc2sms.proto
PROTO=pc2sms.proto
DEST=gen
CPP_PLUGIN=`which grpc_cpp_plugin`
protoc -I . --cpp_out=$DEST $PROTO
protoc -I . --grpc_out=$DEST --plugin=protoc-gen-grpc=$CPP_PLUGIN $PROTO
exit 0

