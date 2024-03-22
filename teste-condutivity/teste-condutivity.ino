/* 
  Sketch generated by the Arduino IoT Cloud Thing "IoT TDS Meter"
  https://create.arduino.cc/cloud/things/a8128a86-ec66-4926-a475-baa36c16c643 

  Arduino IoT Cloud Variables description

  The following variables are automatically generated and updated when changes are made to the Thing

  float ecValue;
  float tDS;
  float temperature;

  Variables which are marked as READ/WRITE in the Cloud Thing will also have functions
  which are called when their values are changed from the Dashboard.
  These functions are generated with the Thing and added at the end of this sketch.
*/

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DFRobot_ESP_EC.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
 
#define ONE_WIRE_BUS 13                // this is the gpio pin 18 on esp32.
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
 
DFRobot_ESP_EC ec;
 
float voltage, temperature, ecValue, tds;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(32);//needed EEPROM.begin to store calibration k in eeprom
  pinMode(27, INPUT);
  ec.begin();
  sensors.begin();
  delay(2000);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
}

void loop() {
  sensors.requestTemperatures();
  voltage = (analogRead(27) * (3.3 / 1024.0));
  Serial.println(voltage);
  temperature = sensors.getTempCByIndex(0);  // read your temperature sensor to execute temperature compensation
  ecValue = ec.readEC(voltage, temperature); // convert voltage to EC with temperature compensation
  tds = ((ecValue * 1000) / 2);
 
  Serial.print("Temperature:");
  Serial.print(temperature, 2);
  Serial.println("ºC");
  
  Serial.print("TDS:");
  Serial.print(tds, 2);
  Serial.println("PPM");
 
  Serial.print("EC:");
  Serial.println(ecValue, 2);
 
  ec.calibration(voltage, temperature); // calibration process by Serail CMD
}