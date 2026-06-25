

# 🦆 ROGUEDUCK V2
**Advanced HID Payload Injector for M5Stack StickS3**

```text
 ██████╗  ██████╗  ██████╗ ██╗   ██╗███████╗██████╗ ██╗   ██╗ ██████╗██╗  ██╗
 ██╔══██╗██╔═══██╗██╔════╝ ██║   ██║██╔════╝██╔══██╗██║   ██║██╔════╝██║ ██╔╝
 ██████╔╝██║   ██║██║  ███╗██║   ██║█████╗  ██║  ██║██║   ██║██║     █████╔╝ 
 ██╔══██╗██║   ██║██║   ██║██║   ██║██╔══╝  ██║  ██║██║   ██║██║     ██╔═██╗ 
 ██║  ██║╚██████╔╝╚██████╔╝╚██████╔╝███████╗██████╔╝╚██████╔╝╚██████╗██║  ██╗
 ╚═╝  ╚═╝ ╚═════╝  ╚═════╝  ╚═════╝ ╚══════╝╚═════╝  ╚═════╝  ╚═════╝╚═╝  ╚═╝

```

ROGUEDUCK V2 is a custom, tactical firmware that transforms the M5Stack StickS3 into a covert, dual-network BadUSB tool. It features a complete mobile-first web interface, on-the-fly DuckyScript parsing, cloud-payload fetching, and a cyberpunk CRT hardware UI.

---

## 🚀 Key Features

### 🌐 Dual-Mode Networking (AP + STA)

* **Station Mode (STA):** Automatically attempts to connect to a predefined mobile hotspot or local network for internet-enabled attacks.
* **Access Point (AP) Fallback:** Always broadcasts its own isolated `RogueDuck_Sync` network so you are never locked out of the command center.

### 📱 Mobile-First Web Command Center

A stealthy, dark-mode web application optimized for fat-thumb operation on smartphones.

* **Over-The-Air Uploads:** Push `.txt` DuckyScript payloads directly to internal storage (LittleFS).
* **Live Editor:** Write, edit, and save payloads directly from your mobile browser.
* **Cloud Vector Deployment:** Paste a raw URL (e.g., GitHub Raw, Pastebin) to fetch and execute a payload entirely in RAM, leaving no trace on the device's physical storage.
* **Remote Execution:** Trigger any stored payload instantly from the web app.
* **Panic Wipe:** A dedicated killswitch at the bottom of the UI that formats the LittleFS partition, permanently wiping all stored payloads in seconds.

### 📟 Hardware Interface

* **Cyberpunk CRT Aesthetic:** Simulated CRT scanlines, dynamic glitch effects during injection, and dynamic IP rendering.
* **Pocket Lock & Battery Saver:** Long-press the side button to dim the screen and disable the physical injection buttons, preventing accidental misfires in your pocket.
* **Live Telemetry:** Real-time battery percentage monitoring directly on the header.

---

## 🛠️ Hardware Requirements

* **Device:** M5Stack StickS3 (ESP32-S3)
* **Connection:** USB-C (For flashing and HID emulation)

---

## ⚙️ Installation & Flashing

### 1. Dependencies

Ensure you have the following libraries installed in the Arduino IDE:

* `M5Unified`
* `LittleFS`
* `USB`
* `USBHIDKeyboard`
* `HTTPClient`
* `WebServer`

### 2. Configuration

Before compiling, open `ROGUEDUCK.cpp` and update the Wi-Fi credentials at the top of the script:

```cpp
const char* AP_SSID = "RogueDuck_Sync";
const char* AP_PASS = "12345678"; 

const char* STA_SSID = "YOUR_HOTSPOT_NAME";
const char* STA_PASS = "YOUR_HOTSPOT_PASSWORD";

```

### 3. CRITICAL: Partition Scheme

Because this firmware utilizes heavy web assets, dual Wi-Fi, and HTTP clients, it exceeds the default 1.2MB ESP32 application limit.

* In Arduino IDE, navigate to **Tools > Partition Scheme**.
* Select **Huge APP (3MB No OTA/1MB SPIFFS)** or **No OTA (2MB APP/2MB SPIFFS)**.
* Compile and Upload.

---

## 🕹️ Operation Guide

### Physical Controls

* **Button A (Front):** `INJECT PAYLOAD` - Executes the currently selected payload.
* **Button B (Side Short-Click):** `SCROLL` - Cycles through stored payloads.
* **Button B (Side Long-Press):** `POCKET LOCK` - Toggles the battery-saver dim mode and locks the physical execution buttons.

### Web Controls

1. Connect your smartphone/PC to either the `RogueDuck_Sync` Wi-Fi network OR the shared Hotspot network.
2. Check the M5Stack's physical screen for the assigned **IP Address**.
3. Enter the IP into your web browser.
4. Use the GUI to upload, edit, fire, or wipe payloads.

---

## 📜 Supported DuckyScript 1.0 Syntax

The internal parser processes standard US-English layout DuckyScript 1.0 commands:

* `STRING` / `STRINGLN`
* `DELAY` / `DEFAULT_DELAY`
* `GUI` / `WINDOWS` / `COMMAND`
* `CTRL`, `SHIFT`, `ALT` (and combos like `CTRL-ALT-DELETE`)
* `ENTER`, `TAB`, `SPACE`, `ESC`, `UP`, `DOWN`, `LEFT`, `RIGHT`
* `F1` - `F12`
* `REM` (Comments)
* `REPEAT`

---

## 🛡️ Credits & Disclaimer

**ROGUEDUCK V2 by [@M5RogueOps**](https://github.com/M5RogueOps) **Powered by [Ethical Hackers Den**](https://www.ethicalhackersden.org)

*This tool is designed for educational purposes, authorized penetration testing, and personal research. The developers assume no liability and are not responsible for any misuse or damage caused by this firmware. Only operate on networks and devices you have explicit permission to test.*

```

```
[![Hardware](https://img.shields.io/badge/Hardware-M5Stack_Stick_S3-orange)](https://docs.m5stack.com/en/core/Sticks3)
[![Framework](https://img.shields.io/badge/Framework-Arduino_Core-blue)](https://www.arduino.cc/)
[![Community](https://img.shields.io/badge/Community-Ethical_Hackers_Den-green)](#community)
