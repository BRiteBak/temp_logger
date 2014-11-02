#include <SdFat.h>
#include <OneWire.h>
//including librarys for hardware

OneWire ds(52); //the pin to use for temp

const int chipSelect = 53; //pin to use for ss channel 

SdFat sd; // file system object
SdFile myFile;

ArduinoOutStream cout(Serial); // create Serial stream

#define error(s) sd.errorHalt_P(PSTR(s)) // store error strings in flash to save RAM

unsigned long time;

void setup(void) {
  char name[] = "TEMPERATURE.TXT"; //filename
  char foo[200]; 
  Serial.begin(9600);
  while (!Serial) {} //wait for arduino
  cout << endl << pstr("Type any character to start brochacho!\n");
  while (Serial.read() <= 0) {}
  delay(400); //cathc due to rest problem
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.  
  
 if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();
  // re-open the file for reading:
  if (!myFile.open("config.txt", O_READ)) {
    sd.errorHalt("opening config.txt for read failed");
  }
  Serial.println("config.txt");
//  // read from the file until there's nothing else in it:
//  int reading;
//  while ((reading = myFile.read()) >= 0) {
//  Serial.write(reading);
//  }

int bar=0;
int in_char;
//Keep reading characters from the file until we get an error or reach the end of the file. (This will output the entire contents of the file).
while((in_char = myFile.read()) >= 0){             //If the value of the character is less than 0 we've reached the end of the file.
    Serial.write(in_char);    //Print the current character`
    foo[bar++] = (char)in_char;
}
Serial.write(foo); 
myFile.close();  

ofstream sdout("file.txt", ios::out | ios::app);
    sdout << " id = " << foo << endl;
    sdout.close();  

}
//-------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
  int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      Serial.print("No more addresses.\n");
      ds.reset_search();
      return;
  }

  Serial.print("R=");
  for( i = 0; i < 8; i++) {
    Serial.print(addr[i], HEX);
    Serial.print(" ");
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n");
      return;
  }

  if ( addr[0] == 0x10) {
      Serial.print("Device is a DS18S20 family device.\n");
  }
  else if ( addr[0] == 0x28) {
      Serial.print("Device is a DS18B20 family device.\n");
  }
  else {
      Serial.print("Device family is not recognized: 0x");
      Serial.println(addr[0],HEX);
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("P=");
  Serial.print(present,HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print( OneWire::crc8( data, 8), HEX);
  Serial.println();
  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25

  Whole = Tc_100 / 100;  // separate off the whole and fractional portions
  Fract = Tc_100 % 100;
  
  

/*  if (SignBit) // If its negative
  {
     Serial.print("-");
  }
  Serial.print(Whole);
  Serial.print(".");
  if (Fract < 10)
  {
     Serial.print("0");
  }
  Serial.print(Fract);

  Serial.print("\n"); */
  writeTemperature(Whole, Fract);
  
Serial.print("time: ");
time = millis();
Serial.println(time);

} 
//------------------------------------------------------------------------------
void writeTemperature(int whole, int fract) {
    Serial.print(whole);
    Serial.print(".");
    Serial.println(fract);
    
    ofstream sdout("file.txt", ios::out | ios::app);
    sdout << " temperature = " << whole << "." << fract << endl;
    sdout.close();  

    
}

