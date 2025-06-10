#!/bin/sh

echo WARNING!!!!
echo UFW FIREWALL SETTINGS ARE VERY SENSITIVE. PLEASE MAKE SURE YOU HAVE
echo A GOOD UNDERSTANDING OF UFW BEFORE MODIFYING OR RUNNING THIS SCRIPT.
read -p "Do you want to continue? (y/n): " answer
if [ "$answer" != "y" ] && [ "$answer" != "Y" ]; then
  echo "Aborted."
  exit 1
fi

# Check if running as root, if not re-run with sudo
if [ "$(id -u)" -ne 0 ]; then
  echo "This script must be run as root. Trying to re-run with sudo..."
  exit
fi

# Install ufw if it is not already installed
if ! command -v ufw > /dev/null 2>&1; then
    echo "ufw not found. Installing..."
    sudo apt update
    sudo apt install -y ufw
fi

set -x
# === Strongly recommended ===
# Reset all existing ufw rules to avoid conflicts with previous configurations.
# This will remove all existing rules and return ufw to its default state.
ufw --force reset

# Set default policies: deny all incoming, allow all outgoing traffic
ufw default deny incoming
ufw default allow outgoing

# Allow DHCP client to receive an IP address (UDP port 68 from port 67)
ufw allow proto udp from any port 67 to any port 68

# Allow SSH from IP(s)
ufw allow from 192.168.43.238 to any port 22 proto tcp
ufw allow from 192.168.43.91 to any port 22 proto tcp
ufw allow from 192.168.43.69 to any port 22 proto tcp

# Deny SSH from all other sources
ufw deny 22

# Whitelisting individual IP addresses separately
sudo ufw allow from 192.168.43.238
sudo ufw allow from 192.168.43.69
sudo ufw allow from 192.168.43.91

# === Optional Examples (CIDR block-based allow rules) ===
# Allow IP range: 192.168.43.1 to 192.168.43.126 (subnet /25)
# ufw allow from 192.168.43.0/25

# Allow IP range: 192.168.43.128 to 192.168.43.191 (subnet /26)
# ufw allow from 192.168.43.128/26

ufw show added

read -p "Do you want to enable ufw (y/n): " answer
if [ "$answer" != "y" ] && [ "$answer" != "Y" ]; then
  echo "Aborted."
  exit 1
fi
# Enable ufw (firewall) with confirmation suppressed
ufw --force enable

# Show current ufw rules
echo ""
echo "✅ Current ufw rules:"
ufw status numbered

