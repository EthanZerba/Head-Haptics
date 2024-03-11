#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

const char* ssid = "yourSSID";
const char* password = "yourPASSWORD";

bool usbConnected = false;
unsigned long startMillis;
const unsigned long usbTimeout = 5000; // 5 seconds to wait for USB connection

WiFiUDP Udp;
const int localPort = 8888; // Choose an appropriate port for OSC

const int rightMotorPin = 25; // Example GPIO pin for right motor
const int leftMotorPin = 26; // Example GPIO pin for left motor
const int freq = 5000; // Frequency for PWM signal
const int rightMotorChannel = 0; // PWM channel for right motor
const int leftMotorChannel = 1; // PWM channel for left motor
const int resolution = 8; // Resolution for PWM signal (8 bits = 0-255)

void headRightHandler(OSCMessage &msg) {
  float intensity = msg.getFloat(0);
  Serial.print("Right Intensity: ");
  Serial.println(intensity);
  // Map intensity to PWM value (assuming intensity is 0.0 to 1.0)
  int pwmValue = intensity * 255;
  ledcWrite(rightMotorChannel, pwmValue);
}

void headLeftHandler(OSCMessage &msg) {
  float intensity = msg.getFloat(0);
  Serial.print("Left Intensity: ");
  Serial.println(intensity);
  // Map intensity to PWM value (assuming intensity is 0.0 to 1.0)
  int pwmValue = intensity * 255;
  ledcWrite(leftMotorChannel, pwmValue);
}

void setup() {
  Serial.begin(115200);
  startMillis = millis();
  
  // Setup PWM for motors
  ledcSetup(rightMotorChannel, freq, resolution);
  ledcAttachPin(rightMotorPin, rightMotorChannel);
  ledcSetup(leftMotorChannel, freq, resolution);
  ledcAttachPin(leftMotorPin, leftMotorChannel);

  while (millis() - startMillis < usbTimeout && !usbConnected) {
    if (Serial.available() > 0) {
      String msg = Serial.readString();
      if (msg.startsWith("connect")) {
        usbConnected = true;
        Serial.println("USB connection established.");
      }
    }
  }

  if (!usbConnected) {
    Serial.println("Switching to WiFi mode...");
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("WiFi connected.");
    Udp.begin(localPort);
    Serial.print("Listening for OSC messages on port ");
    Serial.println(localPort);
  }
}

void loop() {
  if (usbConnected) {
    // Handle USB communication
    if (Serial.available() > 0) {
      String msg = Serial.readString();
      // Process USB message
    }
  } else {
    // Handle WiFi communication
    int packetSize = Udp.parsePacket();
    if (packetSize) {
      OSCMessage oscMsg;
      while (packetSize--) {
        oscMsg.fill(Udp.read());
      }
      if (!oscMsg.hasError()) {
        oscMsg.dispatch("/avatar/parameters/headRight", headRightHandler);
        oscMsg.dispatch("/avatar/parameters/headLeft", headLeftHandler);
      }
    }
  }
}
