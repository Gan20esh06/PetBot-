# Quick Start: ESP32 Robot Car in 5 Minutes ⚡

## 1️⃣ Upload Code (2 minutes)

### Open Arduino IDE

1. Open `ESP32_Car_BLE.ino`
2. Select **Tools → Board → ESP32 Dev Module**
3. Select **Tools → Port → [Your COM Port]**
4. Click **Upload** button (→)
5. Wait for "Done uploading"

### Check Serial Monitor

1. Open **Tools → Serial Monitor**
2. Set baud rate: **115200**
3. You should see:
   ```
   ESP32 BLE Robot Car Ready!
   Waiting for connection...
   Device name: ESP32_Robot_Car
   ```

---

## 2️⃣ Wire Hardware (3 minutes)

### Minimal Wiring

```
ESP32 Pin    →    L298N Pin
---------         ---------
GPIO 16      →    IN1
GPIO 17      →    IN2
GPIO 18      →    IN3
GPIO 19      →    IN4
GND          →    GND

L298N        →    Motors
-----             ------
OUT1, OUT2   →    Left Motor
OUT3, OUT4   →    Right Motor

Battery      →    L298N
-------            -----
+ (7-12V)    →    12V
- (GND)      →    GND
```

**Power ESP32:**

- Via USB cable (for testing), OR
- Via external 5V regulator from battery

---

## 3️⃣ Connect & Test

### From Avatar Web Interface

1. Open: http://127.0.0.1:5500/avatar_ai.html
2. Click **"📶 Connect ESP32"** button
3. Select **"ESP32_Robot_Car"** in the popup
4. Click **Pair**
5. Button changes to **"✅ ESP32 Connected"**

### Voice Test

```
Say: "Hey Robot"
Say: "Move forward"
→ Car should move forward!

Say: "Stop"
→ Car should stop!
```

---

## 🆘 Quick Fixes

### "ESP32 not found in Bluetooth"

```bash
1. Press ESP32 reset button
2. Refresh browser page
3. Try again
```

### "Motors not moving"

```bash
1. Check battery charged?
2. L298N enable jumpers installed?
3. Check wiring (especially GND)
4. Test Serial Monitor: type 'F' then Enter
```

### "Wrong direction"

Swap wires:

- Forward/Backward: Swap IN1↔IN2 or IN3↔IN4
- Left/Right: Swap entire motor connections

---

## 🎯 Voice Commands

| Say This                     | Car Does    |
| ---------------------------- | ----------- |
| "Hey Robot" → "Move forward" | ⬆️ Forward  |
| "Hey Robot" → "Go back"      | ⬇️ Backward |
| "Hey Robot" → "Turn left"    | ⬅️ Left     |
| "Hey Robot" → "Turn right"   | ➡️ Right    |
| "Hey Robot" → "Stop"         | 🛑 Stop     |

---

## 📋 Checklist

- [ ] Arduino IDE installed
- [ ] ESP32 board support added
- [ ] Code uploaded successfully
- [ ] Serial Monitor shows "Ready!"
- [ ] Hardware wired correctly
- [ ] Battery connected and charged
- [ ] ESP32 appears in Bluetooth devices
- [ ] Connected from web interface
- [ ] Motors respond to commands

**All checked? You're ready to drive! 🎉**

---

**Need detailed instructions?** → See `ESP32_CAR_SETUP.md`
