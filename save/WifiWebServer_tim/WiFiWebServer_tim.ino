/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>           //  OLED Display lib
#include <Adafruit_SSD1306.h>       //  OLED Display lib
#include <WiFiUdp.h>



#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>
extern "C" {
  #include "user_interface.h"
}


// ------------------------------------------------------------------------
// ------------------------   NTP Time server definitions 
// ------   working sample :    https://www.geekstips.com/arduino-time-sync-ntp-server-esp8266-udp/

//int stunde, minute, sekunde;

unsigned int localPort = 8888;              // local port to listen for UDP packets
unsigned long  epoch ;

byte timeServer[] = { 192, 43, 244, 18};    // time.nist.gov NTP server
const int NTP_PACKET_SIZE= 48;              // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];        // buffer to hold incoming and outgoing packets

WiFiUDP udp;                                // A UDP instance to let us send and receive packets over UDP

//  ------   Don't hardwire the IP address or we won't get the benefits of the pool.
//  ------   Lookup the IP address for the host name instead *

IPAddress timeServerIP; // time.nist.gov NTP server address
//const char* ntpServerName = "time.nist.gov";
//const char* ntpServerName = "de.pool.ntp.org";
const char* ntpServerName = "ntp1.t-online.de";



// ------------------------------------------------------------------------
// ------------------------  OLED Adafruit_SSD1306 :  definition  

// SCL GPIO5    D1 @ WeMos mini
// SDA GPIO4    D2 @ WeMos mini 
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
 
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
 
 
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

// ------------------------   tesotec logo .bmp  

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




// ------------------------------------------------------------------------
// ------------------------   Timer Interrupt setup 
// ___            http://bienonline.magix.net/public/esp8266-timing.html

os_timer_t  TimerOneSecond ;

// start of timerCallback
void timerCallback(void *pArg) {

      epoch += 1 ;

} // End of timerCallback




  





// ------------------------------------------------------------------------
// ------------------------   WLAN :  initialization 

// constants won't change. Used here to set a pin number :
const int ledPin =  2;      // the number of the LED pin

// Variables will change :
int ledState = LOW;             // ledState used to set the LED

const char* ssid = "myBox";
const char* password = "tesotec2015";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);


/*
void wifiEventHandler(System_Event_t *event) {
  switch (event->event) {
    case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
    case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED: {
      char mac[32] = {0};
      snprintf(mac, 32, MACSTR ", aid: %d" , MAC2STR(event->event_info.sta_connected.mac), event->event_info.sta_connected.aid);
      Serial.println(mac);
    }
    break;
  }
}
*/




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


void  PrintOLED_Line( short int iLineNum, String strOutput ) {

  display.setTextColor( WHITE, BLACK );
  
//  display.setCursor( 33, iLineNum );    //  Small Display offset
  display.setCursor( 0, iLineNum );    //  128x32 Display without offset
  display.println(strOutput);
  display.display();
  
}





void setup() {

    os_timer_setfn(&TimerOneSecond, timerCallback, NULL);
    os_timer_arm(&TimerOneSecond, 1000, true);


 // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
//  display.begin(SSD1306_SWITCHCAPVCC, 0x78);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.clearDisplay();   // Clear the buffer
 
  // text display tests
  display.setTextSize(1);
  display.setTextColor( WHITE, BLACK );


  PrintOLED_Line( 10," Hallo World "  );
delay( 5000 ) ;

 // display.drawBitmap(0, 0, tesotec, 128, 32, WHITE);

  display.clearDisplay();
  display.drawXBitmap(0, 0, tesotec, 128, 64, WHITE);
  display.display();
  delay( 2000 );
  display.clearDisplay();

      /* display.drawPixel(0,0,WHITE );
      display.display();
      delay( 1 ) ;
      
      display.drawPixel(127,31,WHITE );
      display.display();
      delay( 1 ) ;
     delay( 1000 ) ;
 */


/*      for display 128x32     
  for (int iYPos =  0; iYPos < 32; iYPos+=5 ) {
    for (int iXPos = 0; iXPos < 128; iXPos+=5 ) {
      display.drawPixel(iXPos,iYPos,WHITE );
      display.display();
      delay( 1 ) ;
    }
  }
*/
  

  /*
  for (int iYPos =  10; iYPos < 34; iYPos+=10 ) {
    for (int iXPos = 35; iXPos < 140; iXPos+=10 ) {
      display.drawPixel(iXPos,iYPos,WHITE );
      display.display();
      delay( 1 ) ;
    }
  }
  for (int iYPos =  10; iYPos < 34; iYPos+=10 ) {
    for (int iXPos = 35; iXPos < 140; iXPos+=10 ) {
      display.drawPixel(iXPos,iYPos,BLACK );
      display.display();
      delay( 1 ) ;
    }
  }  */



  static WiFiEventHandler e1;
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH );
  
  Serial.begin(57600);
  
  // Connect to WiFi network
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


  
    // Start the server
  server.begin();
  Serial.println("Server started");


//  strLineOut = "http://%s" + WiFi.localIP() ;
  //sprintf( strLineOut,"http://%s", (String)WiFi.localIP().c_str() );
  
    // Print the IP address
  Serial.print("control through web-browser : http://");
  Serial.print( WiFi.localIP() );
  Serial.println("/gpio/0  or /1 \n");

  display.setTextSize(1);

  PrintOLED_Line( 8," http:// " + (String)WiFi.localIP() );

    // Event subscription
  e1 = WiFi.onSoftAPModeStationConnected(onNewStation);
  
  PrintOLED_Line( 16, "Starting UDP" ) ;
  udp.begin(localPort);
  PrintOLED_Line(24,"Local port: " + (String)udp.localPort() );
  delay( 1000 );
  display.clearDisplay();
}

void loop() {

  int iReceiveCNT ;
  char chHour, chMinute, chSeconds, strTime[30] ;
  unsigned long highWord, lowWord, secsSince1900 ;
  const unsigned long seventyYears = 2208988800UL;    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800


    if ( !(epoch % 60) || ( epoch < 100 ) ) {

      WiFi.hostByName( ntpServerName, timeServerIP);   //  get IP from  time server
      sendNTPpacket( timeServerIP );                   //  send an NTP packet to a time server
  
      delay(700);                                      //  wait to see if a reply is available
  
      iReceiveCNT = udp.parsePacket();
      if (!iReceiveCNT) {
        Serial.print(".");
      }
      else {

        Serial.println( "packet received, length = " + (String)iReceiveCNT);
          // We've received a packet, read the data from it
        udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

          //the timestamp starts at byte 40 of the received packet and is four bytes,
          // or two words, long. First, esxtract the two words:

        highWord = word(packetBuffer[40], packetBuffer[41]);
        lowWord = word(packetBuffer[42], packetBuffer[43]);
          // combine the four bytes (two words) into a long integer
          // this is NTP time (seconds since Jan 1 1900):
        secsSince1900 = highWord << 16 | lowWord;

        Serial.println( "Seconds since Jan 1 1900 = " + (String)secsSince1900);
        epoch = secsSince1900 - seventyYears;    // subtract seventy years:
        Serial.println( "Unix time = " + (String)epoch);

        chHour    = (epoch % 86400L) / 3600 ;     //  calculate the hours   (86400 equals secs per day)
        chMinute  = (epoch % 3600) / 60 ;         //  calculate the minutes (3600 equals secs per minute)

                      //  hour + 1 :  NTP time is green which time always.
//        sprintf(strTime,"%02i:%02i:%02i", chHour+1, chMinute, chSeconds) ;    // print the hour, minute and second:
        sprintf(strTime,"%02i:%02i", chHour+1, chMinute) ;        // print the hour, minute and second:

        if( iReceiveCNT ) Serial.println("The UTC time is " + (String)strTime );       // UTC is the time at Greenwich Meridian (GMT)
         
        display.clearDisplay();   // Clear the buffer
        display.setTextSize(1);
        PrintOLED_Line( 0, "tesotec ATE solutions" ) ;
        PrintOLED_Line( 28, "    |           |" ) ;

      }

    }   //  end if if ( !(epoch % 5) )


    chSeconds = epoch % 60 ;                  //  calculate the seconds

    sprintf(strTime," %02i:%02i:%02i ", chHour+1, chMinute, chSeconds) ;    // print the hour, minute and second:
    display.setTextSize(2);
    PrintOLED_Line( 10, strTime );

    Serial.print(".");

    display.drawPixel( chSeconds + 32, 31, WHITE );      //  print pixel every second
    display.display();
  
//    delay(2000) ;
//    display.invertDisplay( true );
//    delay(2000) ;
//    display.invertDisplay( false );



return;

    // ------------------------------------------------------------------------------------------------------------------------------------------
    // ------------------------   Server routines 

      // Check if a client has connected      
    WiFiClient client = server.available();
  
    if (!client) {
      return;
    }

    uint8_t macAddr[6];
    WiFi.softAPmacAddress(macAddr);

//    Serial.printf("own MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);


  
      // Wait until the client sends some data
    Serial.println("new client");
    if (!client.available())  return;


    
//    while(!client.available()){
//      delay(1);
//    }



      
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);





// ---------------------------------------------------------------------
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
// ---------------------------------------------------------------------



  
  client.flush();
  
  // Match the request

  display.clearDisplay();
  display.setCursor(33,16);
  int val;
  if (req.indexOf("/gpio/0") != -1) {
    val = 0;
    display.println("  /gpio/0 ");
  }
  else if (req.indexOf("/gpio/1") != -1) {
    val = 1;
    display.println("  /gpio/1 ");
  }
  else {
    Serial.println("invalid request");
    display.println("!! invalid");
    display.display();

    
    client.stop();
    return;

  }
  display.display();
  

  // Set GPIO2 according to the request
  digitalWrite(ledPin, val);
  
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
  s += (val)?"high":"low";
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed




  
}




// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;            // Stratum, or type of clock
  packetBuffer[2] = 6;            // Polling Interval
  packetBuffer[3] = 0xEC;         // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123);  //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

