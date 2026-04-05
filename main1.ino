#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// -----------------------------
// Konfigurasi Pin
// -----------------------------
#define SOIL_PIN 34
#define PUMP_PIN 18
#define ONE_WIRE_BUS 4

LiquidCrystal_I2C lcd(0x27, 16, 2);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// -----------------------------
// Variabel Global
// -----------------------------
int soilRaw;
int soilPercent;
float temperature;
int pwmOutput;
String soilCondition;
String tempCondition;
String pumpCondition;

// =============================
// MEMBERSHIP FUNCTION (SOIL)
// =============================
float soilDry(float x) {
  if (x <= 20) return 1;
  else if (x >= 40) return 0;
  else return (40 - x) / 20.0;
}

float soilMoist(float x) {
  if (x <= 30 || x >= 70) return 0;
  else if (x == 50) return 1;
  else if (x < 50) return (x - 30) / 20.0;
  else return (70 - x) / 20.0;
}

float soilWet(float x) {
  if (x <= 60) return 0;
  else if (x >= 80) return 1;
  else return (x - 60) / 20.0;
}

// =============================
// MEMBERSHIP FUNCTION (SUHU)
// =============================
float tempCold(float t) {
  if (t <= 25) return 1;
  else if (t >= 28) return 0;
  else return (28 - t) / 3.0;
}

float tempNormal(float t) {
  if (t <= 27 || t >= 32) return 0;
  else if (t == 29.5) return 1;
  else if (t < 29.5) return (t - 27) / 2.5;
  else return (32 - t) / 2.5;
}

float tempHot(float t) {
  if (t <= 30) return 0;
  else if (t >= 35) return 1;
  else return (t - 30) / 5.0;
}

void setup() {
  Serial.begin(115200);

  pinMode(PUMP_PIN, OUTPUT);

  sensors.begin();

  lcd.init();
  lcd.backlight();

  // PWM setup (FIX BUG)
  ledcSetup(0, 5000, 8);
  ledcAttachPin(PUMP_PIN, 0);

  lcd.setCursor(0, 0);
  lcd.print("Smart Irrigation");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {

  // -----------------------------
  // Baca Soil
  // -----------------------------
  soilRaw = analogRead(SOIL_PIN);
  soilPercent = map(soilRaw, 4095, 1500, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  // -----------------------------
  // Baca Suhu
  // -----------------------------
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);

  // -----------------------------
  // HITUNG DERAJAT KEANGGOTAAN
  // -----------------------------
  float dry = soilDry(soilPercent);
  float moist = soilMoist(soilPercent);
  float wet = soilWet(soilPercent);

  float cold = tempCold(temperature);
  float normal = tempNormal(temperature);
  float hot = tempHot(temperature);

  // -----------------------------
  // (MASIH PAKAI RULE LAMA)
  // -----------------------------
  if (soilPercent <= 20) {
    if (temperature >= 32) {
      pwmOutput = 255;
      pumpCondition = "Sgt Cepat";
    } else {
      pwmOutput = 220;
      pumpCondition = "Cepat";
    }
  }
  else if (soilPercent <= 40) {
    if (temperature >= 32) {
      pwmOutput = 200;
      pumpCondition = "Cepat";
    } else {
      pwmOutput = 170;
      pumpCondition = "Sedang";
    }
  }
  else if (soilPercent <= 60) {
    if (temperature >= 32) {
      pwmOutput = 150;
      pumpCondition = "Sedang";
    } else {
      pwmOutput = 100;
      pumpCondition = "Lambat";
    }
  }
  else if (soilPercent <= 80) {
    if (temperature >= 32) {
      pwmOutput = 80;
      pumpCondition = "Lambat";
    } else {
      pwmOutput = 0;
      pumpCondition = "Mati";
    }
  }
  else {
    pwmOutput = 0;
    pumpCondition = "Mati";
  }

  // -----------------------------
  // OUTPUT PWM
  // -----------------------------
  ledcWrite(0, pwmOutput);

  // -----------------------------
  // DEBUG SERIAL (PENTING!)
  // -----------------------------
  Serial.println("===== MEMBERSHIP VALUE =====");
  Serial.print("Dry   : "); Serial.println(dry);
  Serial.print("Moist : "); Serial.println(moist);
  Serial.print("Wet   : "); Serial.println(wet);

  Serial.print("Cold  : "); Serial.println(cold);
  Serial.print("Normal: "); Serial.println(normal);
  Serial.print("Hot   : "); Serial.println(hot);

  Serial.println("============================");

  // -----------------------------
  // LCD
  // -----------------------------
  lcd.setCursor(0, 0);
  lcd.print("Soil:");
  lcd.print(soilPercent);
  lcd.print("% ");
  lcd.print(temperature, 0);
  lcd.print("C   ");

  lcd.setCursor(0, 1);
  lcd.print("Pompa:");
  lcd.print(pumpCondition);
  lcd.print("   ");

  delay(2000);
}
