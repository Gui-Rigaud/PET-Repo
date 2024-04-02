#include <Adafruit_ADS1X15.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ========================================================================================================
// --- Constantes Auxiliares ---
#define ONE_WIRE_BUS 13

// ========================================================================================================
// --- Declaração de Objetos ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = { 0x28, 0xB7, 0x5B, 0x80, 0xE3, 0xE1, 0x3C, 0x2F };

Adafruit_ADS1115 ads;     /* Use this for the 12-bit version */
float tdsValue = 0;
float calibration = 21.94;

void setup(void)
{
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
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  sensors.begin();  //inicializa sensores

  sensors.setResolution(insideThermometer, 10);

  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
}

void loop(void)
{
 // Recebimento dos dados em formato de tensão

  int16_t adc0, adc1;
  float volts0, volts1;

  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);

  volts0 = ads.computeVolts(adc0);
  volts1 = ads.computeVolts(adc1);

  Serial.println("\n---------------------Tensões de Entrada-----------------------------");
  
  Serial.print("\nTensão de entrada do pHmetro: ");
  Serial.print(volts0);
  Serial.println(" V");
  Serial.print("\nTensão de entrada do TDS: ");
  Serial.println(volts1);
  Serial.println(" V");
  
 // Declaração das fórmulas
  
  float ecValue = 996.142*volts1 - 39.846;

  if(ecValue <= 0) {
    ecValue = 0.04;
  }
  
  sensors.requestTemperatures();
  float temp = sensors.getTempC(insideThermometer);
  float tempCalibration = 0.02*(temp - 25);
  float declive = -0.228 * 25;

  float ecCalibrated = ecValue*(1.0 + tempCalibration);

  float ph = declive * volts0 + calibration;
  
  Serial.println("\n----------------------Medições------------------------------");

  Serial.print("\nTemperatura:");
  Serial.println(temp);
  
  Serial.print ("\npH: ");
  Serial.println(ph);

  Serial.print("\nCondutividade em uS/cm: ");
  Serial.println(ecCalibrated);

  delay(1000);

}