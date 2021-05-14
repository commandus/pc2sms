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
