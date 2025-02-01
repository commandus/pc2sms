# Systemd service file

## Install

Edit path in the line 12 in the "install-pc2sms-service.sh" file:

```
ExecStart=/home/andrei/git/pc2sms/build/pc2sms -d --pidfile /var/run/pc2sms.pid
```

for instance:

```
ExecStart=/home/andrei/pc2sms/pc2sms -d --pidfile /var/run/pc2sms.pid
```

Copy file and start service:

```
sudo ./install-pc2sms-service.sh
sudo systemctl start pc2sms.service
systemctl daemon-reload
```

## Reference

[Writing systemd service files](https://patrakov.blogspot.com/2011/01/writing-systemd-service-files.html)
