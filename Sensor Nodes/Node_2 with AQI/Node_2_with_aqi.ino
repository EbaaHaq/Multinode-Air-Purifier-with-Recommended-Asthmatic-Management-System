#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

/////////////////////////////////////////////
#include "DHT.h"
#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
//////////////////////////////////////////////////

int sensorPin = A1;
int sensorData;

///////////////////////////////////////////////////
int measurePin = A0;
int ledPower = 2;

unsigned int samplingTime = 280;
unsigned int deltaTime = 40;
unsigned int sleepTime = 9680;

long voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

// Function to calculate Dust AQI based on dust concentration (μg/m³)
float calculateDustAQI(float dust_ugm3) {
  if (dust_ugm3 <= 12.0)
    return ((50.0 - 0.0) / (12.0 - 0.0)) * (dust_ugm3 - 0.0) + 0.0;
  else if (dust_ugm3 <= 35.4)
    return ((100.0 - 51.0) / (35.4 - 12.1)) * (dust_ugm3 - 12.1) + 51.0;
  else if (dust_ugm3 <= 55.4)
    return ((150.0 - 101.0) / (55.4 - 35.5)) * (dust_ugm3 - 35.5) + 101.0;
  else if (dust_ugm3 <= 150.4)
    return ((200.0 - 151.0) / (150.4 - 55.5)) * (dust_ugm3 - 55.5) + 151.0;
  else if (dust_ugm3 <= 250.4)
    return ((300.0 - 201.0) / (250.4 - 150.5)) * (dust_ugm3 - 150.5) + 201.0;
  else if (dust_ugm3 <= 500.4)
    return ((500.0 - 301.0) / (500.4 - 250.5)) * (dust_ugm3 - 250.5) + 301.0;
  else
    return 500.0; // If very high
}

// Function to determine the air pollution level based on AQI
const char* getPollutionLevel(float aqi) {
  if (aqi <= 50) return "Good";
  else if (aqi <= 100) return "Moderate";
  else if (aqi <= 150) return "Unhealthy for Sensitive Groups";
  else if (aqi <= 200) return "Unhealthy";
  else if (aqi <= 300) return "Very Unhealthy";
  else return "Hazardous";
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
  lcd.begin(16, 2);
  lcd.backlight();
  Serial.println("Dust Concentration (mg/m³)");
  //////////////////////////////////////////////////////
  lcd.setCursor(0, 0);
  lcd.print("AIR QUALITY MGM ");
  lcd.setCursor(0, 1);
  lcd.print("USING PURIFIER..");
  delay(3000);
  lcd.clear();
  ////////////////////////////////////////////////////////
}

void loop() {
  ////////////////////////////////////////
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  //////////////////////////////////////////
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
  dustDensity = 17 * calcVoltage - 0.1;

  if (dustDensity < 0) {
    dustDensity = 0.00;
  }

  // Calculate the AQI for dust (convert to μg/m³)
  float dust_ug = dustDensity * 1000.0;
  float aqi = calculateDustAQI(dust_ug);
  
  // Determine the air pollution level
  const char* pollutionLevel = getPollutionLevel(aqi);

  // Output Dust AQI, pollution level, dust density, and humidity to LCD and Serial Monitor
  Serial.print("Dust AQI: ");
  Serial.print(aqi);
  Serial.print(" - ");
  Serial.println(pollutionLevel);
  Serial.print("Dust Density (mg/m³): ");
  Serial.println(dustDensity);
  Serial.print("Humidity: ");
  Serial.println(h);

  lcd.setCursor(0, 0);
  lcd.print("AQI:");
  lcd.print(aqi);
  lcd.print("   ");

  lcd.setCursor(0, 1);
  lcd.print(pollutionLevel);
  lcd.print("  ");

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

  lcd.setCursor(0, 0);
  lcd.print("HUMIDITY:");
  lcd.print(h);
  lcd.print("%   ");
  delay(2000);
  lcd.clear();
}
