#include <Adafruit_ADS1X15.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include <WiFi.h>

// ========================================================================================================
// --- Constantes Auxiliares ---
#define ONE_WIRE_BUS 13

#define WIFI_AP "fontes"
#define WIFI_PASS "fontes123"

#define TB_SERVER "demo.thingsboard.io"
#define TOKEN "O6AdSUAbcmf1cLD7MkAC" //Reator 2


// ========================================================================================================
// --- Declaração de Objetos ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = { 0x28, 0xE9, 0x82, 0x80, 0xE3, 0xE1, 0x3C, 0xBF };

constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;

WiFiClient espClient;

Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

Adafruit_ADS1115 ads; /* Use this for the 12-bit version */
float tdsValue = 0;
float calibration = 21.94;

void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello!");

  // Serial.println("Getting single-ended readings from AIN0..3");
  // Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.setGain(GAIN_ONE);  // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  sensors.begin();  //inicializa sensores

  sensors.setResolution(insideThermometer, 10);

  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1)
      ;
  }
}

void loop(void) {
  // if (WiFi.status() != WL_CONNECTED) {
  //   initWiFi();
  // }

  // if (!tb.connected()) {
  //   Serial.println("Conectando ao servidor...");
  //   if (!tb.connect(TB_SERVER, TOKEN)) {
  //     Serial.println("Não foi possivel conectar");
  //     return;
  //   }
  // } else {
  //   Serial.println("Conectado ao Servidor!");
  // }

  // Recebimento dos dados em formato de tensão

  int16_t adc0, adc1, adc2;
  float volts0, volts1, volts2;

  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);

  volts0 = ads.computeVolts(adc0);
  volts1 = ads.computeVolts(adc1);
  volts2 = ads.computeVolts(adc2);

  // ========================================================================================================

  // Print das tensões de entrada
  Serial.println("\n---------------------Tensões de Entrada-----------------------------");

  Serial.print("\nTensão de entrada do pHmetro: ");
  Serial.print(volts0);
  Serial.println(" V");
  Serial.print("\nTensão de entrada do TDS: ");
  Serial.println(volts2);
  Serial.println(" V");
  Serial.print("\nTensão de entrada do Turbidímetro: ");
  Serial.println(volts1);
  Serial.println(" V");

  // Declaração das fórmulas

  // Temperatura
  sensors.requestTemperatures();
  float somatemp = 0;

  // Média móvel da temperatura
  for (int i = 0; i < 100; i++) {
    float temp = sensors.getTempC(insideThermometer);

    somatemp = somatemp + temp;
    delay(2);
  }
  float valortemp = somatemp / 100;

  float tempCalibration = 0.02 * (valortemp - 25);

  // pH
  float declive = -0.228 * 25;
  float somaph = 0;

  // Média móvel do pH
  for (int i = 0; i < 100; i++) {
    float ph = declive * volts0 + calibration;

    somaph = somaph + ph;
    delay(2);
  }
  float valorph = somaph / 100;


  // Condutividade
  float somacond = 0;

  // Média móvel da condutividade
  for (int i = 0; i < 100; i++) {
    float parcela1 = 2.53 * volts2 * volts2 * volts2;
    float parcela2 = -5.16 * volts2 * volts2;
    float parcela3 = 3.18 * volts2;
    float parcela4 = -0.12;
    float ecValue = parcela1 + parcela2 + parcela3 + parcela4;
    float ecCalibrated = ecValue * (1.0 + tempCalibration);

    somacond = somacond + ecCalibrated;
    delay(2);
  }
  float valorcond = somacond / 100;

  // Turbidez
  float somaturb = 0;

  // Média móvel da turbidez
  for (int i = 0; i < 500; i++) {
    float primeiro_fator = (8058) * (volts1 * volts1 * volts1);
    float segundo_fator = (-40837)  * (volts1 * volts1);
    float terceiro_fator = 67690 * volts1;
    float quarto_fator = -36450;

    float ntu = primeiro_fator + segundo_fator + terceiro_fator + quarto_fator;

    somaturb = somaturb + ntu;
    delay(2);
  }
  float valorturb = somaturb / 500;

if (volts1 > 1.895){
  valorturb = 0;
}



  // ========================================================================================================

  // Print das medições
  Serial.println("\n----------------------Medições------------------------------");

  Serial.print("\nTemperatura:");
  Serial.println(valortemp);
  // tb.sendTelemetryData("Temp", valortemp);

  Serial.print("\npH: ");
  Serial.println(valorph);
  // tb.sendTelemetryData("pH", valorph);

  Serial.print("\nCondutividade em uS/cm: ");
  Serial.println(valorcond);
  // tb.sendTelemetryData("Condutivity", valorcond);

  Serial.print("\nTurbidez em NTU: ");
  Serial.println(valorturb);
  // tb.sendTelemetryData("NTU", valorturb);


  Serial.println("Enviando dados...");
  delay(1000);

  // tb.loop();
}

//  void initWiFi() {
//   WiFi.mode(WIFI_STA);
//    WiFi.begin(WIFI_AP, WIFI_PASS);
//    Serial.print("Connecting to WiFi ..");
//    while (WiFi.status() != WL_CONNECTED) {
//      Serial.print('.');
//      delay(1000);
//    }
//    Serial.println(WiFi.localIP());
//  }