#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

/////////////////////////////////////////////
#include "DHT.h"
#define DHTPIN 11
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
//////////////////////////////////////////////////
int sensorPin = A0;
int sensorData;
///////////////////////////////////////////////////
int measurePin = A1;
int ledPower = 2;

unsigned int samplingTime = 280;
unsigned int deltaTime = 40;
unsigned int sleepTime = 9680;

long voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

///////////////////////////////////////
// Function to calculate Dust AQI based on dust concentration (μg/m³)
float calculateDustAQI(float dust_ugm3) {
  if (dust_ugm3 <= 12.0)
    return ((50.0-0.0)/(12.0-0.0))*(dust_ugm3-0.0)+0.0;
  else if (dust_ugm3 <= 35.4)
    return ((100.0-51.0)/(35.4-12.1))*(dust_ugm3-12.1)+51.0;
  else if (dust_ugm3 <= 55.4)
    return ((150.0-101.0)/(55.4-35.5))*(dust_ugm3-35.5)+101.0;
  else if (dust_ugm3 <= 150.4)
    return ((200.0-151.0)/(150.4-55.5))*(dust_ugm3-55.5)+151.0;
  else if (dust_ugm3 <= 250.4)
    return ((300.0-201.0)/(250.4-150.5))*(dust_ugm3-150.5)+201.0;
  else if (dust_ugm3 <= 500.4)
    return ((500.0-301.0)/(500.4-250.5))*(dust_ugm3-250.5)+301.0;
  else
    return 500.0; // If very high
}

// Function to calculate the average Dust AQI based on 10 readings
float getAverageDustAQI() {
  float sumAQI = 0;
  for (int i = 0; i < 10; i++) {
    // Read dust sensor value (in mg/m³)
    dustDensity = 17 * calcVoltage - 0.1; // Calculate dust density from sensor voltage
    if (dustDensity < 0) {
      dustDensity = 0.00;
    }
    float dust_ug = dustDensity * 1000.0; // Convert to μg/m³

    // Calculate AQI for this reading
    float aqi = calculateDustAQI(dust_ug);
    sumAQI += aqi;

    delay(200); // Small delay between readings
  }

  // Calculate and return the average AQI
  return sumAQI / 10.0;
}

void setup() {
  //////////////////////////////////////////////////
  Serial.begin(9600);
  pinMode(ledPower, OUTPUT);
  ///////////////////////////////////////////////////
  dht.begin();
  /////////////////////////////////////////////////////////
  pinMode(sensorPin, INPUT);  
  //////////////////////////////////////////////////////
  lcd.begin(16,2);
  lcd.backlight(); 
  lcd.setCursor(0, 0);
  lcd.print("AIR QUALITY MGM ");
  lcd.setCursor(0, 1);
  lcd.print("USING PURIFIER..");
  delay(3000);
  lcd.clear();
  //////////////////////////////////////////////////////
}

void loop() {
  ////////////////////////////////////////
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  //////////////////////////////////////////
  
  // Loop to sample 10 dust sensor readings
  for (int i = 0; i < 30; i++) {
    digitalWrite(ledPower, LOW);
    delayMicroseconds(samplingTime);

    voMeasured += analogRead(measurePin);

    delayMicroseconds(deltaTime);
    digitalWrite(ledPower, HIGH);
    delayMicroseconds(sleepTime);
    delay(50);
  }
  voMeasured = voMeasured / 30;
  calcVoltage = voMeasured * (5.0 / 1024);

  // Calculate the average AQI from the 10 dust sensor readings
  float averageDustAQI = getAverageDustAQI();

  // Output Dust AQI and other data to LCD and Serial Monitor
  Serial.print("Average AQI: ");
  Serial.println(averageDustAQI);

  lcd.setCursor(0, 0);
  lcd.print("AQI: ");
  lcd.print(averageDustAQI);
  lcd.print("   ");

  lcd.setCursor(0, 1);
  lcd.print("HUMD:");
  lcd.print(h);
  lcd.print("% ");
  delay(3000);

  lcd.clear();
  delay(200);

  lcd.setCursor(0, 0);
  lcd.print("CO2:");
  sensorData = analogRead(sensorPin);
  lcd.print(sensorData, DEC);
  lcd.print("ppm     ");
  lcd.setCursor(0, 1);
  lcd.print("TEMP:");
  lcd.print(t);
  lcd.print(char(223));
  lcd.print('C');
  delay(2000);
  lcd.clear();
}
