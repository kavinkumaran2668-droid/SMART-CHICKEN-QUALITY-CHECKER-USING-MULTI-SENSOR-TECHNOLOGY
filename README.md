# Smart Chicken Quality Checker Using Multi-Sensor Technology

An ESP32-based educational prototype that reads gas-sensor, temperature/humidity and colour-sensor values to provide a preliminary chicken-sample indication. The project displays readings and a prototype status on an OLED, with LED and buzzer alerts.

> **Food-safety notice:** This project is not a certified food-safety device and cannot confirm that chicken is safe or unsafe to eat. MQ-135 is a broad gas sensor, not a selective ammonia/spoilage detector. Sensor readings depend on calibration, sample handling, distance, temperature, humidity, airflow and sensor ageing. Do not taste or serve food based on this prototype's result. Follow food-safety rules and use validated laboratory/microbiological methods where safety decisions are required.

## Features

- ESP32 microcontroller
- MQ-135 analog gas reading
- DHT22 temperature and humidity reading
- TCS3200 RGB colour-frequency reading
- SSD1306 128x64 I2C OLED display
- Green / yellow / red status LEDs
- Buzzer alert
- Serial Monitor output for calibration and debugging

## Hardware required

| Component | Quantity |
|---|---:|
| ESP32 DevKit V1 | 1 |
| MQ-135 gas sensor module | 1 |
| DHT22 temperature/humidity sensor | 1 |
| TCS3200 colour sensor module | 1 |
| SSD1306 I2C OLED 128x64 | 1 |
| Green LED | 1 |
| Yellow LED | 1 |
| Red LED | 1 |
| 220Ω LED resistors | 3 |
| Buzzer | 1 |
| Breadboard and jumper wires | As required |
| Suitable regulated power supply | 1 |

## Pin connections (ESP32 DevKit V1)

### Sensors

| Module pin | ESP32 pin |
|---|---|
| MQ-135 AO | GPIO 34 (through safe voltage divider) |
| MQ-135 VCC | Module-rated supply; see voltage warning |
| MQ-135 GND | GND |
| DHT22 DATA | GPIO 4 |
| DHT22 VCC | 3.3V |
| DHT22 GND | GND |
| TCS3200 S0 | GPIO 16 |
| TCS3200 S1 | GPIO 17 |
| TCS3200 S2 | GPIO 18 |
| TCS3200 S3 | GPIO 19 |
| TCS3200 OUT | GPIO 27 |
| TCS3200 VCC | 3.3V (if supported by your module) |
| TCS3200 GND | GND |

### OLED (I2C)

| OLED pin | ESP32 pin |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

### LEDs and buzzer

| Component | ESP32 pin |
|---|---|
| Green LED anode (through 220Ω) | GPIO 25 |
| Yellow LED anode (through 220Ω) | GPIO 26 |
| Red LED anode (through 220Ω) | GPIO 32 |
| Buzzer signal (use driver transistor if needed) | GPIO 33 |
| LED cathodes / buzzer GND | GND |

All grounds must be common.

## Important electrical precautions

1. **ESP32 GPIO pins are not 5V tolerant. Never connect a 5V sensor output directly to an ESP32 pin.**
2. Many MQ-135 modules use a 5V heater and may produce an analog output above 3.3V. Power the module according to its specification and use a correctly calculated voltage divider or level-shifting circuit on AO before GPIO34. Check the voltage with a multimeter.
3. DHT22 data may need a 4.7kΩ–10kΩ pull-up to 3.3V if your module does not include one.
4. Check the exact TCS3200 breakout-board supply and output specifications. Keep its output within ESP32-safe levels.
5. Do not drive a high-current buzzer directly from an ESP32 pin. Use a transistor driver and suitable resistor if required.
6. Use a stable supply. MQ sensor heaters draw appreciable current; do not assume the ESP32 3.3V pin can power every module.

## Arduino IDE setup

1. Install Arduino IDE.
2. Install ESP32 board support using Boards Manager (Espressif Systems).
3. Select **ESP32 Dev Module** and the correct COM port.
4. Install these libraries through Library Manager:
   - **DHT sensor library** by Adafruit
   - **Adafruit Unified Sensor**
   - **Adafruit GFX Library**
   - **Adafruit SSD1306**
5. Open `smart_chicken_quality_checker.ino`.
6. Upload to the ESP32.
7. Open Serial Monitor at **115200 baud**.

## How the program works

1. ESP32 reads the MQ-135 analog output.
2. DHT22 provides ambient temperature and relative humidity.
3. TCS3200 provides red, green and blue frequency readings.
4. The sketch compares the gas reading and a simple colour ratio against editable demonstration thresholds.
5. The OLED displays readings and a preliminary status.
6. LEDs and buzzer indicate the prototype status.

Possible displayed statuses:
- `FRESH*`
- `CHECK SOON`
- `SPOILAGE ALERT`
- `SENSOR ERROR`

The asterisk is intentional: `FRESH*` means only that the current readings did not cross the demonstration thresholds. It does **not** mean food is safe.

## Calibration

Open the Serial Monitor and record readings for repeatable samples under identical conditions. The example thresholds in the sketch are placeholders and must be tuned for your exact hardware and test method.

Edit these constants near the top of the `.ino` file:

```cpp
const int MQ_NEAR_THRESHOLD = 1800;
const int MQ_SPOILED_THRESHOLD = 2600;

const float COLOUR_WARNING_RATIO = 1.35;
const float COLOUR_SPOILED_RATIO = 1.65;
```

The MQ-135 reading is an ESP32 ADC count (nominally 0–4095), not a calibrated gas concentration. The colour ratio is an experimental feature, not a universal chicken freshness metric. Collect multiple readings, control sensor-to-sample distance and lighting, and document your calibration procedure.

## Repository structure

```text
smart-chicken-quality-checker/
├── smart_chicken_quality_checker.ino
├── README.md
├── wiring.md
├── LICENSE
└── .gitignore
```

## Limitations and future improvements

- MQ-135 is non-specific and affected by several gases and environmental conditions.
- A single RGB ratio cannot reliably identify chicken freshness.
- Ambient DHT22 readings do not measure the internal temperature of the chicken.
- Add validated reference datasets and laboratory comparison before making performance claims.
- Consider a selective gas sensor array, controlled sample chamber, repeatable measurement procedure, data logging and a properly validated classification model.

## Project flow

```text
Chicken Sample
      |
      v
MQ-135 + DHT22 + TCS3200
      |
      v
ESP32 Data Acquisition
      |
      v
Threshold-Based Demonstration Logic
      |
      v
OLED + LEDs + Buzzer
```
