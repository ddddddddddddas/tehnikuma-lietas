#define ENCODER1_A 2
#define ENCODER1_B 3

#define ENCODER2_A 4
#define ENCODER2_B 5

long pos1 = 0;
long pos2 = 0;

int lastA1;
int lastA2;

void setup() {
  Serial.begin(9600);

  pinMode(ENCODER1_A, INPUT_PULLUP);
  pinMode(ENCODER1_B, INPUT_PULLUP);

  pinMode(ENCODER2_A, INPUT_PULLUP);
  pinMode(ENCODER2_B, INPUT_PULLUP);

  lastA1 = digitalRead(ENCODER1_A);
  lastA2 = digitalRead(ENCODER2_A);
}

void loop() {
  int a1 = digitalRead(ENCODER1_A);
  if (a1 != lastA1) {
    if (digitalRead(ENCODER1_B) != a1) pos1++;
    else pos1--;
    Serial.print("Encoder1: ");
    Serial.println(pos1);
  }
  lastA1 = a1;

  int a2 = digitalRead(ENCODER2_A);
  if (a2 != lastA2) {
    if (digitalRead(ENCODER2_B) != a2) pos2++;
    else pos2--;
    Serial.print("Encoder2: ");
    Serial.println(pos2);
  }
  lastA2 = a2;
}
