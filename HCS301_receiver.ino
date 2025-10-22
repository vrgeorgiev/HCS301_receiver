#include <Arduino.h>

const uint8_t RX_PIN = 4;



const size_t BUFF_SIZE = 66;
volatile int risingEdge = 0;
volatile uint32_t risingStartTime = 0;
uint32_t now = 0;
volatile bool buff[BUFF_SIZE];
const unsigned int PERIOD = 400;  // µs
const unsigned int TREP = 25;     // µs
uint32_t sampleTime = 0;
bool bit = 0;
bool stopBit = 0;
bool startBit = 0;
volatile uint16_t buffIndex = 0;


void IRAM_ATTR ISR() {
  if (micros() - risingStartTime > 1300) {
    buffIndex = 0;
  }
  risingStartTime = micros();
  risingEdge = 1;
}



void setup() {
  Serial.begin(115200);
  pinMode(RX_PIN, INPUT);  // без PULLUP
  attachInterrupt(digitalPinToInterrupt(RX_PIN), ISR, RISING);
  Serial.println("Waiting for RC signal...");
  claerBuff();
}

void loop() {
  if (risingEdge) {
    risingEdge = 0;

    noInterrupts();
    sampleTime = risingStartTime + PERIOD + PERIOD / 2;
    bit = readLevelAt(sampleTime);

    bool bit2 = readLevelAt(sampleTime + TREP);
    bool bit3 = readLevelAt(sampleTime + TREP);
    bool bit4 = readLevelAt(sampleTime + TREP);
    bool bit5 = readLevelAt(sampleTime + TREP);

    int a = bit + bit2 + bit3 + bit4 + bit5;
    if (a >= 3) {
      bit = 0;
    } else {
      bit = 1;
    };

/*
    sampleTime = sampleTime + PERIOD;
    stopBit = readLevelAt(sampleTime);
    */

    sampleTime = sampleTime + TREP;
    stopBit = stopBit && readLevelAt(sampleTime);
    sampleTime = sampleTime + TREP;
    stopBit = stopBit && readLevelAt(sampleTime);


    interrupts();

    if (stopBit==0) {
      buff[buffIndex++] = bit;
    }



    if (buffIndex >= BUFF_SIZE - 1) {  //buffer full.
      noInterrupts();
      Serial.print("\nBuff:");
      for (uint16_t i = 0; i < BUFF_SIZE; i++) {  // print all
        Serial.print(buff[i]);
        Serial.print(",");
      }
      buffIndex = 0;  //start over.
      claerBuff();
      interrupts();
    }
  }
}

bool readLevelAt(unsigned long targetTime) {
  while (micros() < targetTime) {
    ;
  }
  return digitalRead(RX_PIN);
};

void claerBuff() {
  for (uint16_t i = 0; i < BUFF_SIZE; i++) {
    (buff[i]) = 0;
  }
}