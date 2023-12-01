#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>

// Wifi information
const char* ssid = "Wokwi-GUEST";
const char* password = "";

//***Set server***
const char* mqttServer = "broker.hivemq.com"; 
int port = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

#define DHT11_PIN 8
#define soilPin A0
#define rs 12
#define en 11
#define d4 5
#define d5 4
#define d6 3
#define d7 2

DHT dht(DHT11_PIN, DHT11);
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int soilMoisture;

void wifiConnect() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void mqttConnect() {
  while(!mqttClient.connected()) {
    Serial.println("Attemping MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if(mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");

      //***Subscribe all topic you need***
      mqttClient.subscribe("21127337/led");
     
    }
    else {
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

//MQTT Receiver
void callback(char* topic, byte* message, unsigned int length) {
  Serial.println(topic);
  String strMsg;
  for(int i=0; i<length; i++) {
    strMsg += (char)message[i];
  }
  Serial.println(strMsg);

  //***Code here to process the received package***

}

void MQTTConfig()
{
  wifiConnect();
  mqttClient.setServer(mqttServer, port);
  mqttClient.setCallback(callback);
  mqttClient.setKeepAlive( 90 );
}

void setup(void)
{ 
  Serial.begin(9600);
  Serial.print("Connecting to WiFi");
  // DHT11 setup
  dht.begin();
  // MQTT server setup
  MQTTConfig();
  lcd.begin(16, 2);
}

struct DHT11Readings
{
  float temperature;
  float humidity;
};

DHT11Readings readDHT11()
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  return {temperature, humidity};
}

void connectToServer()
{
  if(!mqttClient.connected()) {
      mqttConnect();
    }
    mqttClient.loop();
}

void loop() {
  connectToServer();

  // Thong so nhiet do, do am DHT11
  DHT11Readings sensorDHT = readDHT11();
  
  Serial.print("Current humidity = ");
  Serial.print(sensorDHT.humidity);
  Serial.print("%  ");
  Serial.print("temperature = ");
  Serial.print(sensorDHT.temperature); 
  Serial.println("C  ");

  // Thong so thoi gian thuc te

  // Thong so do am dat
  soilMoisture = analogRead(soilPin);

  // Thong so muc nuoc

  // Che do tu dong tuoi cay dua vao nhiet do, do am khong khi, do am dat

  // Hien thi thong tin len LCD
  lcd.print(">> Soil Moisture: ");
  lcd.print(soilMoisture);

  for (int positionCounter = 0; positionCounter < 20; positionCounter++) {
    lcd.scrollDisplayLeft();
    delay(150);
  }

  delay(1000);
  lcd.clear();

  lcd.print(">> Humidity: ");
  lcd.print(sensorDHT.humidity);
  lcd.print("%");

  for (int positionCounter = 0; positionCounter < 20; positionCounter++) {
    lcd.scrollDisplayLeft();
    delay(150);
  }

  delay(1000);
  lcd.clear();

  lcd.print(">> Temperature: ");
  lcd.print(sensorDHT.temperature);

  for (int positionCounter = 0; positionCounter < 20; positionCounter++) {
    lcd.scrollDisplayLeft();
    delay(150);
  }

  delay(1000);
  lcd.clear();

  //***Publish data to MQTT Server***
  // data sent to MQTT Server must be a string
  int temp = random(0, 100);
  char buffer[50];

  sprintf(buffer, "%d", temp);
  mqttClient.publish("21127337/temperature", buffer);
}