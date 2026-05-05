#pragma once
// v1.1
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
enum SeqTimerType : uint8_t { STT_DURATION = 0, STT_INTERVAL = 1 };
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
};
