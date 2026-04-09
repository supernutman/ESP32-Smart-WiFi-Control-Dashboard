#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "LedControl.h"
#include <EEPROM.h>

#define EEPROM_SIZE 100
#define BTN_PIN 23   // ปุ่ม reset

// ===== LED =====
int led25 = 25; //G
int led26 = 26; //Y
int led27 = 27; //R

//=======EEPROM===========
String saved_ssid = "";
String saved_pass = "";

void saveWiFi(String ssid, String pass) {
  EEPROM.writeString(0, ssid);
  EEPROM.writeString(50, pass);
  EEPROM.commit();
}

void loadWiFi() {
  saved_ssid = EEPROM.readString(0);
  saved_pass = EEPROM.readString(50);
}

void clearWiFi() {
  EEPROM.writeString(0, "");
  EEPROM.writeString(50, "");
  EEPROM.commit();
}

//=======RESET_BUTTON==========


void checkButton() {
  if(digitalRead(BTN_PIN) == LOW) {
    delay(100);
    clearWiFi();
    Serial.println("RESETED WIFI");
    for(int i=0; i<7; i++){
    digitalWrite(led25, i%2);
    digitalWrite(led26, i%2);
    digitalWrite(led27, i%2);
    delay(250);
    }
  }
}


// ===== MAX7219 =====
LedControl lc = LedControl(15, 14, 13, 1); // DIN, CLK, CS

// ===== Web =====
WebServer server(80);

// ===== HTML =====
String htmlPage() {
  return R"rawliteral(
    <html lang="th">
  <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ESP32 WiFi Setup</title>
      <style>
          :root {
              --primary-color: #007bff;
              --bg-color: #f4f7f9;
              --card-bg: #ffffff;
              --text-color: #333;
          }

          body {
              font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
              background-color: var(--bg-color);
              color: var(--text-color);
              display: flex;
              justify-content: center;
              align-items: center;
              height: 100vh;
              margin: 0;
          }

          .container {
              background: var(--card-bg);
              padding: 2rem;
              border-radius: 15px;
              box-shadow: 0 10px 25px rgba(0,0,0,0.1);
              width: 100%;
              max-width: 350px;
              text-align: center;
          }

          h2 { margin-bottom: 1.5rem; color: var(--primary-color); }

          .form-group {
              text-align: left;
              margin-bottom: 1rem;
          }

          label { display: block; margin-bottom: 5px; font-weight: bold; font-size: 0.9rem; }

          select, input {
              width: 100%;
              padding: 10px;
              border: 1px solid #ddd;
              border-radius: 8px;
              box-sizing: border-box; /* สำคัญเพื่อให้ padding ไม่ทำให้อินพุตล้น */
              font-size: 1rem;
          }

          button {
              width: 100%;
              padding: 12px;
              border: none;
              border-radius: 8px;
              cursor: pointer;
              font-weight: bold;
              transition: 0.3s;
              margin-top: 10px;
          }

          .btn-scan { background-color: #6c757d; color: white; }
          .btn-scan:hover { background-color: #5a6268; }

          .btn-connect { background-color: var(--primary-color); color: white; }
          .btn-connect:hover { background-color: #0056b3; }

          #status {
              margin-top: 1rem;
              font-size: 0.9rem;
              font-weight: bold;
              color: #28a745;
          }

          /* ตกแต่งตอนกำลังโหลด */
          .loading { opacity: 0.5; pointer-events: none; }
      </style>
  </head>
  <body>

  <div class="container">
      <h2>ESP32 WiFi Setup</h2>
      
      <div class="form-group">
          <label>Available Networks</label>
          <select id="ssid">
              <option value="">Press Scan to find WiFi</option>
          </select>
          <button class="btn-scan" id="scanBtn" onclick="scan()">Scan WiFi</button>
      </div>

      <div class="form-group">
          <label>Password</label>
          <input type="password" id="pass" placeholder="Enter password">
      </div>

      <button class="btn-connect" id="connectBtn" onclick="connect()">Connect</button>

      <p id="status"></p>
  </div>

  <script>
      function scan() {
          const btn = document.getElementById("scanBtn");
          const status = document.getElementById("status");
          
          btn.innerText = "Scanning...";
          btn.classList.add("loading");
          status.innerText = "";

          fetch('/scan')
              .then(r => r.json())
              .then(data => {
                  let s = document.getElementById("ssid");
                  s.innerHTML = "";
                  if(data.length === 0) {
                      s.innerHTML = '<option value="">No WiFi found</option>';
                  } else {
                      data.forEach(w => {
                          let o = document.createElement("option");
                          o.value = w;
                          o.text = w;
                          s.appendChild(o);
                      });
                  }
              })
              .catch(err => status.innerText = "Error scanning!")
              .finally(() => {
                  btn.innerText = "Scan WiFi";
                  btn.classList.remove("loading");
              });
      }

      function connect() {
          const btn = document.getElementById("connectBtn");
          const ssid = document.getElementById("ssid").value;
          const pass = document.getElementById("pass").value;
          const status = document.getElementById("status");

          if(!ssid) { alert("Please select a WiFi!"); return; }

          btn.innerText = "Connecting...";
          btn.classList.add("loading");
          status.innerText = "Trying to connect...";

          fetch('/connect', {
              method: 'POST',
              headers: {'Content-Type': 'application/json'},
              body: JSON.stringify({ssid: ssid, password: pass})
          })
          .then(r => r.text())
          .then(t => {status
              .innerText = "Status: " + t;
          })
          .catch(err => status.innerText = "Failed to send request")
          .finally(() => {
              btn.innerText = "Connect";
              btn.classList.remove("loading");
          });
      }
  </script>

  </body>
  </html>
  )rawliteral";
}

// ===== Help page =====
void handleHelp() {
  String page = R"rawliteral(
 <!DOCTYPE html>
  <html lang="th">
  <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>ESP32 Dashboard</title>
      <style>
          :root {
              --primary: #007bff;
              --success: #28a745;
              --danger: #dc3545;
              --bg: #f4f7f9;
              --card: #ffffff;
          }

          body { 
              font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
              background-color: var(--bg);
              margin: 0;
              padding: 20px;
              display: flex;
              flex-direction: column;
              align-items: center;
          }

          .container {
              max-width: 450px;
              width: 100%;
              background: var(--card);
              padding: 20px;
              border-radius: 15px;
              box-shadow: 0 4px 15px rgba(0,0,0,0.1);
          }

          h2 { color: #333; margin: 0 0 20px 0; text-align: center; }
          h3 { font-size: 1rem; color: #666; border-bottom: 1px solid #eee; padding-bottom: 5px; margin-top: 20px; }

          /* Command Display Box */
          .url-box-container {
              background: #2d2d2d;
              border-radius: 8px;
              padding: 10px;
              margin-bottom: 15px;
              display: flex;
              align-items: center;
              gap: 10px;
          }

          #url-display {
              flex: 1;
              color: #00ff00;
              font-family: 'Courier New', Courier, monospace;
              font-size: 0.9rem;
              white-space: nowrap;
              overflow: hidden;
              text-overflow: ellipsis;
          }

          .btn-copy {
              background: #555;
              color: white;
              padding: 5px 10px;
              font-size: 0.75rem;
              border-radius: 4px;
          }
          .btn-copy:active { background: var(--primary); }

          /* LED Control Grid */
          .grid {
              display: grid;
              grid-template-columns: 1fr 1fr 1fr;
              gap: 10px;
              margin-bottom: 10px;
              align-items: center;
          }

          .label { font-weight: bold; }

          button {
              padding: 10px;
              border: none;
              border-radius: 8px;
              cursor: pointer;
              transition: 0.2s;
              font-weight: 600;
          }

          .btn-on { background: var(--success); color: white; }
          .btn-off { background: #e0e0e0; color: #333; }
          .btn-on:hover { opacity: 0.8; }

          /* Input Section */
          .input-group {
              display: flex;
              gap: 5px;
              margin-top: 10px;
          }

          input {
              flex: 1;
              padding: 12px;
              border: 2px solid #ddd;
              border-radius: 8px;
              outline: none;
              font-size: 16px;
          }

          .btn-action { background: var(--primary); color: white; padding: 12px 20px; }
          .btn-clear { background: var(--danger); color: white; }

          /* Result Area */
          .status-box {
              margin-top: 15px;
              padding: 10px;
              background: #f8f9fa;
              border-left: 4px solid var(--primary);
              font-family: monospace;
              font-size: 0.9rem;
          }
      </style>
  </head>

  <body>

      <div class="container">
          <h2>🚀 ESP32 Control</h2>

          <h3>🔗 Last Command Executed</h3>
          <div class="url-box-container">
              <span id="url-display">http://[IP_ADDRESS]/...</span>
              <button class="btn-copy" onclick="copyURL()">Copy</button>
          </div>

          <h3>💡 LED Status</h3>
          <div class="grid">
              <span class="label">Pin 25</span>
              <button class="btn-on" onclick="send('/G/on')">ON</button>
              <button class="btn-off" onclick="send('/G/off')">OFF</button>

              <span class="label">Pin 26</span>
              <button class="btn-on" onclick="send('/Y/on')">ON</button>
              <button class="btn-off" onclick="send('/Y/off')">OFF</button>

              <span class="label">Pin 27</span>
              <button class="btn-on" onclick="send('/R/on')">ON</button>
              <button class="btn-off" onclick="send('/R/off')">OFF</button>
          </div>

          <h3>🔢 MAX7219 Display</h3>
          <div class="input-group">
              <input id="num" type="text" placeholder="1234 หรือ 12.34">
          </div>
          <div class="input-group">
              <button class="btn-action" style="flex:2" onclick="sendNum()">SHOW</button>
              <button class="btn-clear" style="flex:1" onclick="send('/c_clear')">CLEAR</button>
          </div>

          <div class="status-box" id="res">Device Ready</div>
      </div>

      <script>
          // ฟังก์ชันหลักในการส่งคำสั่ง
          async function send(path) {
              const fullURL = window.location.origin + path;
              
              // อัปเดตช่อง Preview ด้านบน
              document.getElementById("url-display").innerText = fullURL;
              
              const resDisplay = document.getElementById("res");
              resDisplay.style.color = "#666";
              resDisplay.innerText = "Sending...";

              try {
                  const response = await fetch(path);
                  const text = await response.text();
                  resDisplay.style.color = "black";
                  resDisplay.innerText = "Response: " + text;
              } catch (err) {
                  resDisplay.style.color = "red";
                  resDisplay.innerText = "Error: Connection failed";
              }
          }

          // ฟังก์ชันส่งตัวเลข
          function sendNum() {
              let val = document.getElementById("num").value;
              if (!val) return alert("กรุณากรอกตัวเลข");
              send('/c_' + encodeURIComponent(val));
          }

          // ฟังก์ชัน Copy URL
          function copyURL() {
              const urlText = document.getElementById("url-display").innerText;
              if (urlText.includes("...")) return; // ถ้ายังไม่มี URL จริง

              navigator.clipboard.writeText(urlText).then(() => {
                  const copyBtn = document.querySelector(".btn-copy");
                  const originalText = copyBtn.innerText;
                  copyBtn.innerText = "Copied!";
                  copyBtn.style.background = "#28a745";
                  
                  setTimeout(() => {
                      copyBtn.innerText = originalText;
                      copyBtn.style.background = "#555";
                  }, 1500);
              });
          }

          // กด Enter เพื่อส่งค่า
          document.getElementById("num").addEventListener("keypress", (e) => {
              if (e.key === "Enter") sendNum();
          });
      </script>

  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", page);
}

// ===== แสดงเลข =====
void showNumber(String data, int startPos) {
  int digit = 0;

  for (int i = 0; i < data.length() && digit < 4; i++) {
    char c = data[i];

    if (isdigit(c)) {
      bool dot = false;

      if (i + 1 < data.length() && data[i + 1] == '.') {
        dot = true;
        i++;
      }

      int pos = startPos - digit;
      if (pos >= 0 && pos <= 7) {
        lc.setDigit(0, pos, c - '0', dot);
      }

      digit++;
    }
  }
}

// ===== Scan WiFi =====
void handleScan() {
  int n = WiFi.scanNetworks();
  DynamicJsonDocument doc(1024);
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < n; i++) {
    arr.add(WiFi.SSID(i));
  }

  String res;
  serializeJson(arr, res);
  server.send(200, "application/json", res);
}

// ===== Connect =====
void handleConnect() {
  DynamicJsonDocument doc(512);
  deserializeJson(doc, server.arg("plain"));

  String ssid = doc["ssid"].as<String>();
  String password = doc["password"].as<String>();

  WiFi.begin(ssid.c_str(), password.c_str());
  digitalWrite(led27, LOW);
  digitalWrite(led26, HIGH);
  int t = 20;
  while (WiFi.status() != WL_CONNECTED && t--) delay(500);

  if (WiFi.status() == WL_CONNECTED) {
    saveWiFi(ssid, password);  // 🔥 save

    String ip = WiFi.localIP().toString();

    // 🔥 ส่งหน้า HTML แจ้ง user + auto redirect
    String page = " " + ip + " ";
    page += "⚠️ กรุณาเปลี่ยน WiFi ไปเครือข่ายเดียวกับ ESP32";
    page += "แล้วเข้า: http://" + ip + "";

    server.send(200, "text/html", page);
    
    delay(2000);
    digitalWrite(led26, LOW);
    // 🔥 เปลี่ยน mode → restart เข้า control mode
    ESP.restart();
  
    //server.send(200, "text/plain", "Connected: " + WiFi.localIP().toString());
  } else {
    server.send(200, "text/plain", "Fail");
  }
}

// ===== JSON LED =====
void handleJSON() {
  DynamicJsonDocument doc(256);
  deserializeJson(doc, server.arg("plain"));

  if (doc.containsKey("G")) digitalWrite(led25, doc["G"]);
  if (doc.containsKey("Y")) digitalWrite(led26, doc["Y"]);
  if (doc.containsKey("R")) digitalWrite(led27, doc["R"]);

  server.send(200, "text/plain", "OK");
}





// ===== ALL ROUTE =====
void handleAll() {
  String uri = server.uri();

  // LED
  if (uri == "/G/on") digitalWrite(led25, HIGH);
  else if (uri == "/G/off") digitalWrite(led25, LOW);
  else if (uri == "/Y/on") digitalWrite(led26, HIGH);
  else if (uri == "/Y/off") digitalWrite(led26, LOW);
  else if (uri == "/R/on") digitalWrite(led27, HIGH);
  else if (uri == "/R/off") digitalWrite(led27, LOW);

  // MAX7219

  else if (uri == "/c_clear") {
    delay(10);
    lc.clearDisplay(0);
  }

  else if (uri.startsWith("/c_")) {
    String val = uri.substring(3);
    showNumber(val, 5);
  }

  else {
    server.send(404, "text/plain", "Not found");
    return;
  }

  server.send(200, "text/plain", "OK");
}

// ===== Setup =====
void setup() {
  delay(1000);
  Serial.begin(115200);

  pinMode(BTN_PIN, INPUT_PULLUP);

  EEPROM.begin(EEPROM_SIZE);

  // ===== MAX7219 =====
  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);

  // ===== LED =====
  pinMode(led25, OUTPUT);
  pinMode(led26, OUTPUT);
  pinMode(led27, OUTPUT);
  // ==== RESET WIFI =====
  delay(500);
  checkButton();  // 🔥 สำคัญ

  loadWiFi();
  // ===== MODE SWITCH =====
  delay(500);
  if (saved_ssid.length() > 0) {
    Serial.println("Connecting saved WiFi...");

    WiFi.begin(saved_ssid.c_str(), saved_pass.c_str());

    int t = 20;
    while (WiFi.status() != WL_CONNECTED && t--) {
      delay(500);
      digitalWrite(led25, HIGH);
    }
    digitalWrite(led25, LOW);
    if (WiFi.status() == WL_CONNECTED) {
      digitalWrite(led27, HIGH);
      Serial.println("Connected!");
      Serial.println(WiFi.localIP());

      // ===== MODE 2 =====
      server.on("/", handleHelp);
      server.on("/on_led", HTTP_POST, handleJSON);
      server.onNotFound(handleAll);
      for(int i=0; i<7; i++){
        digitalWrite(led25, i%2);
        digitalWrite(led26, i%2);
        digitalWrite(led27, i%2);
        delay(250);
      }
      server.begin();
      return;
    }
  }

  // ===== MODE 1 =====
  Serial.println("Start AP Mode");
  digitalWrite(led27, HIGH);
  WiFi.softAP("ESP32-Setup");

  server.on("/", [](){ server.send(200,"text/html",htmlPage()); });
  server.on("/scan", handleScan);
  server.on("/connect", HTTP_POST, handleConnect);

  server.begin();
}
  
void loop() {
  server.handleClient();
}
