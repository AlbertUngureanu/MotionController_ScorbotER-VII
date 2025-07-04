#include <Wire.h>

#define SLAVE_ADDRESS 16
#define DONE_SIGNAL_PIN 7

#define MOTOR_PIN_1 6
#define MOTOR_PIN_2 5
#define ENCODER_A 2
#define ENCODER_B 4
#define ENCODER_REF 3
#define MIN_PWM 0
#define MAX_PWM 255

bool home, start;
float kp = 1;  
long target = 0;

const int analogPin = A3;         
const float sensitivity = 0.066;  
const float Vref = 2.5;
const int numSamples = 200;
const unsigned long interval = 500;

const byte bufferSize = 56;        
float currentBuffer[bufferSize];    
int writeIndex = 0;
uint8_t packetIndex = 0;

volatile long int current_poz;
unsigned long time, currentMillis;
float pwr;

void receiveEvent(int bytes) {
  if (Wire.available() >= 3) {  
    byte highB = Wire.read();   // Byte-ul superior (MSB)
    byte midB  = Wire.read();   // Byte-ul mijlociu
    byte lowB  = Wire.read();   // Byte-ul inferior (LSB)

    target = ((long)highB << 16) | ((long)midB << 8) | lowB;  

    if (target & 0x800000) {
      target |= 0xFF000000;
    }
  }
}

void requestEvent() {
  int start = packetIndex * 8;
  
  if (start + 8 > 56) {
    packetIndex = 0;
    start = 0;
  }

  Wire.write((byte*)(currentBuffer + start), 8 * sizeof(float));

  packetIndex++;
  if (packetIndex >= 7) packetIndex = 0;
}

void Moving() {
  if (digitalRead(ENCODER_A) == HIGH) {
    current_poz += (digitalRead(ENCODER_B) == LOW) ? -1 : 1;
  } else {
    current_poz += (digitalRead(ENCODER_B) == LOW) ? 1 : -1;
  }
}

void isHome() {
  home = true;
}

void motorMove(int dir, int power) {
  power = constrain(power, MIN_PWM, MAX_PWM);
  if (dir == 1) {
    analogWrite(MOTOR_PIN_1, power);
    analogWrite(MOTOR_PIN_2, 0);
  } else if (dir == -1) {
    analogWrite(MOTOR_PIN_1, 0);
    analogWrite(MOTOR_PIN_2, power);
  } else {
    analogWrite(MOTOR_PIN_1, 0);
    analogWrite(MOTOR_PIN_2, 0);
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  pinMode(13, INPUT);
  PCICR |= 0b00000001;  
  PCMSK0 |= 0b00100000;  

  pinMode(MOTOR_PIN_1, OUTPUT);
  pinMode(MOTOR_PIN_2, OUTPUT);
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(ENCODER_REF, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), Moving, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_REF), isHome, RISING);

  home = false;
  time = micros();
  start = false;
  motorMove(0, 0);
}

void loop() {
  float voltage = 0;
  currentMillis = micros();


  long e = target - current_poz;

  // Serial.println(e);
  if (abs(e) > 10) {
    float u = kp * e;  

    pwr = fabs(u);
    pwr = constrain(pwr, MIN_PWM, MAX_PWM);

    if (u < 0)
      motorMove(1, pwr);
    else
      motorMove(-1, pwr);
    
    float voltage = 0;

    for (int i = 0; i < numSamples; i++) {
      voltage += analogRead(analogPin) * (5.0 / 1023.0);
    }

    voltage /= numSamples;

    float current = (voltage - Vref) / sensitivity;

    currentBuffer[writeIndex] = current;
    writeIndex = (writeIndex + 1) % bufferSize;
  } else {
    motorMove(0, 0);
  }
}

// ISR pentru INT5 (D13) - Start motor
ISR(PCINT0_vect) {
  if (digitalRead(13) == HIGH) {
    digitalWrite(DONE_SIGNAL_PIN, LOW);
    Serial.println("Start");
    start = true;
  }
}
