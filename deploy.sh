#!/bin/sh
#
#
HOST=167.172.99.203
PROGS=strip pc2sms send-sms
strip $PROGS
scp -r PROGS root@$HOST:/home/andrei/src/pc2sms/
