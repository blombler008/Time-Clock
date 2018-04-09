#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <WiFiUdp.h>
 
#include <ESP8266WiFi.h>
extern "C" {
  #include "user_interface.h"
}

#include "Webkeys.h"

byte PCF8574D = 0x20;
/* 
	0x20 - 0x27 are the PCF8574D IDS remains
	to the analog pins connectet to ground
	in this case its 0x20 so all 3 analog pins to ground
*/
int LEDRotBig = 0;																	// -------0 -- Pin0
int LEDRot1 = 1;																	// ------0- -- Pin1
int LEDRot2 = 2;																	// -----0-- -- Pin2
int LEDBlau = 3;																	// ----0--- -- Pin3
int LEDGruenBig = 4;																// ---0---- -- Pin4
int LEDGruen2 = 5;																	// --0----- -- Pin5
int LEDGruen1 = 6;																	// -0------ -- Pin6
int LEDOrange = 7;																	// 0------- -- Pin7

int baudRateBegin = baudRate;														// sets the baudrate for the serial monitor
unsigned long  epoch;																// initialise timer secounds
byte timeServer[] = { 192, 43, 244, 18};    										// initialise local tiemr server ip
const int NTP_PACKET_SIZE= 48;              										// initialise max length of package
byte packetBuffer[ NTP_PACKET_SIZE];   												// initialise package size
IPAddress timeServerIP; 															// initialise timer server ip
const char* ntpServerName = "ntp1.t-online.de";										// timer server adress
WiFiUDP udp;   																		// initialise wifi mode
#define OLED_RESET U8G2_R0															// define the display rotation
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(OLED_RESET);								// initialise display mode
static const uint8_t tesotec [] PROGMEM = {											// logo in hex code
    
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
os_timer_t  TimerOneSecond ;														// initialise timer
void timerCallback(void *pArg) {													// timer function to count the secounds time up
    epoch += 1;																		// counting secounds up
} 
const char* ssid = webSSID;															// import the ssid of the network form the custom lib
const char* password = webPawd;														// import the password from the custom lib
boolean waitingDHCP=false;															/* self explaining */
void setup() {
    os_timer_setfn(&TimerOneSecond, timerCallback, NULL);							// set the timer to a function if time is reached
    os_timer_arm(&TimerOneSecond, 1000, true);										// start the timer above
    display.begin();  																// initialise the display
    u8g2_SetI2CAddress(&display, 0x3C);												// sets the I2C adress of the display
    display.setColorIndex(1);														// set color mode on the display to 1 (white)
    display.drawXBMP(0, 0, 128, 64, tesotec);										// initialise the logo into the buffer
    display.sendBuffer();															// sends the tesotec logo to the screen 
    delay( 2000 );																	// waiting 2 secounds
    display.setFont(u8g2_font_5x8_tn);												// small font
    Wire.begin();																	// starts the wire mode for the PCF8574D
    Serial.begin(baudRateBegin);													// starts the serial monitor port for debugging
    Serial.print("\n\n Connecting to ");											/* self explaining */
    Serial.println(ssid);															/* self explaining */

    WiFi.mode(WIFI_AP_STA);															// Set wifi mode to WIFI_AP_STA
    WiFi.begin(ssid, password);														// connect to wifi 
    
    while (WiFi.status() != WL_CONNECTED) {											// waiting for connection estabilaized
		delay(500);																	/* self explaining */
		Serial.print(".");															// identifier for the connection time
    }
	Serial.println("\nWiFi connected");												/* self explaining */				
	display.clearDisplay();															/* self explaining */
	udp.begin(8888);																// starts the utp connection for the time server
	Serial.println("UDP started");													/* self explaining */
}

int switchLight = 0;
void loop() {
	if (switchLight == 1){															/* self explaining */
		Wire.beginTransmission(PCF8574D); 											// begin talking with PCF8574D
		Wire.write(B11110111); 														// LEDs ON
		Wire.endTransmission();														// end talking with PCF8574D
	} else {																		/* self explaining */
		Wire.beginTransmission(PCF8574D); 											// begin talking with PCF8574D
		Wire.write(B11111111); 														// LEDs OFF
		Wire.endTransmission();														// end talking with PCF8574D
	}
	int iReceiveCNT;																/* initialisate the package */
	char chHour, chMinute, chSeconds, strTime[30];									/* initialisations of time numbers */
	unsigned long highWord, lowWord, secsSince1900;									/* initialisations of time strings */
	const unsigned long seventyYears = 2208988800UL;								/* initialisations of 1970 */
	if ( !(epoch % 60) || ( epoch < 100 ) ) {										// waits for the timer to get 60 secounds
		WiFi.hostByName( ntpServerName, timeServerIP);								// getting the server ip adress
		sendNTPpacket( timeServerIP );												// send the package to server to let it response
		delay(700);																	// waiting 700 milisecounds
		iReceiveCNT = udp.parsePacket();											// trying to get the server response
		if (!iReceiveCNT) {															// waiting for the server to response success
			switchLight = 0;														// turns the led off
			Serial.print("A");														// Prints "A" as long as nothing was send yet
		} else {																	/* self explaining */
			switchLight = 1;														// turns the led on
			display.clearBuffer(); 													// clears the screen without removeing the pixels
			Serial.println( "packet received, length = " + (String)iReceiveCNT);	/* self explaining */
			udp.read(packetBuffer, NTP_PACKET_SIZE); 								// reads the package ... there the time comes from server
			highWord = word(packetBuffer[40], packetBuffer[41]);					// get the high word from the package informations
			lowWord = word(packetBuffer[42], packetBuffer[43]);						// get the low word from the package informations
			secsSince1900 = highWord << 16 | lowWord;								// calculate the time since 1900 1st jan
			Serial.println( "Seconds since Jan 1 1900 = " + (String)secsSince1900);	/* self explaining */
			epoch = secsSince1900 - seventyYears;    								// subtract seventy years:
			Serial.println( "Unix time = " + (String)epoch);						/* self explaining */
			chHour    = (epoch % 86400L) / 3600;									// current hour
			chMinute  = (epoch % 3600) / 60;										// current minute
			sprintf(strTime,"%02i:%02i", chHour+2, chMinute) ;       	 			// print the hour, minute and second:
			Serial.println("The UTC time is " + (String)strTime );       			// UTC is the time at Greenwich Meridian (GMT)
			display.clearBuffer();													// clears the screen without removeing the pixels
			display.drawStr(0, 8, "tesotec ATE solutions" ) ;						// emblem
			display.drawStr(0, 63, "|" );											// begin line of the secounds bar
			display.drawStr(122, 63, "|" );											// End linee of the secounds bar
		}
	}
	display.clearBuffer();															// clears the screen without removeing the pixels
	chSeconds = epoch % 60 ;														// get the secounds of the current time
	display.setFont(u8g2_font_profont29_tf );										// larger font
	sprintf(strTime,"%02i:%02i:%02i", chHour+2 , chMinute, chSeconds) ;    			// print the hour, minute and second:
	display.drawStr(0, 35, strTime );												// prints the time ( updated)
	display.setFont(u8g2_font_profont11_tr);										// small font size
	display.drawStr(0, 8, "tesotec ATE solutions" ) ;								// emblem
	display.drawStr(0, 63, "|" );													// begin line of the secounds bar
	display.drawStr(122, 63, "|" );													// End linee of the secounds bar
	display.drawBox( 4, 58, (chSeconds*2) + 1, 5 );									// bar Loading ... counting secounds and print 2 stripes to the bottom line of the screen
	display.sendBuffer();															// sends to the screen whats written in the lines above
}
unsigned long sendNTPpacket(IPAddress& address){									// create the package function
	Serial.println("..sending NTP packet...");										/* self explaining */
	memset(packetBuffer, 0, NTP_PACKET_SIZE);										// sets a hash to send a requirement to the time server
	packetBuffer[0] = 0b11100011;   												// LI, Version, Mode
	packetBuffer[1] = 0;            												// Stratum, or type of clock
	packetBuffer[2] = 6;            												// Polling Interval
	packetBuffer[3] = 0xEC;         												// Peer Clock Precision
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;
	udp.beginPacket(address, 123); 													// NTP requests are to port 123
	udp.write(packetBuffer, NTP_PACKET_SIZE);										// writes the max size to the package
	udp.endPacket();																// end package protocol
}