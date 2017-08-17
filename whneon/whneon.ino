
uint32 start = 0;

void setup() {
  // put your setup code here, to run once:
//  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);

  start = millis();
}

uint8 value = 255;
int16_t halfCycle = 500;
uint32 lastSec = 0;

void setState(bool b) {
//  digitalWrite(BUILTIN_LED, b);  
  digitalWrite(D0, b);  
  digitalWrite(D1, b);  
  digitalWrite(D2, b);  
  digitalWrite(D3, b);  
  digitalWrite(D4, b);  // LED
  digitalWrite(D5, b);  
  digitalWrite(D6, b);  
  digitalWrite(D7, b);  
  digitalWrite(D8, b);  
}

void loop() {
  setState(HIGH);
}
void loopX() {

  int32 elapsedSec = (millis() - start) / 1000;

  if (elapsedSec != lastSec) {
    // New loop
    halfCycle -= 50;
    if (halfCycle < 50) {
      halfCycle = 500;
    }
    lastSec = elapsedSec;
  }

  setState(HIGH); 
//  digitalWrite(D1, HIGH);
  delay(halfCycle);
//  digitalWrite(D1, LOW);
  setState(LOW);
  delay(halfCycle);

//  analogWrite(D1, value);
//  value -= 1;
//  delay(10);
}
