#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>
#include <DFRobot_ESP_EC.h>
#include <EEPROM.h>

const char *SSID = "YOUR_SSID";
const char *PWD = "YOUR_PASSWORD";

// const int red_pin = 5;   
// const int green_pin = 18; 
// const int blue_pin = 19; 

// Setting PWM frequency, channels and bit resolution
const int frequency = 5000;
// const int redChannel = 0;
// const int greenChannel = 1;
// const int blueChannel = 2;
const int resolution = 8;

//Inicia o Webserver na porta 80
WebServer server(80);

//Declara o documento json e o buffer
StaticJsonDocument<250> jsonDocument;
char buffer[250];

#define ONE_WIRE_BUS 18                // this is the gpio pin 18 on esp32.
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
 
DFRobot_ESP_EC ec;
Adafruit_ADS1115 ads;

//Declara as variáveis dos sensores conectados ao esp
float temperature;
float ecValue;
float voltage;
 
void setup_routing() {     
  server.on("/temperature", getTemperature);     
  server.on("/condutivity", getCondutivity);         
  server.on("/data", getData);     
  // server.on("/led", HTTP_POST, handlePost);    
          
  server.begin();    
}

//Função que irá gerar o arquivo Json
void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}

//Função que cria um objeto json
void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit; 
}


void read_sensor_data(void * parameter) {
   for (;;) {
      voltage = ads.readADC_SingleEnded(0) / 10;
      sensors.requestTemperatures();
      temperature = sensors.getTempCByIndex(0);  // read your temperature sensor to execute temperature compensation
      ecValue = ec.readEC(voltage, temperature);
      
      Serial.println("Read sensor data");
  
      vTaskDelay(60000 / portTICK_PERIOD_MS);
   }
}
 
void getTemperature() {
  Serial.println("Get temperature");
  create_json("temperature", temperature, "°C");
  server.send(200, "application/json", buffer);
}
 
void getCondutivity() {
  Serial.println("Get condutivity");
  create_json("condutivity", ecValue, "V");
  server.send(200, "application/json", buffer);
}
 
void getData() {
  Serial.println("Get Sensors Data");
  jsonDocument.clear();
  add_json_object("temperature", temperature, "°C");
  add_json_object("condutivity", ecValue, "V");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

// void handlePost() {
//   if (server.hasArg("plain") == false) {
//   }
//   String body = server.arg("plain");
//   deserializeJson(jsonDocument, body);

//   int red_value = jsonDocument["red"];
//   int green_value = jsonDocument["green"];
//   int blue_value = jsonDocument["blue"];

//   ledcWrite(redChannel, red_value);
//   ledcWrite(greenChannel,green_value);
//   ledcWrite(blueChannel, blue_value);

//   server.send(200, "application/json", "{}");
// }

void setup_task() {    
  xTaskCreate(     
  read_sensor_data,      
  "Read sensor data",      
  1000,      
  NULL,      
  1,     
  NULL     
  );     
}

void setup() {     
  Serial.begin(115200); 

  // ledcSetup(redChannel, frequency, resolution);
  // ledcSetup(greenChannel, frequency, resolution);
  // ledcSetup(blueChannel, frequency, resolution);
 
  EEPROM.begin(32);//needed EEPROM.begin to store calibration k in eeprom
  ec.begin();
  ads.setGain(GAIN_ONE);
  ads.begin();
  sensors.begin();  

  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());
  setup_task();    
  setup_routing();     
   
}    
       
void loop() {    
  server.handleClient();     
}