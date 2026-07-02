#include <WiFi.h>
#include <WebServer.h>

// ใส่ชื่อ Wi-Fi และรหัสผ่านของคุณ (ต้องเป็นวงเดียวกับที่คอมพิวเตอร์เชื่อมต่ออยู่)
const char* ssid = "NtpM";
const char* password = "04074774";

// สร้าง Web Server ที่ Port 80
WebServer server(80);

// กำหนดขา Pin (ผมขออนุญาตตั้งเป็น Pin 2 ที่เป็น LED บนบอร์ดเพื่อให้เทสง่าย แต่ถ้าต้องการใช้ Pin 1 ให้เปลี่ยนเลข 2 เป็น 1 ครับ)
const int outputPin = 2; 

void setup() {
  // หากเปลี่ยน outputPin เป็น 1 ต้องลบคำสั่ง Serial.begin(115200); บรรทัดนี้ทิ้งด้วยครับ
  Serial.begin(115200); 
  
  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, LOW); // ค่าเริ่มต้นให้ปิดไว้ก่อน

  // เริ่มการเชื่อมต่อ Wi-Fi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // **จำ IP นี้ไว้ไปกรอกในหน้าเว็บ HTML**

  // ------ จัดการคำสั่งที่ได้รับจากหน้าเว็บ HTML ------
  
  // เมื่อได้รับคำสั่ง /on
  server.on("/on", []() {
    digitalWrite(outputPin, HIGH);
    // สำคัญ: ต้องใส่ CORS Header เพื่ออนุญาตให้หน้าเว็บจาก VS Code ส่งคำสั่งมาได้
    server.sendHeader("Access-Control-Allow-Origin", "*"); 
    server.send(200, "text/plain", "Turned ON");
    Serial.println("Command Received: ON");
  });

  // เมื่อได้รับคำสั่ง /off
  server.on("/off", []() {
    digitalWrite(outputPin, LOW);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Turned OFF");
    Serial.println("Command Received: OFF");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // ตรวจสอบอย่างต่อเนื่องว่ามีคำสั่งเข้ามาหรือไม่
  server.handleClient();
}