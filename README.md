# 🌟 Glow Guide - ESP32 Skincare Routine Tracker

A smart skincare routine tracker that runs on an ESP32 microcontroller wirelessly. It displays your personalized morning and night skincare routines based on the current time, with progress tracking and beautiful UI.(Mobile and desktop)

If your skincare routine is complex, it can get hard to remember it by order, with this project, u simply hardcode your routine once and itll make an interactive to-do list form of webpage that runs locally on your wifi (Make sure the esp32chip and ur device r connected to the same wifi)

![Preview Screenshot](/preview.jpg) 

## ✨ Features

- **Time-aware routines**: Automatically shows morning or night routine based on current time
- **Day-specific routines**: Different night routines for each day of the week
- **Progress tracking**: Check off completed steps and see your progress
- **Beautiful UI**: Clean, responsive interface with light/dark mode toggle
- **Hand care included**: Separate routine for hand care or such (You can costomize it)
- **WiFi auto-connect**: Connects to available WiFi network and reconnects automatiically
- **Self-contained**: Everything in one Arduino sketch file

## 🛠 Hardware Requirements

- ESP32 development board (any variant)
- Micro USB cable for power/programming
- Computer with Arduino IDE installed

## 🚀 Installation

1. **Clone this repository** or download the `.ino` file
2. **Open the file in Arduino IDE**
3. **Install required libraries**:
   - WiFi
   - AsyncTCP
   - ESPAsyncWebServer
4. **Modify the configuration** (see below)
5. **Degrade your ESP32 bord to a lower version for compatibility issues**![image](https://github.com/user-attachments/assets/59bc80eb-8a09-4a59-a671-fe57d713fbc9)
6. **Upload to your ESP32**
7. **Connect to the ESP32's IP address** in your web browser

## ⚙️ Configuration

Edit the **User Configuration Section** at the top of the file to customize:

### WiFi Networks
```cpp
const char* wifiCredentials[][2] = {
  {"YourWiFiSSID1", "YourPassword1"},
  {"YourWiFiSSID2", "YourPassword2"},
  // Add more networks as needed
};
