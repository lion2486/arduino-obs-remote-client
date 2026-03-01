# ESP32 OBS Studio Recording Controller

A wireless, physical macro pad using an ESP32 microcontroller to remotely control OBS (Open Broadcaster Software) over a local WiFi network. 

It utilizes the OBS WebSocket API (v5) to send commands and listen for real-time status updates. A hardware push-button acts as a physical toggle switch to start and stop recording, while an integrated WS2812 RGB LED provides visual feedback of the current OBS state.

## Features
* **Hardware Recording Toggle:** Start and stop OBS recording with a physical button press.
* **Real-time Sync:** Listens to OBS events, meaning the ESP32 knows if you started recording via your PC and updates the LED accordingly.
* **Hardware Debouncing:** Utilizes non-blocking, interrupt-driven software debouncing for reliable, miss-free button presses.
* **Visual Status Indicator:** NeoPixel LED clearly shows the current state of the connection and recording.

## Hardware Required
* 1x **ESP32 Microcontroller** (Tested on DFRobot FireBeetle ESP32-E)
* 1x **Push Button** (Momentary switch)
* 1x **WS2812 / NeoPixel RGB LED**
* Jumper wires & breadboard



## Software Dependencies
This project requires the following Arduino libraries. You can install them via the Arduino IDE Library Manager:
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) - For parsing and generating WebSocket payloads.
* [ArduinoWebsockets](https://github.com/gilmaimon/ArduinoWebsockets) - For handling the real-time connection to OBS.
* [FastLED](https://github.com/FastLED/FastLED) - For controlling the RGB LED.

## Setup & Installation

### 1. OBS Studio Configuration
1. Open OBS Studio.
2. Go to **Tools** -> **WebSocket Server Settings**.
3. Check **Enable WebSocket server**.
4. Note your **Server Port** (Default is usually `4455`).
5. Note your PC's local IP address.



### 2. Microcontroller Configuration
Open the `.ino` file in the Arduino IDE and update the following variables at the top of the file to match your network and OBS settings:

```cpp
// WIFI
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";

// Websockets
const char* websockets_server_host = "YOUR_PC_LOCAL_IP"; 
const uint16_t websockets_server_port = 4455;