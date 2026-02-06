#include <OneWire.h>
#include <DallasTemperature.h>

#define PIN_D3 3
#define PIN_D4 4
#define PIN_D5 5
#define PIN_D6 6

#define WINDOW_SECONDS 30
#define SAMPLE_PERIOD_MS 1000UL
#define CONVERSION_MS 750

OneWire ow3(PIN_D3), ow4(PIN_D4), ow5(PIN_D5), ow6(PIN_D6);
DallasTemperature dt3(&ow3), dt4(&ow4), dt5(&ow5), dt6(&ow6);

struct Sample {
  uint32_t tms;
  int16_t c10[4];
};

Sample ring[WINDOW_SECONDS];
uint8_t head = 0;
uint8_t count = 0;

uint32_t nextSampleAt = 0;
uint32_t convReadyAt = 0;
bool conversionInFlight = false;

bool ok3=false, ok4=false, ok5=false, ok6=false;

bool hasSensor(DallasTemperature &dt) {
  DeviceAddress addr;
  return dt.getAddress(addr, 0);
}

static inline int16_t clampToI16(long v) {
  if (v > INT16_MAX) return INT16_MAX;
  if (v < INT16_MIN) return INT16_MIN;
  return (int16_t)v;
}

int16_t readC10NoRequest(DallasTemperature &dt) {
  float c = dt.getTempCByIndex(0);
  if (c == DEVICE_DISCONNECTED_C) return INT16_MIN;
  long v = lroundf(c * 10.0f);
  return clampToI16(v);
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
      int16_t whole = v / 10;
      int16_t frac  = abs(v % 10);
      Serial.print(whole);
      Serial.print('.');
      Serial.print(frac);
    }
    if (i < 3) Serial.print(',');
  }
  Serial.println();
}

void setupSensor(DallasTemperature &dt) {
  dt.begin();
  dt.setWaitForConversion(false);
  dt.setResolution(12);
}

void setup() {
  Serial.begin(115200);

  setupSensor(dt3);
  setupSensor(dt4);
  setupSensor(dt5);
  setupSensor(dt6);

  ok3 = hasSensor(dt3);
  ok4 = hasSensor(dt4);
  ok5 = hasSensor(dt5);
  ok6 = hasSensor(dt6);

  Serial.print("D3: "); Serial.println(ok3 ? "OK" : "NONE");
  Serial.print("D4: "); Serial.println(ok4 ? "OK" : "NONE");
  Serial.print("D5: "); Serial.println(ok5 ? "OK" : "NONE");
  Serial.print("D6: "); Serial.println(ok6 ? "OK" : "NONE");

  uint32_t now = millis();
  nextSampleAt = now + SAMPLE_PERIOD_MS;

  if (ok3) dt3.requestTemperatures();
  if (ok4) dt4.requestTemperatures();
  if (ok5) dt5.requestTemperatures();
  if (ok6) dt6.requestTemperatures();

  conversionInFlight = true;
  convReadyAt = now + CONVERSION_MS;
}

void loop() {
  uint32_t now = millis();

  if (conversionInFlight &&
      (int32_t)(now - convReadyAt) >= 0 &&
      (int32_t)(now - nextSampleAt) >= 0) {

    int16_t t1 = ok3 ? readC10NoRequest(dt3) : INT16_MIN;
    int16_t t2 = ok4 ? readC10NoRequest(dt4) : INT16_MIN;
    int16_t t3v = ok5 ? readC10NoRequest(dt5) : INT16_MIN;
    int16_t t4v = ok6 ? readC10NoRequest(dt6) : INT16_MIN;

    pushSample(now, t1, t2, t3v, t4v);
    printLatest();

    do {
      nextSampleAt += SAMPLE_PERIOD_MS;
    } while ((int32_t)(now - nextSampleAt) >= 0);

    if (ok3) dt3.requestTemperatures();
    if (ok4) dt4.requestTemperatures();
    if (ok5) dt5.requestTemperatures();
    if (ok6) dt6.requestTemperatures();

    convReadyAt = now + CONVERSION_MS;
    conversionInFlight = true;
  }
}
