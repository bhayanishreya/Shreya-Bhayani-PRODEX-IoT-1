// C++ code
//
#include <Servo.h>

// ---------- Pins ----------
const int LDR_TL = A0, LDR_TR = A1, LDR_BL = A2, LDR_BR = A3;
const int HSERVO_PIN = 9, VSERVO_PIN = 10;
const int LOCK_LED = 7;           // optional indicator

const unsigned long HOME_DELAY = 5000;   // ms of darkness before returning home
unsigned long darkSince = 0;
bool isDark = false;

// ---------- Tuning (calibrate these) ----------
const int THRESHOLD   = 30;       // dead-band: ignore differences smaller than this
const int DARK_LEVEL  = 60;       // if all LDRs below this -> it's night
const int STEP_DEG    = 1;        // degrees moved per loop
const int LOOP_DELAY  = 40;       // ms; smaller = faster tracking
const int H_MIN = 10,  H_MAX = 170;
const int V_MIN = 30,  V_MAX = 150;
const int H_DIR = 1;              // flip to -1 if panel turns the wrong way
const int V_DIR = 1;              // flip to -1 if panel tilts the wrong way
const int HOME_H = 90, HOME_V = 90;

Servo hServo, vServo;
int hPos = HOME_H, vPos = HOME_V;

// Average several readings to reduce noise
int readAvg(int pin) {
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(pin);
    delay(2);
  }
  return sum / 5;
}

void setup() {
  Serial.begin(9600);
  pinMode(LOCK_LED, OUTPUT);
  hServo.attach(HSERVO_PIN);
  vServo.attach(VSERVO_PIN);
  hServo.write(hPos);
  vServo.write(vPos);
  Serial.println("Solar tracker started");
}

void loop() {
  int tl = readAvg(LDR_TL);
  int tr = readAvg(LDR_TR);
  int bl = readAvg(LDR_BL);
  int br = readAvg(LDR_BR);

  int avgAll = (tl + tr + bl + br) / 4;

    // Edge case: night / very low light
  if (avgAll < DARK_LEVEL) {
    digitalWrite(LOCK_LED, LOW);

    if (!isDark) {              // first loop where it became dark
      isDark = true;
      darkSince = millis();
    }

    if (millis() - darkSince >= HOME_DELAY) {
      // Glide home 1 degree at a time
      if (hPos != HOME_H) {
        hPos += (hPos < HOME_H) ? 1 : -1;
        hServo.write(hPos);
      }
      if (vPos != HOME_V) {
        vPos += (vPos < HOME_V) ? 1 : -1;
        vServo.write(vPos);
      }
      Serial.println("Night - returning to home position");
      delay(30);
    } else {
      Serial.println("Too dark - holding position");
      delay(100);
    }
    return;
  }

  isDark = false;               // light is back, resume tracking
  
    // Edge case: a stuck sensor (open/short) -> skip tracking
  int readings[4] = {tl, tr, bl, br};
  for (int i = 0; i < 4; i++) {
    if (readings[i] <= 2 || readings[i] >= 1021) {
      Serial.println("Sensor fault suspected - holding position");
      digitalWrite(LOCK_LED, LOW);
      delay(500);
      return;
    }
  }

    int top   = (tl + tr) / 2;
    int bot   = (bl + br) / 2;
    int left  = (tl + bl) / 2;
    int right = (tr + br) / 2;

    int vErr = top - bot;     // positive -> light is above
    int hErr = left - right;  // positive -> light is on the left

    bool moved = false;

  if (abs(hErr) > THRESHOLD) {
    hPos += (hErr > 0 ? STEP_DEG : -STEP_DEG) * H_DIR;
    hPos = constrain(hPos, H_MIN, H_MAX);
    hServo.write(hPos);
    moved = true;
  }

  if (abs(vErr) > THRESHOLD) {
    vPos += (vErr > 0 ? STEP_DEG : -STEP_DEG) * V_DIR;
    vPos = constrain(vPos, V_MIN, V_MAX);
    vServo.write(vPos);
    moved = true;
  }

  digitalWrite(LOCK_LED, moved ? LOW : HIGH);  // LED on = locked on the light

  Serial.print("TL:"); Serial.print(tl);
  Serial.print(" TR:"); Serial.print(tr);
  Serial.print(" BL:"); Serial.print(bl);
  Serial.print(" BR:"); Serial.print(br);
  Serial.print(" | H:"); Serial.print(hPos);
  Serial.print(" V:"); Serial.print(vPos);
  Serial.println(moved ? " | tracking" : " | locked");

  delay(LOOP_DELAY);
}