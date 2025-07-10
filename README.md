# Demo-A_Novaduino_Display
This program runs on the Novaduino® Display PCBA available from Nova Radio Labs LLC. Novaduino® is a powerful, customizable platform for creating microcontroller-based projects. It works with a Feather-compatible processor*, a Graphic 2.4-inch Color LCD with a resistive-touch panel and optional buttons and rotary encoder.

// IDE:Arduino 1.8.19
// Date: 06/30/2025
// Author: Nova Radio Labs LLC
/*
For Software and Hardware Demonstration of the Novaduino® Display NRT1000014RevC PCBA

1. install this program onto a SparkFun SAMD51 M4+ Thing Plus or Adafruit M0 or M4 Feather Device.
    (Other Feather* form factor processors may work but have not been tested. Eg: STM32 or ESP32 or 
    or RP2040 or RP2350 etc...)
2. Install the Feather onto the NRT1000014RevC Novaduino(R) Display PCBA
    Display Device with 2.4 Inch 4DLCD-24320240-ips-rtp device installed
3. The keyboard and rotary encoder driver software is already installed on the ATtiny1626 gpio expander chip
   located on the PCBA.
5. Run the program and open the Serial Monitor within the Arduino IDE
6. This program tests the following functions:
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
