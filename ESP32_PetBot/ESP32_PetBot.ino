/*
 * ESP32 PetBot - Avatar App & Dabble Control
 * 
 * ⚠️ IMPORTANT: Choose ONE control mode by uncommenting the appropriate line below
 * The ESP32 can only run one BLE mode at a time!
 * 
 * Components:
 * - ESP32 DevKit
 * - 2x N20 150 RPM DC Motors (differential drive)
 * - TB6612FNG Dual DC Motor Driver
 * - 4x HC-SR04 Ultrasonic Sensors (Front, Back, Left, Right)
 * - 2x Servo Motors (Ear movement)
 * - Base diameter: 200mm
 * 
 * Features:
 * - BLE connection for Avatar App voice commands
 * - Dabble App for manual joystick control
 * - Obstacle avoidance with 4 ultrasonic sensors
 * - Animated ear movements
 */

// ===== CONTROL MODE SELECTION =====
// ⚠️ CHANGE THIS TO SWITCH MODES!
// Comment out ONE line, uncomment the OTHER:

#define USE_AVATAR_APP    // Voice control - shows as "ESP32_Robot_Car"
//#define USE_DABBLE_APP      // Gamepad control - shows as "PetBot"

#ifdef USE_AVATAR_APP
  #include <BLEDevice.h>
  #include <BLEServer.h>
  #include <BLEUtils.h>
  #include <BLE2902.h>
#endif

#ifdef USE_DABBLE_APP
  #define CUSTOM_SETTINGS
  #define INCLUDE_GAMEPAD_MODULE
  #include <DabbleESP32.h>
  #include <nvs_flash.h>  // To clear old BLE name
#endif

#include <ESP32Servo.h>

// ===== PIN DEFINITIONS =====

// TB6612FNG Motor Driver Pins
#define MOTOR_STBY   27  // Standby pin (HIGH = motors enabled)

// Motor A (Left Motor)
#define MOTOR_A_IN1  26  // Direction control 1
#define MOTOR_A_IN2  25  // Direction control 2
#define MOTOR_A_PWM  32  // Speed control (PWM)

// Motor B (Right Motor)
#define MOTOR_B_IN1  33  // Direction control 1
#define MOTOR_B_IN2  14  // Direction control 2
#define MOTOR_B_PWM  12  // Speed control (PWM)

// Ultrasonic Sensor Pins
#define US_FRONT_TRIG  5
#define US_FRONT_ECHO  18
#define US_BACK_TRIG   19
#define US_BACK_ECHO   21
#define US_LEFT_TRIG   22
#define US_LEFT_ECHO   23
#define US_RIGHT_TRIG  4
#define US_RIGHT_ECHO  15

// Servo Pins (Ears)
#define SERVO_LEFT_EAR   13
#define SERVO_RIGHT_EAR  2

// ===== BLE CONFIGURATION (Avatar App) =====
#define SERVICE_UUID        "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_TX   "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_RX   "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define BLE_DEVICE_NAME     "ESP32_Robot_Car"  // Must match Avatar app

// ===== MOTOR SETTINGS =====
#define MOTOR_SPEED     255   // MAX speed for heavy bot
#define TURN_SPEED      255   // MAX turn speed
#define SLOW_SPEED      255   // Keep full speed even when slowing
#define MIN_MOTOR_SPEED 250   // Extra torque to push 1.5kg load

// ⚠️ MOTOR ASSIGNMENT FIX - Change these to fix spinning!
// If bot spins instead of going forward, set SWAP_LEFT_RIGHT to true!
#define SWAP_LEFT_RIGHT false  // Set TRUE if motors are physically swapped
#define INVERT_LEFT     false  // Set TRUE if left motor goes backwards when it should go forward
#define INVERT_RIGHT    false  // Set TRUE if right motor goes backwards when it should go forward

// ===== OBSTACLE DETECTION =====
#define OBSTACLE_DISTANCE_CLOSE  15  // cm - stop
#define OBSTACLE_DISTANCE_SLOW   30  // cm - slow down
#define SENSOR_TIMEOUT           30000 // microseconds

// ===== SERVO SETTINGS =====
// Calibrated safe ranges per ear to prevent buzzing and binding.
// Adjust these after testing to match your mechanical limits.
#define LEFT_EAR_MIN    55
#define LEFT_EAR_MAX    125
#define RIGHT_EAR_MIN   55
#define RIGHT_EAR_MAX   125

// Default positions within safe range
#define EAR_CENTER      90
#define EAR_UP          110
#define EAR_DOWN        70
#define EAR_ALERT       115

// ===== GLOBAL VARIABLES =====
Servo leftEarServo;
Servo rightEarServo;
bool servosAttached = true;
unsigned long lastServoMove = 0;
const unsigned long SERVO_IDLE_DETACH_MS = 6000; // detach after 6s idle to quiet servos
int lastLeftAngle = EAR_CENTER;
int lastRightAngle = EAR_CENTER;

#ifdef USE_AVATAR_APP
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
#endif

#ifdef USE_DABBLE_APP
bool deviceConnected = false; // For Dabble status
#endif

// Motor state
bool isMoving = false;
char currentDirection = 'S'; // S=Stop, F=Forward, B=Backward, L=Left, R=Right

// Sensor distances
float distFront = 100;
float distBack = 100;
float distLeft = 100;
float distRight = 100;

// Timing
unsigned long lastSensorRead = 0;
unsigned long lastEarAnimation = 0;
const int SENSOR_INTERVAL = 100;  // Read sensors every 100ms
const int EAR_ANIM_INTERVAL = 2000; // Ear animation every 2s when idle

// Dabble control
bool dabbleMode = false;
int dabbleX = 0;
int dabbleY = 0;

// Obstacle avoidance
bool obstacleAvoidanceEnabled = true;

// ===== PWM CONFIGURATION =====
const int pwmFreq = 1000;  // 1kHz for MAXIMUM TORQUE (better for heavy bots!)
const int pwmResolution = 8;
const int pwmChannelA = 0;
const int pwmChannelB = 1;

// ===== FORWARD DECLARATIONS =====
void stopMotors();
void playConnectedAnimation();
void playDisconnectedAnimation();
void processCommand(String cmd);
void centerEars();
void playStartupAnimation();
void playMovingEars();
void playAlertEars();
void playIdleEarAnimation();
void wiggleEars();
void setEars(int leftAngle, int rightAngle);
void moveEarsSmooth(int leftTarget, int rightTarget, int step = 3, int delayMs = 15);
void ensureServosAttached();
void detachServosIfIdle();
void markServoMoved();
void setupMotors();
void setupUltrasonics();
void setupServos();
void setupBLE();
void handleBLEConnection();
void sendBLEMessage(String message);
void readAllSensors();
void handleObstacles();
void handleDabbleControl();
void moveForward(int speed);
void moveBackward(int speed);
void turnLeft(int speed);
void turnRight(int speed);
void setMotorSpeeds(int leftSpeed, int rightSpeed);
float readUltrasonic(int trigPin, int echoPin);
bool checkFrontObstacle();
bool checkBackObstacle();
void handleJoystickCommand();
void setLeftMotor(bool forward, int speed);
void setRightMotor(bool forward, int speed);
void testMotors();

// ===== BLE CALLBACKS (Avatar App) =====
#ifdef USE_AVATAR_APP
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("📱 Avatar App connected!");
    playConnectedAnimation();
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("📱 Avatar App disconnected!");
    stopMotors();
    playDisconnectedAnimation();
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue().c_str();
    
    if (rxValue.length() > 0) {
      Serial.print("📨 Received: ");
      Serial.println(rxValue);
      
      processCommand(rxValue);
    }
  }
};
#endif

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  Serial.println("\n🐾 PetBot Starting...");
  
  // Initialize motor pins
  setupMotors();
  
  // Initialize ultrasonic sensors
  setupUltrasonics();
  
  // Initialize servos
  setupServos();
  
  // Initialize communication
  #ifdef USE_AVATAR_APP
    setupBLE();
    Serial.println("========================================");
    Serial.println("📱 MODE: AVATAR APP (Voice Control)");
    Serial.println("📱 BLE Name: ESP32_Robot_Car");
    Serial.println("========================================");
  #endif
  
  #ifdef USE_DABBLE_APP
    // Clear old BLE name from flash
    nvs_flash_erase();
    nvs_flash_init();
    
    Dabble.begin("PetBot");
    Serial.println("========================================");
    Serial.println("🎮 MODE: DABBLE APP (Gamepad Control)");
    Serial.println("🎮 BLE Name: PetBot");
    Serial.println("========================================");
  #endif
  
  // Play startup animation
  playStartupAnimation();
  
  // Test motors to verify configuration
  Serial.println("\n⚠️ MOTOR TEST STARTING...");
  Serial.println("   Watch carefully to see which direction motors spin!");
  testMotors();
  
  Serial.println("✅ PetBot Ready!");
}

// ===== MAIN LOOP =====
void loop() {
  #ifdef USE_AVATAR_APP
    // Handle BLE connection state
    handleBLEConnection();
  #endif
  
  // Read sensors periodically
  if (millis() - lastSensorRead >= SENSOR_INTERVAL) {
    readAllSensors();
    lastSensorRead = millis();
    
    // Check for obstacles if moving
    if (isMoving && obstacleAvoidanceEnabled) {
      handleObstacles();
    }
  }
  
  // Idle ear animation
  if (!isMoving && millis() - lastEarAnimation >= EAR_ANIM_INTERVAL) {
    playIdleEarAnimation();
    lastEarAnimation = millis();
  }
  // Detach servos when idle to prevent buzzing
  detachServosIfIdle();
  
  #ifdef USE_DABBLE_APP
    // Process Dabble gamepad input
    handleDabbleControl();
  #endif
  
  #ifdef USE_AVATAR_APP
    // Handle JOY commands from Avatar app
    if (dabbleMode && (dabbleX != 0 || dabbleY != 0)) {
      handleJoystickCommand();
    }
  #endif
  
  delay(10);
}

// ===== MOTOR FUNCTIONS =====
void setupMotors() {
  // Motor control pins
  pinMode(MOTOR_STBY, OUTPUT);
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);
  
  // PWM setup
  ledcSetup(pwmChannelA, pwmFreq, pwmResolution);
  ledcSetup(pwmChannelB, pwmFreq, pwmResolution);
  ledcAttachPin(MOTOR_A_PWM, pwmChannelA);
  ledcAttachPin(MOTOR_B_PWM, pwmChannelB);
  
  // Enable motor driver
  digitalWrite(MOTOR_STBY, HIGH);
  
  // Start with motors stopped
  stopMotors();
  
  Serial.println("🔧 Motors initialized");
}

void moveForward(int speed) {
  if (obstacleAvoidanceEnabled && checkFrontObstacle()) {
    Serial.println("⚠️ Obstacle ahead!");
    sendBLEMessage("Obstacle ahead!");
    stopMotors();
    return;
  }
  
  currentDirection = 'F';
  isMoving = true;
  
  // Both motors forward at FULL speed
  setLeftMotor(true, speed);
  setRightMotor(true, speed);
  
  Serial.println("⬆️ Forward");
}

void moveBackward(int speed) {
  if (obstacleAvoidanceEnabled && checkBackObstacle()) {
    Serial.println("⚠️ Obstacle behind!");
    sendBLEMessage("Obstacle behind!");
    stopMotors();
    return;
  }
  
  currentDirection = 'B';
  isMoving = true;
  
  // Both motors backward at FULL speed
  setLeftMotor(false, speed);
  setRightMotor(false, speed);
  
  Serial.println("⬇️ Backward");
}

void turnLeft(int speed) {
  currentDirection = 'L';
  isMoving = true;
  
  // Left backward, Right forward (tank turn)
  setLeftMotor(false, speed);
  setRightMotor(true, speed);
  
  Serial.println("⬅️ Left");
}

void turnRight(int speed) {
  currentDirection = 'R';
  isMoving = true;
  
  // Left forward, Right backward (tank turn)
  setLeftMotor(true, speed);
  setRightMotor(false, speed);
  
  Serial.println("➡️ Right");
}

void stopMotors() {
  currentDirection = 'S';
  isMoving = false;
  
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, LOW);
  ledcWrite(pwmChannelA, 0);
  
  digitalWrite(MOTOR_B_IN1, LOW);
  digitalWrite(MOTOR_B_IN2, LOW);
  ledcWrite(pwmChannelB, 0);
  
  Serial.println("⏹️ Stop");
}

// LEFT MOTOR - handles physical assignment and direction inversion
void setLeftMotor(bool forward, int speed) {
  // Apply direction inversion if needed
  if (INVERT_LEFT) forward = !forward;
  
  // Determine which physical motor to use (A or B)
  if (SWAP_LEFT_RIGHT) {
    // Left motor is physically Motor B
    if (forward) {
      digitalWrite(MOTOR_B_IN1, HIGH);
      digitalWrite(MOTOR_B_IN2, LOW);
    } else {
      digitalWrite(MOTOR_B_IN1, LOW);
      digitalWrite(MOTOR_B_IN2, HIGH);
    }
    ledcWrite(pwmChannelB, speed);
  } else {
    // Left motor is physically Motor A (default)
    if (forward) {
      digitalWrite(MOTOR_A_IN1, HIGH);
      digitalWrite(MOTOR_A_IN2, LOW);
    } else {
      digitalWrite(MOTOR_A_IN1, LOW);
      digitalWrite(MOTOR_A_IN2, HIGH);
    }
    ledcWrite(pwmChannelA, speed);
  }
}

// RIGHT MOTOR - handles physical assignment and direction inversion
void setRightMotor(bool forward, int speed) {
  // Apply direction inversion if needed
  if (INVERT_RIGHT) forward = !forward;
  
  // Determine which physical motor to use (A or B)
  if (SWAP_LEFT_RIGHT) {
    // Right motor is physically Motor A
    if (forward) {
      digitalWrite(MOTOR_A_IN1, HIGH);
      digitalWrite(MOTOR_A_IN2, LOW);
    } else {
      digitalWrite(MOTOR_A_IN1, LOW);
      digitalWrite(MOTOR_A_IN2, HIGH);
    }
    ledcWrite(pwmChannelA, speed);
  } else {
    // Right motor is physically Motor B (default)
    if (forward) {
      digitalWrite(MOTOR_B_IN1, HIGH);
      digitalWrite(MOTOR_B_IN2, LOW);
    } else {
      digitalWrite(MOTOR_B_IN1, LOW);
      digitalWrite(MOTOR_B_IN2, HIGH);
    }
    ledcWrite(pwmChannelB, speed);
  }
}

// Test motors on startup to verify configuration
void testMotors() {
  Serial.println("\n🔧 MOTOR TEST - Watch which direction they spin!");
  Serial.println("   Both should spin FORWARD (wheels go forward)");
  
  delay(1000);
  Serial.println("   LEFT motor forward...");
  setLeftMotor(true, 255);
  delay(1000);
  stopMotors();
  delay(500);
  
  Serial.println("   RIGHT motor forward...");
  setRightMotor(true, 255);
  delay(1000);
  stopMotors();
  delay(500);
  
  Serial.println("   BOTH motors forward...");
  setLeftMotor(true, 255);
  setRightMotor(true, 255);
  delay(1000);
  stopMotors();
  
  Serial.println("\n✅ Motor test complete!");
  Serial.println("   If bot spun instead of going forward:");
  Serial.println("   Set SWAP_LEFT_RIGHT = true");
  Serial.println("   If a motor went backward:");
  Serial.println("   Set INVERT_LEFT or INVERT_RIGHT = true\n");
  
  delay(2000);
}

// Variable speed control for joystick
void setMotorSpeeds(int leftSpeed, int rightSpeed) {
  // Left motor
  if (leftSpeed > 0) {
    digitalWrite(MOTOR_A_IN1, HIGH);
    digitalWrite(MOTOR_A_IN2, LOW);
  } else if (leftSpeed < 0) {
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, HIGH);
    leftSpeed = -leftSpeed;
  } else {
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, LOW);
  }
  ledcWrite(pwmChannelA, constrain(leftSpeed, 0, 255));
  
  // Right motor
  if (rightSpeed > 0) {
    digitalWrite(MOTOR_B_IN1, HIGH);
    digitalWrite(MOTOR_B_IN2, LOW);
  } else if (rightSpeed < 0) {
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, HIGH);
    rightSpeed = -rightSpeed;
  } else {
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, LOW);
  }
  ledcWrite(pwmChannelB, constrain(rightSpeed, 0, 255));
  
  isMoving = (leftSpeed != 0 || rightSpeed != 0);
}

// ===== ULTRASONIC FUNCTIONS =====
void setupUltrasonics() {
  pinMode(US_FRONT_TRIG, OUTPUT);
  pinMode(US_FRONT_ECHO, INPUT);
  pinMode(US_BACK_TRIG, OUTPUT);
  pinMode(US_BACK_ECHO, INPUT);
  pinMode(US_LEFT_TRIG, OUTPUT);
  pinMode(US_LEFT_ECHO, INPUT);
  pinMode(US_RIGHT_TRIG, OUTPUT);
  pinMode(US_RIGHT_ECHO, INPUT);
  
  Serial.println("🔧 Ultrasonic sensors initialized");
}

float readUltrasonic(int trigPin, int echoPin) {
  // Clear trigger
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Send pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read echo with timeout
  long duration = pulseIn(echoPin, HIGH, SENSOR_TIMEOUT);
  
  if (duration == 0) {
    return 999; // No echo - assume clear
  }
  
  // Calculate distance in cm
  float distance = duration * 0.034 / 2;
  return distance;
}

void readAllSensors() {
  distFront = readUltrasonic(US_FRONT_TRIG, US_FRONT_ECHO);
  distBack = readUltrasonic(US_BACK_TRIG, US_BACK_ECHO);
  distLeft = readUltrasonic(US_LEFT_TRIG, US_LEFT_ECHO);
  distRight = readUltrasonic(US_RIGHT_TRIG, US_RIGHT_ECHO);
  
  // Debug output (comment out for production)
  // Serial.printf("📏 F:%.0f B:%.0f L:%.0f R:%.0f\n", 
  //               distFront, distBack, distLeft, distRight);
}

bool checkFrontObstacle() {
  return distFront < OBSTACLE_DISTANCE_CLOSE;
}

bool checkBackObstacle() {
  return distBack < OBSTACLE_DISTANCE_CLOSE;
}

void handleObstacles() {
  switch (currentDirection) {
    case 'F':
      if (distFront < OBSTACLE_DISTANCE_CLOSE) {
        Serial.println("🚨 Obstacle detected! Stopping.");
        sendBLEMessage("Obstacle! Stopped.");
        stopMotors();
        playAlertEars();
      } else if (distFront < OBSTACLE_DISTANCE_SLOW) {
        // Slow down
        ledcWrite(pwmChannelA, SLOW_SPEED);
        ledcWrite(pwmChannelB, SLOW_SPEED);
      }
      break;
      
    case 'B':
      if (distBack < OBSTACLE_DISTANCE_CLOSE) {
        Serial.println("🚨 Obstacle behind! Stopping.");
        sendBLEMessage("Obstacle behind! Stopped.");
        stopMotors();
        playAlertEars();
      }
      break;
      
    case 'L':
      if (distLeft < OBSTACLE_DISTANCE_CLOSE) {
        Serial.println("🚨 Obstacle left! Stopping.");
        stopMotors();
      }
      break;
      
    case 'R':
      if (distRight < OBSTACLE_DISTANCE_CLOSE) {
        Serial.println("🚨 Obstacle right! Stopping.");
        stopMotors();
      }
      break;
  }
}

// ===== SERVO FUNCTIONS =====
static inline int clampEarAngleLeft(int a) { return constrain(a, LEFT_EAR_MIN, LEFT_EAR_MAX); }
static inline int clampEarAngleRight(int a) { return constrain(a, RIGHT_EAR_MIN, RIGHT_EAR_MAX); }

void ensureServosAttached() {
  if (!servosAttached) {
    leftEarServo.attach(SERVO_LEFT_EAR);
    rightEarServo.attach(SERVO_RIGHT_EAR);
    servosAttached = true;
  }
}

void markServoMoved() {
  ensureServosAttached();
  lastServoMove = millis();
}

void detachServosIfIdle() {
  if (servosAttached && !isMoving && (millis() - lastServoMove) > SERVO_IDLE_DETACH_MS) {
    leftEarServo.detach();
    rightEarServo.detach();
    servosAttached = false;
  }
}

void setEars(int leftAngle, int rightAngle) {
  markServoMoved();
  lastLeftAngle = clampEarAngleLeft(leftAngle);
  lastRightAngle = clampEarAngleRight(rightAngle);
  leftEarServo.write(lastLeftAngle);
  rightEarServo.write(lastRightAngle);
}

void moveEarsSmooth(int leftTarget, int rightTarget, int step, int delayMs) {
  int targetL = clampEarAngleLeft(leftTarget);
  int targetR = clampEarAngleRight(rightTarget);

  while (lastLeftAngle != targetL || lastRightAngle != targetR) {
    if (lastLeftAngle < targetL) lastLeftAngle = min(lastLeftAngle + step, targetL);
    else if (lastLeftAngle > targetL) lastLeftAngle = max(lastLeftAngle - step, targetL);

    if (lastRightAngle < targetR) lastRightAngle = min(lastRightAngle + step, targetR);
    else if (lastRightAngle > targetR) lastRightAngle = max(lastRightAngle - step, targetR);

    setEars(lastLeftAngle, lastRightAngle);
    delay(delayMs);
  }
}

void setupServos() {
  leftEarServo.attach(SERVO_LEFT_EAR);
  rightEarServo.attach(SERVO_RIGHT_EAR);
  servosAttached = true;
  centerEars();
  lastServoMove = millis();
  Serial.println("🔧 Ear servos initialized");
}

void centerEars() {
  moveEarsSmooth(EAR_CENTER, EAR_CENTER, 3, 10);
}

void playStartupAnimation() {
  Serial.println("🎬 Startup animation");
  moveEarsSmooth(EAR_UP, EAR_UP, 3, 12);
  delay(200);
  moveEarsSmooth(EAR_DOWN, EAR_DOWN, 3, 12);
  delay(200);
  centerEars();
}

void playConnectedAnimation() {
  for (int i = 0; i < 3; i++) {
    moveEarsSmooth(EAR_UP, EAR_DOWN, 3, 10);
    delay(120);
    moveEarsSmooth(EAR_DOWN, EAR_UP, 3, 10);
    delay(120);
  }
  centerEars();
}

void playDisconnectedAnimation() {
  setEars(EAR_DOWN, EAR_DOWN);
  delay(400);
  centerEars();
}

void playMovingEars() {
  setEars(clampEarAngleLeft(EAR_CENTER + 8), clampEarAngleRight(EAR_CENTER + 8));
}

void playAlertEars() {
  setEars(EAR_ALERT, EAR_ALERT);
  delay(400);
  centerEars();
}

void playIdleEarAnimation() {
  int anim = random(0, 4);
  switch (anim) {
    case 0: // Both ears twitch
      moveEarsSmooth(EAR_CENTER + 12, EAR_CENTER + 12, 4, 10);
      delay(150);
      centerEars();
      break;
    case 1: // Left ear twitch
      moveEarsSmooth(EAR_UP, lastRightAngle, 4, 10);
      delay(150);
      moveEarsSmooth(EAR_CENTER, lastRightAngle, 4, 10);
      break;
    case 2: // Right ear twitch
      moveEarsSmooth(lastLeftAngle, EAR_UP, 4, 10);
      delay(150);
      moveEarsSmooth(lastLeftAngle, EAR_CENTER, 4, 10);
      break;
    case 3: // Listening pose
      moveEarsSmooth(EAR_CENTER + 16, EAR_CENTER - 8, 3, 12);
      delay(250);
      centerEars();
      break;
  }
}

void wiggleEars() {
  for (int i = 0; i < 2; i++) {
    moveEarsSmooth(EAR_UP, EAR_DOWN, 4, 10);
    delay(150);
    moveEarsSmooth(EAR_DOWN, EAR_UP, 4, 10);
    delay(150);
  }
  centerEars();
}

// ===== BLE FUNCTIONS (Avatar App) =====
#ifdef USE_AVATAR_APP
void setupBLE() {
  // Use "ESP32_Robot_Car" to match Avatar app expectations
  BLEDevice::init(BLE_DEVICE_NAME);
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pTxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pTxCharacteristic->addDescriptor(new BLE2902());
  
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_RX,
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
  
  Serial.println("🔧 BLE initialized - Advertising as 'ESP32_Robot_Car'");
}

void handleBLEConnection() {
  // Handle reconnection
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("📡 Restarting BLE advertising...");
    oldDeviceConnected = deviceConnected;
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}

void sendBLEMessage(String message) {
  if (deviceConnected) {
    pTxCharacteristic->setValue(message.c_str());
    pTxCharacteristic->notify();
    delay(10);
  }
}
#else
// Dummy functions when not using Avatar App
void setupBLE() {}
void handleBLEConnection() {}
void sendBLEMessage(String message) { Serial.println(message); }
#endif

// ===== COMMAND PROCESSING =====
void processCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();
  
  // Single character motor commands (from Avatar app)
  if (cmd.length() == 1) {
    char c = cmd.charAt(0);
    switch (c) {
      case 'F':
        moveForward(MOTOR_SPEED);
        break;
      case 'B':
        moveBackward(MOTOR_SPEED);
        break;
      case 'L':
        turnLeft(TURN_SPEED);
        break;
      case 'R':
        turnRight(TURN_SPEED);
        break;
      case 'S':
        stopMotors();
        break;
      default:
        Serial.println("❓ Unknown command: " + cmd);
    }
    return;
  }
  
  // Extended commands
  if (cmd.startsWith("SPEED:")) {
    int speed = cmd.substring(6).toInt();
    speed = constrain(speed, 0, 255);
    Serial.printf("⚡ Speed set to %d\n", speed);
    // Apply to current movement
    if (isMoving) {
      ledcWrite(pwmChannelA, speed);
      ledcWrite(pwmChannelB, speed);
    }
  }
  else if (cmd == "EARS:WIGGLE") {
    wiggleEars();
  }
  else if (cmd == "EARS:UP") {
    leftEarServo.write(EAR_UP);
    rightEarServo.write(EAR_UP);
  }
  else if (cmd == "EARS:DOWN") {
    leftEarServo.write(EAR_DOWN);
    rightEarServo.write(EAR_DOWN);
  }
  else if (cmd == "EARS:CENTER") {
    centerEars();
  }
  else if (cmd == "EARS:ALERT") {
    playAlertEars();
  }
  else if (cmd == "SENSORS") {
    // Report sensor distances
    String report = String("F:") + String((int)distFront) + 
                    " B:" + String((int)distBack) +
                    " L:" + String((int)distLeft) +
                    " R:" + String((int)distRight);
    sendBLEMessage(report);
    Serial.println("📏 " + report);
  }
  else if (cmd == "AVOID:ON") {
    obstacleAvoidanceEnabled = true;
    Serial.println("🛡️ Obstacle avoidance ON");
    sendBLEMessage("Obstacle avoidance ON");
  }
  else if (cmd == "AVOID:OFF") {
    obstacleAvoidanceEnabled = false;
    Serial.println("⚠️ Obstacle avoidance OFF");
    sendBLEMessage("Obstacle avoidance OFF");
  }
  else if (cmd == "DABBLE:ON") {
    dabbleMode = true;
    Serial.println("🎮 Dabble mode ON");
    sendBLEMessage("Manual control ON");
  }
  else if (cmd == "DABBLE:OFF") {
    dabbleMode = false;
    stopMotors();
    Serial.println("🎮 Dabble mode OFF");
    sendBLEMessage("Manual control OFF");
  }
  else if (cmd.startsWith("JOY:")) {
    // Joystick control: JOY:x,y (values -100 to 100)
    int commaIndex = cmd.indexOf(',');
    if (commaIndex > 4) {
      dabbleX = cmd.substring(4, commaIndex).toInt();
      dabbleY = cmd.substring(commaIndex + 1).toInt();
    }
  }
  else if (cmd == "HAPPY") {
    playConnectedAnimation();
    sendBLEMessage("😊 Happy!");
  }
  else if (cmd == "SAD") {
    playDisconnectedAnimation();
    sendBLEMessage("😢 Sad");
  }
  else if (cmd == "PING") {
    sendBLEMessage("PONG");
  }
  else if (cmd == "STATUS") {
    String status = String("Connected:") + (deviceConnected ? "Y" : "N") +
                    " Moving:" + (isMoving ? "Y" : "N") +
                    " Dir:" + currentDirection +
                    " Avoid:" + (obstacleAvoidanceEnabled ? "Y" : "N");
    sendBLEMessage(status);
  }
  else {
    Serial.println("❓ Unknown command: " + cmd);
  }
}

// ===== DABBLE/JOYSTICK CONTROL =====
#ifdef USE_DABBLE_APP
void handleDabbleControl() {
  Dabble.processInput();
  
  // D-Pad - simple direct control at FULL SPEED
  if (GamePad.isUpPressed()) {
    setLeftMotor(true, MOTOR_SPEED);
    setRightMotor(true, MOTOR_SPEED);
    isMoving = true;
    currentDirection = 'F';
  } 
  else if (GamePad.isDownPressed()) {
    setLeftMotor(false, MOTOR_SPEED);
    setRightMotor(false, MOTOR_SPEED);
    isMoving = true;
    currentDirection = 'B';
  } 
  else if (GamePad.isLeftPressed()) {
    setLeftMotor(false, MOTOR_SPEED);
    setRightMotor(true, MOTOR_SPEED);
    isMoving = true;
    currentDirection = 'L';
  } 
  else if (GamePad.isRightPressed()) {
    setLeftMotor(true, MOTOR_SPEED);
    setRightMotor(false, MOTOR_SPEED);
    isMoving = true;
    currentDirection = 'R';
  }
  else if (GamePad.isSquarePressed() || GamePad.isCrossPressed() || 
           GamePad.isCirclePressed() || GamePad.isTrianglePressed()) {
    stopMotors();
  }
  else {
    // No button - check joystick
    int joyY = GamePad.getYaxisData();
    int joyX = GamePad.getXaxisData();
    
    if (abs(joyY) > 1 || abs(joyX) > 1) {
      // Joystick active - calculate speeds
      int leftSpeed = map(joyY, -7, 7, -255, 255) + map(joyX, -7, 7, -100, 100);
      int rightSpeed = map(joyY, -7, 7, -255, 255) - map(joyX, -7, 7, -100, 100);
      
      leftSpeed = constrain(leftSpeed, -255, 255);
      rightSpeed = constrain(rightSpeed, -255, 255);
      
      // Set left motor
      if (leftSpeed >= 0) {
        setLeftMotor(true, leftSpeed);
      } else {
        setLeftMotor(false, -leftSpeed);
      }
      
      // Set right motor
      if (rightSpeed >= 0) {
        setRightMotor(true, rightSpeed);
      } else {
        setRightMotor(false, -rightSpeed);
      }
      
      isMoving = true;
    }
    else if (isMoving) {
      stopMotors();
    }
  }
}
#else
void handleDabbleControl() {}
#endif

// Handle JOY: commands from Avatar app
#ifdef USE_AVATAR_APP
void handleJoystickCommand() {
  if (dabbleX == 0 && dabbleY == 0) {
    if (isMoving && dabbleMode) stopMotors();
    return;
  }
  
  int baseSpeed = map(dabbleY, -100, 100, -255, 255);
  int turnAmount = map(dabbleX, -100, 100, -128, 128);
  
  int leftSpeed = baseSpeed + turnAmount;
  int rightSpeed = baseSpeed - turnAmount;
  
  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);
  
  if (baseSpeed > 0 && obstacleAvoidanceEnabled && checkFrontObstacle()) {
    leftSpeed = 0;
    rightSpeed = 0;
  }
  if (baseSpeed < 0 && obstacleAvoidanceEnabled && checkBackObstacle()) {
    leftSpeed = 0;
    rightSpeed = 0;
  }
  
  setMotorSpeeds(leftSpeed, rightSpeed);
}
#else
void handleJoystickCommand() {} // Dummy when not using Avatar
#endif
