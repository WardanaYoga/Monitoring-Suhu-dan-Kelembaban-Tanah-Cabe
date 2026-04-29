#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= PIN =================
#define SOIL_PIN 34
#define ONE_WIRE_BUS 19
#define PUMP_PIN 18

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= SENSOR =================
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ================= OUTPUT SUGENO =================
#define MATI        0
#define CEPAT       2
#define SEDANG      6
#define AGAK_LAMA   8
#define LAMA        12

// ================= VAR =================
float soil_percent = 0;
float temperature  = 0;
int   durasi_pompa = 0;

// ================= KALIBRASI =================
int soil_dry = 3000;
int soil_wet = 1200;

// ================= FILTER SOIL =================
int readSoil() {
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(SOIL_PIN);
    delay(5);
  }
  return total / 10;
}

// ================= MEMBERSHIP KELEMBABAN =================
float mf_kering(float x) {
  if (x <= 30) return 1;
  else if (x >= 40) return 0;
  else return (40 - x) / 10.0;
}

float mf_agak_kering(float x) {
  if (x <= 30 || x >= 55) return 0;
  else if (x <= 42.5) return (x - 30) / 12.5;
  else return (55 - x) / 12.5;
}

float mf_agak_lembab(float x) {
  if (x <= 50 || x >= 65) return 0;
  else if (x <= 57.5) return (x - 50) / 7.5;
  else return (65 - x) / 7.5;
}

float mf_lembab(float x) {
  if (x <= 60 || x >= 75) return 0;
  else if (x <= 67.5) return (x - 60) / 7.5;
  else return (75 - x) / 7.5;
}

float mf_basah(float x) {
  if (x <= 70) return 0;
  else if (x >= 85) return 1;
  else return (x - 70) / 15.0;
}

// ================= MEMBERSHIP SUHU =================
float mf_dingin(float t) {
  if (t <= 21) return 1;
  else if (t >= 24) return 0;
  else return (24 - t) / 3.0;
}

float mf_normal(float t) {
  if (t <= 21 || t >= 29) return 0;
  else if (t <= 25) return (t - 21) / 4.0;
  else return (29 - t) / 4.0;
}

float mf_panas(float t) {
  if (t <= 28) return 0;
  else if (t >= 32) return 1;
  else return (t - 28) / 4.0;
}

// ================= FUNGSI SUGENO IF-THEN =================
int fuzzy_duration(float soil, float temp) {

  float kering       = mf_kering(soil);
  float agak_kering  = mf_agak_kering(soil);
  float agak_lembab  = mf_agak_lembab(soil);
  float lembab       = mf_lembab(soil);
  float basah        = mf_basah(soil);

  float dingin = mf_dingin(temp);
  float normal = mf_normal(temp);
  float panas  = mf_panas(temp);

  float num = 0;
  float den = 0;

  float alpha, z;

  // ================= 15 RULE =================

  // 1
  alpha = min(kering, dingin);
  z = AGAK_LAMA;
  num += alpha * z; den += alpha;

  // 2
  alpha = min(kering, normal);
  z = LAMA;
  num += alpha * z; den += alpha;

  // 3
  alpha = min(kering, panas);
  z = LAMA;
  num += alpha * z; den += alpha;

  // 4
  alpha = min(agak_kering, dingin);
  z = SEDANG;
  num += alpha * z; den += alpha;

  // 5
  alpha = min(agak_kering, normal);
  z = AGAK_LAMA;
  num += alpha * z; den += alpha;

  // 6
  alpha = min(agak_kering, panas);
  z = LAMA;
  num += alpha * z; den += alpha;

  // 7
  alpha = min(agak_lembab, dingin);
  z = CEPAT;
  num += alpha * z; den += alpha;

  // 8
  alpha = min(agak_lembab, normal);
  z = SEDANG;
  num += alpha * z; den += alpha;

  // 9
  alpha = min(agak_lembab, panas);
  z = AGAK_LAMA;
  num += alpha * z; den += alpha;

  // 10
  alpha = min(lembab, dingin);
  z = CEPAT;
  num += alpha * z; den += alpha;

  // 11
  alpha = min(lembab, normal);
  z = CEPAT;
  num += alpha * z; den += alpha;

  // 12
  alpha = min(lembab, panas);
  z = SEDANG;
  num += alpha * z; den += alpha;

  // 13
  alpha = min(basah, dingin);
  z = MATI;
  num += alpha * z; den += alpha;

  // 14
  alpha = min(basah, normal);
  z = MATI;
  num += alpha * z; den += alpha;

  // 15
  alpha = min(basah, panas);
  z = MATI;
  num += alpha * z; den += alpha;

  if (den == 0) return 0;

  return (int)(num / den);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  sensors.begin();
  lcd.init();
  lcd.backlight();
  pinMode(PUMP_PIN, OUTPUT);
}

// ================= LOOP =================
void loop() {

  int soil_adc = readSoil();
  soil_percent = (float)(soil_dry - soil_adc) / (soil_dry - soil_wet) * 100.0;
  soil_percent = constrain(soil_percent, 0, 100);

  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);

  durasi_pompa = fuzzy_duration(soil_percent, temperature);

  if (durasi_pompa > 0) {
    digitalWrite(PUMP_PIN, HIGH);
    delay(durasi_pompa * 1000);
    digitalWrite(PUMP_PIN, LOW);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Soil:");
  lcd.print((int)soil_percent);
  lcd.print("%");

  lcd.setCursor(9, 0);
  lcd.print("T:");
  lcd.print((int)temperature);

  lcd.setCursor(0, 1);
  lcd.print("Pump:");
  lcd.print(durasi_pompa);
  lcd.print("s");

  Serial.print("Soil: "); Serial.print(soil_percent);
  Serial.print("% | Temp: "); Serial.print(temperature);
  Serial.print("C | Durasi: "); Serial.print(durasi_pompa);
  Serial.println(" detik");

  delay(3000);
}
