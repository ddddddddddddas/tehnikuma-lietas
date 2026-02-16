#include <OneWire.h>
#include <DallasTemperature.h>

#define PIN_D3 3
#define PIN_D4 4
#define PIN_D5 5

#define WINDOW_SECONDS 30

OneWire ow3(PIN_D3), ow4(PIN_D4), ow5(PIN_D5), ow6(PIN_D6);
DallasTemperature dt3(&ow3), dt4(&ow4), dt5(&ow5), dt6(&ow6);

struct Sample {
  uint32_t tms;
  int16_t c10[4];
};

Sample ring[WINDOW_SECONDS];
uint8_t head = 0;
uint8_t count = 0;
uint32_t lastTick = 0;

int16_t readC10(DallasTemperature &dt) {
  dt.requestTemperatures();
  float c = dt.getTempCByIndex(0);
  if (c == DEVICE_DISCONNECTED_C) return INT16_MIN;
  long v = lroundf(c * 10.0f);
  if (v > INT16_MAX) v = INT16_MAX;
  if (v < INT16_MIN) v = INT16_MIN;
  return (int16_t)v;
}

bool hasSensor(DallasTemperature &dt) {
  DeviceAddress addr;
  return dt.getAddress(addr, 0);
}

void setup() {
  Serial.begin(115200);

  dt3.begin(); dt4.begin(); dt5.begin(); dt6.begin();
  dt3.setWaitForConversion(true);
  dt4.setWaitForConversion(true);
  dt5.setWaitForConversion(true);
  dt6.setWaitForConversion(true);

  dt3.setResolution(12);
  dt4.setResolution(12);
  dt5.setResolution(12);
  dt6.setResolution(12);

  Serial.print("D3: "); Serial.println(hasSensor(dt3) ? "OK" : "NONE");
  Serial.print("D4: "); Serial.println(hasSensor(dt4) ? "OK" : "NONE");
  Serial.print("D5: "); Serial.println(hasSensor(dt5) ? "OK" : "NONE");
  Serial.print("D6: "); Serial.println(hasSensor(dt6) ? "OK" : "NONE");
}

void pushSample(uint32_t tms, int16_t a, int16_t b, int16_t c, int16_t d) {
  ring[head].tms = tms;
  ring[head].c10[0] = a;
  ring[head].c10[1] = b;
  ring[head].c10[2] = c;
  ring[head].c10[3] = d;
  head = (uint8_t)((head + 1) % WINDOW_SECONDS);
  if (count < WINDOW_SECONDS) count++;
}

void printLatest() {
  if (count == 0) return;
  uint8_t idx = (uint8_t)((head + WINDOW_SECONDS - 1) % WINDOW_SECONDS);
  Serial.print(ring[idx].tms);
  Serial.print(',');
  for (uint8_t i = 0; i < 4; i++) {
    int16_t v = ring[idx].c10[i];
    if (v == INT16_MIN) {
      Serial.print("NA");
    } else {
      Serial.print(v / 10);
      Serial.print('.');
      int16_t frac = abs(v % 10);
      Serial.print(frac);
    }
    if (i < 3) Serial.print(',');
  }
  Serial.println();
}

void loop() {
  uint32_t now = millis();
  if (now - lastTick >= 1000UL) {
    lastTick += 1000UL;

    int16_t t1 = readC10(dt3);
    int16_t t2 = readC10(dt4);
    int16_t t3v = readC10(dt5);
    int16_t t4v = readC10(dt6);

    pushSample(now, t1, t2, t3v, t4v);
    printLatest();
  }
}

