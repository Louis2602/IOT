#include <LiquidCrystal_I2C.h>

#define soilPin A0

// int soilMoisture;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
}

void loop() {
  // soilMoisture = analogRead(soilPin);
  float humidity = 20; //test value
  float temperature = 21.5; //test value

  // lcd.print(">>> Soil moisture: ");
  // lcd.print(soilMoisture);

  // for (int positionCounter = 0; positionCounter < 25; positionCounter++) {
  //   lcd.scrollDisplayLeft();
  //   delay(500);
  // }
  
  // lcd.clear();

  lcd.print(">>> Humidity: ");
  lcd.print(humidity);
  lcd.print("%");

  for (int positionCounter = 0; positionCounter < 25; positionCounter++) {
    lcd.scrollDisplayLeft();
    delay(500);
  }
  
  lcd.clear();

  lcd.print(">>> Temperature: ");
  lcd.print(temperature);

  for (int positionCounter = 0; positionCounter < 25; positionCounter++) {
    lcd.scrollDisplayLeft();
    delay(500);
  }
  
  lcd.clear();
}