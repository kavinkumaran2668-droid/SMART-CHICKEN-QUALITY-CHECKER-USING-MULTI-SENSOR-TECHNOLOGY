# Wiring Guide — ESP32

Refer to the complete pin tables and electrical precautions in `README.md`.

## Main connections

```text
MQ-135 AO  -> voltage divider / level shift -> GPIO34
DHT22 DATA -> GPIO4
TCS3200 S0 -> GPIO16
TCS3200 S1 -> GPIO17
TCS3200 S2 -> GPIO18
TCS3200 S3 -> GPIO19
TCS3200 OUT -> GPIO27

OLED SDA -> GPIO21
OLED SCL -> GPIO22

Green LED  -> GPIO25 through 220 ohm resistor
Yellow LED -> GPIO26 through 220 ohm resistor
Red LED    -> GPIO32 through 220 ohm resistor
Buzzer     -> GPIO33 (driver transistor if needed)
```

All module grounds must be connected to ESP32 GND.

## Voltage warning

ESP32 GPIOs are 3.3V-only and are not 5V tolerant. Many MQ-135 boards run from 5V and their analog output can exceed 3.3V. Use a correctly designed voltage divider or level shifter on AO, and verify the voltage before connecting GPIO34. Check your specific TCS3200 board output level too.
