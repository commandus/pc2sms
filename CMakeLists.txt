#
# cmake -DCMAKE_TOOLCHAIN_FILE=C:\git\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows ..
#
cmake_minimum_required(VERSION 3.9)
set(VERSION_INFO 1.0)
project("pc2sms" VERSION ${VERSION_INFO})

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
	# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
	# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
	set(OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)

	find_package(Protobuf CONFIG REQUIRED)
	find_package(gRPC CONFIG REQUIRED)
	set(LIB_GRPC protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
	set(LIB_GRPC ${LIB_GRPC} gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)
	set(INC_GRPC ${GRPC_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS})
else()
	set(LIB_GRPC protobuf grpc++ grpc pthread ssl crypto z gpr)
endif()

SET(ARGTABLE_SOURCES third-party/argtable3/argtable3.c)

SET(PC2SMS_SOURCES
	pc2sms.cpp
	service-raw.cpp
	sms-listener.cpp
	errlist.cpp
	utilstring.cpp
	file-helper.cpp
	daemonize.cpp
	config-filename.cpp
	pc2sms-config.cpp
	gen/pc2sms.pb.cc
	gen/pc2sms.grpc.pb.cc

)

add_executable(pc2sms ${PC2SMS_SOURCES} ${ARGTABLE_SOURCES})
target_link_libraries(pc2sms PRIVATE ${LIB_GRPC} ${OS_SPECIFIC_LIBS})
target_include_directories(pc2sms PRIVATE "." "third-party" "gen" ${INC_GRPC})

SET(SEND_SMS_SOURCES
	send-sms.cpp errlist.cpp
	third-party/argtable3/argtable3.c
    config-filename.cpp
	utilstring.cpp
	file-helper.cpp
	send-sms-config.cpp
    gen/pc2sms.pb.cc 
    gen/pc2sms.grpc.pb.cc
)

add_executable(send-sms ${SEND_SMS_SOURCES} ${ARGTABLE_SOURCES})
target_link_libraries(send-sms PRIVATE ${LIB_GRPC} ${OS_SPECIFIC_LIBS})
target_include_directories(send-sms PRIVATE "." "third-party" "gen" ${INC_GRPC})
