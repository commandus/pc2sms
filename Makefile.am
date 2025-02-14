SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4
# `pkg-config --libs protobuf grpc++`
LDFLAGS=-L/usr/local/lib -lprotobuf -lgrpc++ -lgrpc -pthread -lssl -lcrypto -lz -lgpr

AM_CXXFLAGS=-std=c++11 -O0

# -g3 -O0
commoncppflags = -Ithird-party -Igen

#
#	Generate protobuf first, then grpc
#
proto = pc2sms.proto
gengrpcs = pc2sms.pb.h pc2sms.pb.сс pc2sms.grpc.pb.h pc2sms.grpc.pb.cc
$(gengrpcs): Makefile
	protoc -I gen --cpp_out=. $(proto)
	protoc -I gen --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $(proto)

#
#	Exclude generated sources from distribution
#
nodist_pc2sms_SOURCES = $(gengrpcs)
BUILT_SOURCES = $(gengrpcs)
CLEANFILES = $(gengrpcs)

#
# Binaries
#
bin_PROGRAMS=pc2sms send-sms
nobase_dist_include_HEADERS = \
	platform.h \
	third-party/argtable3/argtable3.h \
	errlist.h utilstring.h daemonize.h service-raw.h \
	pc2sms-config.h send-sms-config.h sms-listener.h \
	config-filename.h

common_src =

commonlibs = 

pc2sms_SOURCES= \
	pc2sms.cpp errlist.cpp service-raw.cpp sms-listener.cpp \
	utilstring.cpp file-helper.cpp daemonize.cpp \
	config-filename.cpp \
	third-party/argtable3/argtable3.c \
	gen/pc2sms.pb.cc \
	pc2sms-config.cpp \
	$(common_src)
pc2sms_LDADD = $(commonlibs)
pc2sms_CPPFLAGS = $(commoncppflags)

send_sms_SOURCES= \
	send-sms.cpp errlist.cpp \
	third-party/argtable3/argtable3.c config-filename.cpp \
	utilstring.cpp file-helper.cpp gen/pc2sms.grpc.pb.cc gen/pc2sms.pb.cc \
	send-sms-config.cpp \
	$(common_src)
send_sms_LDADD = $(commonlibs) -lgrpc $(LIB_COLLARIE)
send_sms_CPPFLAGS = $(commoncppflags) $(INC_COLLARIE)

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
  README.md HISTORY \
  $(proto)
