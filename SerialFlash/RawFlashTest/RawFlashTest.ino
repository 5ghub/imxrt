// RawHardwareTest - Check if a SPI Flash chip is compatible
// with SerialFlash by performing many read and write tests
// to its memory.
//
// The chip should be fully erased before running this test.
// Use the EraseEverything to do a (slow) full chip erase.
//
// Normally you should NOT access the flash memory directly,
// as this test program does.  You should create files and
// read and write the files.  File creation allocates space
// with program & erase boundaries within the chip, to allow
// reading from any other files while a file is busy writing
// or erasing (if created as erasable).
//

#include <IMXRT_board.h>

#define DSerial SerialUSB

 SerialFlashChip SerialFlash;

const unsigned long testIncrement = 4096;

void setup() 
{
  //some delay
  delay(5000);

  SerialUSB.begin(115200);
  while (!SerialUSB) ; //wait until Serial ready

  DSerial.println(F("Raw Flash Tes"));
  
  if (!SerialFlash.begin(1)) 
  {
    while (1) 
    {
      DSerial.println(F("Unable to access SPI Flash chip"));
      delay(1000);
    }
  }

  if (test()) 
  {
    DSerial.println();
    DSerial.println(F("All Tests Passed  :-)"));
    DSerial.println();
    DSerial.println(F("Test data was written to your chip.  You must run"));
    DSerial.println(F("EraseEverything before using this chip for files."));
  } 
  else
  {
    DSerial.println();
    DSerial.println(F("Tests Failed  :{"));
    DSerial.println();
    DSerial.println(F("The flash chip may be left in an improper state."));
    DSerial.println(F("You might need to power cycle to return to normal."));
  }
}

bool test() 
{
  unsigned char id[8], buf[256], sig[256], buf2[8];
  unsigned long address, count, chipsize, blocksize;
  unsigned long usec;
  bool first;

  // Read the chip identification
  DSerial.println();
  DSerial.println(F("Read Chip Identification:"));
  
  SerialFlash.readID(id);
  DSerial.print(F("  JEDEC ID:     "));
  DSerial.print(id[0], HEX);
  DSerial.print(' ');
  
  DSerial.print(id[1], HEX);
  DSerial.print(' ');
  
  DSerial.println(id[2], HEX);
  
  DSerial.print(F("  Part Number: "));
  DSerial.println(id2chip(id));
  
  DSerial.print(F("  Memory Size:  "));
  chipsize = SerialFlash.capacity(id);
  DSerial.print(chipsize);
  DSerial.println(F(" bytes"));

  DSerial.print(F("  Block Size:   "));
  blocksize = SerialFlash.blockSize();
  DSerial.print(blocksize);
  DSerial.println(F(" bytes"));

   if (chipsize == 0) 
     return false;

  DSerial.println(F("\n Erasing Chip....Please wait!"));

  unsigned long startMillis = millis();
  SerialFlash.eraseAll();
  DSerial.println(F("\n Erase All completed"));

  unsigned long elapsed = millis() - startMillis;

  DSerial.print(F("  actual wait: "));
  DSerial.print(elapsed / 1000ul);
  DSerial.println(F(" seconds."));


   memset(buf, 0, sizeof(buf));
   memset(sig, 0, sizeof(sig));
   memset(buf2, 0, sizeof(buf2));
 
   address = 0;
   count = 0;
   first = true;
 
    //chipsize = 4096 *10000;
    
    while (address < chipsize) 
    {
      SerialFlash.read(address, buf, 8);

      create_signature(address, sig);

      SerialFlash.write(address, sig, 8);      
      
      while (!SerialFlash.ready()) ; // wait
        SerialFlash.read(address, buf, 8);
      
      if (equal_signatures(buf, sig) == false) 
      {
        DSerial.print(F("  error writing signature at "));
        DSerial.println(address);
        DSerial.print(F("  Read this: "));
        printbuf(buf, 8);
        DSerial.print(F("  Expected:  "));
        printbuf(sig, 8);
        return false;
      }

      if (first)
      {
        address = address + (testIncrement - 8);
        first = false;
      } 
      else 
      {
        address = address + 8;
        first = true;
      }
    }
  
  
  // Read all the signatures again, just to be sure
  // checks prior writing didn't corrupt any other data
  DSerial.println();
  DSerial.println(F("Double Checking All Signatures:"));
  memset(buf, 0, sizeof(buf));
  memset(sig, 0, sizeof(sig));
  memset(buf2, 0, sizeof(buf2));
  count = 0;
  address = 0;
  first = true;
  
  while (address < chipsize) 
  {
    SerialFlash.read(address, buf, 8);
    create_signature(address, sig);
    
    if (equal_signatures(buf, sig) == false) 
    {
      DSerial.print(F("  error in signature at "));
      DSerial.println(address);
      DSerial.print(F("  Read this: "));
      printbuf(buf, 8);
      DSerial.print(F("  Expected:  "));
      printbuf(sig, 8);
      return false;
    }

    count = count + 1;
    
    if (first) 
    {
      address = address + (testIncrement - 8);
      first = false;
    } 
    else 
    {
      address = address + 8;
      first = true;
    }
  }

  DSerial.print(F("  all "));
  DSerial.print(count);
  DSerial.println(F(" signatures read ok"));


  // Read pairs of adjacent signatures
  // check read works across boundaries
  DSerial.println();
  DSerial.println(F("Checking Signature Pairs"));
  memset(buf, 0, sizeof(buf));
  memset(sig, 0, sizeof(sig));
  memset(buf2, 0, sizeof(buf2));
  count = 0;
  address = testIncrement - 8;
  first = true;

  while (address < chipsize - 8) 
  {
    SerialFlash.read(address, buf, 16);
    create_signature(address, sig);
    create_signature(address + 8, sig + 8);
    if (memcmp(buf, sig, 16) != 0) 
    {
      DSerial.print(F("  error in signature pair at "));
      DSerial.println(address);
      DSerial.print(F("  Read this: "));
      printbuf(buf, 16);
      DSerial.print(F("  Expected:  "));
      printbuf(sig, 16);
      return false;
    }
    count = count + 1;
    address = address + testIncrement;
  }

  DSerial.print(F("  all "));
  DSerial.print(count);
  DSerial.println(F(" signature pairs read ok"));

  // Write data and read while write in progress
  DSerial.println();
  DSerial.println(F("Checking Read-While-Write (Program Suspend)"));
  

  // find a blank space
  address = 256;
  while (address < chipsize) 
  { 
    SerialFlash.read(address, buf, 256);
    if (is_erased(buf, 256)) 
       break;
    
    address = address + 256;
  }
  
  if (address >= chipsize) 
  {
    DSerial.println(F("  error, unable to find any blank space!"));
    return false;
  }
  
  for (int i=0; i < 256; i += 8) 
  {
    create_signature(address + i, sig + i);
  }

  DSerial.print(F("  write 256 bytes at "));
  DSerial.println(address);
  DSerial.flush();
  SerialFlash.write(address, sig, 256);
  usec = micros();
  if (!SerialFlash.ready()) 
  {
    DSerial.println(F("  error, chip is still busy after write"));
    return false;
  }
  SerialFlash.read(0, buf2, 8); // read while busy writing
  
  while (!SerialFlash.ready()) ; // wait
  usec = micros() - usec;
  DSerial.print(F("  write time was "));
  DSerial.print(usec);
  DSerial.println(F(" microseconds."));
  
  // read the 8 signatures and compare  
  SerialFlash.read(address, buf, 256);
  if (memcmp(buf, sig, 256) != 0) 
  {
    DSerial.println(F("  error writing to flash"));
    DSerial.print(F("  Read this: "));
    printbuf(buf, 256);
    DSerial.print(F("  Expected:  "));
    printbuf(sig, 256);
    return false;
  }

  create_signature(0, sig);
  
  if (memcmp(buf2, sig, 8) != 0) 
  {
    DSerial.println(F("  error, incorrect read while writing"));
    DSerial.print(F("  Read this: "));
    printbuf(buf2, 256);
    DSerial.print(F("  Expected:  "));
    printbuf(sig, 256);
    return false;
  }

  DSerial.print(F("  read-while-writing: "));
  printbuf(buf2, 8);
  DSerial.println(F("  test passed, good read while writing"));

  // Erase a block and read while erase in progress
  //old block 262144
  if (chipsize >= 65536 + blocksize + testIncrement) 
  {
    DSerial.println();
    DSerial.println(F("Checking Read-While-Erase (Erase Suspend)"));
    memset(buf, 0, sizeof(buf));
    memset(sig, 0, sizeof(sig));
    memset(buf2, 0, sizeof(buf2));
    
    // first page
    SerialFlash.eraseSector(65536, 16*4096);

    // third page
    SerialFlash.eraseSector(196608, 16*4096);

    // fifth block
     SerialFlash.eraseBlock(327680);

    // Tenth block
     SerialFlash.eraseBlock(655360);

    usec = micros();
    delayMicroseconds(50); 

    if (!SerialFlash.ready()) 
    {
      DSerial.println(F("  error, chip is still busy after erase"));
      return false;
    }

    SerialFlash.read(0, buf2, 8); // read while busy writing
    while (!SerialFlash.ready()) ; // wait
    usec = micros() - usec;
    DSerial.print(F("  erase time was "));
    DSerial.print(usec);
    DSerial.println(F(" microseconds."));
    // read all signatures, check ones in this block got
    // erased, and all the others are still intact
    address = 0;
    first = true;
    
    while (address < chipsize) 
    {
      SerialFlash.read(address, buf, 8);
      if ((address >= 65536 && address < (65536 + 16*4096)) || 
          (address >= 196608 && address < (196608 + 16*4096))  || 
          (address >= 327680 && address < (327680 + 65536)) ||
          (address >= 655360 && address < (655360 + 65536))) 
      {
        if (is_erased(buf, 8) == false) 
        {
          DSerial.print(F("  error in erasing at "));
          DSerial.println(address);
          DSerial.print(F("  Read this: "));
          printbuf(buf, 8);
          return false;
        }
      } 
      else 
      {
        create_signature(address, sig);
        if (equal_signatures(buf, sig) == false) 
        {
          DSerial.print(F("  error in signature at "));
          DSerial.println(address);
          DSerial.print(F("  Read this: "));
          printbuf(buf, 8);
          DSerial.print(F("  Expected:  "));
          printbuf(sig, 8);
          return false;
        }
      }

      if (first) 
      {
        address = address + (testIncrement - 8);
        first = false;
      } 
      else 
      {
        address = address + 8;
        first = true;
      }
    }
    
    DSerial.print(F("  erase correctly erased "));
    DSerial.print(blocksize);
    DSerial.println(F(" bytes"));
    
    // now check if the data we read during erase is good
    create_signature(0, sig);
    
    if (memcmp(buf2, sig, 8) != 0) 
    {
      DSerial.println(F("  error, incorrect read while erasing"));
      DSerial.print(F("  Read this: "));
      printbuf(buf2, 256);
      DSerial.print(F("  Expected:  "));
      printbuf(sig, 256);
      return false;
    }
    DSerial.print(F("  read-while-erasing: "));
    printbuf(buf2, 8);
    DSerial.println(F("  test passed, good read while erasing"));

  } 
  else 
  {
    DSerial.println(F("Skip Read-While-Erase, this chip is too small"));
  }

  return true;
}


void loop() 
{
  // do nothing after the test
}

const char * id2chip(const unsigned char *id)
{
	if (id[0] == 0xEF) {
		// Winbond
		if (id[1] == 0x40) {
			if (id[2] == 0x14) return "W25Q80BV";
			if (id[2] == 0x15) return "W25Q16DV";
			if (id[2] == 0x17) return "W25Q64FV";
			if (id[2] == 0x18) return "W25Q128FV";
			if (id[2] == 0x19) return "W25Q256FV";
		}
	}
	if (id[0] == 0x01) {
		// Spansion
		if (id[1] == 0x02) {
			if (id[2] == 0x16) return "S25FL064A";
			if (id[2] == 0x19) return "S25FL256S";
			if (id[2] == 0x20) return "S25FL512S";
		}
		if (id[1] == 0x20) {
			if (id[2] == 0x18) return "S25FL127S";
		}
	}
	if (id[0] == 0xC2) {
		// Macronix
		if (id[1] == 0x20) {
			if (id[2] == 0x18) return "MX25L12805D";
		}
	}
	if (id[0] == 0x20) {
		// Micron
		if (id[1] == 0xBA) {
			if (id[2] == 0x20) return "N25Q512A";
			if (id[2] == 0x21) return "N25Q00AA";
		}
		if (id[1] == 0xBB) {
			if (id[2] == 0x22) return "MT25QL02GC";
		}
	}
	if (id[0] == 0xBF) {
		// SST
		if (id[1] == 0x25) {
			if (id[2] == 0x02) return "SST25WF010";
			if (id[2] == 0x03) return "SST25WF020";
			if (id[2] == 0x04) return "SST25WF040";
			if (id[2] == 0x41) return "SST25VF016B";
			if (id[2] == 0x4A) return "SST25VF032";
		}
		if (id[1] == 0x25) {
			if (id[2] == 0x01) return "SST26VF016";
			if (id[2] == 0x02) return "SST26VF032";
			if (id[2] == 0x43) return "SST26VF064";
		}
	}
  	if (id[0] == 0x1F) {
    		// Adesto
   		if (id[1] == 0x89) {
      			if (id[2] == 0x01) return "AT25SF128A";
    		}  
  	} 	
	return "(unknown chip)";
}

void print_signature(const unsigned char *data)
{
	DSerial.print(F("data="));
	for (unsigned char i=0; i < 8; i++) 
  {
		DSerial.print(data[i]);
		DSerial.print(' ');
	}
	DSerial.println();
}

void create_signature(unsigned long address, unsigned char *data)
{
	data[0] = address >> 24;
	data[1] = address >> 16;
	data[2] = address >> 8;
	data[3] = address;
	
  unsigned long hash = 2166136261ul;
	
  for (unsigned char i=0; i < 4; i++) 
  {
		hash ^= data[i];
		hash *= 16777619ul;
	}
	
  data[4] = hash;
	data[5] = hash >> 8;
	data[6] = hash >> 16;
	data[7] = hash >> 24;
}

bool equal_signatures(const unsigned char *data1, const unsigned char *data2)
{
	for (unsigned char i=0; i < 8; i++) 
  {
		if (data1[i] != data2[i]) return false;
	}

	return true;
}

bool is_erased(const unsigned char *data, unsigned int len)
{
	while (len > 0) 
  {
		if (*data++ != 255) 
      return false;
		len = len - 1;
	}

	return true;
}

void printbuf(const void *buf, uint32_t len)
{
  const uint8_t *p = (const uint8_t *)buf;
  do {
    unsigned char b = *p++;
    DSerial.print(b >> 4, HEX);
    DSerial.print(b & 15, HEX);
    //DSerial.printf("%02X", *p++);
    DSerial.print(' ');
  } while (--len > 0);
  DSerial.println();
}


