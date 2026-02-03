/*
 * MakerBuddy V1.0 - ESP32 IoT Development Kit
 *
 * Website: https://makerbuddy.cc
 * Email: info@makerbuddy.cc
 * Version: 1.0
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

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <DNSServer.h>
#include <Wire.h>
#include <Ticker.h>
#include <HTTPClient.h>
#include <Update.h>

// Firmware Version
#define FIRMWARE_VERSION "1.0"

// WiFi credentials - will be overridden by saved preferences
const char* ssid = "";
const char* password = "";

// WiFi Configuration
String hotspotSSID = "";
const char* ap_password = "12345678";

// OTA Update variables
bool otaInProgress = false;
int otaProgress = 0;

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


// Component initialization
AsyncWebServer server(80);
DNSServer dnsServer;
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);
DHT_Unified dht(DHT22_PIN, DHT11);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;
Preferences preferences;


// LED Control Variables
struct LEDState {
  bool manualState = false;
  bool ruleState = false;
  int manualPWM = 255;
  int rulePWM = 255;
  bool manualBlinking = false;
  bool ruleBlinking = false;
  bool ruleOverride = false;
} ledControl;

// RGB LED Control Variables
struct RGBState {
  int manualR = 0, manualG = 0, manualB = 0;
  int ruleR = 0, ruleG = 0, ruleB = 0;
  bool ruleOverride = false;
} rgbControl;

// Global variables
bool relayState = false;
int relayOnDelay = 0;
int relayOffDelay = 0;
unsigned long relayTimer = 0;
bool relayDelayActive = false;
bool relayTargetState = false;
int servoPosition = 0;
String gpsData = "";
bool lcdDefaultMode = true;
String customLcdText1 = "";
String customLcdText2 = "";
bool isConfigMode = false;
bool wifiResetRequested = false;

// Potentiometer Mapping Variables
String potMappingMode = "none"; // none, led, rgb, servo
int potMappedValue = 0;

// WiFi scan cache
String wifiNetworksCache = "";
unsigned long lastScanTime = 0;
const unsigned long SCAN_INTERVAL = 10000; // Scan every 10 seconds

// LED Blinking variables
unsigned long ledBlinkTimer = 0;
bool ledBlinkState = false;

// Ticker objects for timer-based execution
Ticker sensorTicker;
Ticker lcdTicker;
Ticker ruleTicker;
Ticker ledBlinkTicker;

// Flags for timer callbacks
volatile bool readSensorsFlag = false;
volatile bool updateLCDFlag = false;
volatile bool processRulesFlag = false;
volatile bool handleLEDBlinkFlag = false;

// Rule system variables
struct AutomationRule {
  bool enabled;
  String name;
  String condition;
  String operator_type;
  float threshold;
  String action;
  int actionValue;
  int actionDelay;
  String lcdText1;
  String lcdText2;
  String lcdMode;  // "default" or "custom"
};

const int MAX_RULES = 10;
AutomationRule rules[MAX_RULES];
int ruleCount = 0;

// Sensor data structure
struct SensorData {
  float temperature;
  float humidity;
  float ds18b20Temp;
  int lightLevel;
  int potValue;
  int gasLevel;
  int soilMoisture;
  float distance;
  bool motionDetected;
  bool buttonPressed;
  String gpsLocation;
  String nfcTagName;
  String nfcTagValue;
} sensorData;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================
void setLEDManual(bool state, int pwm = 255, bool blink = false);
void setLEDFromRule(bool state, int pwm = 255, bool blink = false, bool override = true);
void updateLEDOutput();
void handleLEDBlinking();
bool getCurrentLEDState();
int getCurrentLEDPWM();
bool getCurrentLEDBlinking();
void setRGBManual(int r, int g, int b);
void setRGBFromRule(uint32_t packedRGB, bool override = true);
void updateRGBOutput();
uint32_t packRGB(int r, int g, int b);
int getCurrentRGBR();
int getCurrentRGBG();
int getCurrentRGBB();
void executeAction(AutomationRule rule);
void setupWebServer();
String getSensorDataJSON();
void checkWiFiReset();
void checkWiFiResetButton();
void connectToWiFi();
void startConfigMode();
void setupConfigServer();
void readSensors();
void processPotentiometerMapping();
void updateLCD();
void loadRules();
void saveRules();
void processRules();
bool evaluateCondition(AutomationRule rule);
float getSensorValue(String condition);
String getRulesJSON();
void setRelayWithDelay(bool state);
void handleRelayDelay();

// Timer callback functions
void sensorTimerCallback();
void lcdTimerCallback();
void ruleTimerCallback();
void ledBlinkTimerCallback();

// ============================================================================
// LED CONTROL FUNCTIONS
// ============================================================================

void setLEDManual(bool state, int pwm, bool blink) {
  ledControl.manualState = state;
  ledControl.manualPWM = constrain(pwm, 0, 255);
  ledControl.manualBlinking = blink;
  ledControl.ruleOverride = false;
  
  Serial.printf("Manual LED Control: State=%s, PWM=%d, Blink=%s\n", 
                state ? "ON" : "OFF", pwm, blink ? "YES" : "NO");
  
  updateLEDOutput();
}

void setLEDFromRule(bool state, int pwm, bool blink, bool override) {
  ledControl.ruleState = state;
  ledControl.rulePWM = constrain(pwm, 0, 255);
  ledControl.ruleBlinking = blink;
  ledControl.ruleOverride = override;
  
  Serial.printf("Rule LED Control: State=%s, PWM=%d, Blink=%s, Override=%s\n", 
                state ? "ON" : "OFF", pwm, blink ? "YES" : "NO", override ? "YES" : "NO");
  
  updateLEDOutput();
}

void updateLEDOutput() {
  bool finalState;
  int finalPWM;
  bool finalBlink;

  if (ledControl.ruleOverride) {
    finalState = ledControl.ruleState;
    finalPWM = ledControl.rulePWM;
    finalBlink = ledControl.ruleBlinking;
  } else {
    finalState = ledControl.manualState;
    finalPWM = ledControl.manualPWM;
    finalBlink = ledControl.manualBlinking;
  }

  if (finalBlink && finalState) {
    return;
    
  }
  
  if (finalState) {
    analogWrite(SINGLE_LED_PIN, finalPWM);
  } else {
    analogWrite(SINGLE_LED_PIN, 0);
  }
}

void handleLEDBlinking() {
  bool shouldBlink = false;
  if(ledControl.manualBlinking || ledControl.ruleBlinking){
    if (ledControl.ruleOverride) {
      shouldBlink = ledControl.ruleState && ledControl.ruleBlinking;
    } else {
      shouldBlink = ledControl.manualState && ledControl.manualBlinking;
    }
    
    if (!shouldBlink) {
      return;
    }
    
    if (millis() - ledBlinkTimer > 500) {
      ledBlinkState = !ledBlinkState;
      
      if (ledBlinkState) {
        int pwmValue = ledControl.ruleOverride ? ledControl.rulePWM : ledControl.manualPWM;
        analogWrite(SINGLE_LED_PIN, pwmValue);

      } else {
        analogWrite(SINGLE_LED_PIN, 0);
        Serial.println("Blinking LED OFF");
      }
      
      ledBlinkTimer = millis();
    }
  }
}

bool getCurrentLEDState() {
  return ledControl.ruleOverride ? ledControl.ruleState : ledControl.manualState;
}

int getCurrentLEDPWM() {
  return ledControl.ruleOverride ? ledControl.rulePWM : ledControl.manualPWM;
}

bool getCurrentLEDBlinking() {
  return ledControl.ruleOverride ? ledControl.ruleBlinking : ledControl.manualBlinking;
}

// ============================================================================
// RGB LED CONTROL FUNCTIONS
// ============================================================================

void setRGBManual(int r, int g, int b) {
  rgbControl.manualR = constrain(r, 0, 255);
  rgbControl.manualG = constrain(g, 0, 255);
  rgbControl.manualB = constrain(b, 0, 255);
  rgbControl.ruleOverride = false;
  
  Serial.printf("Manual RGB Control: R=%d, G=%d, B=%d\n", r, g, b);
  updateRGBOutput();
}

void setRGBFromRule(uint32_t packedRGB, bool override) {
  rgbControl.ruleR = (packedRGB >> 16) & 0xFF;
  rgbControl.ruleG = (packedRGB >> 8) & 0xFF;
  rgbControl.ruleB = packedRGB & 0xFF;
  rgbControl.ruleOverride = override;
  
  Serial.printf("Rule RGB Control: Packed=0x%06X, R=%d, G=%d, B=%d\n", 
                packedRGB, rgbControl.ruleR, rgbControl.ruleG, rgbControl.ruleB);
  updateRGBOutput();
}

void updateRGBOutput() {
  int finalR, finalG, finalB;

  if (rgbControl.ruleOverride) {
    finalR = rgbControl.ruleR;
    finalG = rgbControl.ruleG;
    finalB = rgbControl.ruleB;
  } else {
    finalR = rgbControl.manualR;
    finalG = rgbControl.manualG;
    finalB = rgbControl.manualB;
  }

  // For common anode RGB LED: 255 = OFF, 0 = ON
  // Invert the values: if input is 0, write 255 (off)
  int outputR = 255 - finalR;
  int outputG = 255 - finalG;
  int outputB = 255 - finalB;

  analogWrite(RGB_LED_R_PIN, outputR);
  analogWrite(RGB_LED_G_PIN, outputG);
  analogWrite(RGB_LED_B_PIN, outputB);

  Serial.printf("RGB Output: R=%d->%d, G=%d->%d, B=%d->%d\n",
                finalR, outputR, finalG, outputG, finalB, outputB);
}

uint32_t packRGB(int r, int g, int b) {
  return ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
}

int getCurrentRGBR() {
  return rgbControl.ruleOverride ? rgbControl.ruleR : rgbControl.manualR;
}

int getCurrentRGBG() {
  return rgbControl.ruleOverride ? rgbControl.ruleG : rgbControl.manualG;
}

int getCurrentRGBB() {
  return rgbControl.ruleOverride ? rgbControl.ruleB : rgbControl.manualB;
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
  servo.attach(SERVO_PIN);
  servo.write(servoPosition);

  // Initialize NFC reader
  Wire.begin();

  // Generate hotspot SSID with last 4 chars of MAC address
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  hotspotSSID = "MakerBuddy_" + mac.substring(8);
  Serial.println("Hotspot SSID: " + hotspotSSID);

  loadRules();
  lcdDefaultMode = preferences.getBool("lcdDefault", true);
  customLcdText1 = preferences.getString("customText1", "");
  customLcdText2 = preferences.getString("customText2", "");
  potMappingMode = preferences.getString("potMapping", "none");

  lcd.setCursor(0, 0);
  lcd.print("MakerBuddy Kit");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  checkWiFiReset();
  connectToWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    setupWebServer();
    server.begin();
    Serial.println("Web server started");
  }

  // Start timer-based tasks
  sensorTicker.attach(1.0, sensorTimerCallback);      // Read sensors every 1 second
  lcdTicker.attach(0.1, lcdTimerCallback);            // Update LCD every 100ms
  ruleTicker.attach(1.0, ruleTimerCallback);          // Process rules every 1 second
  ledBlinkTicker.attach(0.5, ledBlinkTimerCallback);  // LED blink every 500ms

  Serial.println("Timers initialized");

  // Initialize RGB LED state (ensure it's off with values 0,0,0)
  Serial.printf("Initial RGB state: manualR=%d, manualG=%d, manualB=%d, ruleOverride=%d\n",
                rgbControl.manualR, rgbControl.manualG, rgbControl.manualB, rgbControl.ruleOverride);
  updateRGBOutput();
  Serial.println("RGB initialized");
}

void loop() {
  if (isConfigMode) {
    dnsServer.processNextRequest();

    // Periodic WiFi scan in config mode (non-blocking)
    if (millis() - lastScanTime > SCAN_INTERVAL) {
      int n = WiFi.scanNetworks(false, false); // Async scan, no hidden networks
      if (n >= 0) {
        wifiNetworksCache = "";
        for (int i = 0; i < n; ++i) {
          wifiNetworksCache += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
        }
        lastScanTime = millis();
      }
    }
  }

  // Check WiFi reset button (non-blocking, runs every loop)
  checkWiFiResetButton();

  // Handle relay delay (runs every loop for precise timing)
  handleRelayDelay();

  // Process flags set by timer callbacks
  if (readSensorsFlag) {
    readSensorsFlag = false;
    readSensors();
  }

  if (updateLCDFlag) {
    updateLCDFlag = false;
    updateLCD();
  }

  if (processRulesFlag) {
    processRulesFlag = false;
    processRules();
  }

  if (handleLEDBlinkFlag) {
    handleLEDBlinkFlag = false;
    handleLEDBlinking();
  }
}

// ============================================================================
// TIMER CALLBACK FUNCTIONS
// ============================================================================

void sensorTimerCallback() {
  readSensorsFlag = true;
}

void lcdTimerCallback() {
  updateLCDFlag = true;
}

void ruleTimerCallback() {
  processRulesFlag = true;
}

void ledBlinkTimerCallback() {
  handleLEDBlinkFlag = true;
}

// ============================================================================
// RULE ENGINE WITH RGB SUPPORT AND LCD
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

void executeAction(AutomationRule rule) {
  static unsigned long lastActionTime[MAX_RULES] = {0};
  static int ruleIndex = 0;
  
  for (int i = 0; i < ruleCount; i++) {
    if (rules[i].name == rule.name) {
      ruleIndex = i;
      break;
    }
  }
  
  if (rule.actionDelay > 0 && (millis() - lastActionTime[ruleIndex]) < (rule.actionDelay * 1000)) {
    return;
  }
  
  lastActionTime[ruleIndex] = millis();
  
  if (rule.action == "led_on") {
    setLEDFromRule(true, rule.actionValue, false, true);
  } else if (rule.action == "led_off") {
    setLEDFromRule(false, rule.actionValue, false, true);
  } else if (rule.action == "led_blink") {
    setLEDFromRule(true, rule.actionValue, true, true);
  } else if (rule.action == "rgb_color") {
    setRGBFromRule(rule.actionValue, true);
  } else if (rule.action == "rgb_red") {
    setRGBFromRule(packRGB(255, 0, 0), true);
  } else if (rule.action == "rgb_green") {
    setRGBFromRule(packRGB(0, 255, 0), true);
  } else if (rule.action == "rgb_blue") {
    setRGBFromRule(packRGB(0, 0, 255), true);
  } else if (rule.action == "rgb_white") {
    setRGBFromRule(packRGB(255, 255, 255), true);
  } else if (rule.action == "rgb_yellow") {
    setRGBFromRule(packRGB(255, 255, 0), true);
  } else if (rule.action == "rgb_purple") {
    setRGBFromRule(packRGB(255, 0, 255), true);
  } else if (rule.action == "rgb_cyan") {
    setRGBFromRule(packRGB(0, 255, 255), true);
  } else if (rule.action == "rgb_off") {
    setRGBFromRule(packRGB(0, 0, 0), true);
  } else if (rule.action == "buzzer_on") {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
  } else if (rule.action == "servo_move") {
    servoPosition = constrain(rule.actionValue, 0, 180);
    servo.write(servoPosition);
  } else if (rule.action == "relay_on") {
    setRelayWithDelay(true);
  } else if (rule.action == "relay_off") {
    setRelayWithDelay(false);
  } else if (rule.action == "lcd_display") {
    if (rule.lcdMode == "default") {
      // Set LCD back to default sensor data mode
      lcdDefaultMode = true;
      preferences.putBool("lcdDefault", true);
    } else {
      // Custom mode with text and wildcards
      String line1 = replaceWildcards(rule.lcdText1);
      String line2 = replaceWildcards(rule.lcdText2);
      lcdDefaultMode = false;
      customLcdText1 = line1;
      customLcdText2 = line2;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(line1);
      lcd.setCursor(0, 1);
      lcd.print(line2);
    }
  }
}

// ============================================================================
// OTA UPDATE FUNCTION
// ============================================================================

void performOTAUpdate(String firmwareURL) {
  otaInProgress = true;
  otaProgress = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("OTA Update");
  lcd.setCursor(0, 1);
  lcd.print("Downloading...");

  HTTPClient http;
  http.begin(firmwareURL);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    bool canBegin = Update.begin(contentLength);

    if (canBegin) {
      WiFiClient *client = http.getStreamPtr();
      size_t written = 0;

      Serial.println("Starting OTA update...");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Installing...");

      uint8_t buff[128] = { 0 };
      while (http.connected() && (written < contentLength)) {
        size_t available = client->available();
        if (available) {
          int bytesRead = client->readBytes(buff, min(available, sizeof(buff)));
          written += Update.write(buff, bytesRead);

          otaProgress = (written * 100) / contentLength;

          // Update LCD every 10%
          if (otaProgress % 10 == 0) {
            lcd.setCursor(0, 1);
            lcd.print("Progress: ");
            lcd.print(otaProgress);
            lcd.print("%   ");
          }

          Serial.printf("Progress: %d%%\n", otaProgress);
        }
        delay(1);
      }

      if (Update.end()) {
        if (Update.isFinished()) {
          Serial.println("Update successfully completed!");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Update Complete");
          lcd.setCursor(0, 1);
          lcd.print("Restarting...");
          delay(3000);
          ESP.restart();
        } else {
          Serial.println("Update not finished!");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Update Failed!");
          lcd.setCursor(0, 1);
          lcd.print("Not Finished");
        }
      } else {
        Serial.println("Error occurred during update!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Update Error!");
        lcd.setCursor(0, 1);
        lcd.print("Code: " + String(Update.getError()));
      }
    } else {
      Serial.println("Not enough space for OTA");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("OTA Failed!");
      lcd.setCursor(0, 1);
      lcd.print("Not enough space");
    }
  } else {
    Serial.printf("HTTP Error: %d\n", httpCode);
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
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Private-Network", "true");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  server.on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getSensorDataJSON());
  });
  
  server.on("/api/led", HTTP_POST, [](AsyncWebServerRequest *request){
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
      request->send(400, "application/json", "{\"error\":\"Missing state parameter\"}");
    }
  });
  
  server.on("/api/rgb", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("r", true) && request->hasParam("g", true) && request->hasParam("b", true)) {
      int r = request->getParam("r", true)->value().toInt();
      int g = request->getParam("g", true)->value().toInt();
      int b = request->getParam("b", true)->value().toInt();
      
      setRGBManual(r, g, b);
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Missing RGB parameters\"}");
    }
  });
  
  server.on("/api/buzzer", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      if (state == "true") {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(500);
        digitalWrite(BUZZER_PIN, LOW);
      }
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Missing state parameter\"}");
    }
  });
  
  server.on("/api/relay", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      bool newState = (state == "true");
      
      if (request->hasParam("onDelay", true)) {
        relayOnDelay = request->getParam("onDelay", true)->value().toInt();
      }
      if (request->hasParam("offDelay", true)) {
        relayOffDelay = request->getParam("offDelay", true)->value().toInt();
      }
      
      setRelayWithDelay(newState);
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Missing state parameter\"}");
    }
  });
  
  server.on("/api/servo", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("position", true)) {
      servoPosition = request->getParam("position", true)->value().toInt();
      servoPosition = constrain(servoPosition, 0, 180);
      servo.write(servoPosition);
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Missing position parameter\"}");
    }
  });

  server.on("/api/rules", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getRulesJSON());
  });
  
  server.on("/api/rules", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("action", true)) {
      String action = request->getParam("action", true)->value();

      if (action == "add" && ruleCount < MAX_RULES) {
        AutomationRule newRule;
        newRule.enabled = request->hasParam("enabled", true) ?
          request->getParam("enabled", true)->value() == "true" : true;
        newRule.name = request->hasParam("name", true) ?
          request->getParam("name", true)->value() : "New Rule";
        newRule.condition = request->hasParam("condition", true) ?
          request->getParam("condition", true)->value() : "temperature";
        newRule.operator_type = request->hasParam("operator", true) ?
          request->getParam("operator", true)->value() : ">";
        newRule.threshold = request->hasParam("threshold", true) ?
          request->getParam("threshold", true)->value().toFloat() : 25.0;
        newRule.action = request->hasParam("actionType", true) ?
          request->getParam("actionType", true)->value() : "led_on";
        newRule.actionValue = request->hasParam("actionValue", true) ?
          request->getParam("actionValue", true)->value().toInt() : 255;
        newRule.actionDelay = request->hasParam("actionDelay", true) ?
          request->getParam("actionDelay", true)->value().toInt() : 0;
        newRule.lcdText1 = request->hasParam("lcdText1", true) ?
          request->getParam("lcdText1", true)->value() : "";
        newRule.lcdText2 = request->hasParam("lcdText2", true) ?
          request->getParam("lcdText2", true)->value() : "";
        newRule.lcdMode = request->hasParam("lcdMode", true) ?
          request->getParam("lcdMode", true)->value() : "custom";

        rules[ruleCount] = newRule;
        ruleCount++;
        saveRules();

        request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Rule added\"}");
      } else if (action == "delete" && request->hasParam("index", true)) {
        int index = request->getParam("index", true)->value().toInt();
        if (index >= 0 && index < ruleCount) {
          for (int i = index; i < ruleCount - 1; i++) {
            rules[i] = rules[i + 1];
          }
          ruleCount--;
          saveRules();
          request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Rule deleted\"}");
        } else {
          request->send(400, "application/json", "{\"error\":\"Invalid rule index\"}");
        }
      } else if (action == "toggle" && request->hasParam("index", true)) {
        int index = request->getParam("index", true)->value().toInt();
        if (index >= 0 && index < ruleCount) {
          rules[index].enabled = !rules[index].enabled;
          saveRules();
          request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Rule toggled\"}");
        } else {
          request->send(400, "application/json", "{\"error\":\"Invalid rule index\"}");
        }
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid action or rule limit reached\"}");
      }
    } else {
      request->send(400, "application/json", "{\"error\":\"Missing action parameter\"}");
    }
  });
  
  server.on("/api/lcd", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("mode", true)) {
      String mode = request->getParam("mode", true)->value();
      if (mode == "default") {
        lcdDefaultMode = true;
        preferences.putBool("lcdDefault", true);
        request->send(200, "application/json", "{\"status\":\"success\",\"mode\":\"default\"}");
      } else if (mode == "custom" && request->hasParam("text1", true)) {
        customLcdText1 = request->getParam("text1", true)->value();
        customLcdText2 = request->getParam("text2", true)->value();
        lcdDefaultMode = false;
        preferences.putBool("lcdDefault", false);
        preferences.putString("customText1", customLcdText1);
        preferences.putString("customText2", customLcdText2);
        request->send(200, "application/json", "{\"status\":\"success\",\"mode\":\"custom\"}");
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid parameters\"}");
      }
    } else {
      request->send(400, "application/json", "{\"error\":\"Missing mode parameter\"}");
    }
  });

  server.on("/api/potmapping", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("mode", true)) {
      String mode = request->getParam("mode", true)->value();
      if (mode == "none" || mode == "led" || mode == "rgb" || mode == "servo") {
        potMappingMode = mode;
        preferences.putString("potMapping", mode);
        request->send(200, "application/json", "{\"status\":\"success\",\"mode\":\"" + mode + "\"}");
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid mode\"}");
      }
    } else {
      request->send(400, "application/json", "{\"error\":\"Missing mode parameter\"}");
    }
  });

  server.on("/api/device", HTTP_GET, [](AsyncWebServerRequest *request){
    DynamicJsonDocument doc(512);
    doc["deviceName"] = "MakerBuddy IoT Kit";
    doc["version"] = FIRMWARE_VERSION;
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    doc["uptime"] = millis() / 1000;
    doc["freeHeap"] = ESP.getFreeHeap();

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  server.on("/api/firmware", HTTP_GET, [](AsyncWebServerRequest *request){
    DynamicJsonDocument doc(256);
    doc["version"] = FIRMWARE_VERSION;
    doc["otaInProgress"] = otaInProgress;
    doc["otaProgress"] = otaProgress;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  server.on("/api/ota/update", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("url", true)) {
      String firmwareURL = request->getParam("url", true)->value();
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"OTA update started\"}");

      // Start OTA update in next loop iteration
      delay(100);
      performOTAUpdate(firmwareURL);
    } else {
      request->send(400, "application/json", "{\"error\":\"Missing url parameter\"}");
    }
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404, "application/json", "{\"error\":\"Endpoint not found\"}");
    }
  });
}

String getSensorDataJSON() {
  DynamicJsonDocument doc(1024);

  doc["temperature"] = sensorData.temperature;
  doc["humidity"] = sensorData.humidity;
  doc["ds18b20Temp"] = sensorData.ds18b20Temp;
  doc["lightLevel"] = sensorData.lightLevel;
  doc["potValue"] = sensorData.potValue;
  doc["gasLevel"] = sensorData.gasLevel;
  doc["soilMoisture"] = sensorData.soilMoisture;
  doc["distance"] = sensorData.distance;
  doc["motionDetected"] = sensorData.motionDetected;
  doc["buttonPressed"] = sensorData.buttonPressed;
  doc["gpsLocation"] = sensorData.gpsLocation;
  doc["nfcTagName"] = sensorData.nfcTagName;
  doc["nfcTagValue"] = sensorData.nfcTagValue;
  
  doc["ledState"] = getCurrentLEDState();
  doc["ledPWM"] = getCurrentLEDPWM();
  doc["ledBlinking"] = getCurrentLEDBlinking();
  
  doc["rgbR"] = getCurrentRGBR();
  doc["rgbG"] = getCurrentRGBG();
  doc["rgbB"] = getCurrentRGBB();
  
  doc["relayState"] = relayState;
  doc["relayOnDelay"] = relayOnDelay;
  doc["relayOffDelay"] = relayOffDelay;
  doc["servoPosition"] = servoPosition;
  doc["lcdMode"] = lcdDefaultMode ? "default" : "custom";
  doc["customLcdText1"] = customLcdText1;
  doc["customLcdText2"] = customLcdText2;
  doc["freeHeap"] = ESP.getFreeHeap();

  doc["potMapping"] = potMappingMode;
  doc["potMappedValue"] = potMappedValue;
  
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
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

    while (attemptCount < maxAttempts && WiFi.status() != WL_CONNECTED && !wifiResetRequested) {
      WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20 && !wifiResetRequested) {
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

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Connected");
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        delay(3000);
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
      lcd.print(savedSSID.length() > 16 ? savedSSID.substring(0, 16) : savedSSID);
      delay(2000);

      WiFi.mode(WIFI_OFF);
      return;
    }
  }

  // Only start config mode if no credentials or reset was requested
  if (savedSSID.length() == 0 || wifiResetRequested) {
    Serial.println("No WiFi credentials or reset requested. Starting config mode...");
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

  WiFi.mode(WIFI_AP);
  WiFi.softAP(hotspotSSID.c_str(), ap_password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.print("Hotspot SSID: ");
  Serial.println(hotspotSSID);

  dnsServer.start(53, "*", IP);

  // Perform initial WiFi scan
  Serial.println("Scanning WiFi networks...");
  int n = WiFi.scanNetworks(false, false);
  wifiNetworksCache = "";
  if (n > 0) {
    for (int i = 0; i < n; ++i) {
      wifiNetworksCache += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
    }
  }
  lastScanTime = millis();

  setupConfigServer();
  server.begin();

  Serial.println("Configuration server started on port 80");
  Serial.print("Connect to ");
  Serial.print(hotspotSSID);
  Serial.println(" network and go to 192.168.4.1");
}

void setupConfigServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><title>MakerBuddy WiFi Setup</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:40px;background:#f0f0f0;}";
    html += ".container{background:white;padding:30px;border-radius:10px;box-shadow:0 4px 6px rgba(0,0,0,0.1);}";
    html += "h1{color:#333;text-align:center;}";
    html += "label{display:block;margin-top:15px;color:#555;font-weight:bold;}";
    html += "select,input{width:100%;padding:10px;margin:10px 0;border:1px solid #ddd;border-radius:5px;box-sizing:border-box;font-size:16px;}";
    html += "button{background:#4CAF50;color:white;padding:12px 20px;border:none;border-radius:5px;cursor:pointer;width:100%;margin-top:20px;font-size:16px;}";
    html += "button:hover{background:#45a049;}";
    html += ".scan-btn{background:#2196F3;margin-top:10px;}";
    html += ".scan-btn:hover{background:#0b7dda;}";
    html += "#networks{margin-top:10px;}</style></head><body>";
    html += "<div class='container'><h1>MakerBuddy IoT Kit WiFi Setup</h1>";
    html += "<form action='/save' method='POST'>";
    html += "<label>WiFi Network:</label>";
    html += "<select id='ssid' name='ssid' required><option value=''>Select a network...</option>";

    // Use cached WiFi networks instead of scanning during request
    html += wifiNetworksCache;

    html += "</select>";
    html += "<label>Or enter manually:</label>";
    html += "<input type='text' id='manual-ssid' placeholder='Enter WiFi SSID manually'>";
    html += "<label>Password:</label>";
    html += "<input type='password' name='password' placeholder='Enter WiFi Password'>";
    html += "<button type='submit'>Save & Connect</button>";
    html += "</form>";
    html += "<script>";
    html += "document.getElementById('manual-ssid').addEventListener('input', function() {";
    html += "  if(this.value) document.getElementById('ssid').value = this.value;";
    html += "});";
    html += "document.getElementById('ssid').addEventListener('change', function() {";
    html += "  document.getElementById('manual-ssid').value = '';";
    html += "});";
    html += "</script>";
    html += "</div></body></html>";
    request->send(200, "text/html", html);
  });
  
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
    String ssid = "";
    String password = "";
    
    if (request->hasParam("ssid", true)) {
      ssid = request->getParam("ssid", true)->value();
    }
    if (request->hasParam("password", true)) {
      password = request->getParam("password", true)->value();
    }
    
    preferences.putString("wifi_ssid", ssid);
    preferences.putString("wifi_password", password);
    
    String html = "<!DOCTYPE html><html><head><title>Saved</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<meta http-equiv='refresh' content='5;url=/'>";
    html += "<style>body{font-family:Arial;margin:40px;background:#f0f0f0;text-align:center;}";
    html += ".container{background:white;padding:30px;border-radius:10px;box-shadow:0 4px 6px rgba(0,0,0,0.1);}";
    html += "h1{color:#4CAF50;}</style></head><body>";
    html += "<div class='container'><h1>Settings Saved!</h1>";
    html += "<p>ESP32 will restart and connect to your WiFi network.</p>";
    html += "<p>Check the LCD display for the IP address.</p>";
    html += "<p>This page will redirect in 5 seconds...</p></div></body></html>";
    request->send(200, "text/html", html);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Saved!");
    lcd.setCursor(0, 1);
    lcd.print("Restarting...");
    
    delay(3000);
    ESP.restart();
  });
  
  server.onNotFound([](AsyncWebServerRequest *request){
    request->redirect("/");
  });
  
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
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
  sensorData.soilMoisture = map(analogRead(SOIL_MOISTURE_PIN), 0, 4095, 0, 100);

  sensorData.buttonPressed = !digitalRead(PUSH_BUTTON_PIN);
  sensorData.motionDetected = digitalRead(PIR_PIN);

  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  sensorData.distance = duration * 0.034 / 2;

  // Process potentiometer mapping
  processPotentiometerMapping();
}

void processPotentiometerMapping() {
  if (potMappingMode == "none") {
    potMappedValue = 0;
    return;
  }

  int rawPotValue = analogRead(POTENTIOMETER_PIN); // 0-4095

  if (potMappingMode == "led") {
    // Map to LED PWM (0-255)
    potMappedValue = map(rawPotValue, 0, 4095, 0, 255);
    setLEDManual(true, potMappedValue, false);
  } else if (potMappingMode == "rgb") {
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
  } else if (potMappingMode == "servo") {
    // Map to Servo angle (0-180)
    potMappedValue = map(rawPotValue, 0, 4095, 0, 180);
    servoPosition = potMappedValue;
    servo.write(servoPosition);
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
      //if (customLcdText1.length() <= 16) {
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
            lcd.setCursor(0, 0);
            lcd.print(hotspotSSID);
            lcd.setCursor(0, 1);
            lcd.print("IP: 192.168.4.1");
          }
          break;
      }
      displayMode = (displayMode + 1) % 4;
      lastUpdate = millis();
    }
  }
}

void loadRules() {
  ruleCount = preferences.getInt("ruleCount", 0);

  for (int i = 0; i < ruleCount && i < MAX_RULES; i++) {
    String prefix = "rule" + String(i) + "_";
    rules[i].enabled = preferences.getBool((prefix + "enabled").c_str(), true);
    rules[i].name = preferences.getString((prefix + "name").c_str(), "Rule " + String(i + 1));
    rules[i].condition = preferences.getString((prefix + "condition").c_str(), "temperature");
    rules[i].operator_type = preferences.getString((prefix + "operator").c_str(), ">");
    rules[i].threshold = preferences.getFloat((prefix + "threshold").c_str(), 25.0);
    rules[i].action = preferences.getString((prefix + "action").c_str(), "led_on");
    rules[i].actionValue = preferences.getInt((prefix + "actValue").c_str(), 0);
    rules[i].actionDelay = preferences.getFloat((prefix + "actDelay").c_str(), 0.0);
    rules[i].lcdText1 = preferences.getString((prefix + "lcdText1").c_str(), "");
    rules[i].lcdText2 = preferences.getString((prefix + "lcdText2").c_str(), "");
    rules[i].lcdMode = preferences.getString((prefix + "lcdMode").c_str(), "custom");
  }
}

void saveRules() {
  preferences.putInt("ruleCount", ruleCount);

  for (int i = 0; i < ruleCount; i++) {
    String prefix = "rule" + String(i) + "_";
    preferences.putBool((prefix + "enabled").c_str(), rules[i].enabled);
    preferences.putString((prefix + "name").c_str(), rules[i].name);
    preferences.putString((prefix + "condition").c_str(), rules[i].condition);
    preferences.putString((prefix + "operator").c_str(), rules[i].operator_type);
    preferences.putFloat((prefix + "threshold").c_str(), rules[i].threshold);
    preferences.putString((prefix + "action").c_str(), rules[i].action);
    preferences.putInt((prefix + "actValue").c_str(), rules[i].actionValue);
    preferences.putFloat((prefix + "actDelay").c_str(), rules[i].actionDelay);
    preferences.putString((prefix + "lcdText1").c_str(), rules[i].lcdText1);
    preferences.putString((prefix + "lcdText2").c_str(), rules[i].lcdText2);
    preferences.putString((prefix + "lcdMode").c_str(), rules[i].lcdMode);
  }
}

void processRules() {
  for (int i = 0; i < ruleCount; i++) {
    if (!rules[i].enabled) continue;
    
    bool conditionMet = evaluateCondition(rules[i]);
    
    if (conditionMet) {
      executeAction(rules[i]);
    }
  }
}

bool evaluateCondition(AutomationRule rule) {
  float sensorValue = getSensorValue(rule.condition);
  
  if (rule.operator_type == ">") {
    return sensorValue > rule.threshold;
  } else if (rule.operator_type == "<") {
    return sensorValue < rule.threshold;
  } else if (rule.operator_type == ">=") {
    return sensorValue >= rule.threshold;
  } else if (rule.operator_type == "<=") {
    return sensorValue <= rule.threshold;
  } else if (rule.operator_type == "==") {
    return abs(sensorValue - rule.threshold) < 0.1;
  }
  
  return false;
}

float getSensorValue(String condition) {
  if (condition == "temperature") return sensorData.temperature;
  if (condition == "humidity") return sensorData.humidity;
  if (condition == "ds18b20Temp") return sensorData.ds18b20Temp;
  if (condition == "lightLevel") return sensorData.lightLevel;
  if (condition == "gasLevel") return sensorData.gasLevel;
  if (condition == "soilMoisture") return sensorData.soilMoisture;
  if (condition == "distance") return sensorData.distance;
  if (condition == "potValue") return sensorData.potValue;
  if (condition == "motionDetected") return sensorData.motionDetected ? 1.0 : 0.0;
  if (condition == "buttonPressed") return sensorData.buttonPressed ? 1.0 : 0.0;
  
  return 0.0;
}

String getRulesJSON() {
  DynamicJsonDocument doc(2048);
  JsonArray rulesArray = doc.createNestedArray("rules");

  for (int i = 0; i < ruleCount; i++) {
    JsonObject ruleObj = rulesArray.createNestedObject();
    ruleObj["id"] = i;
    ruleObj["enabled"] = rules[i].enabled;
    ruleObj["name"] = rules[i].name;
    ruleObj["condition"] = rules[i].condition;
    ruleObj["operator"] = rules[i].operator_type;
    ruleObj["threshold"] = rules[i].threshold;
    ruleObj["action"] = rules[i].action;
    ruleObj["actionValue"] = rules[i].actionValue;
    ruleObj["actionDelay"] = rules[i].actionDelay;
    ruleObj["lcdText1"] = rules[i].lcdText1;
    ruleObj["lcdText2"] = rules[i].lcdText2;
    ruleObj["lcdMode"] = rules[i].lcdMode;
  }

  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

void setRelayWithDelay(bool state) {
  if (state && relayOnDelay > 0) {
    relayDelayActive = true;
    relayTargetState = true;
    relayTimer = millis();
  } else if (!state && relayOffDelay > 0) {
    relayDelayActive = true;
    relayTargetState = false;
    relayTimer = millis();
  } else {
    relayState = state;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  }
}

void handleRelayDelay() {
  if (!relayDelayActive) return;
  
  unsigned long delayTime = relayTargetState ? (relayOnDelay * 1000) : (relayOffDelay * 1000);
  
  if (millis() - relayTimer >= delayTime) {
    relayState = relayTargetState;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    relayDelayActive = false;
  }
}