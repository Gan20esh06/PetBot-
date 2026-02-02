# ESP32 Servo Test - Quick Setup 🎯

## What You'll Need

- ESP32 Dev Board
- 1 Servo Motor (SG90 or any standard servo)
- Jumper wires (3 wires)
- USB cable
- Optional: External 5V power supply (if servo draws too much current)

---

## Step 1: Install Servo Library

### In Arduino IDE:

1. Go to **Sketch → Include Library → Manage Libraries**
2. Search for **"ESP32Servo"**
3. Install **"ESP32Servo by Kevin Harrington"**
4. Wait for installation to complete

---

## Step 2: Wire the Servo

### Simple 3-Wire Connection

```
Servo Wire Color    →    ESP32 Pin
---------------          ---------
Orange/Yellow (Signal) → GPIO 18
Red (Power 5V)         → 5V
Brown/Black (GND)      → GND
```

### Visual Diagram

```
     [ESP32 Board]
         │
    ┌────┴────┐
    │ GPIO 18 ├─── Orange/Yellow ──┐
    │   5V    ├─── Red            │
    │   GND   ├─── Brown/Black    │
    └─────────┘                   │
                                  │
                            [Servo Motor]
                            3 wires on top
```

### Important Notes:

- ⚠️ **If servo jitters or ESP32 resets**: Use external 5V power supply
- Connect external 5V to servo Red wire
- Connect external GND to both servo Brown wire AND ESP32 GND
- ESP32 signal (GPIO 18) still connects to servo Orange wire

---

## Step 3: Upload Code

1. Open **ESP32_Servo_BLE_Test.ino** in Arduino IDE
2. Select **Tools → Board → ESP32 Dev Module**
3. Select **Tools → Port → [Your COM Port]**
4. Click **Upload** (→)
5. Wait for "Done uploading"

---

## Step 4: Test Servo (Before Bluetooth)

### Open Serial Monitor

1. **Tools → Serial Monitor**
2. Set baud rate: **115200**
3. You should see:
   ```
   ESP32 BLE Servo Test Ready!
   Device name: ESP32_Servo_Test
   ```

### Manual Test via Serial Monitor

Type these commands in Serial Monitor and press Enter:

```
F   → Servo moves to 0° (Forward)
L   → Servo moves to 45° (Left)
S   → Servo moves to 90° (Center)
R   → Servo moves to 135° (Right)
B   → Servo moves to 180° (Backward)
```

**Does servo move?** ✅ Continue to Step 5
**Servo not moving?** ⚠️ Check wiring, try external power

---

## Step 5: Connect via Bluetooth

### From Avatar Web Interface

1. Open browser: http://127.0.0.1:5500/avatar_ai.html
2. Click **"📶 Connect ESP32"** button
3. Look for **"ESP32_Servo_Test"** in the list
4. Click **Pair**
5. Button changes to **"✅ ESP32 Connected"**
6. Servo should move to center (90°) as confirmation

---

## Step 6: Test with Voice

### Voice Commands

```
Say: "Hey Robot"
Say: "Move forward"
→ Servo moves to 0° ✅

Say: "Turn left"
→ Servo moves to 45° ✅

Say: "Stop"
→ Servo moves to 90° (center) ✅

Say: "Turn right"
→ Servo moves to 135° ✅

Say: "Move backward"
→ Servo moves to 180° ✅
```

---

## What Each Command Does

| Voice Command                | Servo Angle | Visual          |
| ---------------------------- | ----------- | --------------- |
| "Move forward" or "Go ahead" | 0°          | ⬆️ Full left    |
| "Turn left"                  | 45°         | ↖️ Slight left  |
| "Stop" or "Halt"             | 90°         | ➡️ Center       |
| "Turn right"                 | 135°        | ↗️ Slight right |
| "Move backward" or "Go back" | 180°        | ⬇️ Full right   |

---

## Troubleshooting

### Servo Not Moving

```
✅ Check wiring (especially GND connection)
✅ Try external 5V power supply
✅ Test in Serial Monitor first (type F, L, S, R, B)
✅ Check servo is working (test with another project)
```

### Servo Jitters or Twitches

```
✅ Use external 5V power (ESP32 USB power not enough)
✅ Add 100µF capacitor between servo 5V and GND
✅ Keep servo wire short (under 30cm)
✅ Use shielded cable if available
```

### ESP32 Keeps Resetting

```
✅ Servo drawing too much current
✅ MUST use external 5V power supply
✅ Don't power servo from ESP32 5V pin
```

### Bluetooth Not Connecting

```
✅ Check Serial Monitor shows "BLE Started"
✅ Device name should be "ESP32_Servo_Test"
✅ Press ESP32 reset button
✅ Refresh browser page
```

### Wrong Angles

Modify angles in the code:

```cpp
case 'F': moveServo(0, "Forward");    // Change 0 to your desired angle
case 'L': moveServo(45, "Left");      // Change 45 to your desired angle
case 'S': moveServo(90, "Center");    // Change 90 to your desired angle
case 'R': moveServo(135, "Right");    // Change 135 to your desired angle
case 'B': moveServo(180, "Backward"); // Change 180 to your desired angle
```

---

## External Power Wiring (Recommended)

```
[5V Power Supply]
      │
      ├─── (+5V) ──> Servo Red Wire
      │
      └─── (GND) ──> Common GND ──┬──> Servo Brown Wire
                                  │
                                  └──> ESP32 GND

[ESP32 GPIO 18] ──> Servo Orange/Yellow Wire
```

---

## Advanced: Test Multiple Servos

### For 2 Servos:

```cpp
Servo servo1;
Servo servo2;

servo1.attach(18);  // GPIO 18
servo2.attach(19);  // GPIO 19

// In moveServo():
servo1.write(angle);
servo2.write(angle);
```

---

## Next Steps After Testing

Once servo test works perfectly:

1. ✅ You confirmed Bluetooth works
2. ✅ You confirmed voice commands work
3. ✅ You understand the wiring
4. ✅ Ready to build the car with motors!

**Now upload `ESP32_Car_BLE.ino` and wire the motors!** 🚗

---

## Quick Command Reference

### Serial Monitor Commands

```
F = Forward (0°)
L = Left (45°)
S = Stop/Center (90°)
R = Right (135°)
B = Backward (180°)
```

### Voice Commands

```
"Hey Robot" + "Move forward" = 0°
"Hey Robot" + "Turn left" = 45°
"Hey Robot" + "Stop" = 90°
"Hey Robot" + "Turn right" = 135°
"Hey Robot" + "Move backward" = 180°
```

---

**Happy Testing! 🎯**
