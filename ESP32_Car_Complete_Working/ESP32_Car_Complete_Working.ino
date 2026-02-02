/*
 * ESP32 RC Robot Car - Final Code
 * 
 * This code controls a robot car via Bluetooth Low Energy (BLE)
 * Commands: F=Forward, B=Backward, L=Left, R=Right, S=Stop
 * 
 * Hardware Setup:
 * - L298N Motor Driver
 * - 2 DC Motors (Left and Right)
 * - ESP32 Dev Board
 * - 7-12V Battery for motors
 * 
 * Wiring:
 * GPIO 16 → IN1 (Left Motor)
 * GPIO 17 → IN2 (Left Motor)
 * GPIO 18 → IN3 (Right Motor)
 * GPIO 19 → IN4 (Right Motor)
 * 
 * Date: November 2025
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ===== BLE Configuration =====
#define SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e" // Nordic UART Service
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e" // Write (from app to ESP32)
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e" // Notify (from ESP32 to app)

#define DEVICE_NAME "ESP32_Robot_Car"

// ===== Motor Pin Definitions =====
// Left Motor
#define MOTOR_LEFT_IN1  18  // GPIO 18
#define MOTOR_LEFT_IN2  19  // GPIO 19

// Right Motor
#define MOTOR_RIGHT_IN3 15  // GPIO 15
#define MOTOR_RIGHT_IN4 21  // GPIO 21

// ===== PWM Configuration =====
#define PWM_FREQ 1000      // 1 KHz
#define PWM_RESOLUTION 8   // 8-bit (0-255)
#define PWM_CHANNEL_1 0    // Channel for Left Motor IN1
#define PWM_CHANNEL_2 1    // Channel for Left Motor IN2
#define PWM_CHANNEL_3 2    // Channel for Right Motor IN3
#define PWM_CHANNEL_4 3    // Channel for Right Motor IN4

// ===== Motor Speed (0-255) =====
#define MOTOR_SPEED 200  // Adjust this value: 0 (stop) to 255 (max speed)

// ===== BLE Objects =====
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// ===== Function Declarations (Forward Declarations) =====
void executeCommand(char command);
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopMotors();
void sendResponse(String message);

// ===== Server Callbacks =====
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("✅ Device connected!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("❌ Device disconnected!");
    }
};

// ===== Characteristic Callbacks (Receive Commands) =====
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.print("📥 Received: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();

        // Extract command (first character)
        char command = rxValue[0];
        executeCommand(command);
        
        // Clear the value to accept new commands
        pCharacteristic->setValue("");
      }
    }
};

// ===== Setup Function =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("🚗 ESP32 Robot Car - Initializing...");

  // Configure PWM channels for motor control (ESP32 v3.x syntax)
  ledcAttach(MOTOR_LEFT_IN1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(MOTOR_LEFT_IN2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(MOTOR_RIGHT_IN3, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(MOTOR_RIGHT_IN4, PWM_FREQ, PWM_RESOLUTION);

  // Stop motors initially
  stopMotors();
  Serial.println("✅ Motor pins configured with PWM");
  Serial.println("📍 Left Motor: GPIO 18, 19");
  Serial.println("📍 Right Motor: GPIO 15, 21");

  // Initialize BLE
  setupBLE();
  
  Serial.println("🎯 Ready! Waiting for Bluetooth connection...");
  Serial.println("Device Name: " DEVICE_NAME);
}

// ===== BLE Setup =====
void setupBLE() {
  // Create BLE Device
  BLEDevice::init(DEVICE_NAME);

  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create TX Characteristic (for sending data to app)
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // Create RX Characteristic (for receiving commands from app)
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("✅ BLE initialized and advertising");
}

// ===== Command Execution =====
void executeCommand(char command) {
  Serial.print("🎯 Executing: ");
  
  switch (command) {
    case 'F':
    case 'f':
      Serial.println("FORWARD");
      moveForward();
      sendResponse("Moving forward");
      break;
      
    case 'B':
    case 'b':
      Serial.println("BACKWARD");
      moveBackward();
      sendResponse("Moving backward");
      break;
      
    case 'L':
    case 'l':
      Serial.println("LEFT");
      turnLeft();
      sendResponse("Turning left");
      break;
      
    case 'R':
    case 'r':
      Serial.println("RIGHT");
      turnRight();
      sendResponse("Turning right");
      break;
      
    case 'S':
    case 's':
      Serial.println("STOP");
      stopMotors();
      sendResponse("Stopped");
      break;
      
    default:
      Serial.println("UNKNOWN COMMAND: " + String(command));
      sendResponse("Unknown command");
      break;
  }
}

// ===== Motor Control Functions =====

void moveForward() {
  Serial.println("⚡ Motors: FORWARD");
  // Left motor forward
  ledcWrite(MOTOR_LEFT_IN1, MOTOR_SPEED);
  ledcWrite(MOTOR_LEFT_IN2, 0);
  
  // Right motor forward
  ledcWrite(MOTOR_RIGHT_IN3, MOTOR_SPEED);
  ledcWrite(MOTOR_RIGHT_IN4, 0);
}

void moveBackward() {
  Serial.println("⚡ Motors: BACKWARD");
  // Left motor backward
  ledcWrite(MOTOR_LEFT_IN1, 0);
  ledcWrite(MOTOR_LEFT_IN2, MOTOR_SPEED);
  
  // Right motor backward
  ledcWrite(MOTOR_RIGHT_IN3, 0);
  ledcWrite(MOTOR_RIGHT_IN4, MOTOR_SPEED);
}

void turnLeft() {
  Serial.println("⚡ Motors: LEFT");
  // Left motor backward
  ledcWrite(MOTOR_LEFT_IN1, 0);
  ledcWrite(MOTOR_LEFT_IN2, MOTOR_SPEED);
  
  // Right motor forward
  ledcWrite(MOTOR_RIGHT_IN3, MOTOR_SPEED);
  ledcWrite(MOTOR_RIGHT_IN4, 0);
}

void turnRight() {
  Serial.println("⚡ Motors: RIGHT");
  // Left motor forward
  ledcWrite(MOTOR_LEFT_IN1, MOTOR_SPEED);
  ledcWrite(MOTOR_LEFT_IN2, 0);
  
  // Right motor backward
  ledcWrite(MOTOR_RIGHT_IN3, 0);
  ledcWrite(MOTOR_RIGHT_IN4, MOTOR_SPEED);
}

void stopMotors() {
  Serial.println("⚡ Motors: STOP");
  ledcWrite(MOTOR_LEFT_IN1, 0);
  ledcWrite(MOTOR_LEFT_IN2, 0);
  ledcWrite(MOTOR_RIGHT_IN3, 0);
  ledcWrite(MOTOR_RIGHT_IN4, 0);
}

// ===== Send Response to App =====
void sendResponse(String message) {
  if (deviceConnected) {
    pTxCharacteristic->setValue(message.c_str());
    pTxCharacteristic->notify();
    Serial.println("📤 Sent: " + message);
  }
}

// ===== Main Loop =====
void loop() {
  // Handle connection/disconnection
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Give time for Bluetooth stack
    pServer->startAdvertising(); // Restart advertising
    Serial.println("🔄 Restarting advertising...");
    oldDeviceConnected = deviceConnected;
  }
  
  // New connection
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
  
  delay(10);
}
