SET PROTO=pc2sms.proto
SET DEST=gen
SET CPP_PLUGIN=C:\bin\grpc_cpp_plugin.exe 
protoc -I . --cpp_out=%DEST% %PROTO%
protoc -I . --grpc_out=%DEST% --plugin=protoc-gen-grpc=%CPP_PLUGIN% %PROTO%