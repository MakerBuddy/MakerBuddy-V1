#pragma once
// v2.7
#include <Arduino.h>

enum SeqAction : uint8_t {
  SA_NONE = 0,
  SA_LED_ON = 1,
  SA_LED_OFF = 2,
  SA_LED_PWM = 3,
  SA_RGB_RED = 4,
  SA_RGB_GREEN = 5,
  SA_RGB_BLUE = 6,
  SA_RGB_YELLOW = 7,
  SA_RGB_PURPLE = 8,
  SA_RGB_CYAN = 9,
  SA_RGB_WHITE = 10,
  SA_RGB_OFF = 11,
  SA_BUZZER_ON = 12,
  SA_BUZZER_OFF = 13,
  SA_RELAY_ON = 14,
  SA_RELAY_OFF = 15,
  SA_SERVO_MOVE = 16,
  SA_LCD_DISPLAY = 17
};

enum SeqLcdMode : uint8_t { SLM_LCD_DEFAULT = 0, SLM_LCD_CUSTOM = 1 };
enum SeqLoopMode : uint8_t {
  SLM_NONE = 0,
  SLM_FOREVER = 1,
  SLM_TIMER = 2,
  SLM_CONDITION = 3
};
enum SeqTimerType : uint8_t { STT_INTERVAL = 0 };
enum SeqSensor : uint8_t {
  SS_TEMPERATURE = 0,
  SS_HUMIDITY = 1,
  SS_DS18B20 = 2,
  SS_LIGHT = 3,
  SS_GAS = 4,
  SS_SOIL = 5,
  SS_DISTANCE = 6,
  SS_POT = 7,
  SS_MOTION = 8,
  SS_BUTTON = 9
};
enum SeqOp : uint8_t {
  SOP_GT = 0,
  SOP_LT = 1,
  SOP_GTE = 2,
  SOP_LTE = 3,
  SOP_EQ = 4
};
enum SeqRangeOp : uint8_t { SROP_AND = 0, SROP_OR = 1 };

struct SequenceStep {
  uint8_t action;
  int actionValue;
  unsigned long delayMs;
  uint8_t lcdMode;
  char lcdText1[17];
  char lcdText2[17];
};

struct Sequence {
  bool enabled;
  char name[21];
  bool loop;
  bool autostart;
  uint8_t loopMode;

  uint8_t timerType;
  unsigned long loopDuration;
  unsigned long loopStartTime;

  unsigned long lastIntervalTime;

  uint8_t loopSensor;
  uint8_t loopOperator;
  float loopThreshold;
  bool loopUseRange;
  uint8_t loopRangeOperator;
  uint8_t loopOperator2;
  float loopThreshold2;
  bool manuallyStopped;

  int currentStep;
  unsigned long lastStepTime;
  bool isRunning;
  SequenceStep steps[10];
  int stepCount;

  bool conditionInit = false;
  bool conditionRaw = false;
  unsigned long conditionRawSince = 0;
  bool conditionDebounced = false;

  // Extended condition data is persisted separately from the legacy compact
  // sequence record so existing NVS sequence blobs remain compatible.
  bool loopUseSecondCondition = false;
  uint8_t loopSensor2 = SS_TEMPERATURE;
  uint8_t loopOperator3 = SOP_GT;
  float loopThreshold3 = 0.0f;
  bool loopUseRange2 = false;
  uint8_t loopRangeOperator2 = SROP_AND;
  uint8_t loopOperator4 = SOP_GT;
  float loopThreshold4 = 0.0f;
  uint8_t loopConditionOperator = SROP_AND;
};

struct SequenceConditionExtension {
  uint8_t version;
  uint8_t useSecondCondition;
  uint8_t sensor2;
  uint8_t operator3;
  float threshold3;
  uint8_t useRange2;
  uint8_t rangeOperator2;
  uint8_t operator4;
  float threshold4;
  uint8_t conditionOperator;
};

// Companion records: never resize SequenceStep/Sequence (legacy binary NVS).
struct ServoStepOptionsV1 {
  uint8_t relative;
  uint32_t movementMs;
};

struct SequenceServoExtensionV1 {
  uint8_t version;
  ServoStepOptionsV1 steps[10];
};

struct ServoStepOptions {
  uint8_t relative;
  uint32_t movementMs;
  uint8_t sweep; // 0: current position, 1: start -> destination, 2: return to start
  uint8_t startAngle;
};

struct SequenceServoExtension {
  uint8_t version;
  ServoStepOptions steps[10];
};

struct ServoMappingConfig {
  uint8_t version = 1;
  uint8_t sensor = SS_SOIL;
  float inputMin = 0;
  float inputMax = 100;
  uint8_t angleMin = 0;
  uint8_t angleMax = 180;
  uint8_t inverted = 0;
  uint8_t deadband = 3;
  uint8_t smoothing = 35; // Percent of the new sample used by the EMA.
};

struct ServoStepRuntime {
  int step = -1;
  bool started = false;
  int startAngle = 0;
  int targetAngle = 0;
  int departureAngle = 0;
  bool returning = false;
  uint32_t elapsedMs = 0;
  uint32_t resumeMs = 0;
};
