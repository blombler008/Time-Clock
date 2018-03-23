#include <U8g2lib.h>
/* #include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <WiFiUdp.h>
 
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
} */
#define OLED_RESET U8G2_R0
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(OLED_RESET);
void setup(void){
  display.begin();  
  u8g2_SetI2CAddress(&display, 0x3C);
  display.drawBox(0,0,127,64);
}
void loop(void){  
    //display.drawStr("hello");
    //display.draw
}