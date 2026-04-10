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

// ================= VAR =================
float soil_percent = 0;
float temperature = 0;
int durasi_pompa = 0;

// ================= KALIBRASI =================
int soil_dry = 3000;
int soil_wet = 1200;

// ================= FILTER SOIL =================
int readSoil(){
  int total = 0;
  for(int i=0;i<10;i++){
    total += analogRead(SOIL_PIN);
    delay(5);
  }
  return total/10;
}

// ================= MEMBERSHIP (SAMA) =================
float mf_kering(float x){
  if (x <= 20) return 1;
  else if (x >= 40) return 0;
  else return (40 - x) / 20.0;
}

float mf_agak_kering(float x){
  if (x <= 20 || x >= 60) return 0;
  else if (x == 40) return 1;
  else if (x < 40) return (x - 20) / 20.0;
  else return (60 - x) / 20.0;
}

float mf_agak_lembab(float x){
  if (x <= 40 || x >= 80) return 0;
  else if (x == 60) return 1;
  else if (x < 60) return (x - 40) / 20.0;
  else return (80 - x) / 20.0;
}

float mf_lembab(float x){
  if (x <= 60 || x >= 90) return 0;
  else if (x == 75) return 1;
  else if (x < 75) return (x - 60) / 15.0;
  else return (90 - x) / 15.0;
}

float mf_basah(float x){
  if (x <= 80) return 0;
  else if (x >= 100) return 1;
  else return (x - 80) / 20.0;
}

// ================= SUHU =================
float mf_dingin(float t){
  if (t <= 20) return 1;
  else if (t >= 28) return 0;
  else return (28 - t) / 8.0;
}

float mf_normal(float t){
  if (t <= 22 || t >= 35) return 0;
  else if (t == 28) return 1;
  else if (t < 28) return (t - 22) / 6.0;
  else return (35 - t) / 7.0;
}

float mf_panas(float t){
  if (t <= 30) return 0;
  else if (t >= 40) return 1;
  else return (t - 30) / 10.0;
}

// ================= FUZZY OUTPUT (DURASI) =================
int fuzzy_duration(float soil, float temp){

  float kering      = mf_kering(soil);
  float agak_kering = mf_agak_kering(soil);
  float agak_lembab = mf_agak_lembab(soil);
  float lembab      = mf_lembab(soil);
  float basah       = mf_basah(soil);

  float dingin = mf_dingin(temp);
  float normal = mf_normal(temp);
  float panas  = mf_panas(temp);

  // ===== RULE =====
  float r1 = min(kering, panas);        // 5 detik
  float r2 = min(kering, normal);       // 4 detik
  float r3 = min(kering, dingin);       // 4 detik

  float r4 = min(agak_kering, panas);   // 4 detik
  float r5 = min(agak_kering, normal);  // 3 detik
  float r6 = min(agak_kering, dingin);  // 3 detik

  float r7 = min(agak_lembab, panas);   // 2 detik
  float r8 = min(agak_lembab, normal);  // 2 detik
  float r9 = min(agak_lembab, dingin);  // 1 detik

  float r10 = max(lembab, basah);       // 0 detik

  // ===== OUTPUT =====
  float z1=5, z2=4, z3=4, z4=4, z5=3, z6=3, z7=2, z8=2, z9=1, z10=0;

  float num = r1*z1 + r2*z2 + r3*z3 +
              r4*z4 + r5*z5 + r6*z6 +
              r7*z7 + r8*z8 + r9*z9 +
              r10*z10;

  float den = r1+r2+r3+r4+r5+r6+r7+r8+r9+r10;

  if (den == 0) return 0;

  return (int)(num/den);
}

// ================= SETUP =================
void setup(){
  Serial.begin(115200);
  sensors.begin();
  lcd.init();
  lcd.backlight();

  pinMode(PUMP_PIN, OUTPUT);
}

// ================= LOOP =================
void loop(){

  // ===== READ =====
  int soil_adc = readSoil();
  soil_percent = (float)(soil_dry - soil_adc) / (soil_dry - soil_wet) * 100.0;
  soil_percent = constrain(soil_percent, 0, 100);

  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);

  // ===== FUZZY =====
  durasi_pompa = fuzzy_duration(soil_percent, temperature);

  // ===== OUTPUT =====
  if(durasi_pompa > 0){
    digitalWrite(PUMP_PIN, HIGH);
    delay(durasi_pompa * 1000);
    digitalWrite(PUMP_PIN, LOW);
  }

  // ===== LCD =====
  // TANPA lcd.clear() biar tidak kedip
  lcd.setCursor(0,0);
  lcd.print("Soil:     Temp:   ");

  lcd.setCursor(0,0);
  lcd.print("Soil:");
  lcd.print((int)soil_percent);
  lcd.print("%");

  lcd.setCursor(10,0);
  lcd.print("T:");
  lcd.print((int)temperature);
  lcd.print("C");

  // Baris kedua opsional (status)
  lcd.setCursor(0,1);
  lcd.print("Pump:");
  lcd.print(durasi_pompa);
  lcd.print("s      ");
  // ===== SERIAL =====
  Serial.print("Soil: "); Serial.print(soil_percent);
  Serial.print("% | Temp: "); Serial.print(temperature);
  Serial.print("C | Durasi: "); Serial.println(durasi_pompa);

  delay(3000);
}
