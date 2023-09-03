

#include <IMXRT_board.h>

#define DSerial SerialUSB

static unsigned char id[8];
  
SerialFlashChip Flash;

bool TestPass= true;

void setup() 
{
  unsigned char buf[1024];
  unsigned long elapsed;

  //some delay
  delay(5000);

  SerialUSB.begin(115200);
  while (!SerialUSB) ; //wait until Serial ready

  DSerial.println(F("Serial Flash Erase Test"));

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

  size /=32;
    
  if (size > 0) 
  {
    DSerial.print(F("Flash Memory has "));
    DSerial.print(size);
    DSerial.println(F(" bytes."));
        
    DSerial.println(F("\n Erase All content..Please wait!"));
    unsigned long startMillis = millis();
    Flash.eraseAll();
   
    DSerial.println(F("\n Erase All completed"));

    DSerial.println("Dumping memory: ");
    memset(buf, 0, sizeof(buf));

    for (uint32_t i = 0; i < size; i+=1024) 
    {
      SerialFlash.read(i, buf, 1024);
      is_erased(buf, 1024);

      DSerial.println(""); 
      DSerial.print(i);           
      DSerial.print(":");    
      for (uint32_t j = 0; j < 1024; j++) 
      {
        DSerial.print(buf[j]);
        DSerial.print(".");

       if (j%32 == 0 && j!=0)
        {
          DSerial.println(""); 
          DSerial.print(i);           
          DSerial.print(":");                     
        }
      }
    }

    elapsed = millis() - startMillis;
    
    DSerial.print(F("  actual wait: "));
    DSerial.print(elapsed / 1000ul);
    DSerial.println(F(" seconds."));

    DSerial.println(F("\n Erase Sector..Please wait!"));

    size = 4096*2;

    startMillis = millis();
    Flash.eraseSector(0, size);
   
    DSerial.println("Dumping memory: ");
    memset(buf, 0, sizeof(buf));

    for (uint32_t i = 0; i < size; i+=1024) 
    {
      SerialFlash.read(i, buf, 1024);
      is_erased(buf, 1024);
      
      DSerial.println(""); 
      DSerial.print(i);           
      DSerial.print(":"); 
      for (uint32_t j = 0; j < 1024; j++) 
      {
        DSerial.print(buf[j]);
        DSerial.print(".");

        if (j%32 == 0 && j!=0)
        {
          DSerial.println(""); 
          DSerial.print(i);           
          DSerial.print(":");                     
        }
      }
    }

    DSerial.println(F("\n Erase Sector completed"));
    
    elapsed = millis() - startMillis;
    
    DSerial.print(F("  actual wait: "));
    DSerial.print(elapsed / 1000ul);
    DSerial.println(F(" seconds."));

    DSerial.println(F("\n Erase Block..Please wait!"));

   size = 65536*1;

    startMillis = millis();
    Flash.eraseBlock(0, size);
   
    DSerial.println("Dumping memory: ");   
    memset(buf, 0, sizeof(buf));

    for (uint32_t i = 0; i < size; i+=1024) 
    {
      SerialFlash.read(i, buf, 1024);

      DSerial.println(""); 
      DSerial.print(i);           
      DSerial.print(":");  
      for (uint32_t j = 0; j < 1024; j++) 
      {
        DSerial.print(buf[j]);
        DSerial.print(".");

        if (j%64 == 0 && j!=0)
        {
          DSerial.println(""); 
          DSerial.print(i);           
          DSerial.print(":");                     
        }      
      }  

      if (is_erased(buf, 1024) == false)
        {
          //fail
          break;
        }          

    }

    DSerial.println(F("\n Erase Block completed"));
    
    elapsed = millis() - startMillis;
    
    DSerial.print(F("  actual wait: "));
    DSerial.print(elapsed / 1000ul);
    DSerial.println(F(" seconds."));

  }

  if (TestPass == true)
  {
    DSerial.println(F("\n Tes pass successufly :) "));
  }
  else
  {
    DSerial.println(F("\n Tes failed :( "));    
  }
}

void loop() 
{

}

bool is_erased(const unsigned char *data, unsigned int len)
{
	while (len > 0) 
  {
		if (*data++ != 0xFF) 
    {
      DSerial.println("");
      DSerial.println(F("Address = "));
      DSerial.print((uint32_t)data);
      DSerial.print(F(" is not erased!!!!"));
      DSerial.println("");

      TestPass = false;

      return false;
    }

		len = len - 1;
	}

	return true;
}

