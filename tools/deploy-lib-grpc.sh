#!/bin/sh
#
# 157.230.125.14
# 167.172.99.203
#
HOST=157.230.125.14
scp \
/usr/local/lib/libgpr.so.8 \
/usr/local/lib/libgrpc.so.8 \
/usr/local/lib/libcares.so.2 \
root@$HOST:/usr/local/lib/
