/*
  SMART CHICKEN QUALITY CHECKER USING MULTI-SENSOR TECHNOLOGY
  Board: ESP32 DevKit V1 (3.3V logic)

  Sensors:
    - MQ-135 gas sensor module (analog output)
    - DHT22 temperature and humidity sensor
    - TCS3200 colour sensor

  Outputs:
    - 0.96-inch I2C OLED (SSD1306, 128x64)
    - Green LED: FRESH (prototype indication)
    - Yellow LED: CONSUME SOON (prototype indication)
    - Red LED: SPOILED (prototype indication)
    - Buzzer

  IMPORTANT:
  This is an educational prototype, not a certified food-safety instrument.
  MQ-135 is a broad gas sensor and is NOT selective for ammonia or chicken
  spoilage. Thresholds and colour limits below are starting values only.
  Calibrate using your own sensor, enclosure, distance, lighting and samples.
  Never use this device alone to decide whether chicken is safe to eat.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- Pin configuration ----------------
// ESP32 DevKit V1
const int MQ135_PIN = 34;   // ADC1 input-only pin
const int DHT_PIN   = 4;
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// TCS3200 pins
const int TCS_S0  = 16;
const int TCS_S1  = 17;
const int TCS_S2  = 18;
const int TCS_S3  = 19;
const int TCS_OUT = 27;

// Indicator outputs
const int GREEN_LED  = 25;
const int YELLOW_LED = 26;
const int RED_LED    = 32;
const int BUZZER_PIN = 33;

// ---------------- Calibration settings ----------------
// Adjust these after observing your own sensor readings.
// ESP32 ADC readings are normally 0 to 4095.
const int MQ_NEAR_THRESHOLD   = 1800;
const int MQ_SPOILED_THRESHOLD = 2600;

// Starting colour-ratio thresholds; calibrate for your setup.
// Colour ratio = (red frequency + blue frequency) / (2 * green frequency).
const float COLOUR_WARNING_RATIO = 1.35;
const float COLOUR_SPOILED_RATIO = 1.65;

const unsigned long SENSOR_INTERVAL_MS = 2000;

// ---------------- Data ----------------
float temperatureC = NAN;
float humidity = NAN;
int gasRaw = 0;
unsigned long lastRead = 0;

enum QualityState {
  FRESH,
  CONSUME_SOON,
  SPOILED,
  SENSOR_ERROR
};

QualityState quality = SENSOR_ERROR;

struct RGBReading {
  unsigned long red;
  unsigned long green;
  unsigned long blue;
};

void setColourFilter(bool s2, bool s3) {
  digitalWrite(TCS_S2, s2);
  digitalWrite(TCS_S3, s3);
}

unsigned long readColourFrequency(bool s2, bool s3) {
  setColourFilter(s2, s3);
  delay(60);
  // pulseIn timeout prevents the program from waiting indefinitely.
  unsigned long pulse = pulseIn(TCS_OUT, LOW, 100000UL);
  if (pulse == 0) return 0;
  return 1000000UL / pulse;
}

RGBReading readTCS3200() {
  RGBReading rgb;
  // TCS3200 filter selection:
  // Red: S2=LOW,  S3=LOW
  // Blue: S2=LOW, S3=HIGH
  // Green: S2=HIGH, S3=HIGH
  rgb.red   = readColourFrequency(LOW, LOW);
  rgb.blue  = readColourFrequency(LOW, HIGH);
  rgb.green = readColourFrequency(HIGH, HIGH);
  return rgb;
}

void showMessage(const char* line1, const char* line2 = "") {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 12);
  display.println(line1);
  display.setCursor(0, 30);
  display.println(line2);
  display.display();
}

void setIndicators(QualityState state) {
  digitalWrite(GREEN_LED, state == FRESH);
  digitalWrite(YELLOW_LED, state == CONSUME_SOON);
  digitalWrite(RED_LED, state == SPOILED);

  if (state == SPOILED) {
    tone(BUZZER_PIN, 1800, 500);
  } else if (state == CONSUME_SOON) {
    tone(BUZZER_PIN, 1000, 150);
  } else {
    noTone(BUZZER_PIN);
  }
}

const char* stateText(QualityState state) {
  switch (state) {
    case FRESH:        return "FRESH*";
    case CONSUME_SOON: return "CHECK SOON";
    case SPOILED:      return "SPOILAGE ALERT";
    default:           return "SENSOR ERROR";
  }
}

QualityState classifySample(int gas, float temp, float rh, RGBReading rgb) {
  // Missing DHT data or missing colour pulse is treated as an error.
  if (isnan(temp) || isnan(rh) ||
      rgb.red == 0 || rgb.green == 0 || rgb.blue == 0) {
    return SENSOR_ERROR;
  }

  float colourRatio =
      ((float)rgb.red + (float)rgb.blue) / (2.0f * (float)rgb.green);

  // This is a simple demonstration rule, NOT a scientifically validated
  // freshness model. Tune after collecting reference readings.
  bool gasHigh = gas >= MQ_SPOILED_THRESHOLD;
  bool gasRaised = gas >= MQ_NEAR_THRESHOLD;
  bool colourBad = colourRatio >= COLOUR_SPOILED_RATIO;
  bool colourChanged = colourRatio >= COLOUR_WARNING_RATIO;

  // Temperature/humidity are shown as context. They are not used alone
  // to infer freshness because ambient readings do not establish food safety.
  (void)temp;
  (void)rh;

  if (gasHigh || (gasRaised && colourBad)) {
    return SPOILED;
  }

  if (gasRaised || colourChanged) {
    return CONSUME_SOON;
  }

  return FRESH;
}

void displayReadings(RGBReading rgb) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("CHICKEN QUALITY CHECK");

  display.setCursor(0, 13);
  display.print("Gas ADC: ");
  display.println(gasRaw);

  display.setCursor(0, 24);
  display.print("Temp: ");
  display.print(temperatureC, 1);
  display.print(" C  RH: ");
  display.print(humidity, 0);
  display.println("%");

  display.setCursor(0, 35);
  display.print("R:");
  display.print(rgb.red);
  display.print(" G:");
  display.print(rgb.green);

  display.setCursor(0, 45);
  display.print("B:");
  display.println(rgb.blue);

  display.setCursor(0, 56);
  display.print("Result: ");
  display.print(stateText(quality));

  display.display();
}

void printSerialReadings(RGBReading rgb) {
  float colourRatio = (rgb.green == 0) ? 0 :
    ((float)rgb.red + (float)rgb.blue) / (2.0f * (float)rgb.green);

  Serial.println("\n------ SENSOR READINGS ------");
  Serial.print("MQ-135 ADC: "); Serial.println(gasRaw);
  Serial.print("Temperature: "); Serial.print(temperatureC); Serial.println(" C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
  Serial.print("Red frequency: "); Serial.println(rgb.red);
  Serial.print("Green frequency: "); Serial.println(rgb.green);
  Serial.print("Blue frequency: "); Serial.println(rgb.blue);
  Serial.print("Colour ratio: "); Serial.println(colourRatio, 3);
  Serial.print("Prototype result: "); Serial.println(stateText(quality));
  Serial.println("-----------------------------");
}

void setup() {
  Serial.begin(115200);

  pinMode(TCS_S0, OUTPUT);
  pinMode(TCS_S1, OUTPUT);
  pinMode(TCS_S2, OUTPUT);
  pinMode(TCS_S3, OUTPUT);
  pinMode(TCS_OUT, INPUT);

  // 20% output-frequency scaling for TCS3200
  digitalWrite(TCS_S0, HIGH);
  digitalWrite(TCS_S1, LOW);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  dht.begin();
  analogReadResolution(12);
  analogSetPinAttenuation(MQ135_PIN, ADC_11db);

  Wire.begin(21, 22); // SDA, SCL for ESP32
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED not detected. Check wiring/address.");
  } else {
    showMessage("Chicken Quality", "Warming up sensors...");
  }

  Serial.println("Smart Chicken Quality Checker started.");
  Serial.println("Allow MQ-135 to warm up as recommended by its manufacturer.");
}

void loop() {
  if (millis() - lastRead < SENSOR_INTERVAL_MS) return;
  lastRead = millis();

  gasRaw = analogRead(MQ135_PIN);
  temperatureC = dht.readTemperature();
  humidity = dht.readHumidity();

  RGBReading rgb = readTCS3200();
  quality = classifySample(gasRaw, temperatureC, humidity, rgb);

  setIndicators(quality);
  displayReadings(rgb);
  printSerialReadings(rgb);
}
