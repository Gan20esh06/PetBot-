# Wiring Diagram - ESP32 Robot Car

```
                           ESP32 Robot Car Wiring
                          ========================

                              [ESP32 Board]
                                   |
                    _______________|_______________
                   |                               |
              (Power via USB)              (Bluetooth Connection)
                   |                            To Browser App
                   |
         __________|__________
        |                     |
    GPIO Pins            Common GND ────────┐
        |                                    |
        |                                    |
    ┌───┴──────────────────────┐            |
    │   GPIO 16 ──> IN1       │            |
    │   GPIO 17 ──> IN2       │            |
    │   GPIO 18 ──> IN3       │        [L298N GND]
    │   GPIO 19 ──> IN4       │            |
    └─────────────┬────────────┘            |
                  │                         |
                  │                         |
              [L298N Motor Driver]          |
                  │                         |
        ┌─────────┴─────────┐               |
        │                   │               |
    [OUT1 OUT2]         [OUT3 OUT4]         |
        │                   │               |
        │                   │               |
   [Left Motor]        [Right Motor]        |
    (2 wires)            (2 wires)          |
        │                   │               |
        │                   │               |
        └───────────┬───────┘               |
                    │                       │
                [Battery Pack]              │
                 (7-12V DC)                 │
                    │                       │
              ┌─────┴─────┐                 │
              │           │                 │
           [+ Positive] [- Negative]────────┘
              │
              │
         [L298N 12V Pin]
```

## Pin Connection Table

| ESP32 GPIO | L298N Pin | Function         |
| ---------- | --------- | ---------------- |
| GPIO 16    | IN1       | Motor A Forward  |
| GPIO 17    | IN2       | Motor A Backward |
| GPIO 18    | IN3       | Motor B Forward  |
| GPIO 19    | IN4       | Motor B Backward |
| GND        | GND       | Common Ground    |

| L298N | Motor       | Wire      |
| ----- | ----------- | --------- |
| OUT1  | Left Motor  | Red (+)   |
| OUT2  | Left Motor  | Black (-) |
| OUT3  | Right Motor | Red (+)   |
| OUT4  | Right Motor | Black (-) |

| Battery   | Connection         |
| --------- | ------------------ |
| + (7-12V) | L298N 12V terminal |
| - (GND)   | L298N GND terminal |

---

## Visual Layout

```
     [Front of Car]
         ▲
         │
    ┌────┴────┐
    │         │
    │  ESP32  │ ← USB Cable for programming
    │         │
    └─────────┘
         │
         │ (4 wires: IN1, IN2, IN3, IN4 + GND)
         │
    ┌─────────┐
    │ L298N   │
    │ Driver  │ ← Battery (7-12V)
    └─────────┘
       │   │
       │   └─────┐
       │         │
    ┌──┴──┐   ┌──┴──┐
    │LEFT │   │RIGHT│
    │MOTOR│   │MOTOR│
    └─────┘   └─────┘

    ◄─────────────►
     Robot Chassis
```

---

## Detailed Component Layout

### ESP32 (Top View)

```
    ╔═══════════════════════════╗
    ║  [USB Port]               ║
    ║                           ║
    ║  GND ●   GPIO 16 ● ──┐    ║
    ║          GPIO 17 ● ──┤    ║
    ║          GPIO 18 ● ──┤    ║
    ║          GPIO 19 ● ──┘    ║
    ║                           ║
    ║      [Reset Button]       ║
    ╚═══════════════════════════╝
```

### L298N Motor Driver (Top View)

```
    ╔═══════════════════════════════╗
    ║  [12V] [GND] ← Battery        ║
    ║                               ║
    ║  IN1 ●  ← GPIO 16             ║
    ║  IN2 ●  ← GPIO 17             ║
    ║  IN3 ●  ← GPIO 18             ║
    ║  IN4 ●  ← GPIO 19             ║
    ║                               ║
    ║  [ENA Jumper] [ENB Jumper]    ║
    ║                               ║
    ║  OUT1 OUT2    OUT3 OUT4       ║
    ║    └─┬─┘        └─┬─┘         ║
    ║      │            │           ║
    ║   [Left]       [Right]        ║
    ║   Motor        Motor          ║
    ╚═══════════════════════════════╝
```

---

## Color Code (Standard Motor Wires)

| Color        | Connection                      |
| ------------ | ------------------------------- |
| Red          | Motor +                         |
| Black        | Motor -                         |
| Yellow/Green | (Usually not used in DC motors) |

**Note:** If motors spin wrong direction:

- Swap the two wires of that motor (Red ↔ Black)

---

## Safety Checklist ✓

- [ ] Double-check polarity (+ and -)
- [ ] GND connected between ESP32 and L298N
- [ ] Battery voltage is 7-12V (not more!)
- [ ] **Never connect 12V to ESP32 GPIO pins**
- [ ] ENA/ENB jumpers installed on L298N
- [ ] All connections tight and secure
- [ ] Motors can spin freely (not blocked)

---

## Power Options

### Option 1: USB Power (Testing Only)

```
Computer USB ──> ESP32
Battery 7-12V ──> L298N only
```

**Good for:** Initial testing, low power consumption
**Limitation:** ESP32 and motors on separate power

### Option 2: Battery Power (Recommended)

```
Battery 7-12V ──> L298N 12V
L298N 5V out ──> ESP32 5V/VIN
```

**Good for:** Mobile operation
**Requires:** L298N with 5V regulator output

### Option 3: Dual Power (Most Stable)

```
Battery 7-12V ──> L298N 12V (motors)
Separate 5V ──> ESP32 5V (logic)
Common GND between both
```

**Good for:** High power motors
**Best stability**

---

**Ready to wire? Follow the diagram and use the checklist!**
