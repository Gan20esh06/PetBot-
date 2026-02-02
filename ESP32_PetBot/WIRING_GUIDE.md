# ESP32 PetBot Wiring Guide

## Components

- ESP32 DevKit V1
- TB6612FNG Dual Motor Driver
- 2x N20 150 RPM DC Motors
- 4x HC-SR04 Ultrasonic Sensors
- 2x SG90 Servo Motors (Ears)
- 7.4V Li-Po Battery (2S) or 6x AA batteries
- 5V Regulator (for servos)

## Pin Connections

### TB6612FNG Motor Driver

```
TB6612FNG    →    ESP32
─────────────────────────
STBY         →    GPIO 27
AIN1         →    GPIO 26
AIN2         →    GPIO 25
PWMA         →    GPIO 32
BIN1         →    GPIO 33
BIN2         →    GPIO 14
PWMB         →    GPIO 12
VM           →    Battery + (7.4V)
VCC          →    ESP32 3.3V
GND          →    GND (common)
AO1          →    Left Motor +
AO2          →    Left Motor -
BO1          →    Right Motor +
BO2          →    Right Motor -
```

### Ultrasonic Sensors (HC-SR04)

```
Sensor       TRIG    ECHO
───────────────────────────
Front        GPIO 5   GPIO 18
Back         GPIO 19  GPIO 21
Left         GPIO 22  GPIO 23
Right        GPIO 4   GPIO 15

Power: VCC → 5V, GND → GND
```

### Servo Motors (Ears)

```
Servo            →    ESP32
────────────────────────────
Left Ear Signal  →    GPIO 13
Right Ear Signal →    GPIO 2
VCC              →    5V (use separate 5V regulator!)
GND              →    GND (common)
```

### Power Distribution

```
┌─────────────┐
│  Battery    │
│  7.4V 2S    │
└──────┬──────┘
       │
       ├──────► TB6612FNG VM
       │
       ▼
   ┌───────────┐
   │ 5V Buck   │
   │ Regulator │
   └─────┬─────┘
         │
         ├──────► Ultrasonic VCC (all 4)
         ├──────► Servo VCC (both)
         └──────► ESP32 VIN (or use USB)
```

## Wiring Diagram (ASCII)

```
                    ┌─────────────────────────────────────┐
                    │           ESP32 DevKit              │
                    │                                     │
   Left Ear ◄───────┤ GPIO13                    GPIO27 ├──────► TB6612 STBY
  Right Ear ◄───────┤ GPIO2                     GPIO26 ├──────► TB6612 AIN1
                    │                           GPIO25 ├──────► TB6612 AIN2
US Front Trig ◄─────┤ GPIO5                     GPIO32 ├──────► TB6612 PWMA
US Front Echo ──────┤ GPIO18                    GPIO33 ├──────► TB6612 BIN1
                    │                           GPIO14 ├──────► TB6612 BIN2
US Back Trig ◄──────┤ GPIO19                    GPIO12 ├──────► TB6612 PWMB
US Back Echo ───────┤ GPIO21                           │
                    │                                  │
US Left Trig ◄──────┤ GPIO22                     3.3V ├──────► TB6612 VCC
US Left Echo ───────┤ GPIO23                      GND ├──────► GND Common
                    │                                  │
US Right Trig ◄─────┤ GPIO4                            │
US Right Echo ──────┤ GPIO15                           │
                    └─────────────────────────────────────┘
```

## Motor Direction Notes

For differential drive with 200mm base:

- **Forward**: Both motors spin same direction
- **Backward**: Both motors spin opposite direction
- **Left Turn**: Left motor backward, Right motor forward
- **Right Turn**: Left motor forward, Right motor backward

If your robot moves in the wrong direction, swap the motor wires or change the IN1/IN2 logic in code.

## Troubleshooting

### Motors not spinning

1. Check STBY pin is HIGH (GPIO 27)
2. Check battery voltage (should be 6-12V on VM)
3. Verify motor connections to AO1/AO2 and BO1/BO2

### Wrong direction

- Swap motor + and - wires
- Or swap IN1 and IN2 pins in code

### Ultrasonic not reading

- Ensure 5V power (not 3.3V!)
- Check TRIG and ECHO wires
- Keep sensors away from each other

### Servos jittering

- Use separate 5V supply for servos
- Don't power servos from ESP32 3.3V
- Add 100µF capacitor near servo power

### BLE not connecting

- Ensure no other device is connected
- Restart ESP32
- Clear Bluetooth cache on phone

## Commands Reference

| Command       | Description                  |
| ------------- | ---------------------------- |
| F             | Move forward                 |
| B             | Move backward                |
| L             | Turn left                    |
| R             | Turn right                   |
| S             | Stop                         |
| SPEED:xxx     | Set speed (0-255)            |
| EARS:WIGGLE   | Wiggle ears                  |
| EARS:UP       | Ears up                      |
| EARS:DOWN     | Ears down                    |
| EARS:CENTER   | Center ears                  |
| SENSORS       | Get distance readings        |
| AVOID:ON/OFF  | Toggle obstacle avoidance    |
| DABBLE:ON/OFF | Toggle manual joystick mode  |
| JOY:x,y       | Joystick input (-100 to 100) |
| STATUS        | Get robot status             |
| PING          | Connection test              |
