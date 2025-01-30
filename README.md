# pc2sms

The pc2sms utility sends SMS from Linux and Windows programs using Android client.

First you need to install the Android application on your phone.

Android application is waiting for commands to send sms from the web service.

Commands for sending sms are sent from the web service one by one to the Android application.

Client utility or library sends a request to send SMS via the service.

```
+---------------------+
| send-sms / your app |
+---------------------+
          |
+---------------------+
| pc2sms web service  |
+---------------------+
        |
        |-----------------+-----------------+
        |                 |                 |
+--------------+   +--------------+       . . .
|    Android   |   |    Android   |
| +18001112222 |   | +18001113333 |
+--------------+   +--------------+
```

Phones are selected in turn one by one.


[Android client](https://github.com/commandus/pc2sms-android)


## Build

### Prerequisites

Install gRPC library, gRPC tools and dependencies:

```
sudo apt install libgrpc++-dev libprotobuf-dev protobuf-compiler-grpc protobuf-c-compiler
```

### gRPC versions in Ubuntu's repositories

```
protoc --version
```

| Ubuntu version     | protoc --version |
|--------------------|------------------|
| Ubuntu 18.04       | libprotoc 3.0.0  |
| Ubuntu 22.04.4 LTS | libprotoc 3.12.4 |

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

### Cmake

#### Windows 
Windows must have vcpkg installed, for instance in the C:\git\vcpkg\ directory.

Then install gRPC and Protobuf using vcpkg.

Provide variables

- CMAKE_TOOLCHAIN_FILE
- VCPKG_TARGET_TRIPLET (optionally)

to CMake:

```
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=C:\git\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows ..
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
2. login
3. password
4. robin-bobin | all

Login/password intened for send-sms authorization

Example:

```
0.0.0.0:50053
scott
tiger
robin-bobin
```

- robin-bobin send SMS from one of phones
- all send SMS from all phones

## Run server

Put config file into home directory.

Run with highest verbosity level:

```
./pc2sms -vvv
```

Run as daemon:

```
./pc2sms -d
```

## send-sms client

Options

-s service address:port
-u login
-p password]
-m message (optional)

If -m omitted, read message from stdin.

-s, -u, -p also optional.

If config file exists in the home directory and not empty, cleint read values from config file.

```
./send-sms -s 127.0.0.1:50053 -u root -p 123 -m "Message"
send-sms -s acme.org:50053 "1-800-ORG-ACME" < message.txt 
send-sms -s 127.0.0.1:50053 -u root -p 123 -m "Message in command line" 18001112222 -vvv
```

## Config file

$HOME/.pc2sms

Lines:

1. service address
2. login
3. password

Example:

```
127.0.0.1:50053
scott
tiger
```

./send-sms -m 111 18001112222 -vvv

## Windows

Create service to auto-start

```
sudo sc create pc2sms binPath="C:\git\pc2sms\cmake-build-debug-visual-studio\pc2sms.exe -d" start= manual DisplayName= "Serving gRPC clients for sending SMS messages through connected mobile phones"
```

Start service manually

```
sudo sc start pc2sms 
```

Check service status

```
sc query pc2sms
...
STATE : 4 RUNNING
... 
```
