#!/bin/sh
# Create PID file
touch /var/run/pc2sms.pid
# Copy systemd service file
cp pc2sms.service /etc/systemd/system/

systemctl start pc2sms.service
systemctl daemon-reload

exit 0
