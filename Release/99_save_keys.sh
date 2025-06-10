#!/bin/sh

# Check if running as root, if not re-run with sudo
if [ "$(id -u)" -ne 0 ]; then
  echo "This script must be run as root. Trying to re-run with sudo..."
  exit
fi

sudo tar -C /etc/ssl -czf - dump1090 | openssl enc -aes-256-cbc -salt  -pbkdf2 -out dump1090-keys.enc.tar.gz
