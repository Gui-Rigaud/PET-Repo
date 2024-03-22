/* ========================================================================================================
 
   Curso de Arduino e AVR 149
   
   WR Kits Channel


   Sensor de Temperatura DS18B20 Dallas

    
   Autor: Eng. Wagner Rambo  Data: Dezembro de 2017
   
   www.wrkits.com.br | facebook.com/wrkits | youtube.com/user/canalwrkits

   HARDWARE Termômetro olhando-o de frente:

   Terminal da direita   -> 5V do Arduino
   Terminal da esquerda  -> GND do Arduino
   Terminal central      -> digital 10 do Arduino (ligar um resistor de 4,7k entre esta entrada e os 5V)
   
   
======================================================================================================== */


// ========================================================================================================
// --- Bibliotecas Auxiliares ---
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include <WiFi.h>

#define WIFI_AP "fontes"
#define WIFI_PASS "fontes123"

#define TB_SERVER "thingsboard.cloud"
#define TOKEN "o0u5024pm2u0y9ozw82w"

constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;

WiFiClient espClient;

Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);


// ========================================================================================================
// --- Constantes Auxiliares ---
#define ONE_WIRE_BUS 13


// ========================================================================================================
// --- Declaração de Objetos ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = { 0x28, 0xB7, 0x5B, 0x80, 0xE3, 0xE1, 0x3C, 0x2F };


// ========================================================================================================
// --- Protótipo das Funções ---
float temp = 0.0;
void printTemperature(DeviceAddress deviceAddress);

// ========================================================================================================
// --- Configurações Iniciais ---
void setup() {

  Serial.begin(9600);  //inicializa comunicação serial

  initWiFi();

  sensors.begin();  //inicializa sensores

  sensors.setResolution(insideThermometer, 10);  //configura para resolução de 10 bits

}  //end setup


// ========================================================================================================
// --- Loop Infinito ---
void loop(void) {
  if (WiFi.status() != WL_CONNECTED) {
    initWiFi();
  }
  delay(10);
  Serial.println("Sensor DS18B20");
  sensors.requestTemperatures();
  printTemperature(insideThermometer);

  if (!tb.connected()) {
    Serial.println("Conectando ao servidor...");
    if (!tb.connect(TB_SERVER, TOKEN)) {
      Serial.println("Não foi possivel conectar");
      return;
    }
  } else {
    Serial.println("Conectado ao Servidor!");
  }

  tb.sendTelemetryData("Temp", temp);

  Serial.println("Enviando dados...");

  delay(1000);

  tb.loop();

  delay(1000);

}  //end loop


// ========================================================================================================
// --- Desenvolvimento das Funções ---
void printTemperature(DeviceAddress deviceAddress) {
  temp = sensors.getTempC(deviceAddress);

  if (temp == -127.00) {
    Serial.print("Erro de leitura");
  } else {
    Serial.print(temp);
    Serial.print(" °C ");
  }

  Serial.print("\n\r");

}  //end printTemperature


void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_AP, WIFI_PASS);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
