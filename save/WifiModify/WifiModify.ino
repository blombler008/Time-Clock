#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <WiFiUdp.h>



#include <ESP8266WiFi.h>
extern "C" {
  #include "user_interface.h"
}

unsigned int localPort = 8888;              
unsigned long  epoch ;

byte timeServer[] = { 192, 43, 244, 18};    
const int NTP_PACKET_SIZE= 48;              
byte packetBuffer[ NTP_PACKET_SIZE];       
WiFiUDP udp;            
IPAddress timeServerIP; 
const char* ntpServerName = "ntp1.t-online.de";

#define OLED_RESET U8G2_R0
U8G2_SH1106_128X64_VCOMH0_F_HW_I2C display(OLED_RESET);
 
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM tesotec [] = {
  
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0xc0,0xff,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x00,0x00,0x00,0xfc,0xff,0xff,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x03,0xf8,	0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xe0,0x7f,0x00,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x00,0x00,0xf8,0x07,0x00,0x00,0x3c,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0x01,0x00,0x00,	0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x7e,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x00,0x80,0x1f,0x00,0x00,0x00,0x00,0x03,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0x07,0x00,0x00,0x00,	0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,
	0x07,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x00,0xf8,0x01,0x00,0x00,0x00,0x00,0x10,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0x00,0x00,0x00,0x00,	0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,
	0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x00,0x1f,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x1f,0x00,0x00,0x00,0x00,	0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x07,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0xe0,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0x01,0x00,0x00,0x00,0x00,	0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,	0x00,0x00,0x7c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x78,0x00,0x00,0x00,0x00,0x7e,0x00,0x00,0xe0,0x01,0x00,	0x00,0x00,0x00,0x00,0x00,0x7c,0x00,0x00,0x00,0x00,0x3e,0x00,
	0x00,0xe0,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0x00,0x00,	0x00,0x00,0x1e,0x00,0x00,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x7e,0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0xf0,0x00,0x00,	0x00,0x00,0x00,0x00,0x00,0x3e,0x00,0x00,0x00,0x80,0x07,0x00,
	0x00,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x00,0x00,	0x00,0xc0,0x03,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x7f,0x00,0x3f,0x00,0xe0,0x03,0xf8,0x01,0xfe,0x03,0xfc,	0x00,0xe0,0x07,0x00,0xc0,0xff,0xc0,0xff,0x00,0xe0,0x01,0xfe,
	0x07,0xfe,0x03,0xff,0x03,0xf0,0x1f,0x00,0xc0,0xff,0xf0,0xff,	0x00,0xf0,0x81,0xff,0x07,0xff,0x83,0xff,0x03,0xfc,0x3f,0x00,
	0xc0,0x7f,0xf0,0xff,0x01,0xf0,0x80,0xff,0x0f,0xff,0xc3,0xff,	0x07,0xfe,0x3f,0x00,0x80,0x0f,0xf8,0xf8,0x01,0x78,0xc0,0xcf,
	0x0f,0x7e,0xf0,0xc3,0x07,0x3f,0x7e,0x00,0x80,0x0f,0x7c,0xf0,	0x01,0x78,0xe0,0x83,0x0f,0x3e,0xf0,0x81,0x87,0x0f,0x7e,0x00,
	0x80,0x07,0x3e,0xf0,0x01,0x3c,0xe0,0x01,0x0f,0x3e,0xf0,0x80,	0xc7,0x0f,0x3e,0x00,0x80,0x07,0x1e,0xf0,0x01,0x1e,0xf0,0x00,
	0x0f,0x3e,0xf8,0x80,0xc7,0x07,0x1c,0x00,0xc0,0x07,0xff,0xff,	0x01,0x1f,0xf8,0x80,0x0f,0x1e,0xf8,0xff,0xc7,0x07,0x00,0x00,
	0xc0,0x03,0xff,0xff,0x00,0x0f,0x78,0x80,0x0f,0x1e,0xf8,0xff,	0xc3,0x03,0x00,0x00,0xc0,0x03,0xff,0x7f,0x80,0x0f,0x78,0x80,
	0x07,0x1f,0xfc,0xff,0xe1,0x03,0x00,0x00,0xe0,0x83,0x0f,0x00,	0x80,0x0f,0x7c,0xc0,0x07,0x0f,0x3c,0x00,0xe0,0x03,0x04,0x00,
	0xe0,0x81,0x0f,0x00,0xc0,0x07,0x7c,0xc0,0x07,0x0f,0x3c,0x00,	0xe0,0x01,0x0e,0x00,0xe0,0x81,0x0f,0x70,0xc0,0x03,0x7c,0xe0,
	0x87,0x0f,0x3c,0xc0,0xe0,0x01,0x0f,0x00,0xf0,0x81,0x0f,0x7c,	0xc0,0x03,0x7c,0xe0,0x83,0x07,0x3c,0xe0,0xe0,0x83,0x0f,0x00,
	0xf0,0x89,0x0f,0x3e,0xe0,0x01,0xfc,0xf0,0x81,0x4f,0x7c,0xf8,	0xe0,0xe7,0x07,0x00,0xf0,0x1f,0xff,0x1f,0xe0,0x00,0xf8,0xff,
	0xc0,0x7f,0xfc,0xff,0xe0,0xff,0x03,0x00,0xf0,0x1f,0xff,0x0f,	0xf0,0x00,0xf8,0x7f,0xc0,0x7f,0xf8,0x7f,0xc0,0xff,0x01,0x00,
	0xf0,0x0f,0xfe,0x07,0x78,0x00,0xf0,0x1f,0x80,0x7f,0xf0,0x1f,	0xc0,0xff,0x00,0x00,0xe0,0x07,0xf8,0x01,0x78,0x00,0xc0,0x0f,
	0x00,0x1f,0xe0,0x07,0x80,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,	0x3c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x3e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x07,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,	0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0xe0,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x78,	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01,0x00,0x3c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x80,0x07,	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x10,0xf0,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x00,0x00,0x00,0xc0,0xff,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	0x00,0x00,0x00,0x00
};
os_timer_t  TimerOneSecond ;
void timerCallback(void *pArg) {

      epoch += 1 ;

} 
const int ledPin =  2;      

int ledState = LOW;            

const char* ssid = "myBox";
const char* password = "tesotec2015";
WiFiServer server(80);


boolean waitingDHCP=false;
char last_mac[18];


// Manage incoming device connection on ESP access point
void onNewStation(WiFiEventSoftAPModeStationConnected sta_info) {
  Serial.println("New Station :");
  sprintf(last_mac,"%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(sta_info.mac));
  Serial.printf("MAC address : %s\n",last_mac);
  Serial.printf("Id : %d\n", sta_info.aid);
  waitingDHCP=true;
}

void setup() {

  os_timer_setfn(&TimerOneSecond, timerCallback, NULL);
  os_timer_arm(&TimerOneSecond, 1000, true);
  u8g2_SetI2CAddress(&display, 0x3C);
  display.begin();  
  display.clearDisplay(); 
  display.setColorIndex(1);


 // display.drawStr(0, 10," Hallo World "  );
  delay( 5000 ) ;
  display.clearBuffer();
  display.drawXBMP(0, 0, 128, 64, tesotec);
  display.sendBuffer();
  delay( 2000 );
  display.clearBuffer();
  display.setFont(u8g2_font_profont11_tr );
  static WiFiEventHandler e1;
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH );
  
  Serial.begin(57600);
  Serial.print("\n\n Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  digitalWrite(ledPin, LOW );
  server.begin();
  Serial.println("Server started");


//  strLineOut = "http://%s" + WiFi.localIP() ;
  //sprintf( strLineOut,"http://%s", (String)WiFi.localIP().c_str() );
  Serial.print("control through web-browser : http://");
  Serial.print( WiFi.localIP() );
  Serial.println("/gpio/0  or /1 \n");

  //display.drawStr( 8," http:// " + WiFi.localIP().toString() );

    // Event subscription
  e1 = WiFi.onSoftAPModeStationConnected(onNewStation);
  
  display.drawStr(0,8 /* 16 */, "Starting UDP" ) ;
  udp.begin(localPort);
  //display.drawStr(0,24,"Local port: " + udp.localPort().toString() );
  delay( 1000 );
  display.clearBuffer();
}

void loop() {

  int iReceiveCNT ;
  char chHour, chMinute, chSeconds, strTime[30] ;
  unsigned long highWord, lowWord, secsSince1900 ;
  const unsigned long seventyYears = 2208988800UL;    
    if ( !(epoch % 60) || ( epoch < 100 ) ) {

      WiFi.hostByName( ntpServerName, timeServerIP);  
      sendNTPpacket( timeServerIP );                
  
      delay(700);                                 
      iReceiveCNT = udp.parsePacket();
      if (!iReceiveCNT) {
        Serial.print(".");
      }
      else {
        Serial.println( "packet received, length = " + (String)iReceiveCNT);
        udp.read(packetBuffer, NTP_PACKET_SIZE); 
        highWord = word(packetBuffer[40], packetBuffer[41]);
        lowWord = word(packetBuffer[42], packetBuffer[43]);
        secsSince1900 = highWord << 16 | lowWord;

        Serial.println( "Seconds since Jan 1 1900 = " + (String)secsSince1900);
        epoch = secsSince1900 - seventyYears;    // subtract seventy years:
        Serial.println( "Unix time = " + (String)epoch);
        chHour    = (epoch % 86400L) / 3600 ;    
        chMinute  = (epoch % 3600) / 60 ;         
        sprintf(strTime,"%02i:%02i", chHour+1, chMinute) ;        // print the hour, minute and second:
        if( iReceiveCNT ) Serial.println("The UTC time is " + (String)strTime );       // UTC is the time at Greenwich Meridian (GMT)
        display.clearBuffer(); 
        display.drawStr(0, 0, "tesotec ATE solutions" ) ;
        display.drawStr(0, 28, "    |           |" ) ;

      }

    }
    chSeconds = epoch % 60 ;                  //  calculate the seconds

    sprintf(strTime," %02i:%02i:%02i ", chHour+1, chMinute, chSeconds) ;    // print the hour, minute and second:
    display.drawStr(0, 10, strTime );
    Serial.print(".");
    display.drawPixel( chSeconds + 32, 31 );      //  print pixel every second
    display.sendBuffer();

return;    
    WiFiClient client = server.available();
    if (!client) {
      return;
    }
    uint8_t macAddr[6];
    WiFi.softAPmacAddress(macAddr);
    Serial.println("new client");
    if (!client.available())  return;
  String req = client.readStringUntil('\r');
  Serial.println(req);
Serial.println("-------------Connected Clients List-----------------------");
Serial.print(wifi_softap_get_station_num()); Serial.println(" clients.");

struct station_info *station_list = wifi_softap_get_station_info();
while (station_list != NULL) {
  char station_mac[18] = {0}; sprintf(station_mac, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(station_list->bssid));
  String station_ip = IPAddress((&station_list->ip)->addr).toString();
  Serial.print(station_mac); Serial.print(" "); Serial.println(station_ip);
  station_list = STAILQ_NEXT(station_list, next);
}
wifi_softap_free_station_info();
Serial.println();
  client.flush();
  display.clearBuffer();
  display.setCursor(33,16);
  int val;
  if (req.indexOf("/gpio/0") != -1) {
    val = 0;
    display.print("  /gpio/0 ");
  }
  else if (req.indexOf("/gpio/1") != -1) {
    val = 1;
    display.print("  /gpio/1 ");
  }
  else {
    Serial.println("invalid request");
    display.print("!! invalid");
    display.sendBuffer();
    client.stop();
    return;
  }
  display.sendBuffer();
  // Set GPIO2 according to the request
  digitalWrite(ledPin, val);
  client.flush();
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
  s += (val)?"high":"low";
  s += "</html>\n";
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");
}
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;            // Stratum, or type of clock
  packetBuffer[2] = 6;            // Polling Interval
  packetBuffer[3] = 0xEC;         // Peer Clock Precision
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(address, 123);  //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
