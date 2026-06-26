#!/usr/bin/env python3
import argparse
import subprocess
import sys
import shutil

def check_installed(tool_name):
    if shutil.which(tool_name) is None:
        print(f"[!] Error: '{tool_name}' is not installed or not in your system PATH.")
        sys.exit(1)

def start_cloudflared(ip):
    print(f"[*] Starting Cloudflare Tunnel to ROGUEDUCK at {ip}:80...")
    print("[*] Look for the '[https://...trycloudflare.com](https://...trycloudflare.com)' link in the output below.\n")
    try:
        subprocess.run(["cloudflared", "tunnel", "--url", f"http://{ip}:80"])
    except KeyboardInterrupt:
        print("\n[*] Tunnel closed.")

def start_ngrok(ip):
    print(f"[*] Starting Ngrok Tunnel to ROGUEDUCK at {ip}:80...\n")
    try:
        subprocess.run(["ngrok", "http", f"{ip}:80"])
    except KeyboardInterrupt:
        print("\n[*] Tunnel closed.")

def main():
    print("""
 ██████╗  ██████╗  ██████╗ ██╗   ██╗███████╗██████╗ ██╗   ██╗ ██████╗██╗  ██╗
 ██╔══██╗██╔═══██╗██╔════╝ ██║   ██║██╔════╝██╔══██╗██║   ██║██╔════╝██║ ██╔╝
 ██████╔╝██║   ██║██║  ███╗██║   ██║█████╗  ██║  ██║██║   ██║██║     █████╔╝ 
 ██╔══██╗██║   ██║██║   ██║██║   ██║██╔══╝  ██║  ██║██║   ██║██║     ██╔═██╗ 
 ██║  ██║╚██████╔╝╚██████╔╝╚██████╔╝███████╗██████╔╝╚██████╔╝╚██████╗██║  ██╗
 ╚═╝  ╚═╝ ╚═════╝  ╚═════╝  ╚═════╝ ╚══════╝╚═════╝  ╚═════╝  ╚═════╝╚═╝  ╚═╝
    --- ROGUEDUCK TUNNEL COMPANION ---
    """)

    parser = argparse.ArgumentParser(description="Expose ROGUEDUCK Web UI to the internet.")
    parser.add_argument("-i", "--ip", required=True, help="The local IP address of the M5Stick (e.g., 192.168.1.55)")
    parser.add_argument("-t", "--tool", choices=["cloudflared", "ngrok"], default="cloudflared", help="The tunneling tool to use (default: cloudflared)")

    args = parser.parse_args()

    check_installed(args.tool)

    if args.tool == "cloudflared":
        start_cloudflared(args.ip)
    elif args.tool == "ngrok":
        start_ngrok(args.ip)

if __name__ == "__main__":
    main()