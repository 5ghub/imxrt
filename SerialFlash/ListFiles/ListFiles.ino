#include "board.h"

#include <IMXRT_board.h>

#define DSerial SerialUSB

 SerialFlashChip SerialFlash;

void setup() 
{

  //some delay
  delay(5000);

  SerialUSB.begin(115200);
  while (!SerialUSB) ; //wait until Serial ready

  DSerial.println(F("Flash files List"));
  
  if (!SerialFlash.begin(1)) 
  {
    while (1) 
    {
      DSerial.println(F("Unable to access SPI Flash chip"));
      delay(1000);
    }
  }

  SerialFlash.opendir();
  while (1) 
  {
    char filename[64];
    uint32_t filesize;

    if (SerialFlash.readdir(filename, sizeof(filename), filesize)) 
    {
      DSerial.print(F("  "));
      DSerial.print(filename);
      spaces(20 - strlen(filename));
      DSerial.print(F("  "));
      DSerial.print(filesize);
      DSerial.print(F(" bytes"));
      DSerial.println();
    } 
    else 
    {
      break; // no more files
    }
  }
}

void spaces(int num) 
{
  for (int i=0; i < num; i++) 
  {
    DSerial.print(' ');
  }
}

void loop() 
{
}

void error(const char *message) 
{
  while (1) 
  {
    DSerial.println(message);
    delay(2500);
  }
}