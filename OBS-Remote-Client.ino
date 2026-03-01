/**
 * Project: ESP32 OBS Studio Recording Controller
 * Board used: FireBeetle 2 ESP32-E 
 * * Description: 
 * A wireless, physical macro pad using an ESP32 microcontroller to remotely 
 * control OBS (Open Broadcaster Software) over a local WiFi network. 
 * * It utilizes the OBS WebSocket API to send commands and listen for real-time 
 * status updates. A hardware push-button (with interrupt-driven software 
 * debouncing) acts as a physical toggle switch to start and stop recording. 
 * An integrated WS2812 RGB LED (NeoPixel) provides visual feedback of the 
 * current OBS state:
 * * LED Status Indicators:
 * - Blue:      Initializing / Powered On
 * - Green:     OBS Connected / Standby (Not Recording)
 * - Red:       OBS is actively Recording
 * - DarkCyan:  JSON Parsing Error / Deserialization Failure
 * * Hardware: DFRobot FireBeetle ESP32, Push Button, WS2812 RGB LED
 * Dependencies: ArduinoJson, ArduinoWebsockets, FastLED
 * Protocol: OBS Websocket Protocol v5 https://github.com/obsproject/obs-websocket/blob/master/docs/generated/protocol.md
*/
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <ArduinoWebsockets.h> // https://github.com/gilmaimon/ArduinoWebsockets
#include <WiFi.h> // https://wiki.dfrobot.com/FireBeetle_Board_ESP32_E_SKU_DFR0654#10.3%20WiFi
#include <FastLED.h> // https://wiki.dfrobot.com/FireBeetle_Board_ESP32_E_SKU_DFR0654#9.3%20RGB%20LED

// WIFI
const char *ssid = "*****";  // Change this to your WiFi SSID
const char *password = "*****";  // Change this to your WiFi password

// Websockets
const char* websockets_server_host = "192.168.1.142"; //Enter server adress
const uint16_t websockets_server_port = 4455; // Enter server port
using namespace websockets;
WebsocketsClient client;

// FAST LED
#define NUM_LEDS 1     //Number of RGB LED beads
#define DATA_PIN D8    //The pin for controlling RGB LED
#define LED_TYPE NEOPIXEL    //RGB LED strip type
CRGB leds[NUM_LEDS];    //Instantiate RGB LED

struct Button {          //Define the button struct
    const uint8_t PIN;   //Define button pin
    bool pressed;        //Determine if the button is pressed, return true if it's pressed
    volatile unsigned long lastPressTime;
};
Button button = {27, false, 0};     //Instantiated a button, and use the on-board button.
void ARDUINO_ISR_ATTR isr() {    //Interrupt processing function
    unsigned long currentTime = millis();
    
    // If 250ms have passed since the last trigger, it's a real press
    if (currentTime - button.lastPressTime > 250) { 
        button.pressed = true;
        button.lastPressTime = currentTime;
    }
}

void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    const char *data = message.c_str();
   
    Serial.println(data);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());

      leds[0] = CRGB::DarkCyan;
      FastLED.show();
      
      return;
    }

    if (doc["op"] == 5){
      if (!strcmp(doc["d"]["eventType"].as<const char*>(), "RecordStateChanged")){
        if (doc["d"]["eventData"]["outputActive"]){
          leds[0] = CRGB::Red;
          FastLED.show();
        } else {
          leds[0] = CRGB::Green;
          FastLED.show();
        }
      }
    } else if(doc["op"] == 7) {
      if (!strcmp(doc["d"]["requestType"].as<const char*>(), "GetRecordStatus")){
         if (doc["d"]["responseData"]["outputActive"]){
          leds[0] = CRGB::Red;
          FastLED.show();
         } else {
          leds[0] = CRGB::Green;
          FastLED.show();
        }
      }
    }
}

void toggleRecording() {
  if (client.available()) {
    client.send("{\"op\": 6,\"d\": {\"requestType\": \"ToggleRecord\",\"requestId\": \"f819dcf0-89cc-11eb-8f0e-382c4ac93b9c\"}}");
  } else {
     Serial.println("client not available");
  }
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("WS Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("WS Connnection Closed");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);
  leds[0] = CRGB::Blue;
  FastLED.show();

  // Button setup and interrupt
  pinMode(button.PIN, INPUT_PULLUP);   
  attachInterrupt(button.PIN, isr, FALLING);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup Websocket Callbacks
  client.onMessage(onMessageCallback);
  client.onEvent(onEventsCallback);
  
  // Connect to server
  Serial.print("connecting to WS: ");
  Serial.println(websockets_server_host);
  bool connected = client.connect(websockets_server_host, websockets_server_port, "/");
  if(connected) {
      Serial.println("WS Connected!");
      // client.send("{\"op\": 1,\"d\": {\"rpcVersion\": 1,\"eventSubscriptions\": 33}}"); 
      client.send("{\"op\": 1,\"d\": {\"rpcVersion\": 1}}"); 
      // TODO: Authentication goes here
      // TODO: eventsScubscription bitmap
      client.send("{\"op\": 6, \"d\": { \"requestType\": \"GetRecordStatus\", \"requestId\": \"f819dcf0-89cc-11eb-8f0e-382c4ac93b9c\"}}");

  } else {
      Serial.println("Not Connected!");
  }
}

void loop() {
  if(client.available()) {
      client.poll();
  }

  if (button.pressed) {
    button.pressed = false;

    leds[0] = CRGB::Pink;
    FastLED.show();

    Serial.printf("ToggleRecording in loop called");
    toggleRecording();
   
  }
  delay(50);
}
