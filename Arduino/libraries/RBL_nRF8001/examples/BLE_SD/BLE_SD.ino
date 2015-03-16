/*

Copyright (c) 2012-2014 RedBearLab

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/*
 *    Chat
 *
 *    Simple chat sketch, work with the Chat iOS/Android App.
 *    Type something from the Arduino serial monitor to send
 *    to the Chat App or vice verse.
 *
 */
 
/*
 *    SD card read/write
 *
 *    This example shows how to read and write data to and from an SD card file 
 *    The SD library:
 *       https://github.com/adafruit/SD	
 *    The circuit:
 *       SD card attached to SPI bus as follows:
 *       ** Mega:  MOSI - pin 51, MISO - pin 50, CLK - pin 52, CS - pin 4 (CS pin can be changed) and pin #52 (SS) must be an output
 *       ** UNO: Beacous of BLE Library, it is too large for UNO, it can't run on UNO.
 */
 
//"RBL_nRF8001.h/spi.h/boards.h" is needed in every new project
#include <SPI.h>
#include <EEPROM.h>
#include <boards.h>
#include <RBL_nRF8001.h>

#include <SD.h>

File myFile;
//     change this to match your SD shield or module;
//     Arduino Ethernet shield: pin 4
//     Adafruit SD shields and modules: pin 10
//     Sparkfun SD shield: pin 8
const int chipSelect = 4;


void setup()
{  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;// wait for serial port to connect. Needed for Leonardo only
  }
  // Default pins set to 9 and 8 for REQN and RDYN
  // Set your REQN and RDYN here before ble_begin() if you need
  //ble_set_pins(3, 2);
  
  // Set your BLE Shield name here, max. length 10
  //ble_set_name("My Name");
  
  // Init. and start BLE library.
  ble_begin();
  
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(SS, OUTPUT);
  
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

unsigned char rx_buf[20] = {0};
unsigned char rx_len = 0;
unsigned char ret_dat;

void loop()
{
  if ( ble_available() )
  {
    while ( ble_available() )
    {
      rx_buf[rx_len] = ble_read();
      Serial.write(rx_buf[rx_len]);
      rx_len++;
    } 
    Serial.println();
  }
  
  if((!ble_busy()) && (rx_len > 0))
  {
     Serial.println("Write data to BLE_SD.txt");
     myFile = SD.open("BLE_SD.txt", FILE_WRITE);
     // if the file opened okay, write to it:
     if (myFile) {
       //Serial.print("Writing to BLE_Data.txt...");
       while(--rx_len)
         myFile.write(rx_buf[rx_len]); 
       myFile.write(rx_buf[0]);
       // close the file:
       myFile.close();
       Serial.println("Write done.");
     } else {
       // if the file didn't open, print an error:
       Serial.println("Open error");
     }
     rx_len = 0;
     Serial.println("Read data from BLE_SD.txt");
     myFile = SD.open("BLE_SD.txt");
     if (myFile) {
       // read from the file until there's nothing else in it:
       while (myFile.available()) {
         ret_dat = myFile.read();
         ble_write(ret_dat); 
         Serial.write(ret_dat);
       }
       Serial.println("");
       Serial.println("Read Done");
         // close the file:
         myFile.close();
      } else {
      	 // if the file didn't open, print an error:
         Serial.println("Open error");
      }
   } 
  
   ble_do_events();
}

