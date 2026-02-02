# ESP32 Robot Car Setup Guide 🚗

## What You'll Need

### Hardware

- ESP32 Development Board
- L298N Motor Driver or similar
- 2 DC Motors (for 2-wheel drive) or 4 DC Motors (for 4-wheel drive)
- Robot car chassis
- Battery pack (7.4V - 12V recommended)
- Jumper wires
- USB cable for programming

### Software

- Arduino IDE (latest version)
- ESP32 board support in Arduino IDE

---

## Step 1: Install Arduino IDE & ESP32 Support

### Install Arduino IDE

1. Download from: https://www.arduino.cc/en/software
2. Install and open Arduino IDE

### Add ESP32 Board Support

1. Open Arduino IDE
2. Go to **File → Preferences**
3. In "Additional Board Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click **OK**
5. Go to **Tools → Board → Boards Manager**
6. Search for "esp32"
7. Install "esp32 by Espressif Systems"
8. Wait for installation to complete

---

## Step 2: Hardware Wiring

### ESP32 to L298N Motor Driver Connections

```
ESP32          L298N Motor Driver
-----          ------------------
GPIO 16   -->  IN1 (Motor A Forward)
GPIO 17   -->  IN2 (Motor A Backward)
GPIO 18   -->  IN3 (Motor B Forward)
GPIO 19   -->  IN4 (Motor B Backward)
GND       -->  GND
```

### L298N to Motors

```
Motor A (Left Side):
  OUT1 --> Left Motor +
  OUT2 --> Left Motor -

Motor B (Right Side):
  OUT3 --> Right Motor +
  OUT4 --> Right Motor -
```

### Power Connections

```
Battery (+7.4V to 12V) --> L298N 12V
Battery GND            --> L298N GND
                       --> ESP32 GND (common ground)
```

**Important:**

- Connect ESP32 to computer via USB for programming
- Or use external power regulator (7.4V-12V to 5V) connected to ESP32 VIN pin
- **Never connect 12V directly to ESP32 pins!**

---

## Step 3: Upload Code to ESP32

### 3.1 Open Arduino IDE

### 3.2 Select Board

1. Go to **Tools → Board → ESP32 Arduino**
2. Select your ESP32 board (e.g., "ESP32 Dev Module")

### 3.3 Select Port

1. Connect ESP32 to computer via USB
2. Go to **Tools → Port**
3. Select the COM port (Windows) or /dev/tty\* (Mac/Linux)

### 3.4 Copy the Arduino Code

- The code is in `ESP32_Car_BLE.ino` (created below)

### 3.5 Upload

1. Click the **Upload** button (→) in Arduino IDE
2. Wait for "Done uploading"
3. Open **Serial Monitor** (Tools → Serial Monitor)
4. Set baud rate to **115200**
5. You should see: "ESP32 BLE Robot Car Ready!"

---

## Step 4: Connect from Web Avatar

### 4.1 Open the Avatar in Browser

- Navigate to: http://127.0.0.1:5500/avatar_ai.html
- Or open directly: `file:///d:/CBP/app/src/main/assets/avatar_ai.html`

### 4.2 Connect to ESP32

1. Click the **"📶 Connect ESP32"** button in settings panel
2. Browser will show Bluetooth device picker
3. Select your ESP32 device (should show "ESP32_Robot_Car")
4. Click **Pair**

### 4.3 Test Connection

- Once connected, button will show **"✅ ESP32 Connected"**
- Check the log console (click "Show Log") for confirmation

---

## Step 5: Control Your Car with Voice

### Voice Commands

Say "Hey Robot" to wake up, then say:

| Command                                   | Action             |
| ----------------------------------------- | ------------------ |
| "Move forward" or "Go ahead"              | Car moves forward  |
| "Move backward" or "Go back" or "Reverse" | Car moves backward |
| "Turn left"                               | Car turns left     |
| "Turn right"                              | Car turns right    |
| "Stop" or "Halt"                          | Car stops          |

### Example Conversation

```
You: "Hey Robot"
Avatar: [Listening indicator shows]

You: "Move forward"
Avatar: [Processes] "Moving forward!"
Car: Moves forward

You: "Stop"
Avatar: [Processes] "Stopping!"
Car: Stops
```

---

## Step 6: Troubleshooting

### ESP32 Not Showing in Bluetooth Picker

- Check ESP32 is powered on (LED should blink)
- Make sure code uploaded successfully
- Check Serial Monitor - should show "BLE Started"
- Restart ESP32 (press reset button)
- Refresh browser page and try again

### Car Not Moving

1. **Check Wiring**: Verify all connections match diagram
2. **Check Power**: Battery fully charged?
3. **Check Serial Monitor**: Commands received? (Should show "Received: F/B/L/R/S")
4. **Check Motor Driver**:
   - L298N enable jumpers installed?
   - LED indicators on driver board?
5. **Swap Motor Wires**: If moving wrong direction, swap motor +/- wires

### Car Moves Wrong Direction

- Open `ESP32_Car_BLE.ino`
- In `moveForward()` function, swap HIGH/LOW values
- Same for `moveBackward()`, `turnLeft()`, `turnRight()`

### Bluetooth Connection Drops

- Check ESP32 power supply (voltage stable?)
- Keep avatar browser tab in foreground
- Reduce distance between device and ESP32
- Check for interference from other Bluetooth devices

### One Motor Not Working

- Check that specific motor's wiring
- Test motor directly with battery (does it spin?)
- Check L298N connection for that motor
- Verify ESP32 GPIO pins not damaged

---

## Step 7: Testing Without Voice (Manual Test)

### Using Serial Monitor

1. Open Arduino IDE Serial Monitor
2. Type commands:
   - `F` = Forward
   - `B` = Backward
   - `L` = Left
   - `R` = Right
   - `S` = Stop
3. Send each command to test motors

### Using Browser Console

1. Open browser DevTools (F12)
2. Go to Console tab
3. Type:
   ```javascript
   sendCommand("F"); // Forward
   sendCommand("B"); // Backward
   sendCommand("L"); // Left
   sendCommand("R"); // Right
   sendCommand("S"); // Stop
   ```

---

## Advanced Tips

### Adjust Motor Speed

In Arduino code, look for:

```cpp
int motorSpeed = 200;  // 0-255 (adjust this value)
```

- Lower value = slower
- Higher value = faster
- Maximum: 255

### Add PWM Speed Control

Uncomment the PWM lines in the Arduino code:

```cpp
// ledcWrite(0, motorSpeed);  // Uncomment these
// ledcWrite(1, motorSpeed);
```

### Extend Bluetooth Range

- Use ESP32 with external antenna
- Increase `CONFIG.scanTimeout` in settings
- Keep line-of-sight between device and ESP32

### Add More Commands

Edit `extractMotorCommand()` in `avatar_ai.html`:

```javascript
if (lowerText.includes("spin")) {
  return "X"; // Custom command
}
```

Then handle in Arduino:

```cpp
case 'X':
  spin();
  break;
```

---

## Safety Tips ⚠️

1. **Always test with motors OFF first** (disconnect motor power)
2. **Use emergency stop** - keep "Stop" command ready
3. **Test in open area** - away from obstacles
4. **Battery safety**:
   - Don't over-discharge
   - Don't short circuit
   - Use proper voltage rating
5. **Secure wiring** - prevent shorts during movement
6. **Add emergency cutoff switch** on battery connection

---

## Next Steps

✅ Upload Arduino code
✅ Wire up hardware
✅ Test motors individually
✅ Connect via Bluetooth
✅ Test voice commands
✅ Drive your robot car!

## Need Help?

- Check Serial Monitor for error messages
- Enable "Show Log" in avatar for detailed debug info
- Verify each connection with multimeter
- Test each component separately before combining

---

**Ready to get started? Upload the Arduino code below!** 🚀
