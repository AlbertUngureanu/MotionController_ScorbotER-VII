#include <Wire.h>

#define SLAVE1_ADDRESS 11
#define SLAVE2_ADDRESS 12
#define SLAVE3_ADDRESS 13
#define SLAVE4_ADDRESS 14
#define SLAVE5_ADDRESS 15
#define SLAVE6_ADDRESS 16
#define START_SIGNAL_PIN 4  // Pinul de start (D4)
#define DONE_PIN 3          // Pin pentru întrerupere (D3)
#define MAX_DIFF 300.0      // de modificat aici

volatile bool readyForInput = true; // Flag pentru a permite introducerea datelor
int pos1 = 0;
int pos2 = 0;
int pos3 = 0;
int pos4 = 0;
int pos5 = 0;
int i;

const byte bufferSize = 56;
float currentBuffer1[bufferSize];
float currentBuffer2[bufferSize];
float currentBuffer3[bufferSize];
float currentBuffer4[bufferSize];
float currentBuffer5[bufferSize];
byte readIndex[5];

void setup() {
  Serial.begin(250000);
  Wire.begin();
  Wire.setClock(400000);

  pinMode(START_SIGNAL_PIN, OUTPUT);
  digitalWrite(START_SIGNAL_PIN, LOW);  // Inițial LOW

  pinMode(DONE_PIN, INPUT_PULLUP);  // Configurare D3 ca intrare cu pull-up activ

  // Activare PCI pentru D3 (Pin Change Interrupt pe Port D)
  PCICR  |= 0b00000100;  // Activează PCI pe Port D
  PCMSK2 |= 0b00001000;  // Activează întreruperea pe D3 (Bit 3)
}

void loop() {
  if (Serial.available()) {
    String receivedData = Serial.readStringUntil('\n');

    char movementType = receivedData.charAt(0);
    String valuePart = receivedData.substring(2);
    long values[5] = {0};  // Array pentru cele 5 valori
    parseData(valuePart, values);
    
    switch(movementType) {
      case '0':
            controlTool(0);
            break;
      case '1':
            controlTool(1);
            break;
      case 'X':
          for(int i = 0; i < 4; i++) {
            readCurrentBuffer(SLAVE1_ADDRESS, currentBuffer1, bufferSize, 0);
            readCurrentBuffer(SLAVE2_ADDRESS, currentBuffer2, bufferSize, 1);
            readCurrentBuffer(SLAVE3_ADDRESS, currentBuffer3, bufferSize, 2);
            readCurrentBuffer(SLAVE4_ADDRESS, currentBuffer4, bufferSize, 3);
            readCurrentBuffer(SLAVE5_ADDRESS, currentBuffer5, bufferSize, 4);
            delay(500);
          }
            sendAllBuffersOverSerial();
            break;
      case 'A':
            sendIntermediateValues(SLAVE1_ADDRESS, SLAVE2_ADDRESS, SLAVE3_ADDRESS, SLAVE4_ADDRESS, SLAVE5_ADDRESS,
                           pos1, values[0], pos2, values[1], pos3, values[2], pos4, values[3], pos5, values[4]);
            pos1 = values[0];
            pos2 = values[1];    
            pos3 = values[2];
            pos4 = values[3];
            pos5 = values[4];
            break;
      case 'C':
            sendToSlave(SLAVE1_ADDRESS, values[0]);
            sendToSlave(SLAVE2_ADDRESS, values[1]);
            sendToSlave(SLAVE3_ADDRESS, values[2]);
            sendToSlave(SLAVE4_ADDRESS, values[3]);
            sendToSlave(SLAVE5_ADDRESS, values[4]);
            pos1 = values[0];
            pos2 = values[1];    
            pos3 = values[2];
            pos4 = values[3];
            pos5 = values[4];
            break;

    }
  }
}

void readCurrentBuffer(uint8_t slaveAddress, float* targetBuffer, int bufferSize, int nR) {
  for (int p = 0; p < 2; p++) {
    Wire.requestFrom(slaveAddress, 8 * sizeof(float));  // 32 bytes

    byte* ptr = (byte*)(targetBuffer + readIndex[nR]);
    int i = 0;
    while (Wire.available() && i < 32) {
      ptr[i++] = Wire.read();
    }

    readIndex[nR] += 8;
    if (readIndex[nR] >= bufferSize) readIndex[nR] = 0;
  }
  Wire.end();     
  Wire.begin();   // Repornește
}

void sendAllBuffersOverSerial() {
  for (int i = 0; i < bufferSize; i++) {
    Serial.print(currentBuffer1[i], 3); Serial.print(',');
    Serial.print(currentBuffer2[i], 3); Serial.print(',');
    Serial.print(currentBuffer3[i], 3); Serial.print(',');
    Serial.print(currentBuffer4[i], 3); Serial.print(',');
    Serial.print(currentBuffer5[i], 3); Serial.println();
  }
}

void parseData(String data, long values[]) {
  int index = 0;
  char *ptr = strtok((char*)data.c_str(), ",");
  while (ptr != NULL && index < 5) {
    values[index] = atol(ptr);

    ptr = strtok(NULL, ",");
    index++;
  }
}

void sendIntermediateValues(int address1, int address2, int address3, int address4, int address5, 
                            long a, long b, long c, long d, long e, long f, long g, long h, long x, long y) {
  long delta1 = b - a;
  long delta2 = d - c;
  long delta3 = f - e;
  long delta4 = h - g;
  long delta5 = x - y;

  // Creăm o listă cu doar deltele nenule
  long validDeltas[] = { abs(delta1), abs(delta2), abs(delta3), abs(delta4), abs(delta5)};
  int maxSteps = 0;

  // Calculăm steps doar din deltele nenule
  for (int i = 0; i < 5; i++) {
    if (validDeltas[i] > 0) {
      maxSteps = max(maxSteps, int(ceil(validDeltas[i] / MAX_DIFF)));
    }
  }

  if (maxSteps == 0) return;  // Dacă nu avem pași de făcut, ieșim

  float inc1 = (delta1 != 0) ? delta1 / float(maxSteps) : 0;
  float inc2 = (delta2 != 0) ? delta2 / float(maxSteps) : 0;
  float inc3 = (delta3 != 0) ? delta3 / float(maxSteps) : 0;
  float inc4 = (delta4 != 0) ? delta4 / float(maxSteps) : 0;
  float inc5 = (delta5 != 0) ? delta5 / float(maxSteps) : 0;

  for (int i = 0; i <= maxSteps; i++) {
    int value1 = a + i * inc1;
    int value2 = c + i * inc2;
    int value3 = e + i * inc3;
    int value4 = g + i * inc4;
    int value5 = x + i * inc5;

    if (delta1 != 0) sendToSlave(address1, value1);
    if (delta2 != 0) sendToSlave(address2, value2);
    if (delta3 != 0) sendToSlave(address3, value3);
    if (delta4 != 0) sendToSlave(address4, value4);
    if (delta5 != 0) sendToSlave(address5, value5);

    delay(50);
  }
}


ISR(PCINT2_vect) {
  if (digitalRead(DONE_PIN) == HIGH) {
    Serial.println("\nGATA - Semnal primit de la Slave!");
    readyForInput = true;  // Activează introducerea de date
  }
}

void sendToSlave(int address, long value) {
  Wire.beginTransmission(address);
  Wire.write((value >> 16) & 0xFF); // Byte-ul superior (al treilea byte)
  Wire.write((value >> 8) & 0xFF);  // Byte-ul mijlociu (al doilea byte)
  Wire.write(value & 0xFF);         // Byte-ul inferior (primul byte)
  Wire.endTransmission();

  // Debugging (dacă e nevoie)
  // Serial.print("Trimis către ");
  // Serial.print(address);
  // Serial.print(": ");
  // Serial.println(value);
}

void controlTool(int state) {
  Wire.beginTransmission(SLAVE6_ADDRESS);
  Wire.write(state);       
  Wire.endTransmission();
}