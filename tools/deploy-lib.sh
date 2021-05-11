#!/bin/sh
#
#
# HOST=157.230.125.14
HOST=167.172.99.203
scp -r \
/usr/local/lib/libabsl_absl_int128.so                /usr/local/lib/libabsl_absl_strings_internal.so  /usr/local/lib/libabsl_absl_throw_delegate.so  /usr/local/lib/libgrpc++.so.1  libupb.so.9 \
/usr/local/lib/libabsl_absl_raw_logging_internal.so  /usr/local/lib/libabsl_absl_strings.so           /usr/local/lib/libgpr.so.9                     /usr/local/lib/libgrpc.so.9 \
root@$HOST:/usr/local/lib/
