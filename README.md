```markdown
# 🦆 ROGUEDUCK // Stealth HID Injector

[![Hardware](https://img.shields.io/badge/Hardware-M5Stack_Stick_S3-orange?style=flat)](https://docs.m5stack.com/en/core/Sticks3) [![Framework](https://img.shields.io/badge/Framework-Arduino_Core-blue?style=flat)](https://www.arduino.cc/) [![Community](https://img.shields.io/badge/Community-Ethical_Hackers_Den-green?style=flat)](#community)

**RogueDuck** is a standalone, covert DuckyScript payload injector disguised as a pocket-sized cyberpunk terminal. 

Built for the **M5Stack Stick S3**, this firmware bypasses the need to carry a laptop for payload
management. It generates a local Wi-Fi uplink, allowing you to deploy, edit, and purge `.txt` payload
vectors directly from your smartphone's browser. Once loaded, execution is handled entirely offline
via a hardware-rendered, retro-CRT graphical interface.

---

## ⚡ System Features

* **Netrunner Terminal UI:** A custom hardware-level TFT display featuring CRT scanlines, Matrix-green typography, and randomized screen-tear/glitch effects during execution.
* **Wireless Uplink (Web UI):** Serves a captive web portal over a local AP to upload, paste, or delete payloads.
* **LittleFS Storage:** Payloads are safely compiled and stored in the device's onboard flash memory.
* **Plug & Play Injection:** Emulates a standard USB Keyboard to the target host. No drivers required.
* **DuckyScript 1.0 Parser:** Natively understands classic Rubber Ducky syntax (delays, strings, chords, and modifier keys).

---

## 🛠️ Hardware Requirements

* 1x **M5Stack Stick S3** (or StickC Plus2)
* 1x USB-C Data Cable

---

## 🚀 Installation & Flashing

### Dependencies
Ensure you have the ESP32 board manager installed in your Arduino IDE or PlatformIO environment. You will need the following libraries:
* `M5Unified` (by M5Stack)
* `LittleFS` (built-in)
* `WiFi` & `WebServer` (built-in)
* `USB` & `USBHIDKeyboard` (built-in for ESP32-S3)

### Setup Instructions
1. Clone this repository:
   ```bash
   git clone [https://github.com/yourusername/RogueDuck.git](https://github.com/yourusername/RogueDuck.git)

```

2. Open the `RogueDuck.ino` file in your Arduino IDE.
3. In the `Tools` menu, configure your board parameters:
* **Board:** ESP32S3 Dev Module (or exact M5Stack Stick S3 equivalent)
* **USB Mode:** Hardware CDC and JTAG
* **USB CDC On Boot:** Enabled


4. Compile and upload to your device.

*(Note: A pre-compiled binary is also available on the M5Burner marketplace.)*

---

## 📖 Deployment Guide

### 1. Establish the Uplink (Payload Management)

Provide power to the Stick S3. The screen will initialize the `[ ROGUEDUCK ]` boot sequence.

* Search for local Wi-Fi networks on your smartphone or secondary device.
* Connect to **`RogueDuck_Sync`** (Default Password: `12345678`).
* Open a browser and navigate to the gateway: `http://192.168.4.1`.
* Use the web portal to upload `.txt` scripts, paste raw code, or purge old vectors.

### 2. Execution Phase

Plug the Stick S3 into your target hardware.

* Use **`BTN_B`** (Side Button) to scroll through your stored `.txt` files.
* Press **`BTN_A`** (Main Front Button) to execute.
* The screen will glitch red, lock the terminal, and instantly inject the keystrokes.

---
## ⚙️ Payload Generation

Don't want to write your payloads by hand? We built a web-based tool to make vector creation effortless. 

Use the official **[DuckyScript Generator](https://www.ethicalhackersden.org/p/ducky-script-generator.html)** to quickly build, format, and copy your payloads. Once generated, simply connect to the RogueDuck Uplink and paste the script directly into the hardware buffer.

## ⌨️ Scripting Syntax

RogueDuck uses standard DuckyScript 1.0. Create simple text files (`.txt`) using the following syntax:

```text
REM Example Stealth Payload
DEFAULT_DELAY 50
GUI r
DELAY 500
STRING cmd
ENTER
DELAY 500
STRING echo "We are in."
ENTER

```

*Supported Modifiers:* `GUI`, `WINDOWS`, `CTRL`, `SHIFT`, `ALT`, `ENTER`, `TAB`, `ESC`, `BACKSPACE`, `UP`, `DOWN`, `LEFT`, `RIGHT`, `F1-F12`, and standard repeating chords (e.g., `CTRL-ALT-DEL`).

---

## 🌐 Community & Tutorials

RogueDuck was developed as part of the **Ethical Hackers Den**.

We are systematically building an online platform dedicated to hardware hacking, embedded electronics, and ethical security research. For deep-dive technical articles on ESP32 microcontrollers, M5Stack deployments, and more cybersecurity projects, join the community:

* **Blog & Tutorials:** [[blog link here](https://www.ethicalhackersden.org/)]
* **Discord Community:** [[Discord invite link here](https://discord.gg/AcAWqND87G)]

---

## ⚠️ Disclaimer

**Educational Use Only.** RogueDuck is designed strictly for system administration, authorized penetration testing, and educational purposes. Never deploy payloads on hardware or networks you do not own or do not have explicit, written permission to audit. The developers and the Ethical Hackers Den assume no liability for misuse.

```

```
