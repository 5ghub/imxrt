#include <IMXRT_board.h>

#define DSerial SerialUSB

 SerialFlashChip SerialFlash;

void setup() 
{

  //some delay
  delay(5000);

  SerialUSB.begin(115200);
  while (!SerialUSB) ; //wait until Serial ready

  DSerial.println(F("Flash File System Test"));
  
  if (!SerialFlash.begin(1)) 
  {
    while (1) 
    {
      DSerial.println(F("Unable to access SPI Flash chip"));
      delay(1000);
    }
  }
}

void loop() {

  // MAIN MENU
  DSerial.println("");
  DSerial.println("");
  DSerial.println("SerialFlash Read and Write");
  DSerial.println("(switch your terminal to no line endings)");
  DSerial.println("--------------------------");
  DSerial.println("1) Create a new file");
  DSerial.println("2) Open a file");
  DSerial.println("3) Delete a file");
  DSerial.println("4) Erase All");  
  DSerial.println("--------------------------");
  DSerial.println("Select a number");
  while (!DSerial.available()) {}
  char choice = DSerial.read();
  while (DSerial.available()) { DSerial.read(); }

  switch (choice) {

    case '1':
      newFile();
      break;

    case '2':
      openFile();
      break;

    case '3':
      deleteFile();
      break;

    case '4':
      eraseAll();
      break;      

    default:
      DSerial.println("Invalid Selection");
  }
}

/* Create a new file
 * 
 * Request filename up to 20 chars
 * Request a size up to 256 bytes
 * Request some contents
 * Create a file
 */
void newFile() {

  DSerial.println("Enter a filename");  // Request filename from user
  while (!DSerial.available()) {}        // Wait for user

  char filename[20] = {};  // buffer to store user filename
  DSerial.readBytesUntil(' ', filename, 20);
  while (DSerial.available()) { DSerial.read(); }

  DSerial.println("Enter a filesize in bytes");  // Request file size from user
  while (!DSerial.available()) {}                 // Wait for user

  char sizeArray[3] = {};  // buffer to store requested file size
  DSerial.readBytesUntil(' ', sizeArray, 3);
  while (DSerial.available()) { DSerial.read(); }
  int filesize = atoi(sizeArray);  // Convert char array to int (i.e. "40" to 40)

  if (SerialFlash.create(filename, filesize)) {  // Returns false if file already exists

    SerialFlashFile file;  // Open the file we just created for writing
    file = SerialFlash.open(filename);
    DSerial.println("Write some file contents:");  // Request file contents from user
    while (!DSerial.available()) {}                 // Wait for user

    char contents[256] = {};  // buffer to store file contents
    DSerial.readBytesUntil(255, contents, 256);
    while (DSerial.available()) { DSerial.read(); }  // Empty read buffer
    file.write(contents, filesize);                // Write the contents buffer
    DSerial.println("");
    DSerial.print("New file ");
    DSerial.print(filename);
    DSerial.print(" created with size ");
    DSerial.print(filesize);
    DSerial.println(" bytes!");
  } 
  else 
  {
    DSerial.println("");
    DSerial.println("There was an error creating this file (does it already exist?)");
  }

  return;
}

/* Open a file
 * 
 * Print the directory listing
 * Request filename up to 20 chars
 * Open file and display contents
 */
void openFile() 
{

  printDir();  // Function to print the directory listing

  DSerial.println("Enter a filename to OPEN");  // Request file name from user
  DSerial.println();
  while (!DSerial.available()) {}  // Wait for user

  char filename[20] = {};  // buffer to store the file name
  DSerial.readBytesUntil(' ', filename, 20);
  while (DSerial.available()) 
  { DSerial.read(); }

  DSerial.println(filename);

  SerialFlashFile file;
  file = SerialFlash.open(filename);  // open the file
  if (file) 
  {
    DSerial.print("File Name: ");
    DSerial.println(filename);
    DSerial.println();
    DSerial.print("File Size: ");
    DSerial.print(file.size());
    DSerial.println(" bytes");
    DSerial.println();
    DSerial.println("File Contents:");
    char buffer[256] = {};   // create a buffer for the file contents
    file.read(buffer, 256);  // read file to buffer
    DSerial.print(buffer);
  } 
  else 
  {
    DSerial.println("File not found!");
  }

  return;
}

/* Delete a file
 * 
 * Print the directory listing
 * Request filename up to 20 chars
 * Delete File
 */
void deleteFile() 
{
  printDir();  // Function to print the directory listing

  DSerial.println("Enter a filename to DELETE");  // Request file name from user
  while (!DSerial.available()) {}                 // Wait for user

  char filename[20] = {};  // buffer to store file name
  DSerial.readBytesUntil(' ', filename, 20);
  while (DSerial.available()) 
  { DSerial.read(); }

  SerialFlash.remove(filename);  // Delete the file

  return;
}

void eraseAll() 
{
  DSerial.println(F("\n Erase All content..Please wait!"));
  
  SerialFlash.eraseAll();

  DSerial.println(F("\n Erase All completed"));
  
  return;
}


/* Print Directory
 * 
 * Print a list of all files on the chip
 * Stolen from SerialFlash library example "ListFiles"
 */
void printDir() 
{
  DSerial.println("Directory Listing");
  DSerial.println("-----------------");

  SerialFlash.opendir();
  while (1) 
  {
    char filename[64];
    uint32_t filesize;

    if (SerialFlash.readdir(filename, sizeof(filename), filesize)) 
    {
      DSerial.print("  ");
      DSerial.print(filename);
      spaces(20 - strlen(filename));
      DSerial.print("  ");
      DSerial.print(filesize);
      DSerial.print(" bytes");
      DSerial.println();
    } 
    else 
    {
      break;  // no more files
    }
  }
}

void spaces(int num) 
{
  for (int i = 0; i < num; i++) 
  {
    DSerial.print(" ");
  }
}
