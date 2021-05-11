#!/bin/sh
INC=.
OUT=.
protoc -I=$INC gpstrack.proto --js_out=import_style=commonjs:$OUT
# Download plugin https://github.com/grpc/grpc-web/releases into /usr/local/bin/protoc-gen-grpc-web
protoc -I=$INC gpstrack.proto --grpc-web_out=import_style=commonjs,mode=grpcwebtext:$OUT