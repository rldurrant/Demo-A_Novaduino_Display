//*******10********20********30********40********50********60********70*******80********90*******100
#define LABEL "NOVADUINO(R):Demo_A_NovaduinoDisplay_V1p1"  // File Name

// IDE:Arduino 1.8.19
// Date: 06/30/2025
// Author: Nova Radio Labs LLC
/*
For Software and Hardware Demonstration and Testing of the Noavaduino(R) Display NRT1000014RevC PCBA

1. install this program onto a SparkFun SAMD51 M4+ Thing Plus or Adafruit M0 or M4 Feather Device.
    [Other Feather* form factor processors may work but have not been tested. Eg: STM32 or ESP32 or 
    or RP2040 or RP2350 etc...]
2. Install the Feather onto the NRT1000014RevC Novaduino(R) Display PCBA
    Display Device with 2.4 Inch 4DLCD-24320240-ips-rtp device installed
3. (already installed) keyboard and rotary encoder driver onto the ATtiny1626 gpio expander chip
    using MPLAB and project: "Novaduino_kbd_renc-rev7" or newer revision.
4. Run the program and open the Serial Monitor within the Arduino IDE
5. This program tests the following functions:
    5.1 writing to the display over SPI
    5.2 key and rotary encoder interrupt interface, reading and displaying key number
        and running tally of Rotary Encoder increment or decrement ( do not exceed +/-127 for now)
    5.3 Touch screen: displays the scaled value of x/y/pressure on the LCD when touched and paints
        a red dot. Also displays the raw touch screen x/y/pressure on the serial monitor.      
    5.4 The RGB LED . starts at Red, then Green, then Blue according to ColorNum 0,1,2. Simultaneously 
        displays the color number on the LCD.
    5.5 The uSD card size is read and displayed on the LCD.

 *  Copyright (c) 2025 Nova Radio Labs LLC

 *Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 *and associated documentation files (the “Software”), to deal in the Software without restriction, 
 *including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 *sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
 *furnished to do so, subject to the following conditions:

 *The above copyright notice and this permission notice shall be included in all copies or 
 *substantial portions of the Software.

 *THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
 *BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 *NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 *DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *  
 *  This software is released under the MIT License(http://opensource.org/licenses/MIT).
 *  
 *  Graphics and display and other driver software is from Adafruit Industries. Thank you Adafruit.
 *  Other software contained herein is from other sources as noted.
 *  * Feather form factor is from Adafruit.
 *  
 */
//*******10********20********30********40********50********60********70*******80********90*******100
//***************LCD Drivers************************************************************************

#define ST7789                // either ILI9341 or ST7789. must correspond to the display you have
#include <SPI.h>               // by Arduino.cc
#include <Adafruit_GFX.h>      // graphics library by Adafruit https://www.adafruit.com/product/3787

#if defined(ST7789)
  #include <Adafruit_ST7789.h> //  library for ST7789 by Adafruit
#endif

#if defined(ILI9341)
  #include <Adafruit_ILI9341.h>  // library for ILI9341 display by Adafruit
#endif

#define SPISPEED 16000000      // in Hz

#define TFT_CS        5     // CS is on D5
#define TFT_RST       A5    // Disp_RST(Display Reset) is on A5
#define TFT_DC        6     // DC(Data/Command) is on D6

#if defined(ST7789)
// For the 2.4" TFT with ST7789 Driver Chip
// instantiate the Adafruit_ST7789 object: tft (if using ST7789 then un-comment the following line)
  Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST); //instantiate tft.
#endif

#if defined(ILI9341)
// For the 2.4" TFT with ILI9341 Driver Chip
// instantiate the Adafruit_ILI9341 object: tft (if using ILI9341 then un-comment the following line)
  Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST ); //instantiate tft.
#endif

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREY    0xCE79
#define LIGHTGREY 0xDEDB

//**************WS2812 RGB LED is on Pin D13********************************************************

#include <Adafruit_NeoPixel.h> // from Adafruit

#define NEOPIN    13  // Novaduino NEOPIX is on pin D13
#define NUMPIXELS 1   // only 1 NEOPIX on the Novaduino 2.4 Display
uint8_t red = 0;      // initial NEOPIX colors
uint8_t blue = 0;
uint8_t green = 0; 
uint8_t ColorNum = 1;

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);     //instantiate pixels.


//*******setup TWI(I2C) driver and ATtiny1628 Keyscan/Rotary Encoder read Interrupt variables *****

#include <Wire.h>                // by Arduino. Used to read keyboard & Rot Encoder

const uint8_t interruptPin = 12; //the I2C interrupt signal from the kbd proc is on D12

// variables that are used in the ISR should be declared volatile.

volatile bool twiIntFlag = false; // if the interrupt occurs, this flag is set true
volatile int8_t button[4];        // receives the data from the keyboard processor over I2C
volatile int8_t buttonTot =0;     // accumulates the button presses for display
volatile int8_t buttonNum = 0;

char Bbuffer[2];     // Button buffer
char REbuffer[3];    // Rotary Encoder buffer

//*******10********20********30********40********50********60********70*******80********90*******100
//***************Touchscreen Driver from Adafruit *************************************************
// This is calibration data for the raw touch data translation to the screen coordinates

#include "Adafruit_TSC2007.h"

#define TS_MINX 300
#define TS_MINY 300
#define TS_MAXX 3800
#define TS_MAXY 3850
#define TS_MIN_PRESSURE 100
#define TSC_IRQ 9     //the Novaduino(R) TI TSC2007 interrupt pin is connected to D9

Adafruit_TSC2007 ts = Adafruit_TSC2007();  //instantiate touch screen as ts.

#define PENRADIUS 2   // used for drawing dots on the screen

//*************************************************************************************************
//***********************SDCard, CardInfo from Arduino Examples************************************
/*  created  28 Mar 2011
  by Limor Fried
  modified 9 Apr 2012
  by Tom Igoe
  SD card test

  This example shows how use the utility libraries on which the'
  SD library is based in order to get info about your SD card.
  Very useful for testing a card when you're not sure whether its working or not.
*/
// include the SD library:

#include <SD.h>

// set up variables using the SD utility library functions:

Sd2Card card;
SdVolume volume;
SdFile root;

const int chipSelect = A4;  // The microSD CS pin is on A4

//**********************initialize a simple test timer/counter**************************************
// this is not an accurate timer but it does show how to implement a non-blockinig delay

int seconds = 0;
int minutes = 0;
int hours = 0;

// the following timeInterval is checked against millis to determine when to increment the timer

const long timeInterval = 1000;
unsigned long timeA = 0; // this is a time marker



//*******10********20********30********40********50********60********70*******80********90*******100
//*************************SETUP********************************************************************
//**************************************************************************************************

void setup(void) {
  
    uint16_t boottime = millis();    // time of boot
    
//SETUP**********setup serial port******************************************************************

   Serial.begin(115200);
   //while (!Serial) delay(10);  // if this is un-commented, you must open the serial monitor or 
                                 // the program wont run


//SETUP*********initialize the Wire interface as Host***********************************************

  Wire.begin();

//SETUP********** setup LCD Backlight for NRT1000014r2 *********************************************

  pinMode(A2, OUTPUT);    // LCD backlight
  digitalWrite(A2, HIGH); // using A2 as a digital pin.
  //(change to PWM controlled brightness by Rotary Encoder in future release)


//SETUP********** setup LCD TFT Display ************************************************************

  #if defined(ST7789)
    tft.init(240, 320);    // only needed for ST7789
    tft.setRotation(1);    // rotate screen to landscape mode
  #endif
  


  
  #if defined(ILI9341)
    tft.begin();
    tft.invertDisplay(1);  // only needed for ILI9341
    tft.setRotation(3);    // rotate screen to landscape mode
  #endif



//SETUP*********** setup SPI interface *************************************************************
 
  tft.setSPISpeed(SPISPEED);
  //Serial.println("TFT Initialized");  // un-comment for de-bugging

//SETUP********look for touch controller, send status to serial port*******************************

    if (!ts.begin(0x48, &Wire)) {
    Serial.println("Couldn't find touch controller");
    while (1) delay(10);   // the program stops here if the touch screen controller is not found
    }
  pinMode(TSC_IRQ, INPUT);    // making the tsc2007 processor pin an input
  Serial.println("Found touch controller");
  
//SETUP********* measure time to boot and show on flash screen ************************************

  //tft.fillScreen(BLACK);
  boottime = millis() - boottime;  // time since boot

//SETUP**********print the flash screen with file information**************************************

  tft.fillScreen(WHITE);
  tft.setTextSize(3);
  tft.setTextColor(BLUE);
  tft.println(LABEL);  //LABEL is set to the file name at the top of this program
  tft.println(boottime, DEC);
  delay(3000);          // a little bit of time to read the flash screen

//*******10********20********30********40********50********60********70*******80********90*******100
//SETUP*******************Regular information Display Starts Here***********************************

//******************Set the overall Screen color****************************************************

  tft.fillScreen(BLUE); 

//SETUP****************print the small top label box ************************************************

  tft.fillRoundRect(5, 5, 315, 25, 8, RED);  // adds the filled and rounded rectangle
  tft.setTextWrap(false);
  tft.setCursor(10, 11);   //start text here
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.println(LABEL);
   
//**************************************************************************************************
//SETUP ***************initialize digital pin LED_BUILTIN as an output******************************

  pinMode(13, OUTPUT); // on Novaduino this controls the RGB LED

//SETUP********set up buffers for displaying Rotary Encoder and Key numbers*************************

  sprintf(Bbuffer, "%02d", 0);   // 2 places for Button string buffer
  sprintf(REbuffer, "%03d", 0);  // 3 places for Rotary Encoder string buffer

//SETUP***********set up I2C/TWI Interrupt for reading key and RE processor ************************

  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), readTWI, FALLING); //see readTWI ISR below

//SETUP****************initialize the NEOPIX RGB LED ***********************************************

  pixels.begin();

//**************************************************************************************************
//SETUP***********************SDCard Setup/ CardInfo from Arduino Examples**************************


  Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!

   pinMode(chipSelect, OUTPUT);
 
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    //while (1);  // this is commented out so that the program will still run if no card is inserted.
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  //SETUP***** print the type of card
  
  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  //SETUP**** Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    //while (1);
  }

  Serial.print("Clusters:          ");
  Serial.println(volume.clusterCount());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(volume.blocksPerCluster());

  Serial.print("Total Blocks:      ");
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  //SETUP***** print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                         // SD card blocks are always 512 bytes(2 blocks are 1KB)
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);

   //********************Print the microSD file size to TFT ****************************************

  tft.setCursor(10, 40);   //start text here
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  
  tft.print("uSD Volume size (Mb):");
  volumesize /= 1024;
  tft.println(volumesize);

  
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);
  

  
  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  //SETUP**** list all files in the card with date and size
  
   root.ls(LS_R | LS_DATE | LS_SIZE);
  


 }   
// ***********END Of SETUP**************************************************************************
//**************************************************************************************************



//**************************************************************************************************
//**********************MAIN LOOP*******************************************************************
//*******10********20********30********40********50********60********70*******80********90*******100

void loop() {

//**********cursor starting point*******************************************************************

  tft.setCursor(25, 70);   //start LCD text here
  tft.setTextSize(3);
  tft.setTextColor(GREEN,BLUE);  //be sure to include the background color

//*****a string buffer for formatting the timer display *********

  static char ClkBuf[10];
  sprintf(ClkBuf, "%02d:%02d:%02d", hours, minutes, seconds);//string print hrs, mins, sec to ClkBuf
  tft.print(ClkBuf);                                         //then print string formatted ClkBuf
 
//**********************Timer, increment when timeIntraval is exceeded******************************
// this demonstrates how to implement a non-blocking delay

 if (millis()-timeA >= timeInterval) {
      timeA = millis();
      seconds +=1;
      if (seconds >= 60)
      { seconds =0;
      minutes +=1;
      }
      if (minutes >= 60)
      { minutes =0;
      hours +=1;
      }
      if (hours >= 24)
      { hours =0;
      }

     //***************** increment the NEOPIX color ---------
     
      ColorNum += 1; // color number 0 is red, 1 is green, and 2 is blue
      if(ColorNum >= 3)
         {
          ColorNum = 0;
         }
      switch (ColorNum) {
        case 0:
        red = 128;
        green = 0;
        blue = 0;
        break;
        case 1:
        red = 0;
        green = 128;
        blue = 0;
        break;
        case 2:
        red = 0;
        green = 0;
        blue = 128;
        break;
        default:   // make the LED white if an error state happens
        red = 128;
        green = 128;
        blue = 128;
        break;
      }
      pixels.clear();
      pixels.setPixelColor(0, pixels.Color(red, green, blue)); // 0 to 255, Red,Green, Blue
      pixels.show();
 }

//*******10********20********30********40********50********60********70*******80********90*******100
//**************Send key and rotary encoder values to the LCD***************************************

  tft.setCursor(30, 90);   //start the text here
  tft.setTextSize(2);
  tft.println();
  tft.println(" TWI data from kbd proc"); 

  //*********print the button number****************************************************************
  
  tft.print(" Button #: ");
  sprintf(Bbuffer, "%02d", buttonNum);
  tft.println(Bbuffer);
  //Serial.print(Bbuffer);  //debug statement

  //**********Print the accumalated rotary encoder**************************************************
  
  tft.print(" RotryEnc: ");
  sprintf(REbuffer, "%03d", buttonTot);
  tft.println(REbuffer);
  
  //Serial.print(REbuffer);  //debug statement

  // print the color number
  tft.println();
  tft.print(" Color Number: ");
  tft.print(ColorNum); 


//*******10********20********30********40********50********60********70*******80********90*******100
//**********************from Adafruit touchpaint tsc2007 example code ******************************

#if defined(TSC_IRQ)
  if (digitalRead(TSC_IRQ)) {
    // IRQ pin is high, nothing to read!
    return;
  }
#endif
  TS_Point p = ts.getPoint();

  Serial.print("X = "); Serial.print(p.y);       // swap x and y for touchscreen orientation
  Serial.print("\tY = "); Serial.println(p.x);   // prints the raw data to the serial monitor
  Serial.print("\tPressure = "); Serial.println(p.z);
  
  if (((p.x == 0) && (p.y == 0)) || (p.z < 10)) return; // no pressure, no touch
   
    // Scale from ~0->4000 to tft.width using the calibration #'s
    p.y = map(p.y, TS_MINX, TS_MAXX, 0, tft.width());
    p.x = map(p.x, TS_MINY, TS_MAXY, 0, tft.height());

    tft.println();
    tft.print("X = "); tft.print(p.y);       //swap x and y for touchscreen
    tft.print("\tY = "); tft.println(p.x);   // orientation
    tft.print("\tPressure = "); tft.println(p.z);

    int currentcolor = RED; 
    tft.fillCircle(p.y, p.x, PENRADIUS, currentcolor); //this draws a little dot at x,y  

 
}   //********End of Main*****
//*******10********20********30********40********50********60********70*******80********90*******100


/******fUNCTIONS AND STRUCTURES********************************************************************/
//
//***********TWI read Interrupt Service Routine*****
//
void readTWI(){

    Wire.requestFrom(0x40, 4, true);    // Request 4 bytes from slave device number 0x40

    // Slave may send less than requested
        //while(Wire.available())
        for(int i=0; i<4; i++)
        {
            button[i] = Wire.read();    // Receive a byte as int8_t
            //Serial.print(button);         // debug statement
        }
        buttonTot += button[2];
        buttonNum = button[0];
        
        twiIntFlag = false;
} 
