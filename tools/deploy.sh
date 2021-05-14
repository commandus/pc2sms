#!/bin/sh
# 157.230.125.14
# 167.172.99.203
#
HOST=157.230.125.14
strip ./send-sms ./pc2sms
ssh root@$HOST mkdir /home/andrei/src/pc2sms/
scp ./send-sms ./pc2sms root@$HOST:/home/andrei/src/pc2sms/
scp ~/.pc2sms root@$HOST:/home/andrei/src/pc2sms/
scp ~/.pc2sms root@$HOST:/home/andrei/
scp ~/.pc2sms root@$HOST:/root/
ssh root@$HOST chown -R andrei:andrei /home/andrei/src/pc2sms/
