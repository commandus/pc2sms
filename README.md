# pc2sms

Send SMS using Android client.

Android client registers to the service and wait command to send sms.

Client utility send request to send SMS via service.

## Build

### Prerequisites

### gcc

Download protoc's plugin grpc-web from https://github.com/grpc/grpc-web/releases into /usr/local/bin/protoc-gen-grpc-web

```
./autogen.sh
./configure
make
sudo make install
```

### clang:

```
./configure CC=clang CXX=clang++
```

## Prerequisites

## Dependencies

- libglog.so.0
- libgflags.so.2.2
- libunwind.so.8

```
scp /usr/local/lib/libglog.so.0 root@www.itctrack.ru:/usr/local/lib/
```

## config file

~/.pc2sms

Lines:

1. service address
2. login (optional)
3. password (optional)
5. config file (optional)

Login/password intened for send-sms authorization

Example:

```
0.0.0.0:50053
scott
tiger
```

Run with highest verbosity level:

```
./pc2sms -vvv
```

Run as daemon:

```
./pc2sms -d
```

## send-sms client

Example:

```
send-sms "1-800-ORG-ACME" < message.txt 

```
