/*
 * ESP32 BLE Robot Car Controller
 * 
 * This code allows your ESP32 to receive commands via Bluetooth (BLE)
 * from the AI Avatar web interface and control a robot car.
 * 
 * Hardware Setup:
 * - ESP32 Dev Board
 * - L298N Motor Driver
 * - 2 DC Motors (or 4 motors)
 * - Battery Pack (7.4V - 12V)
 * 
 * Pin Connections:
 * GPIO 16 -> IN1 (Motor A Forward)
 * GPIO 17 -> IN2 (Motor A Backward)
 * GPIO 18 -> IN3 (Motor B Forward)
 * GPIO 19 -> IN4 (Motor B Backward)
 * 
 * Commands:
 * F = Forward
 * B = Backward
 * L = Left
 * R = Right
 * S = Stop
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Motor Driver Pins (L298N)
#define MOTOR_A_IN1 16  // Left Motor Forward
#define MOTOR_A_IN2 17  // Left Motor Backward
#define MOTOR_B_IN3 18  // Right Motor Forward
#define MOTOR_B_IN4 19  // Right Motor Backward

// Optional: PWM Speed Control (if L298N supports it)
#define MOTOR_A_ENA 25  // Enable A (PWM)
#define MOTOR_B_ENB 26  // Enable B (PWM)

// BLE Service UUIDs (Nordic UART Service)
#define SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

int motorSpeed = 200;  // Default speed (0-255), adjust as needed

// BLE Server Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("✅ Device connected!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("❌ Device disconnected!");
      stopMotors();  // Safety: stop when disconnected
    }
};

// BLE Characteristic Callbacks
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.print("📥 Received: ");
        
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();

        // Process command (first character)
        char command = rxValue[0];
        executeCommand(command);
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("🤖 ESP32 BLE Robot Car Starting...");

  // Initialize motor pins
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN3, OUTPUT);
  pinMode(MOTOR_B_IN4, OUTPUT);

  // Optional: PWM setup for speed control
  // ledcSetup(0, 5000, 8);  // Channel 0, 5kHz, 8-bit resolution
  // ledcAttachPin(MOTOR_A_ENA, 0);
  // ledcSetup(1, 5000, 8);  // Channel 1
  // ledcAttachPin(MOTOR_B_ENB, 1);

  // Initialize motors to stopped state
  stopMotors();

  // Create BLE Device
  BLEDevice::init("ESP32_Robot_Car");

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

  Serial.println("✅ ESP32 BLE Robot Car Ready!");
  Serial.println("📡 Waiting for connection...");
  Serial.println("Device name: ESP32_Robot_Car");
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
      moveForward();
      sendResponse("Moving Forward");
      break;

    case 'B':
    case 'b':
      moveBackward();
      sendResponse("Moving Backward");
      break;

    case 'L':
    case 'l':
      turnLeft();
      sendResponse("Turning Left");
      break;

    case 'R':
    case 'r':
      turnRight();
      sendResponse("Turning Right");
      break;

    case 'S':
    case 's':
      stopMotors();
      sendResponse("Stopped");
      break;

    default:
      Serial.println("⚠️ Unknown command");
      sendResponse("Unknown Command");
      break;
  }
}

// ===== MOTOR CONTROL FUNCTIONS =====

void moveForward() {
  Serial.println("⬆️ Moving Forward");
  
  // Left motor forward
  digitalWrite(MOTOR_A_IN1, HIGH);
  digitalWrite(MOTOR_A_IN2, LOW);
  
  // Right motor forward
  digitalWrite(MOTOR_B_IN3, HIGH);
  digitalWrite(MOTOR_B_IN4, LOW);

  // Optional: Set speed via PWM
  // ledcWrite(0, motorSpeed);
  // ledcWrite(1, motorSpeed);
}

void moveBackward() {
  Serial.println("⬇️ Moving Backward");
  
  // Left motor backward
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, HIGH);
  
  // Right motor backward
  digitalWrite(MOTOR_B_IN3, LOW);
  digitalWrite(MOTOR_B_IN4, HIGH);

  // Optional: Set speed via PWM
  // ledcWrite(0, motorSpeed);
  // ledcWrite(1, motorSpeed);
}

void turnLeft() {
  Serial.println("⬅️ Turning Left");
  
  // Left motor backward
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, HIGH);
  
  // Right motor forward
  digitalWrite(MOTOR_B_IN3, HIGH);
  digitalWrite(MOTOR_B_IN4, LOW);

  // Optional: Set speed via PWM
  // ledcWrite(0, motorSpeed);
  // ledcWrite(1, motorSpeed);
}

void turnRight() {
  Serial.println("➡️ Turning Right");
  
  // Left motor forward
  digitalWrite(MOTOR_A_IN1, HIGH);
  digitalWrite(MOTOR_A_IN2, LOW);
  
  // Right motor backward
  digitalWrite(MOTOR_B_IN3, LOW);
  digitalWrite(MOTOR_B_IN4, HIGH);

  // Optional: Set speed via PWM
  // ledcWrite(0, motorSpeed);
  // ledcWrite(1, motorSpeed);
}

void stopMotors() {
  Serial.println("🛑 Stopping");
  
  // Stop both motors
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, LOW);
  digitalWrite(MOTOR_B_IN3, LOW);
  digitalWrite(MOTOR_B_IN4, LOW);

  // Optional: Set speed to 0
  // ledcWrite(0, 0);
  // ledcWrite(1, 0);
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

// ===== HELPER FUNCTIONS =====

// Adjust motor speed (0-255)
void setMotorSpeed(int speed) {
  if (speed < 0) speed = 0;
  if (speed > 255) speed = 255;
  motorSpeed = speed;
  Serial.print("⚡ Motor speed set to: ");
  Serial.println(motorSpeed);
}

// Auto-stop after timeout (safety feature)
// Call this in loop() if you want motors to auto-stop after X seconds
unsigned long lastCommandTime = 0;
void checkAutoStop() {
  if (millis() - lastCommandTime > 5000) {  // 5 seconds timeout
    stopMotors();
    lastCommandTime = millis();
  }
}
