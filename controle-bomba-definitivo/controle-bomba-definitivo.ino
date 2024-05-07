#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "POCO X5 Pro 5G de Gui";
const char* password = "rigaud2212";
const char* mqttServer = "demo.thingsboard.io";
const int mqttPort = 1883;
const char* deviceToken = "O6AdSUAbcmf1cLD7MkAC"; // Token do dispositivo

int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 14;
String mensagem, value = "";
int pwm = 170;

const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  Serial.print("Mensagem: ");
  for (int i = 0; i < length; i++) {
    mensagem += (char)message[i];
  }

  value = mensagem.substring(mensagem.lastIndexOf('p')+8, mensagem.length()-1);
  pwm = map((int) value.toDouble(), 0, 120, 0, 255);
  ledcWrite(pwmChannel, pwm);
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando se conectar ao broker MQTT...");
    if (client.connect("ArduinoClient", deviceToken, NULL)) {
      Serial.println("Conectado");
      client.subscribe("v1/devices/me/rpc/request/+");
    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando se reconectar em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcWrite(pwmChannel, pwm);
  Serial.begin(115200);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  delay(500);

  client.loop();
}