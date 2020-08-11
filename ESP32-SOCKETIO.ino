enum connectionStatus {
  pairing,
  paired,
  detached
};

connectionStatus connection = detached;

//#define STAGING

#define VERSION "v0.2"
#define ESP32

#define WIFICONNECTTIMEOUT 60000

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//socketIO
#include <SocketIoClient.h>
/* HAS TO BE INITIALISED BEFORE WEBSOCKETSCLIENT LIB */
SocketIoClient socketIO;

//Local Websockets
#include <WebSocketsClient.h>


#include <Preferences.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiAP.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>


//Access Point credentials
String scads_ssid = "";
String scads_pass = "blinksandbleeps";

const byte DNS_PORT = 53;
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);

Preferences preferences;

//local server
AsyncWebServer server(80);
AsyncWebSocket socket_server("/ws");

//local websockets client
WebSocketsClient socket_client;
byte webSocketClientID;


WiFiMulti wifiMulti;

//state handling variables
bool isClient = false;
String wifiCredentials = "";
String macCredentials = "";
bool hasPairedMac = false;
bool setupFinished = false;


/// Led Settings ///
int onBoardLed = 2;
bool isBlinking = false;
bool readyToBlink = false;
unsigned long blinkTime;
int blinkDuration = 200;

//Button Settings
int buttonPin = 0;
bool buttonDebounce;
unsigned long buttonPressTime;

/// Socket.IO Settings ///
#ifndef STAGING
char host[] = "irs-socket-server.herokuapp.com"; // Socket.IO Server Address
#else
char host[] = "irs-socket-server-staging.herokuapp.com"; // Socket.IO Staging Server Address
#endif
int port = 80; // Socket.IO Port Address
char path[] = "/socket.io/?transport=websocket"; // Socket.IO Base Path


void setup() {
  Serial.begin(115200);
  pinMode(onBoardLed, OUTPUT);
  pinMode(buttonPin, INPUT);

  //Check if device already has a pair macaddress
  preferences.begin("scads", false);
  wifiCredentials = preferences.getString("wifi", "");
  macCredentials = preferences.getString("mac", "");
  preferences.end();
  Serial.println("Stored wifi and mac addresses");
  Serial.println(macCredentials);
  if (macCredentials != "") {
    if (getMacJSONSize() < 2) {
      //check it has a paired mac address
      Serial.println(macCredentials);
      hasPairedMac = false;
      Serial.println("Already have local mac address in preferences, but nothing else");
    } else {
      hasPairedMac = true;
      Serial.println("Already has paired mac address");
    }
  } else {
    hasPairedMac = false;
    Serial.println("setting up JSON database for mac addresses");
    preferences.clear();
    addToMacAddressJSON(WiFi.macAddress());
  }
  if (hasPairedMac == false) {
    Serial.println("Scanning for available SCADS");
    scanningForSCADS();
    if (isClient == false) {
      //become server
      createSCADSAP();
      setupCaptivePortal();
      setupLocalServer();
      connection = detached;
    } else {
      //become client
      setupSocketClientEvents();
    }
  } else {
    Serial.print("List of Mac addresses:");
    Serial.println(macCredentials);
    //connect to router to talk to server
    connection = paired;
    if (wifiCredentials != "") {
      connectToWifi(wifiCredentials);
      setupSocketIOEvents();
      setupFinished = true;
    } else {
      Serial.println("wifi setup failed, clearing credentials for you to please try again");
      preferences.begin("scads", false);
      preferences.clear();
      preferences.end();
      ESP.restart();
    }
  }
}

void loop() {

  if (isClient == false) {
    //Local Server Mode
    socket_server.cleanupClients();
    dnsServer.processNextRequest();
  } else if (isClient == true && setupFinished == false) {
    //Local Client Mode
    socket_client.loop();
  } else if (setupFinished == true) {
    //Connected Mode
    socketIO.loop();
    buttonHandler();
    ledHandler();
  }
}

void blinkDevice() {
  if (readyToBlink == false) {
    readyToBlink = true;
  }
}

void ledHandler() {
  if (readyToBlink == true && isBlinking == false) {
    isBlinking = true;
    blinkTime = millis();
    digitalWrite(onBoardLed, 1);
  }
  if (millis() - blinkTime > blinkDuration && isBlinking == true) {
    digitalWrite(onBoardLed, 0);
    isBlinking = false;
    readyToBlink = false;
  }
}

void blinkOnConnect() {
  byte NUM_BLINKS = 3;
  for (byte i = 0; i < NUM_BLINKS; i++) {
    digitalWrite(onBoardLed, 1);
    delay(100);
    digitalWrite(onBoardLed, 0);
    delay(400);
  }
}

void buttonHandler() {
  const bool buttonState = digitalRead(buttonPin);
  if (!buttonState && buttonDebounce == false) {
    buttonPressTime = millis();
    buttonDebounce = true;
    socketIO_sendButtonPress();
  }
  if (buttonState && buttonDebounce == true && millis() - buttonPressTime > 500) {
    buttonDebounce = false;
  }
}
