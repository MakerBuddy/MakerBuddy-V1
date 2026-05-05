/*
 * MakerBuddy V1.x - ESP32 IoT Development Kit
 *
 * Website: https://makerbuddy.cc
 * Email: info@makerbuddy.cc
 * Version: 1.2
 * Release Date: 20 Apr 2026
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "MakerBuddyTypes.h"
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <DHT.h>
#include <DHT_U.h>
#include <DNSServer.h>
#include <DallasTemperature.h>
// ESP32Servo removed — LEDC is driven directly to avoid channel/timer conflicts
// with analogWrite() which uses channels 0-7. Servo uses its own dedicated channel.
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <Preferences.h>
#include <Ticker.h>
#include <Update.h>
#include <WiFi.h>
#include <Wire.h>
#include "esp_wifi.h"

// Firmware Version
#define FIRMWARE_VERSION "1.2"

// WiFi credentials - will be overridden by saved preferences
const char *ssid = "";
const char *password = "";

// WiFi Configuration
String hotspotSSID = "";
const char *ap_password = "12345678";
String deviceName = "";

// OTA Update variables
bool otaInProgress = false;
int otaProgress = 0;
bool otaRequested = false;
String otaFirmwareURL = "";

// Pin definitions
#define PUSH_BUTTON_PIN 5
#define LDR_PIN 35
#define SINGLE_LED_PIN 12
#define RGB_LED_R_PIN 13
#define RGB_LED_G_PIN 14
#define RGB_LED_B_PIN 27
#define POTENTIOMETER_PIN 34
#define BUZZER_PIN 4
#define ULTRASONIC_ECHO_PIN 2
#define ULTRASONIC_TRIG_PIN 15
#define DS18B20_PIN 26
#define DHT22_PIN 25
#define MQ2_PIN 33
#define SOIL_MOISTURE_PIN 32
#define PIR_PIN 18
#define SERVO_PIN 19
#define RELAY_PIN 23

// Servo LEDC — Arduino-ESP32 v3.x pin-based API (no explicit channel numbers).
// Each pin gets its own internal LEDC channel, so there is no timer conflict
// with analogWrite() channels used by the RGB/LED pins.
#define SERVO_LEDC_FREQ  50    // 50 Hz = 20 ms period
#define SERVO_LEDC_RES   16    // 16-bit for fine duty steps
#define SERVO_US_MIN     500   // pulse width at 0°
#define SERVO_US_MAX     2500  // pulse width at 180°

// Component initialization
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);
DHT_Unified dht(DHT22_PIN, DHT11);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Preferences preferences;

// LED Control Variables
struct LEDState {
  bool state = false;
  int pwm = 255;
  bool blinking = false;
} ledControl;

// RGB LED Control Variables
struct RGBState {
  int r = 0, g = 0, b = 0;
} rgbControl;

// Global variables
bool relayState = false;
int servoPosition = 0;
bool lcdDefaultMode = true;
String customLcdText1 = "";
String customLcdText2 = "";
bool isConfigMode = false;
bool wifiResetRequested = false;

// Potentiometer Mapping Variables
enum PotMappingMode : uint8_t {
  PM_NONE = 0,
  PM_LED = 1,
  PM_RGB = 2,
  PM_SERVO = 3
};
uint8_t potMappingMode = PM_NONE;
int potMappedValue = 0;

int soilMoistureDryRaw = 3000;
int soilMoistureWetRaw = 1200;

// WebSocket telemetry timing
unsigned long lastWsSensorPushMs = 0;
unsigned long lastWsCleanupMs = 0;
const unsigned long WS_SENSOR_PUSH_INTERVAL_MS = 2000;
const unsigned long WS_CLEANUP_INTERVAL_MS = 10000;

// WiFi scan cache (populated once on AP start, never updated again)
String wifiNetworksCache = "";

// LED Blinking variables
unsigned long ledBlinkTimer = 0;
bool ledBlinkState = false;

// Ticker objects for timer-based execution
Ticker sensorTicker;
Ticker lcdTicker;
Ticker ledBlinkTicker;
Ticker buzzerTicker;

// Flags for timer callbacks
volatile bool readSensorsFlag = false;
volatile bool updateLCDFlag = false;
volatile bool handleLEDBlinkFlag = false;
volatile bool saveSequencesFlag = false;
volatile bool buzzerOffFlag = false;

SeqAction seqActionFromString(const String &s) {
  if (s == "led_on")
    return SA_LED_ON;
  if (s == "led_off")
    return SA_LED_OFF;
  if (s == "led_pwm")
    return SA_LED_PWM;
  if (s == "rgb_red")
    return SA_RGB_RED;
  if (s == "rgb_green")
    return SA_RGB_GREEN;
  if (s == "rgb_blue")
    return SA_RGB_BLUE;
  if (s == "rgb_yellow")
    return SA_RGB_YELLOW;
  if (s == "rgb_purple")
    return SA_RGB_PURPLE;
  if (s == "rgb_cyan")
    return SA_RGB_CYAN;
  if (s == "rgb_white")
    return SA_RGB_WHITE;
  if (s == "rgb_off")
    return SA_RGB_OFF;
  if (s == "buzzer_on")
    return SA_BUZZER_ON;
  if (s == "buzzer_off")
    return SA_BUZZER_OFF;
  if (s == "relay_on")
    return SA_RELAY_ON;
  if (s == "relay_off")
    return SA_RELAY_OFF;
  if (s == "servo_move")
    return SA_SERVO_MOVE;
  if (s == "lcd_display")
    return SA_LCD_DISPLAY;
  return SA_NONE;
}

const char *seqActionToString(SeqAction a) {
  switch (a) {
  case SA_LED_ON:
    return "led_on";
  case SA_LED_OFF:
    return "led_off";
  case SA_LED_PWM:
    return "led_pwm";
  case SA_RGB_RED:
    return "rgb_red";
  case SA_RGB_GREEN:
    return "rgb_green";
  case SA_RGB_BLUE:
    return "rgb_blue";
  case SA_RGB_YELLOW:
    return "rgb_yellow";
  case SA_RGB_PURPLE:
    return "rgb_purple";
  case SA_RGB_CYAN:
    return "rgb_cyan";
  case SA_RGB_WHITE:
    return "rgb_white";
  case SA_RGB_OFF:
    return "rgb_off";
  case SA_BUZZER_ON:
    return "buzzer_on";
  case SA_BUZZER_OFF:
    return "buzzer_off";
  case SA_RELAY_ON:
    return "relay_on";
  case SA_RELAY_OFF:
    return "relay_off";
  case SA_SERVO_MOVE:
    return "servo_move";
  case SA_LCD_DISPLAY:
    return "lcd_display";
  default:
    return "";
  }
}

SeqLcdMode seqLcdModeFromString(const String &s) {
  return (s == "default") ? SLM_LCD_DEFAULT : SLM_LCD_CUSTOM;
}

const char *seqLcdModeToString(SeqLcdMode m) {
  return (m == SLM_LCD_DEFAULT) ? "default" : "custom";
}

SeqLoopMode seqLoopModeFromString(const String &s) {
  if (s == "forever")
    return SLM_FOREVER;
  if (s == "timer")
    return SLM_TIMER;
  if (s == "condition")
    return SLM_CONDITION;
  return SLM_NONE;
}

const char *seqLoopModeToString(SeqLoopMode m) {
  switch (m) {
  case SLM_FOREVER:
    return "forever";
  case SLM_TIMER:
    return "timer";
  case SLM_CONDITION:
    return "condition";
  default:
    return "none";
  }
}

SeqTimerType seqTimerTypeFromString(const String &s) {
  if (s == "interval")
    return STT_INTERVAL;
  return STT_DURATION;
}

const char *seqTimerTypeToString(SeqTimerType t) {
  switch (t) {
  case STT_INTERVAL:
    return "interval";
  default:
    return "duration";
  }
}

SeqSensor seqSensorFromString(const String &s) {
  if (s == "humidity")
    return SS_HUMIDITY;
  if (s == "ds18b20Temp")
    return SS_DS18B20;
  if (s == "lightLevel")
    return SS_LIGHT;
  if (s == "gasLevel")
    return SS_GAS;
  if (s == "soilMoisture")
    return SS_SOIL;
  if (s == "distance")
    return SS_DISTANCE;
  if (s == "potValue")
    return SS_POT;
  if (s == "motionDetected")
    return SS_MOTION;
  if (s == "buttonPressed")
    return SS_BUTTON;
  return SS_TEMPERATURE;
}

const char *seqSensorToString(SeqSensor s) {
  switch (s) {
  case SS_HUMIDITY:
    return "humidity";
  case SS_DS18B20:
    return "ds18b20Temp";
  case SS_LIGHT:
    return "lightLevel";
  case SS_GAS:
    return "gasLevel";
  case SS_SOIL:
    return "soilMoisture";
  case SS_DISTANCE:
    return "distance";
  case SS_POT:
    return "potValue";
  case SS_MOTION:
    return "motionDetected";
  case SS_BUTTON:
    return "buttonPressed";
  default:
    return "temperature";
  }
}

SeqOp seqOpFromString(const String &s) {
  if (s == "<")
    return SOP_LT;
  if (s == ">=")
    return SOP_GTE;
  if (s == "<=")
    return SOP_LTE;
  if (s == "==")
    return SOP_EQ;
  return SOP_GT;
}

const char *seqOpToString(SeqOp o) {
  switch (o) {
  case SOP_LT:
    return "<";
  case SOP_GTE:
    return ">=";
  case SOP_LTE:
    return "<=";
  case SOP_EQ:
    return "==";
  default:
    return ">";
  }
}

SeqRangeOp seqRangeOpFromString(const String &s) {
  return (s == "OR") ? SROP_OR : SROP_AND;
}

const char *seqRangeOpToString(SeqRangeOp r) {
  return (r == SROP_OR) ? "OR" : "AND";
}

const int MAX_SEQUENCES = 10;
Sequence sequences[MAX_SEQUENCES];
int sequenceCount = 0;

const float MAX_ULTRASONIC_DISTANCE_CM = 500.0;

// Sensor data structure
struct SensorData {
  float temperature;
  float humidity;
  float ds18b20Temp;
  int lightLevel;
  int potValue;
  int gasLevel;
  int soilMoisture;
  int soilRaw;
  float distance;
  bool motionDetected;
  bool buttonPressed;
} sensorData;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================
void setLEDManual(bool state, int pwm = 255, bool blink = false);
void updateLEDOutput();
void handleLEDBlinking();
bool getCurrentLEDState();
int getCurrentLEDPWM();
bool getCurrentLEDBlinking();
void setRGBManual(int r, int g, int b);
void updateRGBOutput();
void writeServo(int angle);
uint32_t packRGB(int r, int g, int b);
int getCurrentRGBR();
int getCurrentRGBG();
int getCurrentRGBB();
void setupWebServer();
String getSensorDataJSONCompact();
String getDeviceInfoJSONCompact();
String getFirmwareInfoJSONCompact();
String getSequencesJSONCompact();
void wsMaybeBroadcastSensors();
void wsSendTextAll(const String &payload);
void wsSendText(AsyncWebSocketClient *client, const String &payload);
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len);
void checkWiFiReset();
void checkWiFiResetButton();
void connectToWiFi();
void startConfigMode();
void setupConfigServer();
void readSensors();
void processPotentiometerMapping();
void updateLCD();
void setRelayWithDelay(bool state);

int computeSoilMoisturePercent(int raw);
void loadSequences();
void saveSequences();
void clearSequenceNVS(int index);
void processSequences();
void executeSequenceStep(SequenceStep step);
void executeSequenceStepWithPriority(SequenceStep step, bool deviceLocked[]);
int getDeviceFromAction(SeqAction action);
void markSequenceDevices(Sequence &seq, bool deviceLocked[]);
bool evaluateOperator(float value, SeqOp op, float threshold);
bool evaluateGenericCondition(SeqSensor sensor, SeqOp op1, float thresh1,
                              bool useRange, SeqRangeOp rangeOp, SeqOp op2,
                              float thresh2);
float getSensorValueById(SeqSensor sensor);
String getSequencesJSON();
bool getDebouncedCondition(Sequence &seq, bool rawCondition);

// Timer callback functions
void sensorTimerCallback();
void lcdTimerCallback();
void ledBlinkTimerCallback();

// ============================================================================
// LED CONTROL FUNCTIONS
// ============================================================================

void setLEDManual(bool state, int pwm, bool blink) {
  ledControl.state = state;
  ledControl.pwm = constrain(pwm, 0, 255);
  ledControl.blinking = blink;

  if (!ledControl.blinking) {
    if (ledControl.state) {
      analogWrite(SINGLE_LED_PIN, ledControl.pwm);
    } else {
      analogWrite(SINGLE_LED_PIN, 0);
    }
  }
}

void handleLEDBlinking() {
  if (ledControl.blinking && ledControl.state) {
    if (millis() - ledBlinkTimer > 500) {
      ledBlinkState = !ledBlinkState;

      if (ledBlinkState) {
        analogWrite(SINGLE_LED_PIN, ledControl.pwm);
      } else {
        analogWrite(SINGLE_LED_PIN, 0);
      }

      ledBlinkTimer = millis();
    }
  }
}

bool getCurrentLEDState() { return ledControl.state; }

int getCurrentLEDPWM() { return ledControl.pwm; }

bool getCurrentLEDBlinking() { return ledControl.blinking; }

// ============================================================================
// RGB LED CONTROL FUNCTIONS
// ============================================================================

void setRGBManual(int r, int g, int b) {
  rgbControl.r = constrain(r, 0, 255);
  rgbControl.g = constrain(g, 0, 255);
  rgbControl.b = constrain(b, 0, 255);

  // For common anode RGB LED: 255 = OFF, 0 = ON
  int outputR = 255 - rgbControl.r;
  int outputG = 255 - rgbControl.g;
  int outputB = 255 - rgbControl.b;

  analogWrite(RGB_LED_R_PIN, outputR);
  analogWrite(RGB_LED_G_PIN, outputG);
  analogWrite(RGB_LED_B_PIN, outputB);
}

int getCurrentRGBR() { return rgbControl.r; }

int getCurrentRGBG() { return rgbControl.g; }

int getCurrentRGBB() { return rgbControl.b; }

// Write a servo angle (0-180°) via dedicated LEDC channel.
// Using direct LEDC instead of ESP32Servo avoids the timer-frequency conflict
// that occurs when analogWrite() (1000 Hz) and ESP32Servo (50 Hz) happen to
// share the same LEDC timer, causing the servo to receive ~75 µs pulses
// (at 1000 Hz period) instead of the expected 1500 µs — explaining the
// "moves only a little bit" symptom.
void writeServo(int angle) {
  angle = constrain(angle, 0, 180);
  uint32_t us = (uint32_t)map(angle, 0, 180, SERVO_US_MIN, SERVO_US_MAX);
  uint32_t duty = (uint32_t)((uint64_t)us * 65535UL / 20000UL);
  ledcWrite(SERVO_PIN, duty);
}

// ============================================================================
// MAIN SETUP AND LOOP
// ============================================================================

void setup() {
  Serial.begin(115200);

  preferences.begin("iot-kit", false);

  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SINGLE_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // Configure RGB LED pins as OUTPUT
  pinMode(RGB_LED_R_PIN, OUTPUT);
  pinMode(RGB_LED_G_PIN, OUTPUT);
  pinMode(RGB_LED_B_PIN, OUTPUT);

  // Set to OFF based on common anode (HIGH = OFF)
  Serial.println("Setting RGB to OFF (HIGH for common anode)...");
  analogWrite(RGB_LED_R_PIN, 255);
  analogWrite(RGB_LED_G_PIN, 255);
  analogWrite(RGB_LED_B_PIN, 255);

  dht.begin();
  ds18b20.begin();
  lcd.init();
  lcd.backlight();

  ledcAttach(SERVO_PIN, SERVO_LEDC_FREQ, SERVO_LEDC_RES);
  writeServo(servoPosition);

  // Initialize NFC reader
  Wire.begin();

  // Initialize WiFi driver in STA mode — required for a reliable STA→AP
  // transition in startConfigMode(). Must happen early in setup().
  WiFi.mode(WIFI_STA);
  delay(500); // wait for driver to fully start before reading MAC
  {
    uint8_t mac[6];
    WiFi.macAddress(mac); // now returns real MAC (driver is ready)
    char suffix[5];
    snprintf(suffix, sizeof(suffix), "%02X%02X", mac[4], mac[5]);
    hotspotSSID = "MakerBuddy_" + String(suffix);
  }
  Serial.println("Hotspot SSID: " + hotspotSSID);

  loadSequences();

  // Autostart sequences marked for boot
  for (int i = 0; i < sequenceCount; i++) {
    if (sequences[i].autostart && sequences[i].enabled) {
      if (sequences[i].manuallyStopped) {
        sequences[i].isRunning = false;
        sequences[i].currentStep = 0;
        continue;
      }
      // Condition-based sequences don't start immediately, they wait for
      // condition
      if (sequences[i].loopMode == SLM_CONDITION) {
        // Just mark as ready, processSequences() will handle starting based on
        // condition
        sequences[i].isRunning = false;
        sequences[i].currentStep = 0;
      } else {
        // All other sequences start immediately
        sequences[i].isRunning = true;
        sequences[i].currentStep = 0;
        sequences[i].lastStepTime = millis();
        if (sequences[i].loopMode == SLM_TIMER) {
          sequences[i].loopStartTime = millis();
          sequences[i].lastIntervalTime = millis();
        }
        // Execute first step immediately
        if (sequences[i].stepCount > 0) {
          executeSequenceStep(sequences[i].steps[0]);
        }
      }
    }
  }

  lcdDefaultMode = preferences.getBool("lcdDefault", true);
  customLcdText1 = preferences.getString("customText1", "");
  customLcdText2 = preferences.getString("customText2", "");
  uint8_t savedPotMap = preferences.getUChar("potMappingId", 255);
  if (savedPotMap <= PM_SERVO) {
    potMappingMode = savedPotMap;
  } else {
    String legacy = preferences.getString("potMapping", "none");
    if (legacy == "led")
      potMappingMode = PM_LED;
    else if (legacy == "rgb")
      potMappingMode = PM_RGB;
    else if (legacy == "servo")
      potMappingMode = PM_SERVO;
    else
      potMappingMode = PM_NONE;
    preferences.putUChar("potMappingId", potMappingMode);
    preferences.remove("potMapping");
  }

  // Force pot mapping to none on startup to prevent unexpected servo/jitter behavior
  potMappingMode = PM_NONE;

  soilMoistureDryRaw = preferences.getInt("soilDry", 3000);
  soilMoistureWetRaw = preferences.getInt("soilWet", 1200);
  soilMoistureDryRaw = constrain(soilMoistureDryRaw, 0, 4095);
  soilMoistureWetRaw = constrain(soilMoistureWetRaw, 0, 4095);

  lcd.setCursor(0, 0);
  lcd.print("MakerBuddy Kit");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  checkWiFiReset();
  connectToWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    // Load device name and setup mDNS
    deviceName = preferences.getString("deviceName", "");
    if (deviceName.length() > 0) {
      if (MDNS.begin(deviceName.c_str())) {
        Serial.println("mDNS responder started");
        Serial.print("Device accessible at: http://");
        Serial.print(deviceName);
        Serial.println(".local");
        MDNS.addService("http", "tcp", 80);
      } else {
        Serial.println("Error setting up mDNS responder!");
      }
    }

    setupWebServer();
    server.begin();
    Serial.println("Web server started");
  }

  // Start timer-based tasks
  sensorTicker.attach(1.0, sensorTimerCallback); // Read sensors every 1 second
  lcdTicker.attach(0.1, lcdTimerCallback);       // Update LCD every 100ms
  ledBlinkTimerCallback();
  ledBlinkTicker.attach(0.5, ledBlinkTimerCallback); // LED blink every 500ms

  Serial.println("Timers initialized");

  // Initialize RGB LED state (ensure it's off with values 0,0,0)
  Serial.printf("Initial RGB state: r=%d, g=%d, b=%d\n", rgbControl.r,
                rgbControl.g, rgbControl.b);
  setRGBManual(rgbControl.r, rgbControl.g, rgbControl.b);
  Serial.println("RGB initialized");
}

void loop() {
  // Check if OTA update is requested
  if (otaRequested) {
    otaRequested = false;
    performOTAUpdate(otaFirmwareURL);
    return; // If OTA fails, it will restart or continue
  }

  if (isConfigMode) {
    dnsServer.processNextRequest();
  }

  // Check WiFi reset button (non-blocking, runs every loop)
  checkWiFiResetButton();

  // Process flags set by timer callbacks
  if (readSensorsFlag) {
    readSensorsFlag = false;
    readSensors();
  }

  if (updateLCDFlag) {
    updateLCDFlag = false;
    updateLCD();
  }

  // Process sequences with priority: condition(1st) > timer(2nd) > forever(3rd)
  // > none(4th) Skip processing if we are about to save sequences (avoids race
  // condition)
  if (!saveSequencesFlag) {
    processSequences();
  }

  if (handleLEDBlinkFlag) {
    handleLEDBlinkFlag = false;
    handleLEDBlinking();
  }

  if (buzzerOffFlag) {
    buzzerOffFlag = false;
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Handle sequence saving in main loop context (prevent stack overflow/WDT
  // reset)
  if (saveSequencesFlag) {
    saveSequencesFlag = false;
    saveSequences();
  }

  if (millis() - lastWsCleanupMs > WS_CLEANUP_INTERVAL_MS) {
    lastWsCleanupMs = millis();
    ws.cleanupClients();
  }

}

// ============================================================================
// TIMER CALLBACK FUNCTIONS
// ============================================================================

void sensorTimerCallback() { readSensorsFlag = true; }

void lcdTimerCallback() { updateLCDFlag = true; }

void ledBlinkTimerCallback() { handleLEDBlinkFlag = true; }

void buzzerTimerCallback() { buzzerOffFlag = true; }

// ============================================================================
// RULE ENGINE WITH RGB SUPPORT AND LCD (Functions removed)
// ============================================================================

String replaceWildcards(String text) {
  // Replace sensor value wildcards with actual values
  text.replace("{temp}", String(sensorData.temperature, 1));
  text.replace("{humidity}", String(sensorData.humidity, 1));
  text.replace("{ds18b20}", String(sensorData.ds18b20Temp, 1));
  text.replace("{light}", String(sensorData.lightLevel));
  text.replace("{gas}", String(sensorData.gasLevel));
  text.replace("{soil}", String(sensorData.soilMoisture));
  text.replace("{distance}", String(sensorData.distance, 1));
  text.replace("{pot}", String(sensorData.potValue));
  text.replace("{motion}", sensorData.motionDetected ? "YES" : "NO");
  text.replace("{button}", sensorData.buttonPressed ? "YES" : "NO");

  return text;
}

// ============================================================================
// OTA UPDATE FUNCTION
// ============================================================================

void performOTAUpdate(String firmwareURL) {
  // Clear all preferences to ensure clean state after firmware update
  preferences.clear();

  // Stop all tickers to prevent LCD interference
  sensorTicker.detach();
  lcdTicker.detach();
  ledBlinkTicker.detach();

  otaInProgress = true;
  otaProgress = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("OTA Update");
  lcd.setCursor(0, 1);
  lcd.print("Downloading...");
  delay(1000);

  HTTPClient http;
  http.begin(firmwareURL);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    bool canBegin = Update.begin(contentLength);

    if (canBegin) {
      WiFiClient *client = http.getStreamPtr();
      size_t written = 0;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Installing...");

      uint8_t buff[128] = {0};
      int lastProgressPercent = -1;

      while (http.connected() && (written < contentLength)) {
        size_t available = client->available();
        if (available) {
          int bytesRead = client->readBytes(buff, min(available, sizeof(buff)));
          written += Update.write(buff, bytesRead);
          otaProgress = (written * 100) / contentLength;

          // Update LCD every 10%
          if (otaProgress != lastProgressPercent && otaProgress % 10 == 0) {
            lcd.setCursor(0, 1);
            lcd.print("Progress: ");
            lcd.print(otaProgress);
            lcd.print("%   ");
            lastProgressPercent = otaProgress;
          }
        }
        delay(1);
      }

      if (Update.end()) {
        if (Update.isFinished()) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Update Complete");
          lcd.setCursor(0, 1);
          lcd.print("Restarting...");
          delay(3000);
          ESP.restart();
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Update Failed!");
          lcd.setCursor(0, 1);
          lcd.print("Not Finished");
        }
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Update Error!");
        lcd.setCursor(0, 1);
        lcd.print("Code: " + String(Update.getError()));
      }
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("OTA Failed!");
      lcd.setCursor(0, 1);
      lcd.print("Not enough space");
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Download Failed");
    lcd.setCursor(0, 1);
    lcd.print("HTTP: " + String(httpCode));
  }

  http.end();
  otaInProgress = false;
  delay(5000);
}

// ============================================================================
// WEB SERVER SETUP
// ============================================================================

void setupWebServer() {
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Private-Network",
                                       "true");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods",
                                       "GET, POST, PUT, DELETE, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers",
                                       "Content-Type");

  server.on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response =
        request->beginResponseStream("application/json");
    StaticJsonDocument<512> doc;
    doc["te"] = sensorData.temperature;
    doc["hu"] = sensorData.humidity;
    doc["d8"] = sensorData.ds18b20Temp;
    doc["li"] = sensorData.lightLevel;
    doc["po"] = sensorData.potValue;
    doc["ga"] = sensorData.gasLevel;
    doc["sm"] = sensorData.soilMoisture;
    doc["sr"] = sensorData.soilRaw;
    doc["di"] = sensorData.distance;
    doc["md"] = sensorData.motionDetected;
    doc["bp"] = sensorData.buttonPressed;

    doc["ls"] = getCurrentLEDState();
    doc["lp"] = getCurrentLEDPWM();
    doc["lb"] = getCurrentLEDBlinking();

    JsonArray rgb = doc.createNestedArray("r");
    rgb.add(getCurrentRGBR());
    rgb.add(getCurrentRGBG());
    rgb.add(getCurrentRGBB());

    doc["rs"] = relayState;
    doc["sp"] = servoPosition;
    doc["lm"] = lcdDefaultMode ? 0 : 1;
    doc["fh"] = ESP.getFreeHeap();

    doc["pm"] = potMappingMode;
    doc["pv"] = potMappedValue;

    serializeJson(doc, *response);
    request->send(response);
  });

  server.on("/api/led", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("state", true)) {
      String stateStr = request->getParam("state", true)->value();
      bool state = (stateStr == "true");

      int pwm = 255;
      if (request->hasParam("pwm", true)) {
        pwm = request->getParam("pwm", true)->value().toInt();
        pwm = constrain(pwm, 0, 255);
      }

      bool blink = false;
      if (request->hasParam("blink", true)) {
        blink = request->getParam("blink", true)->value() == "true";
      }

      setLEDManual(state, pwm, blink);
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(400, "application/json",
                    "{\"error\":\"Missing state parameter\"}");
    }
  });

  server.on("/api/rgb", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("r", true) && request->hasParam("g", true) &&
        request->hasParam("b", true)) {
      int r = request->getParam("r", true)->value().toInt();
      int g = request->getParam("g", true)->value().toInt();
      int b = request->getParam("b", true)->value().toInt();

      setRGBManual(r, g, b);
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(400, "application/json",
                    "{\"error\":\"Missing RGB parameters\"}");
    }
  });

  server.on("/api/buzzer", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      if (state == "true") {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(500);
        digitalWrite(BUZZER_PIN, LOW);
      }
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(400, "application/json",
                    "{\"error\":\"Missing state parameter\"}");
    }
  });

  server.on("/api/relay", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      bool newState = (state == "true");

      setRelayWithDelay(newState);
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(400, "application/json",
                    "{\"error\":\"Missing state parameter\"}");
    }
  });

  server.on("/api/servo", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("position", true)) {
      servoPosition = request->getParam("position", true)->value().toInt();
      servoPosition = constrain(servoPosition, 0, 180);
      writeServo(servoPosition);
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(400, "application/json",
                    "{\"error\":\"Missing position parameter\"}");
    }
  });

  server.on("/api/lcd", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode", true)) {
      String mode = request->getParam("mode", true)->value();
      if (mode == "default") {
        lcdDefaultMode = true;
        preferences.putBool("lcdDefault", true);
        request->send(200, "application/json",
                      "{\"status\":\"success\",\"mode\":\"default\"}");
      } else if (mode == "custom" && request->hasParam("text1", true)) {
        customLcdText1 = request->getParam("text1", true)->value();
        customLcdText2 = request->getParam("text2", true)->value();
        lcdDefaultMode = false;
        preferences.putBool("lcdDefault", false);
        preferences.putString("customText1", customLcdText1);
        preferences.putString("customText2", customLcdText2);
        request->send(200, "application/json",
                      "{\"status\":\"success\",\"mode\":\"custom\"}");
      } else {
        request->send(400, "application/json",
                      "{\"error\":\"Invalid parameters\"}");
      }
    } else {
      request->send(400, "application/json",
                    "{\"error\":\"Missing mode parameter\"}");
    }
  });

  server.on("/api/sequences", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response =
        request->beginResponseStream("application/json");
    DynamicJsonDocument doc(6144);
    JsonArray seqArray = doc.createNestedArray("sequences");

    for (int i = 0; i < sequenceCount; i++) {
      JsonObject seqObj = seqArray.createNestedObject();
      seqObj["id"] = i;
      seqObj["enabled"] = sequences[i].enabled;
      seqObj["name"] = sequences[i].name;
      seqObj["loop"] = sequences[i].loop;
      seqObj["loopMode"] =
          seqLoopModeToString((SeqLoopMode)sequences[i].loopMode);
      seqObj["autostart"] = sequences[i].autostart;
      seqObj["isRunning"] = sequences[i].isRunning;

      if (sequences[i].loopMode == SLM_TIMER) {
        seqObj["timerType"] =
            seqTimerTypeToString((SeqTimerType)sequences[i].timerType);
        seqObj["loopDuration"] = sequences[i].loopDuration;

      } else if (sequences[i].loopMode == SLM_CONDITION) {
        seqObj["loopSensor"] =
            seqSensorToString((SeqSensor)sequences[i].loopSensor);
        seqObj["loopOperator"] =
            seqOpToString((SeqOp)sequences[i].loopOperator);
        seqObj["loopThreshold"] = sequences[i].loopThreshold;
        seqObj["loopUseRange"] = sequences[i].loopUseRange;
        seqObj["manuallyStopped"] = sequences[i].manuallyStopped;
        if (sequences[i].loopUseRange) {
          seqObj["loopRangeOperator"] =
              seqRangeOpToString((SeqRangeOp)sequences[i].loopRangeOperator);
          seqObj["loopOperator2"] =
              seqOpToString((SeqOp)sequences[i].loopOperator2);
          seqObj["loopThreshold2"] = sequences[i].loopThreshold2;
        }
      }

      int safeStepCount = sequences[i].stepCount;
      if (safeStepCount < 0)
        safeStepCount = 0;
      if (safeStepCount > 10)
        safeStepCount = 10;
      JsonArray stepsArray = seqObj.createNestedArray("steps");
      for (int j = 0; j < safeStepCount; j++) {
        JsonObject stepObj = stepsArray.createNestedObject();
        stepObj["action"] =
            seqActionToString((SeqAction)sequences[i].steps[j].action);
        stepObj["actionValue"] = sequences[i].steps[j].actionValue;
        stepObj["delay"] = sequences[i].steps[j].delayMs;
        if (sequences[i].steps[j].action == SA_LCD_DISPLAY) {
          stepObj["lcdMode"] =
              seqLcdModeToString((SeqLcdMode)sequences[i].steps[j].lcdMode);
          stepObj["lcdText1"] = sequences[i].steps[j].lcdText1;
          stepObj["lcdText2"] = sequences[i].steps[j].lcdText2;
        }
      }
    }
    serializeJson(doc, *response);
    request->send(response);
  });

  server.on("/api/sequences", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("action", true)) {
      String action = request->getParam("action", true)->value();

      if (action == "add" && sequenceCount < MAX_SEQUENCES) {
        Sequence newSeq;
        newSeq.enabled =
            request->hasParam("enabled", true)
                ? request->getParam("enabled", true)->value() == "true"
                : true;

        String nameStr = request->hasParam("name", true)
                             ? request->getParam("name", true)->value()
                             : "New Sequence";
        strncpy(newSeq.name, nameStr.c_str(), 20);
        newSeq.name[20] = 0;

        newSeq.autostart =
            request->hasParam("autostart", true)
                ? request->getParam("autostart", true)->value() == "true"
                : false;

        newSeq.loopMode = (uint8_t)seqLoopModeFromString(
            request->hasParam("loopMode", true)
                ? request->getParam("loopMode", true)->value()
                : "none");
        newSeq.loop = (newSeq.loopMode == SLM_FOREVER);

        newSeq.timerType = (uint8_t)seqTimerTypeFromString(
            request->hasParam("timerType", true)
                ? request->getParam("timerType", true)->value()
                : "duration");
        newSeq.loopDuration =
            request->hasParam("loopDuration", true)
                ? request->getParam("loopDuration", true)->value().toInt()
                : 0;
        newSeq.loopStartTime = 0;
        newSeq.lastIntervalTime = 0;

        newSeq.manuallyStopped = false;
        newSeq.loopSensor = (uint8_t)seqSensorFromString(
            request->hasParam("loopSensor", true)
                ? request->getParam("loopSensor", true)->value()
                : "temperature");
        newSeq.loopOperator = (uint8_t)seqOpFromString(
            request->hasParam("loopOperator", true)
                ? request->getParam("loopOperator", true)->value()
                : ">");
        newSeq.loopThreshold =
            request->hasParam("loopThreshold", true)
                ? request->getParam("loopThreshold", true)->value().toFloat()
                : 0.0;
        newSeq.loopUseRange =
            request->hasParam("loopUseRange", true)
                ? request->getParam("loopUseRange", true)->value() == "true"
                : false;
        newSeq.loopRangeOperator = (uint8_t)seqRangeOpFromString(
            request->hasParam("loopRangeOperator", true)
                ? request->getParam("loopRangeOperator", true)->value()
                : "AND");
        newSeq.loopOperator2 = (uint8_t)seqOpFromString(
            request->hasParam("loopOperator2", true)
                ? request->getParam("loopOperator2", true)->value()
                : ">");
        newSeq.loopThreshold2 =
            request->hasParam("loopThreshold2", true)
                ? request->getParam("loopThreshold2", true)->value().toFloat()
                : 0.0;

        newSeq.currentStep = 0;
        newSeq.isRunning = false;
        newSeq.lastStepTime = 0;

        int requestedStepCount =
            request->hasParam("stepCount", true)
                ? request->getParam("stepCount", true)->value().toInt()
                : 0;
        if (requestedStepCount < 0)
          requestedStepCount = 0;
        if (requestedStepCount > 10)
          requestedStepCount = 10;
        newSeq.stepCount = requestedStepCount;

        for (int i = 0; i < newSeq.stepCount; i++) {
          String actionParam = "step" + String(i) + "_action";
          String valueParam = "step" + String(i) + "_value";
          String delayParam = "step" + String(i) + "_delay";

          if (request->hasParam(actionParam.c_str(), true)) {
            newSeq.steps[i].action = (uint8_t)seqActionFromString(
                request->getParam(actionParam.c_str(), true)->value());
            newSeq.steps[i].actionValue =
                request->hasParam(valueParam.c_str(), true)
                    ? request->getParam(valueParam.c_str(), true)
                          ->value()
                          .toInt()
                    : 0;
            long parsedDelay = request->hasParam(delayParam.c_str(), true)
                                   ? request->getParam(delayParam.c_str(), true)
                                         ->value()
                                         .toInt()
                                   : 1000;
            if (parsedDelay < 0)
              parsedDelay = 0;
            newSeq.steps[i].delayMs = (unsigned long)parsedDelay;
            String lcdModeParam = "step" + String(i) + "_lcdMode";
            String lcdText1Param = "step" + String(i) + "_lcdText1";
            String lcdText2Param = "step" + String(i) + "_lcdText2";
            newSeq.steps[i].lcdMode = (uint8_t)seqLcdModeFromString(
                request->hasParam(lcdModeParam.c_str(), true)
                    ? request->getParam(lcdModeParam.c_str(), true)->value()
                    : "custom");

            String t1 =
                request->hasParam(lcdText1Param.c_str(), true)
                    ? request->getParam(lcdText1Param.c_str(), true)->value()
                    : "";
            String t2 =
                request->hasParam(lcdText2Param.c_str(), true)
                    ? request->getParam(lcdText2Param.c_str(), true)->value()
                    : "";

            strncpy(newSeq.steps[i].lcdText1, t1.c_str(), 16);
            strncpy(newSeq.steps[i].lcdText2, t2.c_str(), 16);
            newSeq.steps[i].lcdText1[16] = 0;
            newSeq.steps[i].lcdText2[16] = 0;
          }
        }

        sequences[sequenceCount] = newSeq;
        sequenceCount++;
        saveSequencesFlag = true;

        request->send(
            200, "application/json",
            "{\"status\":\"success\",\"message\":\"Sequence added\"}");
      } else if (action == "delete" && request->hasParam("index", true)) {
        int index = request->getParam("index", true)->value().toInt();
        if (index >= 0 && index < sequenceCount) {
          sequences[index].isRunning = false;
          for (int i = index; i < sequenceCount - 1; i++) {
            sequences[i] = sequences[i + 1];
          }
          sequenceCount--;
          saveSequencesFlag = true;
          request->send(
              200, "application/json",
              "{\"status\":\"success\",\"message\":\"Sequence deleted\"}");
        } else {
          request->send(400, "application/json",
                        "{\"error\":\"Invalid sequence index\"}");
        }
      } else if (action == "start" && request->hasParam("index", true)) {
        int index = request->getParam("index", true)->value().toInt();
        if (index >= 0 && index < sequenceCount) {
          sequences[index].isRunning = true;
          sequences[index].currentStep = 0;
          sequences[index].lastStepTime = millis();
          sequences[index].manuallyStopped = false;
          saveSequencesFlag = true;
          if (sequences[index].loopMode == SLM_TIMER) {
            sequences[index].loopStartTime = millis();
            sequences[index].lastIntervalTime = millis();
          }
          if (sequences[index].stepCount > 0 &&
              sequences[index].loopMode != SLM_CONDITION) {
            executeSequenceStep(sequences[index].steps[0]);
          }
          request->send(
              200, "application/json",
              "{\"status\":\"success\",\"message\":\"Sequence started\"}");
        }
      } else if (action == "stop" && request->hasParam("index", true)) {
        int index = request->getParam("index", true)->value().toInt();
        if (index >= 0 && index < sequenceCount) {
          sequences[index].isRunning = false;
          sequences[index].currentStep = 0;
          sequences[index].manuallyStopped = true;
          saveSequencesFlag = true;
          request->send(
              200, "application/json",
              "{\"status\":\"success\",\"message\":\"Sequence stopped\"}");
        }
      } else if (action == "toggle" && request->hasParam("index", true)) {
        int index = request->getParam("index", true)->value().toInt();
        if (index >= 0 && index < sequenceCount) {
          sequences[index].enabled = !sequences[index].enabled;
          saveSequencesFlag = true;
          request->send(
              200, "application/json",
              "{\"status\":\"success\",\"enabled\":" +
                  String(sequences[index].enabled ? "true" : "false") + "}");
        }
      } else {
        request->send(
            400, "application/json",
            "{\"error\":\"Invalid action or sequence limit reached\"}");
      }
    } else {
      request->send(400, "application/json",
                    "{\"error\":\"Missing action parameter\"}");
    }
  });

  server.on("/api/potmapping", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode", true)) {
      String mode = request->getParam("mode", true)->value();
      if (mode == "none" || mode == "led" || mode == "rgb" || mode == "servo") {
        if (mode == "led")
          potMappingMode = PM_LED;
        else if (mode == "rgb")
          potMappingMode = PM_RGB;
        else if (mode == "servo")
          potMappingMode = PM_SERVO;
        else
          potMappingMode = PM_NONE;
        preferences.putUChar("potMappingId", potMappingMode);
        preferences.remove("potMapping");
        request->send(200, "application/json",
                      "{\"status\":\"success\",\"mode\":\"" + mode + "\"}");
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
      }
    } else {
      request->send(400, "application/json",
                    "{\"error\":\"Missing mode parameter\"}");
    }
  });

  server.on("/api/device", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response =
        request->beginResponseStream("application/json");
    StaticJsonDocument<320> doc;
    doc["dn"] = "MakerBuddy IoT Kit";
    doc["fv"] = FIRMWARE_VERSION;
    doc["ip"] = WiFi.localIP().toString();
    doc["mc"] = WiFi.macAddress();
    doc["up"] = millis() / 1000;
    doc["fh"] = ESP.getFreeHeap();
    doc["sd"] = soilMoistureDryRaw;
    doc["sw"] = soilMoistureWetRaw;
    serializeJson(doc, *response);
    request->send(response);
  });

  server.on("/api/firmware", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response =
        request->beginResponseStream("application/json");
    StaticJsonDocument<128> doc;
    doc["fv"] = FIRMWARE_VERSION;
    doc["oi"] = otaInProgress;
    doc["op"] = otaProgress;
    serializeJson(doc, *response);
    request->send(response);
  });

  server.on("/api/ota/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("url", true)) {
      otaFirmwareURL = request->getParam("url", true)->value();
      otaRequested = true;
      request->send(
          200, "application/json",
          "{\"status\":\"success\",\"message\":\"OTA update started\"}");
    } else {
      request->send(400, "application/json",
                    "{\"error\":\"Missing url parameter\"}");
    }
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404, "application/json",
                    "{\"error\":\"Endpoint not found\"}");
    }
  });

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
}

// String getSensorDataJSONCompact() removed
// String getDeviceInfoJSONCompact() removed
// String getFirmwareInfoJSONCompact() removed
// String getSequencesJSON() removed

void wsSendTextAll(const String &payload) { ws.textAll(payload); }

void wsSendText(AsyncWebSocketClient *client, const String &payload) {
  if (!client)
    return;
  client->text(payload);
}

void wsMaybeBroadcastSensors() {
  const unsigned long now = millis();
  if (now - lastWsSensorPushMs < WS_SENSOR_PUSH_INTERVAL_MS)
    return;
  lastWsSensorPushMs = now;

  StaticJsonDocument<640> doc;
  doc["m"] = "s";
  doc["te"] = sensorData.temperature;
  doc["hu"] = sensorData.humidity;
  doc["d8"] = sensorData.ds18b20Temp;
  doc["li"] = sensorData.lightLevel;
  doc["po"] = sensorData.potValue;
  doc["ga"] = sensorData.gasLevel;
  doc["sm"] = sensorData.soilMoisture;
  doc["sr"] = sensorData.soilRaw;
  doc["di"] = sensorData.distance;
  doc["md"] = sensorData.motionDetected ? 1 : 0;
  doc["bp"] = sensorData.buttonPressed ? 1 : 0;

  doc["ls"] = getCurrentLEDState() ? 1 : 0;
  doc["lp"] = getCurrentLEDPWM();
  doc["lb"] = getCurrentLEDBlinking() ? 1 : 0;

  JsonArray rgb = doc.createNestedArray("r");
  rgb.add(getCurrentRGBR());
  rgb.add(getCurrentRGBG());
  rgb.add(getCurrentRGBB());

  doc["rs"] = relayState ? 1 : 0;
  doc["sp"] = servoPosition;
  doc["lm"] = lcdDefaultMode ? 0 : 1;
  doc["fh"] = ESP.getFreeHeap();

  doc["pm"] = potMappingMode;
  doc["pv"] = potMappedValue;

  String out;
  serializeJson(doc, out);
  wsSendTextAll(out);
}

String getSequencesJSONCompact() {
  StaticJsonDocument<3072> doc;
  doc["m"] = "q";
  JsonArray arr = doc.createNestedArray("sq");

  for (int i = 0; i < sequenceCount && i < MAX_SEQUENCES; i++) {
    JsonObject seq = arr.createNestedObject();
    seq["i"] = i;
    seq["e"] = sequences[i].enabled ? 1 : 0;
    seq["n"] = sequences[i].name;
    seq["lm"] = seqLoopModeToString((SeqLoopMode)sequences[i].loopMode);
    seq["tt"] = seqTimerTypeToString((SeqTimerType)sequences[i].timerType);
    seq["ld"] = sequences[i].loopDuration;

    seq["lr"] = seqSensorToString((SeqSensor)sequences[i].loopSensor);
    seq["lo"] = seqOpToString((SeqOp)sequences[i].loopOperator);
    seq["lt"] = sequences[i].loopThreshold;
    seq["rg"] = sequences[i].loopUseRange ? 1 : 0;
    seq["ro"] = seqRangeOpToString((SeqRangeOp)sequences[i].loopRangeOperator);
    seq["o2"] = seqOpToString((SeqOp)sequences[i].loopOperator2);
    seq["t2"] = sequences[i].loopThreshold2;
    seq["ms"] = sequences[i].manuallyStopped ? 1 : 0;
    seq["rn"] = sequences[i].isRunning ? 1 : 0;
    seq["as"] = sequences[i].autostart ? 1 : 0;

    JsonArray steps = seq.createNestedArray("st");
    const int sc = min(sequences[i].stepCount, 10);
    for (int j = 0; j < sc; j++) {
      JsonObject st = steps.createNestedObject();
      st["a"] = seqActionToString((SeqAction)sequences[i].steps[j].action);
      st["v"] = sequences[i].steps[j].actionValue;
      st["d"] = sequences[i].steps[j].delayMs;
      if (sequences[i].steps[j].action == SA_LCD_DISPLAY) {
        st["lm"] =
            seqLcdModeToString((SeqLcdMode)sequences[i].steps[j].lcdMode);
        st["t1"] = sequences[i].steps[j].lcdText1;
        st["t2"] = sequences[i].steps[j].lcdText2;
      }
    }
  }

  String out;
  serializeJson(doc, out);
  return out;
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    StaticJsonDocument<320> info;
    info["m"] = "i";
    info["dn"] = "MakerBuddy IoT Kit";
    info["fv"] = FIRMWARE_VERSION;
    info["ip"] = WiFi.localIP().toString();
    info["mc"] = WiFi.macAddress();
    info["up"] = millis() / 1000;
    info["fh"] = ESP.getFreeHeap();
    info["sd"] = soilMoistureDryRaw;
    info["sw"] = soilMoistureWetRaw;
    String out;
    serializeJson(info, out);
    wsSendText(client, out);

    StaticJsonDocument<128> fw;
    fw["m"] = "f";
    fw["fv"] = FIRMWARE_VERSION;
    fw["oi"] = otaInProgress ? 1 : 0;
    fw["op"] = otaProgress;
    out = "";
    serializeJson(fw, out);
    wsSendText(client, out);

    out = getSequencesJSONCompact();
    wsSendText(client, out);

    wsMaybeBroadcastSensors();
    return;
  }

  if (type != WS_EVT_DATA)
    return;

  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (!info->final || info->index != 0 || info->len != len ||
      info->opcode != WS_TEXT)
    return;

  // Use a stack buffer if message is small, otherwise heap
  // This avoids String reallocation for every message
  if (len > 1024)
    return; // Ignore huge messages to prevent crash

  char msg[len + 1];
  memcpy(msg, data, len);
  msg[len] = 0;

  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, msg);
  if (err)
    return;

  const char *m = doc["m"] | "";
  if (!m || m[0] == 0)
    return;

  Serial.printf("[WS] Received command: %c\n", m[0]);
  switch (m[0]) {
  case 'L': { // LED
    bool st = (doc["s"] | 0) == 1;
    int pwm = doc["p"] | 255;
    bool bl = (doc["b"] | 0) == 1;
    pwm = constrain(pwm, 0, 255);
    setLEDManual(st, pwm, bl);
    break;
  }
  case 'R': { // RGB
    int r = doc["r"] | 0;
    int g = doc["g"] | 0;
    int b = doc["b"] | 0;
    setRGBManual(r, g, b);
    break;
  }
  case 'Y': { // Relay
    bool st = (doc["s"] | 0) == 1;

    Serial.printf("[RELAY] Command received: state=%d\n", st);
    setRelayWithDelay(st);
    Serial.printf("[RELAY] relayState is now: %d\n", relayState);
    break;
  }
  case 'V': { // Servo
    int pos;
    if (doc.containsKey("p")) {
      pos = doc["p"].as<int>();
    } else {
      pos = servoPosition;
    }
    servoPosition = constrain(pos, 0, 180);
    writeServo(servoPosition);
    break;
  }
  case 'Z': { // Buzzer
    bool st = (doc["s"] | 0) == 1;
    if (st) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
    }
    break;
  }
  case 'D': { // LCD
    int mode = doc["md"] | 0;
    if (mode == 0) {
      lcdDefaultMode = true;
      preferences.putBool("lcdDefault", true);
    } else {
      const char *t1 = doc["t1"] | "";
      const char *t2 = doc["t2"] | "";
      customLcdText1 = String(t1);
      customLcdText2 = String(t2);
      lcdDefaultMode = false;
      preferences.putBool("lcdDefault", false);
      preferences.putString("customText1", customLcdText1);
      preferences.putString("customText2", customLcdText2);
    }
    break;
  }
  case 'P': { // Pot mapping
    int mode = doc["md"] | 0;
    if (mode < 0)
      mode = 0;
    if (mode > PM_SERVO)
      mode = PM_NONE;
    potMappingMode = (uint8_t)mode;
    preferences.putUChar("potMappingId", potMappingMode);
    preferences.remove("potMapping");
    break;
  }
  case 'M': { // Soil moisture calibration
    bool changed = false;
    if (doc.containsKey("d")) {
      int v = doc["d"] | soilMoistureDryRaw;
      soilMoistureDryRaw = constrain(v, 0, 4095);
      preferences.putInt("soilDry", soilMoistureDryRaw);
      changed = true;
    }
    if (doc.containsKey("w")) {
      int v = doc["w"] | soilMoistureWetRaw;
      soilMoistureWetRaw = constrain(v, 0, 4095);
      preferences.putInt("soilWet", soilMoistureWetRaw);
      changed = true;
    }
    if (changed) {
      StaticJsonDocument<96> resp;
      resp["m"] = "m";
      resp["d"] = soilMoistureDryRaw;
      resp["w"] = soilMoistureWetRaw;
      String out;
      serializeJson(resp, out);
      wsSendText(client, out);
    }
    break;
  }
  case 'O': { // OTA request
    const char *url = doc["u"] | "";
    if (url && url[0] != 0) {
      otaFirmwareURL = String(url);
      otaRequested = true;
    }
    break;
  }
  case 'X': {                   // Sequences control
    int action = doc["a"] | -1; // 0 add, 1 delete, 2 start, 3 stop, 4 toggle
    int index = doc["i"] | -1;

    if (action == 0 && sequenceCount < MAX_SEQUENCES) {
      JsonObject s = doc["s"];
      if (!s.isNull()) {
        Sequence newSeq;
        newSeq.enabled = (s["e"] | 1) == 1;

        String nameStr = String((const char *)(s["n"] | "New Sequence"));
        strncpy(newSeq.name, nameStr.c_str(), 20);
        newSeq.name[20] = 0;

        newSeq.loopMode = (uint8_t)seqLoopModeFromString(
            String((const char *)(s["lm"] | "none")));
        newSeq.loop = (newSeq.loopMode == SLM_FOREVER);
        newSeq.autostart = (s["as"] | 0) == 1;

        newSeq.timerType = (uint8_t)seqTimerTypeFromString(
            String((const char *)(s["tt"] | "duration")));
        newSeq.loopDuration = s["ld"] | 0;
        newSeq.loopStartTime = 0;
        newSeq.lastIntervalTime = 0;

        newSeq.manuallyStopped = false;
        newSeq.loopSensor = (uint8_t)seqSensorFromString(
            String((const char *)(s["lr"] | "temperature")));
        newSeq.loopOperator =
            (uint8_t)seqOpFromString(String((const char *)(s["lo"] | ">")));
        newSeq.loopThreshold = s["lt"] | 0.0;
        newSeq.loopUseRange = (s["rg"] | 0) == 1;
        newSeq.loopRangeOperator = (uint8_t)seqRangeOpFromString(
            String((const char *)(s["ro"] | "AND")));
        newSeq.loopOperator2 =
            (uint8_t)seqOpFromString(String((const char *)(s["o2"] | ">")));
        newSeq.loopThreshold2 = s["t2"] | 0.0;

        newSeq.currentStep = 0;
        newSeq.isRunning = false;
        newSeq.lastStepTime = 0;

        JsonArray steps = s["st"];
        int sc = 0;
        if (!steps.isNull()) {
          for (JsonObject st : steps) {
            if (sc >= 10)
              break;
            newSeq.steps[sc].action = (uint8_t)seqActionFromString(
                String((const char *)(st["a"] | "")));
            newSeq.steps[sc].actionValue = st["v"] | 0;
            long parsedDelay = st["d"] | 1000;
            if (parsedDelay < 0)
              parsedDelay = 0;
            newSeq.steps[sc].delayMs = (unsigned long)parsedDelay;
            newSeq.steps[sc].lcdMode = (uint8_t)seqLcdModeFromString(
                String((const char *)(st["lm"] | "custom")));

            String t1 = String((const char *)(st["t1"] | ""));
            String t2 = String((const char *)(st["t2"] | ""));
            strncpy(newSeq.steps[sc].lcdText1, t1.c_str(), 16);
            strncpy(newSeq.steps[sc].lcdText2, t2.c_str(), 16);
            newSeq.steps[sc].lcdText1[16] = 0;
            newSeq.steps[sc].lcdText2[16] = 0;

            sc++;
          }
        }
        newSeq.stepCount = sc;

        sequences[sequenceCount] = newSeq;
        sequenceCount++;
        saveSequencesFlag = true;
      }
    } else if (action == 1 && index >= 0 && index < sequenceCount) {
      sequences[index].isRunning = false;
      for (int i = index; i < sequenceCount - 1; i++)
        sequences[i] = sequences[i + 1];
      sequenceCount--;
      saveSequencesFlag = true;
    } else if (action == 2 && index >= 0 && index < sequenceCount) {
      sequences[index].isRunning = true;
      sequences[index].currentStep = 0;
      sequences[index].lastStepTime = millis();
      sequences[index].manuallyStopped = false;
      saveSequencesFlag = true;
      if (sequences[index].loopMode == SLM_TIMER) {
        sequences[index].loopStartTime = millis();
        sequences[index].lastIntervalTime = millis();
      }
      if (sequences[index].stepCount > 0 &&
          sequences[index].loopMode != SLM_CONDITION) {
        executeSequenceStep(sequences[index].steps[0]);
      }
    } else if (action == 3 && index >= 0 && index < sequenceCount) {
      sequences[index].isRunning = false;
      sequences[index].currentStep = 0;
      sequences[index].manuallyStopped = true;
      saveSequencesFlag = true;
    } else if (action == 4 && index >= 0 && index < sequenceCount) {
      sequences[index].enabled = !sequences[index].enabled;
      saveSequencesFlag = true;
    }

    String out = getSequencesJSONCompact();
    wsSendTextAll(out);
    break;
  }
  default:
    break;
  }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void checkWiFiReset() {
  lcd.setCursor(0, 0);
  lcd.print("Hold button for");
  lcd.setCursor(0, 1);
  lcd.print("WiFi reset...");

  unsigned long startTime = millis();
  while (millis() - startTime < 3000) {
    if (!digitalRead(PUSH_BUTTON_PIN)) {
      unsigned long pressStart = millis();
      while (!digitalRead(PUSH_BUTTON_PIN) && (millis() - pressStart < 10000)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Reset:");
        lcd.setCursor(0, 1);
        lcd.print(String(10 - (millis() - pressStart) / 1000) + " seconds");
        delay(100);
      }

      if (millis() - pressStart >= 10000) {
        preferences.remove("wifi_ssid");
        preferences.remove("wifi_password");
        lcdDefaultMode = true;
        preferences.putBool("lcdDefault", true);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Reset!");
        lcd.setCursor(0, 1);
        lcd.print("Restarting...");
        delay(2000);
        ESP.restart();
      }
    }
    delay(100);
  }
}

void checkWiFiResetButton() {
  static bool buttonPressed = false;
  static unsigned long pressStartTime = 0;

  bool currentState = !digitalRead(PUSH_BUTTON_PIN);

  if (currentState && !buttonPressed) {
    buttonPressed = true;
    pressStartTime = millis();
  } else if (!currentState && buttonPressed) {
    buttonPressed = false;
  } else if (buttonPressed && (millis() - pressStartTime > 10000)) {
    wifiResetRequested = true;
    preferences.remove("wifi_ssid");
    preferences.remove("wifi_password");
    lcdDefaultMode = true;
    preferences.putBool("lcdDefault", true);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Reset!");
    lcd.setCursor(0, 1);
    lcd.print("Restarting...");
    delay(2000);
    ESP.restart();
  }
}

void connectToWiFi() {
  String savedSSID = preferences.getString("wifi_ssid", "");
  String savedPassword = preferences.getString("wifi_password", "");

  // If WiFi credentials are saved and not a reset request, try to connect
  if (savedSSID.length() > 0 && !wifiResetRequested) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");

    Serial.println("WiFi credentials found, attempting to connect...");
    Serial.print("SSID: ");
    Serial.println(savedSSID);

    // Try to connect with a timeout (max 3 attempts)
    int maxAttempts = 3;
    int attemptCount = 0;

    while (attemptCount < maxAttempts && WiFi.status() != WL_CONNECTED &&
           !wifiResetRequested) {
      WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20 &&
             !wifiResetRequested) {
        delay(500);
        Serial.print(".");
        lcd.setCursor(attempts % 16, 1);
        lcd.print(".");
        attempts++;

        // Check button during connection attempts
        if (!digitalRead(PUSH_BUTTON_PIN)) {
          checkWiFiResetButton();
        }
      }

      attemptCount++;

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected successfully!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        // First screen: Show SSID and IP
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(WiFi.SSID());
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        delay(3000);

        // Second screen: Show Device Name
        String devName = preferences.getString("deviceName", "");
        if (devName.length() > 0) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Device Name:");
          lcd.setCursor(0, 1);
          lcd.print(devName);
          delay(3000);
        }

        return;
      }
    }

    // If connection failed after all attempts, continue without WiFi
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nWiFi connection failed. Continuing without WiFi...");
      Serial.println("Device will work in standalone mode.");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Not Connected to");
      lcd.setCursor(0, 1);
      lcd.print(savedSSID.length() > 16 ? savedSSID.substring(0, 16)
                                        : savedSSID);
      delay(2000);

      WiFi.mode(WIFI_OFF);
      return;
    }
  }

  // Only start config mode if no credentials or reset was requested
  if (savedSSID.length() == 0 || wifiResetRequested) {
    startConfigMode();
  }
}

void startConfigMode() {
  isConfigMode = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(hotspotSSID);
  lcd.setCursor(0, 1);
  lcd.print("192.168.4.1");

  // Scan while still in STA mode — scanning in AP mode is unsupported on IDF v5.x.
  wifiNetworksCache = "";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    wifiNetworksCache += "<option value='" + WiFi.SSID(i) + "'>" +
                         WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
  }
  WiFi.scanDelete();

  // Switch to AP mode after scan is complete.
  WiFi.mode(WIFI_AP);
  WiFi.softAP(hotspotSSID.c_str(), ap_password);

  // IDF v5.x defaults PMF to capable+required, which breaks fresh WPA2
  // handshakes on many phones. Disable PMF entirely for max compatibility.
  {
    wifi_config_t ap_conf;
    esp_wifi_get_config(WIFI_IF_AP, &ap_conf);
    ap_conf.ap.pmf_cfg.required = false;
    ap_conf.ap.pmf_cfg.capable = false;
    esp_wifi_set_config(WIFI_IF_AP, &ap_conf);
  }

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.print("Hotspot SSID: ");
  Serial.println(hotspotSSID);

  dnsServer.start(53, "*", IP);

  setupConfigServer();
  server.begin();

  Serial.println("Configuration server started on port 80");
  Serial.print("Connect to ");
  Serial.print(hotspotSSID);
  Serial.println(" network and go to 192.168.4.1");
}

void setupConfigServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Load existing device name if available
    String savedDeviceName = preferences.getString("deviceName", "");

    String html =
        "<!DOCTYPE html><html><head><title>MakerBuddy WiFi Setup</title>";
    html +=
        "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<meta charset='UTF-8'>";
    html += "<style>body{font-family:Arial;margin:40px;background:#f0f0f0;}";
    html += ".container{background:white;padding:30px;border-radius:10px;box-"
            "shadow:0 4px 6px rgba(0,0,0,0.1);}";
    html += "h1{color:#333;text-align:center;}";
    html += "label{display:block;margin-top:15px;color:#555;font-weight:bold;}";
    html +=
        "select,input{width:100%;padding:10px;margin:10px 0;border:1px solid "
        "#ddd;border-radius:5px;box-sizing:border-box;font-size:16px;}";
    html += "button{background:#4CAF50;color:white;padding:12px "
            "20px;border:none;border-radius:5px;cursor:pointer;width:100%;"
            "margin-top:20px;font-size:16px;}";
    html += "button:hover{background:#45a049;}";
    html += ".radio-group{margin:15px "
            "0;display:flex;gap:18px;align-items:center;flex-wrap:nowrap;}";
    html += ".radio-group "
            "label{display:flex;align-items:center;gap:8px;margin:0;font-"
            "weight:normal;white-space:nowrap;}";
    html += ".password-container{position:relative;}";
    html += ".password-toggle{position:absolute;right:15px;top:50%;transform:"
            "translateY(-50%);cursor:pointer;color:#666;font-size:20px;user-"
            "select:none;}";
    html += ".hidden{display:none;}</style></head><body>";
    html += "<div class='container'><h1>MakerBuddy IoT Kit WiFi Setup</h1>";
    html += "<form action='/save' method='POST'>";
    html += "<label>Device Name:</label>";
    html += "<input type='text' name='deviceName' value='" + savedDeviceName +
            "' placeholder='Enter device name (e.g., makerbuddy-01)' required>";
    html += "<div class='radio-group'>";
    html += "<label><input type='radio' name='wifiMode' value='dropdown' "
            "checked onclick='toggleWifiInput()'> Select WiFi Network</label>";
    html += "<label><input type='radio' name='wifiMode' value='manual' "
            "onclick='toggleWifiInput()'> Enter SSID Manually</label>";
    html += "</div>";
    html += "<div id='dropdownMode'>";
    html += "<label>WiFi Network:</label>";
    html += "<select id='ssid' name='ssid'><option value=''>Select a "
            "network...</option>";

    // Use cached WiFi networks instead of scanning during request
    html += wifiNetworksCache;

    html += "</select></div>";
    html += "<div id='manualMode' class='hidden'>";
    html += "<label>WiFi SSID:</label>";
    html += "<input type='text' id='manual-ssid' name='manualSsid' "
            "placeholder='Enter WiFi SSID manually'>";
    html += "</div>";
    html += "<label>Password:</label>";
    html += "<div class='password-container'>";
    html += "<input type='password' id='password' name='password' "
            "placeholder='Enter WiFi Password'>";
    html += "<span class='password-toggle' id='pwdToggle' "
            "onclick='togglePassword()'>&#128065;</span>";
    html += "</div>";
    html += "<button type='submit'>Save & Connect</button>";
    html += "</form>";
    html += "<script>";
    html += "function toggleWifiInput() {";
    html += "  var dropdown = document.getElementById('dropdownMode');";
    html += "  var manual = document.getElementById('manualMode');";
    html += "  var mode = "
            "document.querySelector('input[name=\"wifiMode\"]:checked').value;";
    html += "  if(mode === 'dropdown') {";
    html += "    dropdown.classList.remove('hidden');";
    html += "    manual.classList.add('hidden');";
    html += "    document.getElementById('ssid').required = true;";
    html += "    document.getElementById('manual-ssid').required = false;";
    html += "  } else {";
    html += "    dropdown.classList.add('hidden');";
    html += "    manual.classList.remove('hidden');";
    html += "    document.getElementById('ssid').required = false;";
    html += "    document.getElementById('manual-ssid').required = true;";
    html += "  }";
    html += "}";
    html += "function togglePassword() {";
    html += "  var pwd = document.getElementById('password');";
    html += "  var toggle = document.getElementById('pwdToggle');";
    html += "  if(pwd.type === 'password') {";
    html += "    pwd.type = 'text';";
    html += "    toggle.innerHTML = '&#128064;';";
    html += "  } else {";
    html += "    pwd.type = 'password';";
    html += "    toggle.innerHTML = '&#128065;';";
    html += "  }";
    html += "}";
    html += "</script>";
    html += "</div></body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    String ssid = "";
    String password = "";
    String devName = "";

    // Get device name
    if (request->hasParam("deviceName", true)) {
      devName = request->getParam("deviceName", true)->value();
      // Clean device name (remove spaces, special chars)
      devName.replace(" ", "-");
      devName.toLowerCase();
    }

    // Get SSID from dropdown or manual input
    if (request->hasParam("manualSsid", true) &&
        request->getParam("manualSsid", true)->value().length() > 0) {
      ssid = request->getParam("manualSsid", true)->value();
    } else if (request->hasParam("ssid", true)) {
      ssid = request->getParam("ssid", true)->value();
    }

    if (request->hasParam("password", true)) {
      password = request->getParam("password", true)->value();
    }

    preferences.putString("wifi_ssid", ssid);
    preferences.putString("wifi_password", password);
    preferences.putString("deviceName", devName);

    String html = "<!DOCTYPE html><html><head><title>Saved</title>";
    html +=
        "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<script>";
    html += "setTimeout(function() {";
    html += "  window.open('http://app.makerbuddy.cc', '_blank');";
    html += "}, 3000);";
    html += "</script>";
    html += "<style>body{font-family:Arial;margin:40px;background:#f0f0f0;text-"
            "align:center;}";
    html += ".container{background:white;padding:30px;border-radius:10px;box-"
            "shadow:0 4px 6px rgba(0,0,0,0.1);}";
    html += "h1{color:#4CAF50;}</style></head><body>";
    html += "<div class='container'><h1>Settings Saved!</h1>";
    html += "<p>ESP32 will restart and connect to your WiFi network.</p>";
    html += "<p>Device Name: <strong>" + devName + "</strong></p>";
    html += "<p>Opening MakerBuddy app in 3 seconds...</p>";
    html += "<p><a href='http://app.makerbuddy.cc' target='_blank'>Click here "
            "if it doesn't open automatically</a></p></div></body></html>";
    request->send(200, "text/html", html);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Saved!");
    lcd.setCursor(0, 1);
    lcd.print("Restarting...");

    delay(3000);
    ESP.restart();
  });

  server.onNotFound(
      [](AsyncWebServerRequest *request) { request->redirect("/"); });
}

int computeSoilMoisturePercent(int raw) {
  const int dry = soilMoistureDryRaw;
  const int wet = soilMoistureWetRaw;
  if (dry == wet)
    return 0;
  long pct = map((long)raw, (long)dry, (long)wet, 0L, 100L);
  return (int)constrain(pct, 0L, 100L);
}

void readSensors() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    sensorData.temperature = 0.0;
  } else {
    sensorData.temperature = event.temperature;
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    sensorData.humidity = 0.0;
  } else {
    sensorData.humidity = event.relative_humidity;
  }

  ds18b20.requestTemperatures();
  float ds18b20Reading = ds18b20.getTempCByIndex(0);
  if (ds18b20Reading == DEVICE_DISCONNECTED_C) {
    sensorData.ds18b20Temp = 0.0;
  } else {
    sensorData.ds18b20Temp = ds18b20Reading;
  }

  // Convert analog readings (0-4095) to percentage (0-100)
  sensorData.lightLevel = map(analogRead(LDR_PIN), 0, 4095, 0, 100);
  sensorData.potValue = map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 100);
  sensorData.gasLevel = map(analogRead(MQ2_PIN), 0, 4095, 0, 100);
  sensorData.soilRaw = analogRead(SOIL_MOISTURE_PIN);
  sensorData.soilMoisture = computeSoilMoisturePercent(sensorData.soilRaw);

  sensorData.buttonPressed = !digitalRead(PUSH_BUTTON_PIN);
  sensorData.motionDetected = digitalRead(PIR_PIN);

  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH,
                          24000); // Reduced timeout to ~24ms (approx 4m)
  float distance = duration > 0 ? (duration * 0.034f / 2.0f) : -1.0f;
  static float lastValidDistance = 999.0f;
  bool validDistance = duration > 0 && distance >= 2.0f &&
                       distance <= MAX_ULTRASONIC_DISTANCE_CM;
  if (validDistance) {
    sensorData.distance = distance;
    lastValidDistance = distance;
  } else {
    sensorData.distance = lastValidDistance;
  }

  // Process potentiometer mapping
  processPotentiometerMapping();

  wsMaybeBroadcastSensors();
}

void processPotentiometerMapping() {
  if (potMappingMode == PM_NONE) {
    potMappedValue = 0;
    return;
  }

  int rawPotValue = analogRead(POTENTIOMETER_PIN); // 0-4095

  if (potMappingMode == PM_LED) {
    // Map to LED PWM (0-255)
    potMappedValue = map(rawPotValue, 0, 4095, 0, 255);
    setLEDManual(true, potMappedValue, false);
  } else if (potMappingMode == PM_RGB) {
    // Map to percentage (0-100)
    int percentage = map(rawPotValue, 0, 4095, 0, 100);
    potMappedValue = percentage;

    // Color wheel with smooth transitions
    int r = 0, g = 0, b = 0;

    if (percentage <= 16) {
      // Red zone (0-16%)
      r = 255;
      g = 0;
      b = 0;
    } else if (percentage <= 33) {
      // Red to Yellow transition (17-33%)
      r = 255;
      g = map(percentage, 17, 33, 0, 255);
      b = 0;
    } else if (percentage <= 50) {
      // Yellow to Green transition (34-50%)
      r = map(percentage, 34, 50, 255, 0);
      g = 255;
      b = 0;
    } else if (percentage <= 66) {
      // Green to Cyan transition (51-66%)
      r = 0;
      g = 255;
      b = map(percentage, 51, 66, 0, 255);
    } else if (percentage <= 83) {
      // Cyan to Blue transition (67-83%)
      r = 0;
      g = map(percentage, 67, 83, 255, 0);
      b = 255;
    } else {
      // Blue to Magenta transition (84-100%)
      r = map(percentage, 84, 100, 0, 255);
      g = 0;
      b = 255;
    }

    setRGBManual(r, g, b);
  } else if (potMappingMode == PM_SERVO) {
    // Map to Servo angle (0-180)
    potMappedValue = map(rawPotValue, 0, 4095, 0, 180);
    servoPosition = potMappedValue;
    writeServo(servoPosition);
  }
}

void updateLCD() {
  static unsigned long lastUpdate = 0;
  static int displayMode = 0;

  static bool preferencesLoaded = false;
  if (!preferencesLoaded) {
    lcdDefaultMode = preferences.getBool("lcdDefault", true);
    customLcdText1 = preferences.getString("customText1", "");
    customLcdText2 = preferences.getString("customText2", "");
    preferencesLoaded = true;
  }

  // If in config mode, always show hotspot info
  if (isConfigMode) {
    if (millis() - lastUpdate > 2000) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(hotspotSSID);
      lcd.setCursor(0, 1);
      lcd.print("192.168.4.1");
      lastUpdate = millis();
    }
    return;
  }

  if (!lcdDefaultMode && customLcdText1.length() > 0) {
    if (millis() - lastUpdate > 5000) {
      lcd.clear();
      // if (customLcdText1.length() <= 16) {
      lcd.setCursor(0, 0);
      lcd.print(customLcdText1);
      lcd.setCursor(0, 1);
      lcd.print(customLcdText2);
      //}
      lastUpdate = millis();
    }
  } else {
    if (millis() - lastUpdate > 3000) {
      lcd.clear();
      switch (displayMode) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(sensorData.temperature, 1);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("Humidity: ");
        lcd.print(sensorData.humidity, 1);
        lcd.print("%");
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print("Light: ");
        lcd.print(sensorData.lightLevel);
        lcd.setCursor(0, 1);
        lcd.print("Distance: ");
        lcd.print(sensorData.distance, 1);
        lcd.print("cm");
        break;
      case 2:
        lcd.setCursor(0, 0);
        lcd.print("Soil: ");
        lcd.print(sensorData.soilMoisture);
        lcd.setCursor(0, 1);
        lcd.print("Gas: ");
        lcd.print(sensorData.gasLevel);
        break;
      case 3:
        if (WiFi.status() == WL_CONNECTED) {
          lcd.setCursor(0, 0);
          lcd.print(WiFi.SSID());
          lcd.setCursor(0, 1);
          lcd.print(WiFi.localIP());
        } else {
          // Check if we have saved WiFi credentials but failed to connect
          String savedSSID = preferences.getString("wifi_ssid", "");
          if (savedSSID.length() > 0 && WiFi.getMode() == WIFI_OFF) {
            lcd.setCursor(0, 0);
            lcd.print("Unable to connect");
            lcd.setCursor(0, 1);
            lcd.print(savedSSID.length() > 16 ? savedSSID.substring(0, 16)
                                              : savedSSID);
          } else {
            // Only show hotspot info if we're actually in config mode
            lcd.setCursor(0, 0);
            lcd.print(hotspotSSID);
            lcd.setCursor(0, 1);
            lcd.print("IP: 192.168.4.1");
          }
        }
        break;
      }
      displayMode = (displayMode + 1) % 4;
      lastUpdate = millis();
    }
  }
}

void saveSequences() {
  preferences.putInt("seqCount", sequenceCount);

  // Save each sequence individually to avoid NVS blob size limits
  for (int i = 0; i < MAX_SEQUENCES; i++) {
    String key = "seq_" + String(i);
    if (i < sequenceCount) {
      preferences.putBytes(key.c_str(), &sequences[i], sizeof(Sequence));
    } else {
      // Remove unused slots to keep NVS clean
      if (preferences.isKey(key.c_str())) {
        preferences.remove(key.c_str());
      }
    }
  }
}

void loadSequences() {
  // Try to load from new individual keys first
  int count = preferences.getInt("seqCount", 0);
  if (count > MAX_SEQUENCES)
    count = MAX_SEQUENCES;

  sequenceCount = 0;
  for (int i = 0; i < count; i++) {
    if (sequenceCount >= MAX_SEQUENCES)
      break;
    String key = "seq_" + String(i);
    size_t len = preferences.getBytes(key.c_str(), &sequences[sequenceCount],
                                      sizeof(Sequence));

    if (len == sizeof(Sequence)) {
      // SANITIZE: Reset runtime state to prevent "ghost" running or invalid
      // timers
      sequences[sequenceCount].isRunning = false;
      sequences[sequenceCount].currentStep = 0;
      sequences[sequenceCount].lastStepTime = 0;
      sequences[sequenceCount].loopStartTime = 0;

      sequences[sequenceCount].lastIntervalTime = 0;
      sequences[sequenceCount].conditionInit = false;
      sequences[sequenceCount].conditionRaw = false;
      sequences[sequenceCount].conditionRawSince = 0;
      sequences[sequenceCount].conditionDebounced = false;

      sequenceCount++;
    } else {
      Serial.printf("Failed to load sequence key %s\n", key.c_str());
    }
  }

  // Fallback: Check for legacy binary blob if no individual sequences found
  if (sequenceCount == 0 && count > 0) {
    size_t len = preferences.getBytes("seqBlob", sequences, sizeof(sequences));
    if (len == sizeof(sequences)) {
      Serial.println("Migrating sequences from binary blob...");
      sequenceCount = count;
      // Sanitize loaded blob data
      for (int i = 0; i < sequenceCount; i++) {
        sequences[i].isRunning = false;
        sequences[i].currentStep = 0;
        sequences[i].lastStepTime = 0;
        sequences[i].loopStartTime = 0;

        sequences[i].lastIntervalTime = 0;
        sequences[i].conditionInit = false;
        sequences[i].conditionRaw = false;
        sequences[i].conditionRawSince = 0;
        sequences[i].conditionDebounced = false;
      }
      // Save in new format immediately
      saveSequences();
      // Remove old blob
      preferences.remove("seqBlob");
    }
  }

  Serial.printf("Loaded %d sequences\n", sequenceCount);
}

// Sequence loop condition - uses unified condition evaluation engine (same as
// rule engine)
bool evaluateLoopCondition(Sequence &seq) {
  return evaluateGenericCondition(
      (SeqSensor)seq.loopSensor, (SeqOp)seq.loopOperator, seq.loopThreshold,
      seq.loopUseRange, (SeqRangeOp)seq.loopRangeOperator,
      (SeqOp)seq.loopOperator2, seq.loopThreshold2);
}

bool getDebouncedCondition(Sequence &seq, bool rawCondition) {
  const unsigned long debounceMs = 300;
  unsigned long now = millis();

  if (!seq.conditionInit) {
    seq.conditionInit = true;
    seq.conditionRaw = rawCondition;
    seq.conditionRawSince = now;
    seq.conditionDebounced = rawCondition;
    return seq.conditionDebounced;
  }

  if (rawCondition != seq.conditionRaw) {
    seq.conditionRaw = rawCondition;
    seq.conditionRawSince = now;
  }

  if (seq.conditionDebounced != seq.conditionRaw &&
      (now - seq.conditionRawSince) >= debounceMs) {
    seq.conditionDebounced = seq.conditionRaw;
  }

  return seq.conditionDebounced;
}

void processSequences() {
  bool deviceLocked[6] = {false, false, false, false, false, false};

  const SeqLoopMode priorityModes[] = {SLM_CONDITION, SLM_TIMER, SLM_FOREVER,
                                       SLM_NONE};

  for (int p = 0; p < 4; p++) {
    SeqLoopMode mode = priorityModes[p];

    // Process all sequences of this priority level
    for (int i = 0; i < sequenceCount; i++) {
      if (!sequences[i].enabled)
        continue;
      if ((SeqLoopMode)sequences[i].loopMode != mode)
        continue;

      Sequence &seq = sequences[i];

      // --- CONDITION-BASED: auto-start/stop based on sensor condition ---
      if (mode == SLM_CONDITION) {
        // If manually stopped by user, don't auto-start (like disabled rule)
        if (seq.manuallyStopped) {
          if (seq.isRunning) {
            seq.isRunning = false;
            seq.currentStep = 0;
          }
          continue;
        }

        // Evaluate condition using unified rule engine
        bool conditionRaw = evaluateLoopCondition(seq);
        bool conditionMet = getDebouncedCondition(seq, conditionRaw);

        if (conditionMet && !seq.isRunning) {
          // Auto-start: condition just became true
          if (seq.stepCount <= 0) {
            continue;
          }
          seq.isRunning = true;
          seq.currentStep = 0;
          seq.lastStepTime = millis();
          if (seq.stepCount > 0) {
            executeSequenceStepWithPriority(seq.steps[0], deviceLocked);
          }
          continue; // Will process steps on next cycle
        } else if (!conditionMet && seq.isRunning) {
          // Auto-stop: condition became false
          seq.isRunning = false;
          seq.currentStep = 0;
          continue;
        } else if (!conditionMet && !seq.isRunning) {
          continue; // Waiting for condition to be met
        }
        // If conditionMet && isRunning, fall through to step execution
      }

      // Skip if not running (timer/forever/none need manual start or autostart)
      if (!seq.isRunning)
        continue;
      if (seq.stepCount <= 0) {
        seq.isRunning = false;
        seq.currentStep = 0;
        continue;
      }

      // --- INTERVAL WAIT CHECK ---
      // If this is an interval sequence paused at the end waiting for next
      // cycle
      if (mode == SLM_TIMER && (SeqTimerType)seq.timerType == STT_INTERVAL &&
          seq.currentStep >= seq.stepCount) {
        unsigned long now = millis();
        unsigned long elapsed = now - seq.lastIntervalTime;
        if (elapsed >= seq.loopDuration) {
          // Interval elapsed — restart the sequence from step 0
          Serial.printf("[SEQ %d] Interval elapsed (%lums). Restarting.\n", i,
                        elapsed);
          seq.lastIntervalTime = now;
          seq.currentStep = 0;
          seq.lastStepTime = now;
          if (seq.stepCount > 0) {
            executeSequenceStepWithPriority(seq.steps[0], deviceLocked);
          }
        }
        continue; // Either restarted or still waiting
      }

      // --- STEP EXECUTION (shared by all modes) ---
      unsigned long currentTime = millis();
      unsigned long stepDelay = seq.steps[seq.currentStep].delayMs;
      if (currentTime - seq.lastStepTime >= stepDelay) {
        // Advance to next step
        int prevStep = seq.currentStep;
        seq.currentStep++;
        Serial.printf(
            "[SEQ %d] Step %d done (delay %lums). Moving to step %d/%d\n", i,
            prevStep, stepDelay, seq.currentStep, seq.stepCount);

        // Check if sequence finished all steps
        if (seq.currentStep >= seq.stepCount) {
          bool shouldLoop = false;

          if (mode == SLM_CONDITION) {
            shouldLoop = true;
          } else if (mode == SLM_FOREVER) {
            shouldLoop = true;
          } else if (mode == SLM_TIMER) {
            if ((SeqTimerType)seq.timerType == STT_DURATION) {
              shouldLoop = (currentTime - seq.loopStartTime < seq.loopDuration);
            } else if ((SeqTimerType)seq.timerType == STT_INTERVAL) {
              // Interval: park at end and wait for the interval to elapse
              Serial.printf(
                  "[SEQ %d] All steps done. Parking for interval (%lums)\n", i,
                  seq.loopDuration);
              seq.lastIntervalTime = currentTime;
              seq.currentStep = seq.stepCount; // Stay parked at end
              continue;
            } else {
              shouldLoop = seq.loop;
            }

            if (shouldLoop) {
              seq.currentStep = 0;
            } else {
              seq.isRunning = false;
              seq.currentStep = 0;
              continue;
            }
          }

          if (seq.currentStep < seq.stepCount) {
            // Execute the first step of the new loop
            Serial.printf("[SEQ %d] Looping. Executing step %d\n", i,
                          seq.currentStep);
            executeSequenceStepWithPriority(seq.steps[seq.currentStep],
                                            deviceLocked);
            seq.lastStepTime = currentTime;
          }
        } else {
          // Execute the next step
          Serial.printf("[SEQ %d] Executing step %d\n", i, seq.currentStep);
          executeSequenceStepWithPriority(seq.steps[seq.currentStep],
                                          deviceLocked);
          seq.lastStepTime = currentTime;
        }
      }

      // After processing this priority level, lock devices used by its running
      // sequences. This prevents lower priority sequences from overriding
      // higher priority ones.
      for (int i = 0; i < sequenceCount; i++) {
        if (!sequences[i].enabled)
          continue;
        if ((SeqLoopMode)sequences[i].loopMode != mode)
          continue;
        markSequenceDevices(sequences[i], deviceLocked);
      }
    }
  }
}

void executeSequenceStep(SequenceStep step) {
  int value = step.actionValue;

  switch ((SeqAction)step.action) {
  case SA_LED_ON:
    setLEDManual(true, 255, false);
    break;
  case SA_LED_OFF:
    setLEDManual(false, 0, false);
    break;
  case SA_LED_PWM:
    setLEDManual(true, value, false);
    break;
  case SA_RGB_RED:
    setRGBManual(255, 0, 0);
    break;
  case SA_RGB_GREEN:
    setRGBManual(0, 255, 0);
    break;
  case SA_RGB_BLUE:
    setRGBManual(0, 0, 255);
    break;
  case SA_RGB_YELLOW:
    setRGBManual(255, 255, 0);
    break;
  case SA_RGB_PURPLE:
    setRGBManual(128, 0, 128);
    break;
  case SA_RGB_CYAN:
    setRGBManual(0, 255, 255);
    break;
  case SA_RGB_WHITE:
    setRGBManual(255, 255, 255);
    break;
  case SA_RGB_OFF:
    setRGBManual(0, 0, 0);
    break;
  case SA_BUZZER_ON:
    digitalWrite(BUZZER_PIN, HIGH);
    break;
  case SA_BUZZER_OFF:
    digitalWrite(BUZZER_PIN, LOW);
    break;
  case SA_RELAY_ON:
    setRelayWithDelay(true);
    break;
  case SA_RELAY_OFF:
    setRelayWithDelay(false);
    break;
  case SA_SERVO_MOVE:
    servoPosition = constrain(value, 0, 180);
    writeServo(servoPosition);
    break;
  case SA_LCD_DISPLAY:
    if ((SeqLcdMode)step.lcdMode == SLM_LCD_DEFAULT) {
      lcdDefaultMode = true;
      preferences.putBool("lcdDefault", true);
    } else {
      String line1 = replaceWildcards(step.lcdText1);
      String line2 = replaceWildcards(step.lcdText2);
      lcdDefaultMode = false;
      customLcdText1 = line1;
      customLcdText2 = line2;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(line1);
      lcd.setCursor(0, 1);
      lcd.print(line2);
    }
    break;
  default:
    break;
  }
}

// ============================================================================
// UNIFIED CONDITION EVALUATION ENGINE (shared by rules and sequences)
// ============================================================================

bool evaluateOperator(float value, SeqOp op, float threshold) {
  switch (op) {
  case SOP_LT:
    return value < threshold;
  case SOP_GTE:
    return value >= threshold;
  case SOP_LTE:
    return value <= threshold;
  case SOP_EQ:
    return abs(value - threshold) < 0.1;
  default:
    return value > threshold;
  }
}

bool evaluateGenericCondition(SeqSensor sensor, SeqOp op1, float thresh1,
                              bool useRange, SeqRangeOp rangeOp, SeqOp op2,
                              float thresh2) {
  float sensorValue = getSensorValueById(sensor);

  if (useRange) {
    bool condition1 = evaluateOperator(sensorValue, op1, thresh1);
    bool condition2 = evaluateOperator(sensorValue, op2, thresh2);
    return (rangeOp == SROP_AND) ? (condition1 && condition2)
                                 : (condition1 || condition2);
  }

  return evaluateOperator(sensorValue, op1, thresh1);
}

// ============================================================================
// DEVICE PRIORITY HELPERS (for sequence priority system)
// Device index: 0=LED, 1=RGB, 2=Buzzer, 3=Servo, 4=Relay, 5=LCD
// ============================================================================

int getDeviceFromAction(SeqAction action) {
  switch (action) {
  case SA_LED_ON:
  case SA_LED_OFF:
  case SA_LED_PWM:
    return 0;
  case SA_RGB_RED:
  case SA_RGB_GREEN:
  case SA_RGB_BLUE:
  case SA_RGB_YELLOW:
  case SA_RGB_PURPLE:
  case SA_RGB_CYAN:
  case SA_RGB_WHITE:
  case SA_RGB_OFF:
    return 1;
  case SA_BUZZER_ON:
  case SA_BUZZER_OFF:
    return 2;
  case SA_SERVO_MOVE:
    return 3;
  case SA_RELAY_ON:
  case SA_RELAY_OFF:
    return 4;
  case SA_LCD_DISPLAY:
    return 5;
  default:
    return -1;
  }
}

void markSequenceDevices(Sequence &seq, bool deviceLocked[]) {
  if (!seq.isRunning)
    return;
  int safeStepCount = seq.stepCount;
  if (safeStepCount < 0)
    safeStepCount = 0;
  if (safeStepCount > 10)
    safeStepCount = 10;
  for (int s = 0; s < safeStepCount; s++) {
    int dev = getDeviceFromAction((SeqAction)seq.steps[s].action);
    if (dev >= 0 && dev < 6)
      deviceLocked[dev] = true;
  }
}

void executeSequenceStepWithPriority(SequenceStep step, bool deviceLocked[]) {
  int dev = getDeviceFromAction((SeqAction)step.action);
  if (dev >= 0 && dev < 6 && deviceLocked[dev])
    return; // Higher priority owns this device
  executeSequenceStep(step);
}

float getSensorValueById(SeqSensor sensor) {
  switch (sensor) {
  case SS_HUMIDITY:
    return sensorData.humidity;
  case SS_DS18B20:
    return sensorData.ds18b20Temp;
  case SS_LIGHT:
    return sensorData.lightLevel;
  case SS_GAS:
    return sensorData.gasLevel;
  case SS_SOIL:
    return sensorData.soilMoisture;
  case SS_DISTANCE:
    return sensorData.distance;
  case SS_POT:
    return sensorData.potValue;
  case SS_MOTION:
    return sensorData.motionDetected ? 1.0 : 0.0;
  case SS_BUTTON:
    return sensorData.buttonPressed ? 1.0 : 0.0;
  default:
    return sensorData.temperature;
  }
}

void setRelayWithDelay(bool state) {
  relayState = state;
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  Serial.printf("[RELAY] Set to %s\n", relayState ? "ON" : "OFF");
}
