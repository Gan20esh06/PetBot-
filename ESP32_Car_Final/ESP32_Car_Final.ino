/*
 * ESP32 RC Robot Car - FINAL WORKING VERSION
 * 
 * Wiring:
 * GPIO 18 → IN1, GPIO 19 → IN2, GPIO 23 → ENA (Left Motor + Enable)
 * GPIO 15 → IN3, GPIO 21 → IN4, GPIO 22 → ENB (Right Motor + Enable)
 * ESP32 GND → L298N GND (MUST CONNECT!)
 * Battery 7-12V → L298N +12V & GND
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
#define DEVICE_NAME "ESP32_Robot_Car"

// Motor Pins
#define MOTOR_LEFT_IN1  18
#define MOTOR_LEFT_IN2  19
#define MOTOR_LEFT_ENA  23
#define MOTOR_RIGHT_IN3 15
#define MOTOR_RIGHT_IN4 21
#define MOTOR_RIGHT_ENB 22

#define PWM_FREQ 1000
#define PWM_RESOLUTION 8
#define MOTOR_SPEED 255

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

void executeCommand(char command);
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopMotors();
void sendResponse(const char* message);

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("✅ Connected!");
    }
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("❌ Disconnected!");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      uint8_t* pData = pCharacteristic->getData();
      size_t len = pCharacteristic->getValue().length();

      if (len > 0) {
        Serial.print("📥 Received: ");
        Serial.println((char)pData[0]);
        
        char command = (char)pData[0];
        executeCommand(command);
        
        pCharacteristic->setValue("");
      }
    }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n🚗 ESP32 Robot Car Starting...");

  pinMode(MOTOR_LEFT_IN1, OUTPUT);
  pinMode(MOTOR_LEFT_IN2, OUTPUT);
  pinMode(MOTOR_RIGHT_IN3, OUTPUT);
  pinMode(MOTOR_RIGHT_IN4, OUTPUT);

  ledcAttach(MOTOR_LEFT_ENA, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(MOTOR_RIGHT_ENB, PWM_FREQ, PWM_RESOLUTION);

  stopMotors();
  
  Serial.println("✅ Motors configured");
  Serial.println("📍 Left: GPIO 18,19,23 | Right: GPIO 15,21,22");

  BLEDevice::init(DEVICE_NAME);
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

  Serial.println("✅ BLE Ready: " DEVICE_NAME);
  Serial.println("🎯 Waiting for connection...\n");
}

void executeCommand(char command) {
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.print("🎯 Command: ");
  Serial.println(command);
  
  switch (command) {
    case 'F':
    case 'f':
      Serial.println("🔼 FORWARD");
      moveForward();
      sendResponse("Moving forward");
      break;
      
    case 'B':
    case 'b':
      Serial.println("🔽 BACKWARD");
      moveBackward();
      sendResponse("Moving backward");
      break;
      
    case 'L':
    case 'l':
      Serial.println("◀️ LEFT");
      turnLeft();
      sendResponse("Turning left");
      break;
      
    case 'R':
    case 'r':
      Serial.println("▶️ RIGHT");
      turnRight();
      sendResponse("Turning right");
      break;
      
    case 'S':
    case 's':
      Serial.println("⏹️ STOP");
      stopMotors();
      sendResponse("Stopped");
      break;
      
    default:
      Serial.println("❓ UNKNOWN");
      break;
  }
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━\n");
}

void moveForward() {
  digitalWrite(MOTOR_LEFT_IN1, HIGH);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  ledcWrite(MOTOR_LEFT_ENA, MOTOR_SPEED);
  
  digitalWrite(MOTOR_RIGHT_IN3, HIGH);
  digitalWrite(MOTOR_RIGHT_IN4, LOW);
  ledcWrite(MOTOR_RIGHT_ENB, MOTOR_SPEED);
  
  Serial.println("⚡ L: IN1=H IN2=L ENA=" + String(MOTOR_SPEED));
  Serial.println("⚡ R: IN3=H IN4=L ENB=" + String(MOTOR_SPEED));
}

void moveBackward() {
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, HIGH);
  ledcWrite(MOTOR_LEFT_ENA, MOTOR_SPEED);
  
  digitalWrite(MOTOR_RIGHT_IN3, LOW);
  digitalWrite(MOTOR_RIGHT_IN4, HIGH);
  ledcWrite(MOTOR_RIGHT_ENB, MOTOR_SPEED);
  
  Serial.println("⚡ L: IN1=L IN2=H ENA=" + String(MOTOR_SPEED));
  Serial.println("⚡ R: IN3=L IN4=H ENB=" + String(MOTOR_SPEED));
}

void turnLeft() {
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, HIGH);
  ledcWrite(MOTOR_LEFT_ENA, MOTOR_SPEED);
  
  digitalWrite(MOTOR_RIGHT_IN3, HIGH);
  digitalWrite(MOTOR_RIGHT_IN4, LOW);
  ledcWrite(MOTOR_RIGHT_ENB, MOTOR_SPEED);
  
  Serial.println("⚡ L: IN1=L IN2=H | R: IN3=H IN4=L");
}

void turnRight() {
  digitalWrite(MOTOR_LEFT_IN1, HIGH);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  ledcWrite(MOTOR_LEFT_ENA, MOTOR_SPEED);
  
  digitalWrite(MOTOR_RIGHT_IN3, LOW);
  digitalWrite(MOTOR_RIGHT_IN4, HIGH);
  ledcWrite(MOTOR_RIGHT_ENB, MOTOR_SPEED);
  
  Serial.println("⚡ L: IN1=H IN2=L | R: IN3=L IN4=H");
}

void stopMotors() {
  ledcWrite(MOTOR_LEFT_ENA, 0);
  ledcWrite(MOTOR_RIGHT_ENB, 0);
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  digitalWrite(MOTOR_RIGHT_IN3, LOW);
  digitalWrite(MOTOR_RIGHT_IN4, LOW);
  Serial.println("⚡ STOPPED (ENA=0 ENB=0)");
}

void sendResponse(const char* message) {
  if (deviceConnected) {
    pTxCharacteristic->setValue(message);
    pTxCharacteristic->notify();
    Serial.println("📤 Sent: " + String(message));
  }
}

void loop() {
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("🔄 Restarting BLE...");
    oldDeviceConnected = deviceConnected;
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
  
  delay(10);
}
