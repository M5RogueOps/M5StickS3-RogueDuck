/*
 *  ██████╗  ██████╗  ██████╗ ██╗   ██╗███████╗██████╗ ██╗   ██╗ ██████╗██╗  ██╗
 *  ██╔══██╗██╔═══██╗██╔════╝ ██║   ██║██╔════╝██╔══██╗██║   ██║██╔════╝██║ ██╔╝
 *  ██████╔╝██║   ██║██║  ███╗██║   ██║█████╗  ██║  ██║██║   ██║██║     █████╔╝ 
 *  ██╔══██╗██║   ██║██║   ██║██║   ██║██╔══╝  ██║  ██║██║   ██║██║     ██╔═██╗ 
 *  ██║  ██║╚██████╔╝╚██████╔╝╚██████╔╝███████╗██████╔╝╚██████╔╝╚██████╗██║  ██╗
 *  ╚═╝  ╚═╝ ╚═════╝  ╚═════╝  ╚═════╝ ╚══════╝╚═════╝  ╚═════╝  ╚═════╝╚═╝  ╚═╝
 *
 *  =============================================================================
 *  PROJECT   : M5Stack StickC S3 - RogueDuck HID Injector (Cyberpunk CRT Edition)
 *  AUTHOR    : @M5RogueOps
 *  COMMUNITY : https://www.ethicalhackersden.org
 *  =============================================================================
 *  
 *  FEATURES:
 *  - Upload .txt DuckyScript payloads via Wi-Fi AP web UI (file or paste)
 *  - List & delete stored payloads from the web UI
 *  - Select payload with BtnB, execute with BtnA
 *  - Parses Classic DuckyScript 1.0 (US English layout)
 *  - Custom hardware-level CRT scanline UI
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

// --- Cyberpunk CRT Colors (RGB565 format) ---
#define CYBER_GREEN M5.Display.color565(0, 255, 65)
#define CYBER_DARK  M5.Display.color565(0, 40, 10)
#define CYBER_RED   M5.Display.color565(255, 0, 60)

// --- Configuration ---
const char* AP_SSID = "RogueDuck_Sync";
const char* AP_PASS = "12345678"; // Must be at least 8 chars

WebServer server(80);
USBHIDKeyboard Keyboard;
File fsUploadFile;

// --- State ---
std::vector<String> payloadFiles;
int selectedFileIndex = 0;
bool isSending = false;

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

// --- Web App HTML UI (Original unchanged) ---
String buildIndexHtml() {
    String html;
    html.reserve(4096);
    html += F(
        "<!DOCTYPE html><html><head>"
        "<title>Payload Uploader</title>"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<style>"
        "*{box-sizing:border-box}"
        "body{font-family:Arial,sans-serif;margin:16px;background:#222;color:#fff;}"
        "h2{text-align:center;margin-top:0;}"
        ".container{background:#333;padding:18px;border-radius:10px;margin:0 auto 18px;max-width:560px;width:100%;}"
        "input[type=file],input[type=text]{padding:8px;font-size:14px;border-radius:4px;border:1px solid #555;background:#111;color:#fff;width:100%;}"
        "input[type=submit],button.btn{background:#4CAF50;color:#fff;padding:12px 22px;border:none;border-radius:4px;cursor:pointer;font-size:15px;}"
        "input[type=submit]:hover,button.btn:hover{background:#45a049;}"
        "button.del{background:#c0392b;color:#fff;border:none;border-radius:4px;padding:6px 12px;cursor:pointer;font-size:13px;}"
        "button.del:hover{background:#e74c3c;}"
        "textarea{width:100%;height:220px;background:#111;color:#0f0;border:1px solid #555;border-radius:4px;padding:10px;"
        "font-family:'Courier New',monospace;font-size:13px;resize:vertical;}"
        ".row{display:flex;gap:10px;align-items:center;margin-bottom:10px;flex-wrap:wrap;}"
        ".row label{min-width:80px;}"
        ".row input[type=text]{flex:1 1 160px;}"
        ".center{text-align:center;}"
        ".file-row{display:flex;justify-content:space-between;align-items:center;background:#2a2a2a;padding:8px 12px;border-radius:6px;margin-bottom:6px;gap:10px;}"
        ".file-row span{word-break:break-all;}"
        ".empty{opacity:.6;text-align:center;padding:8px;}"
        "@media (max-width:480px){"
        "  body{margin:8px;}"
        "  .container{padding:14px;}"
        "  textarea{height:180px;font-size:14px;}"
        "  input[type=submit],button.btn{width:100%;}"
        "  .row label{min-width:auto;width:100%;}"
        "}"
        "</style></head><body>"

        "<div class=\"container\">"
        "<h2>Upload .txt Payload</h2>"
        "<form method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\" class=\"center\">"
        "<input type=\"file\" name=\"file\" accept=\".txt\"><br><br>"
        "<input type=\"submit\" value=\"Upload to Device\">"
        "</form></div>"

        "<div class=\"container\">"
        "<h2>Paste DuckyScript</h2>"
        "<form method=\"POST\" action=\"/save\">"
        "<div class=\"row\"><label for=\"filename\">Filename:</label>"
        "<input type=\"text\" id=\"filename\" name=\"filename\" value=\"payload.txt\" required></div>"
        "<textarea name=\"content\" placeholder=\"REM My payload&#10;DELAY 1000&#10;GUI r&#10;DELAY 500&#10;STRINGLN notepad&#10;STRING Hello from M5!\"></textarea>"
        "<div class=\"center\" style=\"margin-top:12px;\"><input type=\"submit\" value=\"Save to Device\"></div>"
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
            html += F("</span>"
                      "<form method=\"POST\" action=\"/delete\" "
                      "onsubmit=\"return confirm('Delete this file?');\" style=\"margin:0;\">"
                      "<input type=\"hidden\" name=\"filename\" value=\"");
            html += f;
            html += F("\"><button class=\"del\" type=\"submit\">Delete</button></form></div>");
        }
    }

    html += F("</div></body></html>");
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
        int glitchX = random(-3, 4);
        int glitchY = random(-2, 3);
        
        M5.Display.setTextColor(CYBER_RED); 
        M5.Display.setTextSize(2);
        M5.Display.setCursor(10 + glitchX, h / 2 - 15 + glitchY);
        M5.Display.print("> INJECT");
        
        M5.Display.setTextSize(1);
        M5.Display.setCursor(10, h / 2 + 10);
        M5.Display.setTextColor(CYBER_GREEN);
        if (!payloadFiles.empty()) {
            M5.Display.print(payloadFiles[selectedFileIndex]);
        }
        
        M5.Display.fillRect(random(0, w), random(0, h), random(10, 60), random(2, 6), CYBER_GREEN);
        return;
    }

    // --- NORMAL TERMINAL UI ---
    M5.Display.setTextSize(1); 

    // Header
    M5.Display.setTextColor(CYBER_GREEN);
    M5.Display.setCursor(2, 2);
    M5.Display.print("[ ROGUEDUCK ]");
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(90, 2);
    M5.Display.print("192.168.4.1");
    
    // Top Divider
    M5.Display.drawFastHLine(0, 12, w, CYBER_GREEN);

    // Payload List
    M5.Display.setCursor(2, 16);
    if (payloadFiles.empty()) {
        M5.Display.setTextColor(CYBER_RED);
        M5.Display.setCursor(2, 30);
        M5.Display.println("NO VECTORS");
        M5.Display.setCursor(2, 42);
        M5.Display.println("AWAITING UPLINK");
    } else {
        int maxItems = 3;
        int startIdx = max(0, selectedFileIndex - 1);
        if (startIdx + maxItems > (int)payloadFiles.size()) {
            startIdx = max(0, (int)payloadFiles.size() - maxItems);
        }

        int startY = 16;
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
    M5.Display.print("BTN_A:EXEC   BTN_B:NXT");
}

// ---------------------------------------------------------------
// Web server handlers
// ---------------------------------------------------------------
void handleRoot() {
    server.send(200, "text/html", buildIndexHtml());
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
// DuckyScript 1.0 Parser
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
void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    // Set to landscape rotation for the terminal UI
    M5.Display.setRotation(3);
    M5.Display.setTextSize(1);

    Serial.begin(115200);

    if (!LittleFS.begin(true)) {
        M5.Display.println("LittleFS Mount Failed!");
        return;
    }
    refreshFileList();

    WiFi.softAP(AP_SSID, AP_PASS);

    server.on("/", HTTP_GET, handleRoot);
    server.on("/upload", HTTP_POST, []() {
        server.send(200, "text/html",
                    "<h2>Upload Complete!</h2><br><a href='/'>Go Back</a>");
    }, handleUpload);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/delete", HTTP_POST, handleDelete);
    server.begin();

    Keyboard.begin();
    USB.begin();

    drawUI();
}

void loop() {
    server.handleClient();
    M5.update();

    if (M5.BtnB.wasPressed() && !payloadFiles.empty()) {
        selectedFileIndex++;
        if (selectedFileIndex >= (int)payloadFiles.size()) selectedFileIndex = 0;
        drawUI();
    }

    if (M5.BtnA.wasPressed() && !payloadFiles.empty() && !isSending) {
        sendSelectedPayload();
    }

    delay(2);
}
