# 🌍 ROGUEDUCK - Global Cloud Tunneling

By default, the ROGUEDUCK Web Command Center is only accessible if you are on the same local Wi-Fi network (or connected to the device's AP). 

To control the device from anywhere in the world (over mobile data or a completely different network), you can use a **Reverse Tunnel**. This companion script bridges the local M5StickS3 IP to a secure, public HTTPS link.

## Prerequisites
You need a "Companion Device" (a laptop, a Raspberry Pi, or a mini PC) connected to the same Wi-Fi network as the M5StickS3.

You also need one of the following command-line tools installed on the companion device:
* **Cloudflared (Recommended):** Free, no account required. ([Download](https://developers.cloudflare.com/cloudflare-one/connections/connect-networks/downloads/))
* **Ngrok:** Requires a free account and auth token. ([Download](https://ngrok.com/download))

## Using the Python Companion Script

The `rogueduck_tunnel.py` script automates the tunnel creation. It requires Python 3.

**Usage:**

```bash
python3 rogueduck_tunnel.py --ip <M5StickS3_IP> --tool <cloudflared|ngrok>
```

## The Step-by-Step Process

Here is exactly how you would deploy this in the real world using Cloudflare (which is free and doesn't require an account).

Step 1: The Local Connection

Turn on your mobile phone's Wi-Fi Hotspot.

Turn on your M5StickS3. Let it connect to your Hotspot.

Turn on your Laptop (the Companion Device) and connect it to the same mobile Hotspot.

Look at the M5StickS3's physical screen and write down the IP address it displays (e.g., 172.20.10.5).

Step 2: Install Cloudflared on the Laptop

On your laptop, download the Cloudflared command-line tool. (If you are on Windows, you download cloudflared-windows-amd64.exe and rename it to cloudflared.exe).

Make sure Python is installed on your laptop.

Put the rogueduck_tunnel.py script in the same folder as cloudflared.exe.

Step 3: Build the Tunnel

Open your laptop's Command Prompt or Terminal in that folder.

Run the script using the IP address from your M5StickS3 screen:

Bash
python rogueduck_tunnel.py --ip 172.20.10.5 --tool cloudflared
The script will output some text. Look for a line that contains a random URL like: https://rapid-pencil-network.trycloudflare.com.

Step 4: Global Control

You can now leave your laptop open in your car or backpack (as long as it stays connected to the hotspot).

Take your smartphone, turn OFF your Wi-Fi so you are strictly using your cellular 4G/5G data.

Open Safari or Chrome on your phone and go to the cloudflare link:  https://...trycloudflare.com link.

Boom. You are now looking at the RogueDuck Web UI from outside the local network. You can be on the other side of the planet, and hitting "INJECT" on your phone will travel through Cloudflare, down to your laptop, over the hotspot to the M5StickS3, and type the payload into whatever computer the M5StickS3 is plugged into.

