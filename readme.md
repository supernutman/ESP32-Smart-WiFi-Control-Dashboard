# ESP32 Smart WiFi Control Dashboard

ESP32 Smart WiFi Control Dashboard เป็นโปรเจกต์ IoT สำหรับควบคุมอุปกรณ์ผ่าน WiFi โดยใช้ ESP32  
รองรับการตั้งค่า WiFi ผ่านหน้าเว็บ, ควบคุม LED หลายดวง, และแสดงผลตัวเลขบน MAX7219 ผ่าน Web Dashboard

---

## Features

- WiFi Setup ผ่าน Access Point Mode
- บันทึก SSID / Password ลง EEPROM
- Auto Connect WiFi ที่เคยบันทึกไว้
- Reset WiFi ด้วยปุ่มกด
- Web Dashboard สำหรับควบคุม:
  - LED Green (GPIO25)
  - LED Yellow (GPIO26)
  - LED Red (GPIO27)
- แสดงตัวเลขบน MAX7219 LED Display
- REST-style HTTP API สำหรับควบคุมอุปกรณ์
- Responsive Web Interface ใช้งานผ่านมือถือและคอมพิวเตอร์ได้

---

## Hardware Required

- ESP32 Dev Board
- MAX7219 7-Segment Display
- LED 3 ดวง
- Push Button Reset
- Resistors ตามเหมาะสม
- WiFi Network

---

## Pin Configuration

| Device | GPIO |
|--------|------|
| Green LED | 25 |
| Yellow LED | 26 |
| Red LED | 27 |
| Reset Button | 23 |
| MAX7219 DIN | 15 |
| MAX7219 CLK | 14 |
| MAX7219 CS | 13 |

---

## Working Modes

### 1. Setup Mode (AP Mode)

ถ้ายังไม่มี WiFi ที่บันทึกไว้:
ESP32 จะสร้าง WiFi ชื่อ:

ESP32-Setup

จากนั้นเข้าเว็บ:

http://192.168.4.1

เพื่อเลือก WiFi และกรอกรหัสผ่าน

---

### 2. Control Mode

เมื่อเชื่อม WiFi สำเร็จ:
- ESP32 จะเชื่อม WiFi อัตโนมัติ
- เปิด Web Dashboard ผ่าน IP Address ที่ได้รับ

ตัวอย่าง:

http://192.168.1.100

---

## API Endpoints

### LED Control

| Endpoint | Action |
|--------|--------|
| /G/on | เปิด Green LED |
| /G/off | ปิด Green LED |
| /Y/on | เปิด Yellow LED |
| /Y/off | ปิด Yellow LED |
| /R/on | เปิด Red LED |
| /R/off | ปิด Red LED |

---

### MAX7219 Display

| Endpoint | Action |
|--------|--------|
| /c_1234 | แสดงเลข 1234 |
| /c_clear | ล้างหน้าจอ |

---

### JSON LED Control

POST ไปที่:

/on_led

ตัวอย่าง JSON:

```json
{
  "G": 1,
  "Y": 0,
  "R": 1
}