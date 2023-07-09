
/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  This example code is in the public domain.

  Serial1: Tx - J23-1, Rx - J23-2
  Serial2: Tx - J22-7, Rx - J22-8
*/
#include <fnet.h> //TCP/IP stack
#include <NativeEthernet.h>

// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
//int led = USER_LED_R;
int led = 13;

// the setup routine runs once when you press reset:
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

    Serial1.begin(115200);
    while (!Serial1) {
      ; // wait for serial port to connect. Needed for native USB port only
    }

    Serial2.begin(115200);
    while (!Serial2) {
      ; // wait for serial port to connect. Needed for native USB port only
    }

    SerialUSB.begin(115200);
    while (!SerialUSB) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
  
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);

  //delay(5000);
/*extern void (* _VectorsRam[])(void);
extern void (* __isr_vector[])(void);
char text[32];
uint32_t *s;
  for (int i = 0; i < 175; i++) {
    //s = (uint32_t*)(0x60012000 + i * 4);
    s = (uint32_t*)(_VectorsRam + i);
    //s = (uint32_t*)(__isr_vector + i);
    sprintf(text, " %d %04x", i, *s);
    Serial.println(text);
  }*/
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(500);               // wait for a second
  SerialUSB.write('c');
  //println(F("TEST"));
  Serial.write('c');
  //Serial1.write('c');
  //Serial2.write('c');
}
