

#include <IMXRT_board.h>

#define DSerial SerialUSB

static unsigned char id[8];
  
SerialFlashChip Flash;

void setup() 
{
  //some delay
  delay(5000);

  SerialUSB.begin(115200);
  while (!SerialUSB) ; //wait until Serial ready

  DSerial.println(F("Flash Erase Everything Test"));

  // wait up to 10 seconds for Arduino Serial Monitor
  unsigned long startMillis = millis();
  while (!Serial && (millis() - startMillis < 10000)) ;
  delay(100);

  if (!Flash.begin(1)) 
  {
    while (1) 
    {
      DSerial.println(F("Unable to access SPI Flash chip"));
      delay(1000);
    }
  }
  
  Flash.readID(id);

  SerialUSB.print("Manufacturer ID: 0x");

	for (unsigned i = 0; i < sizeof(id); i++) 
      SerialUSB.print(id[i], HEX);
 
 SerialUSB.println("");

  unsigned long size = Flash.capacity(id);

  if (size > 0) 
  {
    DSerial.print(F("Flash Memory has "));
    DSerial.print(size);
    DSerial.println(F(" bytes."));
        
    DSerial.println(F("\n Erase All content..Please wait!"));
    unsigned long startMillis = millis();
    Flash.eraseAll();
   
    DSerial.println(F("\n Erase All completed"));
    
    unsigned long elapsed = millis() - startMillis;
    
    DSerial.print(F("  actual wait: "));
    DSerial.print(elapsed / 1000ul);
    DSerial.println(F(" seconds."));


    DSerial.println(F("\n Erase Sector..Please wait!"));

    startMillis = millis();
    Flash.eraseSector(0, 4096*10);
   
    DSerial.println(F("\n Erase Sector completed"));
    
    elapsed = millis() - startMillis;
    
    DSerial.print(F("  actual wait: "));
    DSerial.print(elapsed / 1000ul);
    DSerial.println(F(" seconds."));


    DSerial.println(F("\n Erase Block..Please wait!"));

    startMillis = millis();
    Flash.eraseBlock(0, 65536*2);
   
    DSerial.println(F("\n Erase Block completed"));
    
    elapsed = millis() - startMillis;
    
    DSerial.print(F("  actual wait: "));
    DSerial.print(elapsed / 1000ul);
    DSerial.println(F(" seconds."));

  }
}

void loop() 
{

}

