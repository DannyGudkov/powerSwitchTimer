# Power Switch Timer with OLED Display and Rotary Encoder

This is an Arduino-based **smart timer controller** designed to control a solid-state relay (SSR) based on user-defined ON/OFF times. It features an OLED display interface and is controlled with a rotary encoder and a push button.

## 💡 Features

- ⏰ Programmable ON/OFF schedule with hour and minute precision  
- 🌀 Rotary encoder control for setting times and navigating pages  
- 👆 Single click to change operating mode; double click to adjust system time  
- 🌡️ Temperature and humidity readings from a DHT22 sensor  
- 🧠 EEPROM memory to store the schedule across power cycles  
- 📺 OLED Display (128x64) to show:
  - Current time  
  - ON and OFF timers  
  - Temperature & humidity (on second page)  
- 🔌 Solid-state relay control based on schedule

## 🔧 Hardware Used

- Arduino (Uno/Nano/Mega)  
- Rotary Encoder (with push button)  
- DHT22 sensor  
- DS1302 Real Time Clock (RTC) module  
- SSD1306 128x64 OLED Display (I2C)  
- Solid State Relay (SSR)  
- EEPROM (built-in)  
- Optional: pull-up resistors for encoder/button

## 📦 Libraries Required

Install the following libraries via Library Manager:

```cpp
#include <RotaryEncoder.h>
#include <Bounce2.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include <DHT.h>
#include <DS1302.h>
```

## 🖱 Controls

- **Rotate encoder**: Navigate menus, change values  
- **Single click**:
  - On main page: cycle between setting ON/OFF time  
- **Double click**:
  - Enter/exit system clock setting mode  

## 🧠 EEPROM Usage

- If EEPROM byte 0 equals `0x42`, stored ON/OFF times are loaded on boot.  
- Times are saved to EEPROM when leaving OFF minute setting.  

## 🖼️ Display Pages

- **Page 0**:
  - Time  
  - ON time  
  - OFF time  
  - Underline indicates active field for editing  
- **Page 1**:
  - Temperature (°C)  
  - Humidity (%)  

## 🧪 Setup

1. Connect components according to your pin setup:  
   - Rotary Encoder: pins 5 (DT), 6 (CLK), 4 (SW)  
   - SSR: pin 10  
   - DHT22: pin 16  
   - RTC (DS1302): pins 9, 8, 7  
2. Flash the sketch to your Arduino.  
3. Use the encoder to set your desired ON and OFF times.  
4. Optional: use double click to correct time via encoder.  

## 📄 License

This project is licensed under the **GNU General Public License v3.0**  
See the [LICENSE](./LICENSE) file for more details.

---

### 📸 Screenshots (Optional)

![powerSwitchTimer](https://github.com/user-attachments/assets/1f2cafa1-bfc2-4718-9319-fdf4058b882d)

---
