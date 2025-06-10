#!/bin/sh

# Check if running as root, if not re-run with sudo
if [ "$(id -u)" -ne 0 ]; then
  echo "This script must be run as root. Trying to re-run with sudo..."
  exit
fi

cp -p adsbhub.service /etc/systemd/system/
cp -p dump1090.service /etc/systemd/system/

# Copy service unit files to systemd directory
cp -p adsbhub.service /etc/systemd/system/
cp -p dump1090.service /etc/systemd/system/

# Reload systemd manager configuration
systemctl daemon-reload

# Enable services to start on boot
systemctl enable dump1090
systemctl enable adsbhub

# Start services immediately
systemctl start dump1090
systemctl start adsbhub

# Show service status (omit pager output)
systemctl status dump1090 --no-pager
systemctl status adsbhub --no-pager
