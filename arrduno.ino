#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <DHT.h>

// time
int timezone = 7 * 3600;
int dst = 0;

// Wifi
const char* ssid = "AndroidAP5BC3";
const char* password = "qqqqqqqqq";

// MQQT
const char* mqttServer = "broker.hivemq.com";
int port = 1883;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Khai bao cac bien chan
const int pinTemperature = D5;
DHT dht(pinTemperature, DHT11);
int relayMayBom = D1;
int pinWater = D0;
int pinSoilMoisture = A0;
bool congTac = false;
int TemperatureSwitch = D2;

//biến đếm số lần tưới nước
int count = 0;

//biến yêu cầu tưới nước từ người dùng
int humanRequest = 0;

// Notification: Thong bao tinh trang het nuoc ve app IFTTT
const char* host = "maker.ifttt.com";
String request = "";
const int portWeb = 80;

// Kiểm tra đã gửi thông báo về IFTTT
int flagReport = 0;

// Ket noi wifi
void wifiConnect() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
}

// MQTT Receiver
void callback(char* topic, byte* message, unsigned int length) {
  Serial.println(topic);
  String strMsg;
  for (int i = 0; i < length; i++) {
    strMsg += (char)message[i];
  }
  if(strcmp(topic,"MAIN/waterRequest") == 0){
    if(strMsg == "1") humanRequest = 1;
  }
  if(strcmp(topic,"MAIN/IFTTT") == 0){
   Serial.println(strMsg);
    request = strMsg;
  }

  // Code here to process the received package

}

// Ket noi MQQT
void mqttConnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attemping MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Connected");
      // Subscribe all topic you need
      mqttClient.subscribe("MAIN/waterRequest");
      mqttClient.subscribe("MAIN/IFTTT");
    } else {
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

// Khai bao thong so MQTT
void defineMQTT() {
  wifiConnect();
  mqttClient.setServer(mqttServer, port);
  mqttClient.setCallback(callback);
  mqttClient.setKeepAlive(90);
}

// Ket noi
void connect() {
  if (!mqttClient.connected()) {
    mqttConnect();
  }
  mqttClient.loop();
}

// Ham lay du lieu cac sensor
int printSoilMoisture() {
  int valueSoilMoisture = analogRead(pinSoilMoisture);
  return valueSoilMoisture;
}

int printStatusWater() {
  int valueWater = digitalRead(pinWater);
  return valueWater;
}

float printTemperature() {
  float valueTemperature = dht.readTemperature();
  delay(100);
  return valueTemperature;
}

//mode tự động
int checkAction(int valueSoilMoisture, int valueWater, float valueTemperature, int hour)
{
  if(valueSoilMoisture > 60 ) return 0;
  if(valueTemperature < 25 || valueTemperature > 35) return 0;
  if(valueWater == 0) return 0;
  if((hour >= 11 && hour <= 23)||(hour >= 0 && hour <= 5)) return 0;
  return 1;
}

// Gui du lieu ket noi: parameter request: requestOutOfWater or requestWatering
void sendWebRequest() {
  while(!wifiClient.connect(host, portWeb)) {
    Serial.println("Connection fail");
    delay(1000);
  }
  wifiClient.print(String("GET ") + request + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  delay(500);
}

void setup() {
  Serial.begin(9600);
  // Khai bao thong so
  dht.begin();
  pinMode(relayMayBom, OUTPUT);
  pinMode(pinWater, INPUT);
  pinMode(pinSoilMoisture, INPUT);
  //pinMode(pinTemperature, OUTPUT);
  pinMode(TemperatureSwitch, OUTPUT);
  digitalWrite(TemperatureSwitch,HIGH);
  // Wifi
  defineMQTT();
  
}

void tuoinuoc(){
  digitalWrite(relayMayBom,HIGH);
  delay(10000);
  digitalWrite(relayMayBom,LOW);
  count++;
}

void loop() {
  // Ket noi MQTT
  connect();

  // Thong so DHT
  float temperature = printTemperature();
  Serial.print("Temperature: ");
  Serial.println(temperature);

  // Thong so muc do nuoc
  int statusWater = printStatusWater();
  Serial.print("Status water: ");
  Serial.println(statusWater);

  
  // Thong so do am dat
  int soilMoisture = printSoilMoisture();
  soilMoisture = map(soilMoisture, 440, 1023, 100, 0);
  soilMoisture = max(0,min(soilMoisture, 100));
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);

  if(statusWater == 0 && flagReport == 0){
    sendWebRequest();
    Serial.println("Đã gửi thông báo hết nước!");
    flagReport = 1;
  }
  if(statusWater == 1) flagReport = 0;
  // time
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  int hour = p_tm->tm_hour;
  if(hour == 0) count == 0;


  // Nhận biết tưới hoặc không tưới tự động
  int switchAction = checkAction(soilMoisture, statusWater, temperature, hour);

  if((switchAction == 1 && count < 1) || humanRequest == 1) {
    tuoinuoc();
    switchAction = 1;
    humanRequest = 0;
    Serial.printf("Đã tưới nước!");

    // digitalWrite(TemperatureSwitch,LOW);
    // delay(2000);
    // digitalWrite(TemperatureSwitch,HIGH);
    // delay(1000);
  }

  // Lưu dưới mảng list
  float sensorValues[] = {soilMoisture, temperature, statusWater}; 

  // Tạo mảng buffer dạng list
  char buffer[50]; 
  char bufferAction[50]; 
  char bufferCount[2];

  //Chuyển dữ liệu thành dạng chuỗi
  sprintf(bufferAction, "%d", switchAction);
  sprintf(bufferCount, "%d", count);
  String valueString = "";
  for (int i = 0; i < sizeof(sensorValues) / sizeof(sensorValues[0]); i++) {
      valueString += String(sensorValues[i], 2); // Lấy 2 số thập phân
      if (i < sizeof(sensorValues) / sizeof(sensorValues[0]) - 1) {
          valueString += ","; // Ngắn cách các dữ liệu
      }
  }
  // Chuyển string thành mảng char
  valueString.toCharArray(buffer, sizeof(buffer));
  
  // Publish lên mqtt
  mqttClient.publish("MAIN/count",bufferCount);
  mqttClient.publish("MAIN/list_sensor_values", buffer);
  mqttClient.publish("MAIN/switchAction", bufferAction);
  delay(2000);
}