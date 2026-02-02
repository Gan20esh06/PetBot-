/*
 * ESP32 BLE Servo Test - FIXED VERSION
 * 
 * This version removes library conflicts and uses only the built-in ESP32 BLE library.
 * 
 * Hardware Setup:
 * - ESP32 Dev Board
 * - 1 Servo Motor (SG90 or similar)
 * - Optional: External 5V power for servo
 * 
 * Pin Connections:
 * Servo Orange/Yellow (Signal) -> GPIO 18
 * Servo Red (5V)              -> ESP32 5V or External 5V
 * Servo Brown/Black (GND)     -> ESP32 GND
 * 
 * Commands from Avatar:
 * F = Servo to 0° (Forward position)
 * B = Servo to 180° (Backward position)
 * L = Servo to 45° (Left position)
 * R = Servo to 135° (Right position)
 * S = Servo to 90° (Stop/Center position)
 * 
 * IMPORTANT: 
 * 1. Remove any BLE libraries from Documents/Arduino/libraries folder
 * 2. Only use the built-in ESP32 BLE library
 * 3. Install "ESP32Servo" library from Library Manager
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32Servo.h>

// Servo Setup
Servo myServo;
#define SERVO_PIN 18  // GPIO pin for servo signal

// BLE Service UUIDs (Nordic UART Service)
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

// BLE Server Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("✅ Device connected!");
      
      // Move servo to center as confirmation
      myServo.write(90);
      delay(500);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("❌ Device disconnected!");
      
      // Return servo to center on disconnect
      myServo.write(90);
    }
};

// BLE Characteristic Callbacks
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue().c_str();

      if (rxValue.length() > 0) {
        Serial.print("📥 Received: ");
        Serial.println(rxValue);

        // Process command (first character)
        char command = rxValue.charAt(0);
        executeCommand(command);
      }
    }
};

void setup() {
  Serial.begin(115200);
  delay(1000);  // Wait for serial to stabilize
  Serial.println("\n🤖 ESP32 BLE Servo Test Starting...");

  // Initialize servo with proper PWM settings
  // ESP32Servo uses 50Hz PWM, 1000-2000us pulse width
  myServo.setPeriodHertz(50);    // Standard 50Hz servo
  myServo.attach(SERVO_PIN, 500, 2500);  // (pin, min, max in microseconds)
  
  delay(500);  // Give servo time to initialize
  
  myServo.write(90);  // Start at center position
  delay(1000);  // Hold position to stabilize
  
  Serial.println("🎯 Servo initialized at center (90°)");

  // Create BLE Device
  BLEDevice::init("ESP32_Servo_Test");

  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristic for TX (ESP32 -> App)
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // Create BLE Characteristic for RX (App -> ESP32)
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("✅ ESP32 BLE Servo Test Ready!");
  Serial.println("📡 Waiting for connection...");
  Serial.println("Device name: ESP32_Servo_Test");
  Serial.println("\nTest commands:");
  Serial.println("  F = 0° (Forward)");
  Serial.println("  L = 45° (Left)");
  Serial.println("  S = 90° (Center/Stop)");
  Serial.println("  R = 135° (Right)");
  Serial.println("  B = 180° (Backward)");
}

void loop() {
  // Handle disconnection
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("🔄 Restarting advertising...");
    oldDeviceConnected = deviceConnected;
  }

  // Handle new connection
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  delay(10);
}

// ===== COMMAND EXECUTION =====
void executeCommand(char command) {
  Serial.print("⚙️ Executing command: ");
  Serial.println(command);

  switch (command) {
    case 'F':
    case 'f':
      moveServo(0, "Forward (0°)");
      break;

    case 'B':
    case 'b':
      moveServo(180, "Backward (180°)");
      break;

    case 'L':
    case 'l':
      moveServo(45, "Left (45°)");
      break;

    case 'R':
    case 'r':
      moveServo(135, "Right (135°)");
      break;

    case 'S':
    case 's':
      moveServo(90, "Center (90°)");
      break;

    default:
      Serial.println("⚠️ Unknown command");
      sendResponse("Unknown Command");
      break;
  }
}

// ===== SERVO CONTROL =====
void moveServo(int angle, const char* description) {
  Serial.print("🎯 Moving servo to: ");
  Serial.print(angle);
  Serial.print("° (");
  Serial.print(description);
  Serial.println(")");
  
  myServo.write(angle);
  delay(15);  // Small delay for servo to register command
  
  sendResponse(description);
}

// ===== SEND RESPONSE TO APP =====
void sendResponse(const char* message) {
  if (deviceConnected) {
    pTxCharacteristic->setValue(message);
    pTxCharacteristic->notify();
    Serial.print("📤 Sent response: ");
    Serial.println(message);
  }
}
