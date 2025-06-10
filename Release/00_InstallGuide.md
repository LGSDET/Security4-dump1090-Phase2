# Security 4 Edition - Flight Tracker Installation Guide

This document provides a step-by-step guide for setting up the **dump1090** environment, the core binary for Flight Tracker, enhanced by the **Security 4 Team** for improved security.

> ✅ **Note:**  
> The script `/usr/bin/adsbhub.sh` must be installed for dump1090 to function properly. Please refer to the official setup guide on [www.adsbhub.org](https://www.adsbhub.org) for details.

## Prerequisites

Before starting the installation:

- Security-enhanced `dump1090` requires TLS certificates and encryption keys for secure communication and log protection.
- Firewall rules will be configured using `ufw` (Uncomplicated Firewall).
- To simplify operation, both `dump1090` and `adsbhub.sh` will be registered as `systemd` services to enable automatic startup after system reboot.

---

## Installation Steps

### Step 1 - Install Binaries

Run the script:

```bash
sudo ./01_Install_bins.sh
```

- Installs two binaries (`dump1090`, `sqlog_viewer`) to `/home/lg/dump1090/`.
- **Do not change the installation path**, as hardcoded paths are used and may cause malfunction.
- Root privileges may be required.

### Step 2 - Install TLS and Log Keys

Run the script:

```bash
sudo ./02_Install_keys.sh
```

- Decrypts and extracts security keys into `/etc/ssl/dump1090/`.
- Keys are encrypted with a password. To obtain the password, please contact:
  - **Jaehoon Lee** (jhoon3@andrew.cmu.edu)
  - **Hyundo Park** (hyundop@andrew.cmu.edu)
- This step only needs to be done **once**, unless updated keys are provided in future releases.
- This step requires root privileges.

### Step 3 - Register systemd Services

Run the script:

```bash
sudo ./03_Install_systemd.sh
```

- Registers both `dump1090` and `adsbhub.sh` as systemd services.
- Ensures they start automatically upon system reboot.
- Must be run with root privileges.

### Step 4 - Configure Firewall

Run the script:

```bash
sudo ./04_Setup_firewall.sh
```

- Configures `ufw` firewall rules to allow or restrict network access.
- Default setup assumes a local IP range of `192.168.43.xx`.
- **Adjust the script according to your network environment** before running.

#### ⚠️ Important:

- Understand the basics of `ufw` before applying firewall rules.
- You may want to test that `dump1090` functions correctly **without firewall first**.
- This helps with debugging in case of network issues.

---

## Final Notes

We hope you enjoy using the **Security 4 Edition** of dump1090.

For questions or support, please contact:

**Author:** Jaehoon Lee  
**Email:** jaehoon3@andrew.cmu.edu