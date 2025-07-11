#include <RotaryEncoder.h>
#include <Bounce2.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include <DHT.h>
#include "RTClib.h"

RTC_DS3231 rtc;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
#define SSR_PIN 10

RotaryEncoder encoderMain(5, 6, RotaryEncoder::LatchMode::FOUR3);
const int SW = 4;
Bounce button = Bounce();

#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

int lastDetent = 0;

unsigned long previousMillis = 0, interval = 1000, lastInteractionTime = 0, lastClickTime = 0, currentClickTime = 0;
const unsigned long interactionTimeout = 10000, clickInterval = 300;

uint8_t currentHour, currentMinute, currentSecond = 0, newHr, newMin;
int onHour, onMinute, offHour, offMinute;
int page = 0;

float temp = 0.0;
float humidity = 0.0;
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 2000;

bool showSavedMessage = false, timeAdjusted = false;
unsigned long savedMessageStart = 0;

enum settingMenu {
  NORMAL,
  SET_ON_HOUR,
  SET_ON_MINUTE,
  SET_OFF_HOUR,
  SET_OFF_MINUTE,
  EXIT
};

enum timeSettingMenu {
  TIME_NORMAL,
  SET_HOUR,
  SET_MINUTE,
  EXIT2
};

settingMenu settingMode = NORMAL;
settingMenu lastSettingMode = NORMAL;
timeSettingMenu timeSettingMode = TIME_NORMAL;
timeSettingMenu lastTimeSettingMode = TIME_NORMAL;

bool onInterval() {
  int currentTime = currentHour * 60 + currentMinute;
  int onTime = onHour * 60 + onMinute;
  int offTime = offHour * 60 + offMinute;

  if (onTime < offTime) {
    return (currentTime >= onTime && currentTime < offTime);
  } else {
    return (currentTime >= onTime || currentTime < offTime);
  }
}

void setup() {
  pinMode(SW, INPUT_PULLUP);
  button.attach(SW);
  button.interval(50);
  Serial.begin(9600);
  unsigned long start = millis();
  while (!Serial && millis() - start < 3000);  // wait up to 3 seconds

  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    while (1); // Don't continue if RTC is missing
  }

  pinMode(SSR_PIN, OUTPUT);
  u8g2.begin();
  dht.begin();

  if (EEPROM.read(0) == 0x42) {
    onHour = EEPROM.read(1);
    onMinute = EEPROM.read(2);
    offHour = EEPROM.read(3);
    offMinute = EEPROM.read(4);    
  } else {
    onHour = 14;
    onMinute = 15;
    offHour = 16;
    offMinute = 17;
  }
}

void loop() {

  readEncoder();
  readButton();
  controlSSR();
  if (timeSettingMode == TIME_NORMAL) {
  readRTC();
  timeAdjusted = false;
  } 
  if ((settingMode != NORMAL || timeSettingMode != TIME_NORMAL) && millis() - lastInteractionTime > interactionTimeout) {
    settingMode = NORMAL;
    timeSettingMode = TIME_NORMAL;
    updateDisplay();
  }
  
  if (showSavedMessage && millis() - savedMessageStart >= 4000) {
    showSavedMessage = false;
    updateDisplay();
  }

  lastSettingMode = settingMode;
  lastTimeSettingMode = timeSettingMode;

  if (millis() - lastSensorRead >= sensorInterval) {
  lastSensorRead = millis();
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) temp = t;   // Only update if valid reading
  if (!isnan(h)) humidity = h;

  if (page == 1) updateDisplay();  // refresh display if sensor values shown
  }
}

void readEncoder() {
  encoderMain.tick();
  int detent = encoderMain.getPosition();
  if (detent != lastDetent) {
    lastInteractionTime = millis();

  if (settingMode != NORMAL && timeSettingMode == TIME_NORMAL) {
  if (detent > lastDetent) {
    switch (settingMode) {
      case SET_ON_HOUR:
      onHour = (onHour + 1) % 24;
      updateDisplay();
      break;
      case SET_ON_MINUTE:
      onMinute = (onMinute + 1) % 60;
      updateDisplay();
      break;
      case SET_OFF_HOUR:
      offHour = (offHour + 1) % 24;
      updateDisplay();
      break;
      case SET_OFF_MINUTE:
      offMinute = (offMinute + 1) % 60;
      updateDisplay();
      break;
      default:
      break;
    }
  } else if (detent < lastDetent) {
    switch (settingMode) {
      case SET_ON_HOUR:
      onHour = (onHour + 23) % 24;
      updateDisplay();
      break;
      case SET_ON_MINUTE:
      onMinute = (onMinute + 59) % 60;
      updateDisplay();
      break;
      case SET_OFF_HOUR:
      offHour = (offHour + 23) % 24;
      updateDisplay();
      break;
      case SET_OFF_MINUTE:
      offMinute = (offMinute + 59) % 60;
      updateDisplay();
      break;
      default:
      break;
    }
  }
  } else if (settingMode == NORMAL && timeSettingMode != TIME_NORMAL) {
    if (detent > lastDetent) {
    switch (timeSettingMode) {
      case SET_HOUR:
      newHr = (newHr + 1) % 24;
      updateDisplay();
      break;
      case SET_MINUTE:
      newMin = (newMin + 1) % 60;
      updateDisplay();
      break;
      default:
      break;
    }
    } else if (detent < lastDetent) {
    switch (timeSettingMode) {
      case SET_HOUR:
      newHr = (newHr + 23) % 24;
      updateDisplay();
      break;
      case SET_MINUTE:
      newMin = (newMin + 59) % 60;
      updateDisplay();
      break;
      default:
      break;
    }
  }
  } else if (settingMode == NORMAL && timeSettingMode == TIME_NORMAL) {
    if (detent > lastDetent) {
      page = (page + 1) % 2;
      updateDisplay();
    } else if (detent < lastDetent) {
      page = (page - 1 + 2) % 2;
      updateDisplay();
      }
    }
  } lastDetent = detent;
}

void readButton() {
  button.update();

  if (button.fell() ) {
    currentClickTime = millis();
    lastInteractionTime = millis();
    if (currentClickTime - lastClickTime < clickInterval) {
      handleDoubleClick();
      lastClickTime = 0;
    } else {
      lastClickTime = currentClickTime;
    }
  }

  if (lastClickTime > 0 && (millis() - lastClickTime >= clickInterval)) {
    handleSingleClick();
    lastClickTime = 0;
  }
}

void handleSingleClick() {
  if (page == 0 && timeSettingMode == TIME_NORMAL) {
    settingMode = static_cast<settingMenu>((settingMode + 1) % 6);
    if (settingMode == EXIT) {
    storeTime();
    settingMode = NORMAL;
    }
    updateDisplay();
  }
}

void handleDoubleClick() {
  if (page == 0) {
    timeSettingMode = static_cast<timeSettingMenu>((timeSettingMode + 1) % 4);
    if (timeSettingMode == TIME_NORMAL) {
      timeAdjusted = false;
    } else if (!timeAdjusted && (timeSettingMode == SET_HOUR && lastTimeSettingMode == TIME_NORMAL)) {
    DateTime now = rtc.now();
    newHr = now.hour();
    newMin = now.minute();
    timeAdjusted = true;
    } else if (timeSettingMode == EXIT2) {
      DateTime now = rtc.now();
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), newHr, newMin, 0));
      readRTC();
      showSavedMessage = true;
      savedMessageStart = millis();
      timeSettingMode = TIME_NORMAL;
  }
    updateDisplay();
  }
}

void storeNewTime() {
    if (!timeAdjusted && (timeSettingMode == SET_HOUR && lastTimeSettingMode == TIME_NORMAL)) {
    newHr = currentHour;
    newMin = currentMinute;
    timeAdjusted = true;
    updateDisplay();
  }
}

void readRTC() {
  DateTime now = rtc.now();
  if (now.second() != currentSecond) {
    currentSecond = now.second();
    currentMinute = now.minute();
    currentHour = now.hour();
    updateDisplay();
  }
}

void controlSSR() {
  if (onInterval()) {
    digitalWrite(SSR_PIN, HIGH);
  } else {
    digitalWrite(SSR_PIN, LOW);
  }
}

void storeTime() {
  EEPROM.write(0, 0x42);
  EEPROM.write(1, onHour);
  EEPROM.write(2, onMinute);
  EEPROM.write(3, offHour);
  EEPROM.write(4, offMinute);
 
 showSavedMessage = true;
 savedMessageStart = millis();
 updateDisplay();
}

void updateDisplay() {

  if (page == 0) {
  char timeBuffer[9], onTimeBuffer[7], offTimeBuffer[7];
  if (timeSettingMode == SET_HOUR || timeSettingMode == SET_MINUTE) {
    sprintf(timeBuffer, "%02d:%02d:%02d", newHr, newMin, currentSecond);
  } else {
    sprintf(timeBuffer, "%02d:%02d:%02d", currentHour, currentMinute, currentSecond);
  }
  sprintf(onTimeBuffer, "%02d:%02d", onHour, onMinute);
  sprintf(offTimeBuffer, "%02d:%02d", offHour, offMinute);
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_profont22_tf);
  u8g2.setCursor(18, 14);
  u8g2.print(timeBuffer);
  u8g2.setCursor(18, 41);
  u8g2.print(onTimeBuffer);
  u8g2.setCursor(18, 61);
  u8g2.print(offTimeBuffer);

  if (settingMode == SET_ON_HOUR) {
    u8g2.drawLine(18, 42, 39, 42);
    u8g2.drawLine(18, 43, 39, 43);
  } else if (settingMode == SET_ON_MINUTE) {
    u8g2.drawLine(54, 42, 75, 42);
    u8g2.drawLine(54, 43, 75, 43);
  } else if (settingMode == SET_OFF_HOUR) {
    u8g2.drawLine(18, 62, 39, 62);
    u8g2.drawLine(18, 63, 39, 63);
  } else if (settingMode == SET_OFF_MINUTE) {
    u8g2.drawLine(54, 62, 75, 62);
    u8g2.drawLine(54, 63, 75, 63);
  } else if (timeSettingMode == SET_HOUR) {
    u8g2.drawLine(18, 15, 39, 15);
    u8g2.drawLine(18, 16, 39, 16);
  } else if (timeSettingMode == SET_MINUTE) {
    u8g2.drawLine(54, 15, 75, 15);
    u8g2.drawLine(54, 16, 75, 16);
  }

  } else if (page == 1) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_profont22_tf);
    u8g2.setCursor(25, 30);
    u8g2.print(temp, 1);
    u8g2.print(" C");
    u8g2.setFont(u8g2_font_profont10_tf);
    u8g2.setCursor(80, 20);
    u8g2.print("o");
    u8g2.setFont(u8g2_font_profont22_tf);
    u8g2.setCursor(25, 50);
    u8g2.print(humidity, 1);
    u8g2.print(" %");
  }

  if (showSavedMessage && page == 0) {
    u8g2.setCursor(85, 47);
    u8g2.setFont(u8g2_font_profont10_tf);
    u8g2.print("saved");
  }
  u8g2.sendBuffer();
}

