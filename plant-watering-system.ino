#include <DHT.h>
#include <DHT_U.h>

#define DHT11_PIN 8
DHT dht(DHT11_PIN, DHT11);
void setup(void)
{ 
  dht.begin();
  Serial.begin(9600);
  Serial.println("Humidity and temperature\n\n");
  delay(700);

}

struct DHT11Readings
{
  float temperature;
  float humidity;
};

DHT11Readings readDHT11()
{
  float humidity = dht.readHumidity();
  float temperatureInC = dht.readTemperature();
  return {temperatureInC, humidity};
}

void loop() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();         
    Serial.print("Current humidity = ");
    Serial.print(h);
    Serial.print("%  ");
    Serial.print("temperature = ");
    Serial.print(t); 
    Serial.println("C  ");
  delay(800);
}