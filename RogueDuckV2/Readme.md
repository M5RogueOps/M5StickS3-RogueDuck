# 🦆 ROGUEDUCK V2
**Stealth HID Payload Injector for M5Stack StickS3**

ROGUEDUCK V2 transforms the M5Stack StickS3 into a covert, dual-network BadUSB tool featuring a mobile-optimized web command center, on-the-fly DuckyScript parsing, cloud-payload fetching, and a tactical cyberpunk CRT hardware UI.

## 🔥 V2 Upgrades & Features
* **Dual-Mode Networking (AP + STA):** Connects to your mobile hotspot for internet access while simultaneously broadcasting its own `RogueDuck_Sync` fallback network.
* **Mobile-First Web UI:** Dark-mode, anti-zoom CSS designed specifically for rapid, fat-thumb smartphone operation in the field.
* **Cloud Vector Deployment:** Fetch and execute payloads entirely in RAM from raw URLs (GitHub/Pastebin) leaving zero trace on the device.
* **Live Web Editor:** Write, edit, and save DuckyScript payloads directly from your mobile browser.
* **Pocket Lock & Battery Saver:** Long-press the side button to dim the screen and physically lock the injection buttons to prevent accidental misfires.
* **Cyberpunk CRT UI:** Simulated scanlines, dynamic glitch injection effects, real-time battery monitoring, and dynamic IP display.
* **Panic Wipe Killswitch:** Dedicated web UI button to instantly format the LittleFS partition and destroy all stored payloads.

## ⚙️ Quick Start
1. Update `STA_SSID` and `STA_PASS` in `RogueDuckV2.ino` with your mobile hotspot credentials.
2. Set your Arduino IDE Partition Scheme to **Huge APP (3MB No OTA)**.
3. Flash to your M5Stack StickS3.
4. Connect your phone to your hotspot (or the `RogueDuck_Sync` AP) and navigate to the IP address displayed on the M5Stack screen.

## 📝 DuckyScript Support
Executes standard US-English DuckyScript 1.0 commands line-by-line:
`STRING`, `STRINGLN`, `DELAY`, `DEFAULT_DELAY`, `REPEAT`, `REM`, and standard modifiers (`GUI`, `CTRL`, `SHIFT`, `ALT`, `ENTER`, `TAB`, `ESC`, etc.) including chorded combinations.

---

**ROGUEDUCK V2 BY [@M5RogueOps](https://github.com/M5RogueOps)** **POWERED BY [Ethical Hackers Den](https://www.ethicalhackersden.org)**
