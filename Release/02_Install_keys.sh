#!/bin/bash

# Check if running as root, if not re-run with sudo
if [ "$(id -u)" -ne 0 ]; then
  echo "This script must be run as root. Trying to re-run with sudo..."
  exit
fi

set -o pipefail

if openssl enc -d -aes-256-cbc -pbkdf2 -in dump1090-keys.enc.tar.gz | sudo tar xz -C /etc/ssl/ ; then
    :
else
    echo "openssl or tar has failed."
    exit
fi


chmod o-rx /etc/ssl/dump1090
echo "===== After installing key files"
echo "ls -la /etc/ssl/dump1090"
ls -la /etc/ssl/dump1090
echo "===== You must see like below from above this line ======================"
echo "===== Be carefule with the permission ==================================="
echo "-rw-r----- 1 root root 1424 Jun  2 15:04 lgess2025s4clientcert.pem"
echo "-rw------- 1 root root 1704 Jun  2 15:04 lgess2025s4clientkey.pem"
echo "-rw-r----- 1 root root 1419 Jun  2 15:04 lgess2025s4localhostcert.pem"
echo "-rw------- 1 root root 1704 Jun  2 15:04 lgess2025s4localhostkey.pem"
echo "-rw-r----- 1 root root 1428 Jun  2 15:04 lgess2025s4rpicert.pem"
echo "-rw------- 1 root root 1708 Jun  2 15:04 lgess2025s4rpikey.pem"
echo "-rw-r----- 1 root root   65 Jun  5 23:26 lgess2025s4rpilogkey.hex"
