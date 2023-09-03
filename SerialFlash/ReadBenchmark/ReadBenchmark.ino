

#include <IMXRT_board.h>

#define DSerial SerialUSB

 SerialFlashChip SerialFlash;

void setup() 
{
  //some delay
  delay(5000);

  SerialUSB.begin(115200);
  while (!SerialUSB) ; //wait until Serial ready

  DSerial.println(F("Read files Test"));
  
  if (!SerialFlash.begin(1)) 
  {
    while (1) 
    {
      DSerial.println(F("Unable to access SPI Flash chip"));
      delay(1000);
    }
  }

  SerialFlash.opendir();
  int filecount = 0;
  while (1) 
  {
    char filename[64];
    uint32_t filesize;

    if (SerialFlash.readdir(filename, sizeof(filename), filesize)) 
    {
      DSerial.print(F("  "));
      DSerial.print(filename);
      DSerial.print(F(", "));
      DSerial.print(filesize);
      DSerial.print(F(" bytes"));
      SerialFlashFile file = SerialFlash.open(filename);
      if (file) 
      {
        unsigned long usbegin = micros();
        unsigned long n = filesize;
        char buffer[256];
        while (n > 0) 
        {
          unsigned long rd = n;
          if (rd > sizeof(buffer)) 
           rd = sizeof(buffer);
          
          file.read(buffer, rd);
          n = n - rd;
        }

        DSerial.println("File Contents:");
        DSerial.println(buffer);

        unsigned long usend = micros();
        DSerial.print(F(" Read in "));
        DSerial.print(usend - usbegin);
        DSerial.print(F(" us, speed = "));
        DSerial.print((float)filesize * 1000.0 / (float)(usend - usbegin));
        DSerial.println(F(" kbytes/sec"));
        file.close();
      } 
      else 
      {
        DSerial.println(F(" error reading this file!"));
      }
      filecount = filecount + 1;
    } 
    else 
    {
      if (filecount == 0) 
      {
        DSerial.println(F("No files found in SerialFlash memory."));
      }
      break; // no more files
    }
  }
}

void loop() 
{
}

