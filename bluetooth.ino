#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;

// กำหนดรหัส UUID สำหรับช่องทางสื่อสาร (มาตรฐาน Nordic UART)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // ช่องบริการหลัก
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // ช่องรับข้อมูล
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // ช่องส่งข้อมูล

// ฟังก์ชันเช็คสถานะการเชื่อมต่อ
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("มีอุปกรณ์เชื่อมต่อเข้ามาแล้ว!");
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("อุปกรณ์ตัดการเชื่อมต่อ รอรับการเชื่อมต่อใหม่...");
      delay(500); // ให้เวลา ESP32 รีเซ็ตคลื่น
      pServer->startAdvertising(); // เริ่มปล่อยสัญญาณใหม่
    }
};

// ฟังก์ชันเมื่อได้รับข้อความจากมือถือ (แก้ไขให้รองรับ ESP32 Core 3.x)
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      // เปลี่ยนจาก std::string เป็น String
      String rxValue = pCharacteristic->getValue(); 
      
      if (rxValue.length() > 0) {
        // สามารถพิมพ์ข้อความออกทาง Serial Monitor ได้เลย
        Serial.print(rxValue); 
      }
    }
};

void setup() {
  Serial.begin(115200);

  // ตั้งชื่อ Bluetooth (เปลี่ยนชื่อในวงเล็บได้)
  BLEDevice::init("ESP32_BLE_Chat");
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  // สร้างช่องสำหรับส่งข้อมูลให้มือถือ (TX)
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // สร้างช่องสำหรับรับข้อมูลจากมือถือ (RX)
  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                                             CHARACTERISTIC_UUID_RX,
                                             BLECharacteristic::PROPERTY_WRITE
                                           );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();
  
  Serial.println("=========================================");
  Serial.println("ระบบ BLE พร้อมทำงานแล้ว!");
  Serial.println("กรุณาเปิดแอปบน iPhone แล้วเชื่อมต่อกับ 'ESP32_BLE_Chat'");
  Serial.println("=========================================");
}

void loop() {
  // ตรวจสอบว่ามีการพิมพ์ข้อความจากฝั่งคอมพิวเตอร์หรือไม่
  if (deviceConnected && Serial.available()) {
    String message = Serial.readString(); // อ่านข้อความทั้งหมดที่พิมพ์
    
    // ส่งข้อความไปที่ iPhone
    pTxCharacteristic->setValue(message.c_str());
    pTxCharacteristic->notify();
    delay(10); // ป้องกันการส่งรัวเกินไป
  }
}