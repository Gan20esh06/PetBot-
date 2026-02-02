/*
 * ESP32 BLE Servo Test
 * 
 * Simple code to test Bluetooth connection with a servo motor.
 * 
 * Hardware:
 * - ESP32 Dev Board
 * - 1 Servo Motor (SG90 or similar)
 * 
 * Wiring:
 * Servo Orange/Yellow -> GPIO 18
 * Servo Red          -> ESP32 5V
 * Servo Brown/Black  -> ESP32 GND
 * 
 * Commands: F=Forward, B=Backward, L=Left, R=Right, S=Stop
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32Servo.h>

// Servo
Servo myServo;
#define SERVO_PIN 18

// BLE UUIDs
#define SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Forward declarations
void executeCommand(char command);
void moveServo(int angle, const char* description);
void sendResponse(const char* message);

// BLE Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("✅ Connected!");
      myServo.write(90);
      delay(500);
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("❌ Disconnected!");
      myServo.write(90);
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue().c_str();
      
      if (rxValue.length() > 0) {
        Serial.print("📥 Received: ");
        Serial.println(rxValue);
        
        char command = rxValue.charAt(0);
        executeCommand(command);
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("\n🤖 ESP32 Servo BLE Starting...");

  // Init servo
  myServo.attach(SERVO_PIN);
  myServo.write(90);
  Serial.println("🎯 Servo ready at 90°");

  // Init BLE
  BLEDevice::init("ESP32_Servo_Test");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("✅ Ready! Device: ESP32_Servo_Test");
  Serial.println("Commands: F=0° L=45° S=90° R=135° B=180°");
}

void loop() {
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("🔄 Advertising...");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  delay(10);
}

void executeCommand(char command) {
  Serial.print("⚙️ Command: ");
  Serial.println(command);

  switch (command) {
    case 'F':
    case 'f':
      moveServo(0, "Forward");
      break;
    case 'B':
    case 'b':
      moveServo(180, "Backward");
      break;
    case 'L':
    case 'l':
      moveServo(45, "Left");
      break;
    case 'R':
    case 'r':
      moveServo(135, "Right");
      break;
    case 'S':
    case 's':
      moveServo(90, "Center");
      break;
    default:
      Serial.println("⚠️ Unknown");
      sendResponse("Unknown");
      break;
  }
}

void moveServo(int angle, const char* description) {
  Serial.print("🎯 ");
  Serial.print(angle);
  Serial.print("° ");
  Serial.println(description);
  
  myServo.write(angle);
  sendResponse(description);
}

void sendResponse(const char* message) {
  if (deviceConnected) {
    pTxCharacteristic->setValue(message);
    pTxCharacteristic->notify();
  }
}
