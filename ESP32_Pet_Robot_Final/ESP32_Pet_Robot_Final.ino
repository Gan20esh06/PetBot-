/*
 * ESP32 Pet Robot Car with BLE Control
 * Features: Motor control, speed settings, BLE communication
 * Compatible with ESP32 Arduino Core 2.x and 3.x
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Motor pins
#define MOTOR_LEFT_IN1 18
#define MOTOR_LEFT_IN2 19
#define MOTOR_RIGHT_IN3 15
#define MOTOR_RIGHT_IN4 21

#define MOTOR_LEFT_ENA 23   // PWM pin for left motor
#define MOTOR_RIGHT_ENB 22  // PWM pin for right motor

// PWM settings
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

// PWM channels (for ESP32 Core 2.x)
#define PWM_CHANNEL_LEFT 0
#define PWM_CHANNEL_RIGHT 1

// Speed settings
#define SPEED_GENTLE 150
#define SPEED_NORMAL 200
#define SPEED_EXCITED 255
#define SPEED_TURN 180
#define SPEED_BOOST 255
#define BOOST_DURATION 150

// BLE UUIDs
#define SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
int currentSpeed = SPEED_NORMAL;

// Built-in LED for status
#define LED_PIN 2

// Forward declarations
void moveForward(int speed);
void moveBackward(int speed);
void turnLeft(int speed);
void turnRight(int speed);
void stopMotors();
void processCommand(String command);

// BLE Server Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      digitalWrite(LED_PIN, HIGH);
      Serial.println("Client connected");
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      digitalWrite(LED_PIN, LOW);
      stopMotors();
      Serial.println("Client disconnected");
    }
};

// BLE Characteristic Callbacks
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue().c_str();

      if (value.length() > 0) {
        value.trim();
        Serial.print("Received command: ");
        Serial.println(value);
        processCommand(value);
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Pet Robot Car Starting...");

  // Setup LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Setup motor pins
  pinMode(MOTOR_LEFT_IN1, OUTPUT);
  pinMode(MOTOR_LEFT_IN2, OUTPUT);
  pinMode(MOTOR_RIGHT_IN3, OUTPUT);
  pinMode(MOTOR_RIGHT_IN4, OUTPUT);

  // Setup PWM - Compatible with both ESP32 Core 2.x and 3.x
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    // ESP32 Core 3.x - uses ledcAttach
    ledcAttach(MOTOR_LEFT_ENA, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(MOTOR_RIGHT_ENB, PWM_FREQ, PWM_RESOLUTION);
  #else
    // ESP32 Core 2.x - uses ledcSetup and ledcAttachPin
    ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(MOTOR_LEFT_ENA, PWM_CHANNEL_LEFT);
    ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(MOTOR_RIGHT_ENB, PWM_CHANNEL_RIGHT);
  #endif

  stopMotors();

  // Create BLE Device
  BLEDevice::init("ESP32_Robot_Car");

  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics
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

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE started. Device name: ESP32_Robot_Car");
  Serial.println("Waiting for client connection...");
}

void loop() {
  // Handle BLE connection changes
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Reconnecting...");
    oldDeviceConnected = deviceConnected;
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  // Blink LED when connected
  if (deviceConnected) {
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 1000) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      lastBlink = millis();
    }
  }

  delay(10);
}

void processCommand(String command) {
  command.toUpperCase();
  
  // Motor movement commands
  if (command == "F" || command.startsWith("F\n")) {
    moveForward(currentSpeed);
  }
  else if (command == "B" || command.startsWith("B\n")) {
    moveBackward(currentSpeed);
  }
  else if (command == "L" || command.startsWith("L\n")) {
    turnLeft(SPEED_TURN);
  }
  else if (command == "R" || command.startsWith("R\n")) {
    turnRight(SPEED_TURN);
  }
  else if (command == "S" || command.startsWith("S\n")) {
    stopMotors();
  }
  // Speed commands
  else if (command == "GENTLE" || command.startsWith("GENTLE\n")) {
    currentSpeed = SPEED_GENTLE;
    Serial.println("Speed: GENTLE");
  }
  else if (command == "NORMAL" || command.startsWith("NORMAL\n")) {
    currentSpeed = SPEED_NORMAL;
    Serial.println("Speed: NORMAL");
  }
  else if (command == "EXCITED" || command.startsWith("EXCITED\n")) {
    currentSpeed = SPEED_EXCITED;
    Serial.println("Speed: EXCITED");
  }
  else {
    Serial.print("Unknown command: ");
    Serial.println(command);
  }
}

void moveForward(int speed) {
  Serial.print("Moving forward at speed: ");
  Serial.println(speed);
  
  // Left motor forward
  digitalWrite(MOTOR_LEFT_IN1, HIGH);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  
  // Right motor forward
  digitalWrite(MOTOR_RIGHT_IN3, HIGH);
  digitalWrite(MOTOR_RIGHT_IN4, LOW);
  
  // Initial boost to overcome static friction
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(MOTOR_LEFT_ENA, SPEED_BOOST);
    ledcWrite(MOTOR_RIGHT_ENB, SPEED_BOOST);
  #else
    ledcWrite(PWM_CHANNEL_LEFT, SPEED_BOOST);
    ledcWrite(PWM_CHANNEL_RIGHT, SPEED_BOOST);
  #endif
  
  delay(BOOST_DURATION);
  
  // Then set normal speed
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(MOTOR_LEFT_ENA, speed);
    ledcWrite(MOTOR_RIGHT_ENB, speed);
  #else
    ledcWrite(PWM_CHANNEL_LEFT, speed);
    ledcWrite(PWM_CHANNEL_RIGHT, speed);
  #endif
}

void moveBackward(int speed) {
  Serial.print("Moving backward at speed: ");
  Serial.println(speed);
  
  // Left motor backward
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, HIGH);
  
  // Right motor backward
  digitalWrite(MOTOR_RIGHT_IN3, LOW);
  digitalWrite(MOTOR_RIGHT_IN4, HIGH);
  
  // Initial boost to overcome static friction
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(MOTOR_LEFT_ENA, SPEED_BOOST);
    ledcWrite(MOTOR_RIGHT_ENB, SPEED_BOOST);
  #else
    ledcWrite(PWM_CHANNEL_LEFT, SPEED_BOOST);
    ledcWrite(PWM_CHANNEL_RIGHT, SPEED_BOOST);
  #endif
  
  delay(BOOST_DURATION);
  
  // Then set normal speed
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(MOTOR_LEFT_ENA, speed);
    ledcWrite(MOTOR_RIGHT_ENB, speed);
  #else
    ledcWrite(PWM_CHANNEL_LEFT, speed);
    ledcWrite(PWM_CHANNEL_RIGHT, speed);
  #endif
}

void turnLeft(int speed) {
  Serial.print("Turning left at speed: ");
  Serial.println(speed);
  
  // Left motor backward (slower)
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, HIGH);
  
  // Right motor forward (faster)
  digitalWrite(MOTOR_RIGHT_IN3, HIGH);
  digitalWrite(MOTOR_RIGHT_IN4, LOW);
  
  // Initial boost
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(MOTOR_LEFT_ENA, SPEED_BOOST / 2);
    ledcWrite(MOTOR_RIGHT_ENB, SPEED_BOOST);
  #else
    ledcWrite(PWM_CHANNEL_LEFT, SPEED_BOOST / 2);
    ledcWrite(PWM_CHANNEL_RIGHT, SPEED_BOOST);
  #endif
  
  delay(BOOST_DURATION);
  
  // Set speed (gentle turn)
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(MOTOR_LEFT_ENA, speed / 2);  // Left slower
    ledcWrite(MOTOR_RIGHT_ENB, speed);     // Right faster
  #else
    ledcWrite(PWM_CHANNEL_LEFT, speed / 2);
    ledcWrite(PWM_CHANNEL_RIGHT, speed);
  #endif
}

void turnRight(int speed) {
  Serial.print("Turning right at speed: ");
  Serial.println(speed);
  
  // Left motor forward (faster)
  digitalWrite(MOTOR_LEFT_IN1, HIGH);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  
  // Right motor backward (slower)
  digitalWrite(MOTOR_RIGHT_IN3, LOW);
  digitalWrite(MOTOR_RIGHT_IN4, HIGH);
  
  // Initial boost
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(MOTOR_LEFT_ENA, SPEED_BOOST);
    ledcWrite(MOTOR_RIGHT_ENB, SPEED_BOOST / 2);
  #else
    ledcWrite(PWM_CHANNEL_LEFT, SPEED_BOOST);
    ledcWrite(PWM_CHANNEL_RIGHT, SPEED_BOOST / 2);
  #endif
  
  delay(BOOST_DURATION);
  
  // Set speed (gentle turn)
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(MOTOR_LEFT_ENA, speed);      // Left faster
    ledcWrite(MOTOR_RIGHT_ENB, speed / 2); // Right slower
  #else
    ledcWrite(PWM_CHANNEL_LEFT, speed);
    ledcWrite(PWM_CHANNEL_RIGHT, speed / 2);
  #endif
}

void stopMotors() {
  Serial.println("Stopping motors");
  
  // Stop both motors
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  digitalWrite(MOTOR_RIGHT_IN3, LOW);
  digitalWrite(MOTOR_RIGHT_IN4, LOW);
  
  // Set speed to 0
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(MOTOR_LEFT_ENA, 0);
    ledcWrite(MOTOR_RIGHT_ENB, 0);
  #else
    ledcWrite(PWM_CHANNEL_LEFT, 0);
    ledcWrite(PWM_CHANNEL_RIGHT, 0);
  #endif
}
