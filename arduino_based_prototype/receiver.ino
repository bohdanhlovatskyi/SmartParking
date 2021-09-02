#include <SPI.h> 
#include <nRF24L01.h> 
#include <RF24.h>
#define PIN_CE  7
#define PIN_CSN 8

#define DATA_SIZE 3

const byte address[6] = "00001";

RF24 radio(PIN_CE, PIN_CSN);

// array for data receiving
float rc_data[DATA_SIZE] = {0};

void setup() {
  Serial.begin(9600);
  if (!radio.begin()) {
    Serial.println("There was a problem with connection");
    while(1);
  }

  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

}

void loop() {
  while(radio.available())
  {
    radio.read(&rc_data, sizeof(rc_data));
    for (int i = 0; i < DATA_SIZE; i++) {
      Serial.print(rc_data[i]); Serial.print("; ");
    }
    Serial.println();
  }
}
