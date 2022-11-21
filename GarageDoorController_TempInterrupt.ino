#include <Adafruit_Sensor.h>
//#include <DHT.h>			Code is included for a future temperature sensor, not currently installed
//#include <DHT_U.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <WiFiNINA.h>
#include "pgmspace.h"
#include <splash.h>
#include "thingProperties.h"


#define led1 LEDR
#define led2 LEDB
#define led3 LEDG
#define houseOnePin 15
#define houseTwoPin 16
#define leftPosit 8	//Hall effect Position sensors
#define rightPosit 7

//#define DHTPIN 9     // Digital pin connected to the DHT sensor 
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)

#define adj (-5*3600)	//UTC offset for NTP time. -5 is the UTC offset for the US East Coast in winter, -4 in summer.

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", adj);

//DHT_Unified dht(DHTPIN, DHTTYPE);


int dts = 20;
char status;
int i=0;
//int j=0;
time_t t;
unsigned long update = (20*60*60);
uint32_t delayMS;
String temp;
String watch;
String currently;



void Words(String message) {  //Prints 'message' to the screen, holds for 1 sec, clears screen
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 10);
  display.println(message);
  display.display();      // Show initial text
  delay(1000);
  display.clearDisplay();
  display.display();
  }
 


void rlylrinitialize() {	//Set up the LoRa txcvr. It will only run if the txcvr is not set up properly.
  String lora_setup;
  Serial1.write("AT+ADDRESS?\r\n");
    lora_setup = Serial1.readString();
    Words(lora_setup);
    if (lora_setup.indexOf("1") < 1 || lora_setup.indexOf("ERR")>1) {
      Serial1.write("AT+ADDRESS=1\r\n");
      delay(200);
      Serial1.write("AT+NETWORKID=2\r\n");
      delay(200);
      Serial1.write("AT+BAND=912000000\r\n");
      delay(200);
      Serial1.write("AT+MODE=0\r\n");
      delay(200);
      Serial1.write("AT+PARAMETER=10,7,1,7\r\n");
      delay(200);
      Serial1.write("AT+IPR=115200\r\n");
      delay(200);
    }

  Words("LoRa");
  }


void Pixels() {		//Set up the display for output
  display.begin();
  delay(1000);

  display.display();
  delay(1000); // Pause for 1 seconds

  // Clear the buffer
  display.clearDisplay();
  display.display();
  }

void FastWords(String message) {	//print 'message' to the screen, hold until next update, no program delay
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 10);
  display.println(message);
  display.display();
  //delay(200);
  }

void Temp() {		//return temperature in Celsius, currently not used, for future use
  sensors_event_t event;
  if (isnan(event.temperature)) {
    FastWords("T ERR");
    temp = "T ERR";
    }
  else {
    temp = (String(event.temperature)+" C");
    }
  }  


void setup() {
  
  pinMode(houseOnePin, OUTPUT);
  pinMode(houseTwoPin, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(leftPosit, INPUT);
  pinMode(rightPosit, INPUT);

  digitalWrite(houseOnePin, LOW);
  digitalWrite(houseTwoPin, LOW);
  digitalWrite(13, LOW);
  
  Pixels();	//Init display

  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  delay(1500); 


  Serial1.begin(115200);
    while (!Serial1) {
      Words("Serial 1");
    }
  

  
 rlylrinitialize(); //This function sets up the LORA module and only needs to be run once
/* the function checks if the device address has been configured, but no other settings. If you're re-using a LoRa board
    it's best to manually set it up using a serial passthrough program or comment out the auto config check
  */

  
  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  delay(2000);
      //ArduinoIoTPreferredConnection.addCallback(NetworkConnectionEvent::CONNECTED, onNetworkConnect);


    // Defined in thingProperties.h
  initProperties();

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  //dht.begin();

  timeClient.begin();		//Start the NTP internet time capture
  timeClient.setUpdateInterval(update);


}

void loop() { 
static int clockWatcher = millis(); 
if (millis()-clockWatcher > 999) {
		Currently(); 	//format the time String
		clockWatcher = millis();
		}
  ArduinoCloud.update(); 	//Check for input from the cloud dashboard
  house1=0;
  house2=0;
  middle=0;
  north=0;
  south=0;
  
FastWords(currently);

/*The left and right doors are in the same garage as the cloud MCU and the position sensors are wired directly to it. The North, Middle, South doors are
	in another building and communicate with the cloud board via a 933mHz LoRa connection*/
if (leftPosit == HIGH) {lpos = 1;}
  else {lpos = 0;}

if (rightPosit == HIGH) {rpos = 1;}
  else {rpos = 0;}

if (Serial1.available()) {	//The LoRa txcvr is connected to the hard serial pins accessed through Serial1.
String feedback = Serial1.readString();
  if (feedback.indexOf("SOUTHUP")>0) {sPos= 1;}
    else {sPos = 0;}
  if (feedback.indexOf("MIDDLEUP")>0) {mPos= 1;}
    else {mPos = 0;}
  if (feedback.indexOf("NORTHUP")>0) {nPos= 1;}
    else {nPos = 0;}
    
  }  
}

void Currently() {  		//Get the time from an NTP server, format a string for display
    timeClient.update();

String h, m, s;
if (timeClient.getHours()<10) {h = "0"+String(timeClient.getHours());}
  else {h = String(timeClient.getHours());}
if (timeClient.getMinutes()<10) {m = "0"+String(timeClient.getMinutes());}
  else {m = String(timeClient.getMinutes());}
if (timeClient.getSeconds()<10) {s = "0"+String(timeClient.getSeconds());}
  else {s = String(timeClient.getSeconds());}
currently = (h+":"+m+":"+s);
//Serial.println(currently);

//i++;
  
/*if (i > 29) {
  Temp();
  Serial.println(temp);  
  i = 0;
  temperature = temp;  
  } */
}
//******************************************** Cloud Input Routines ***********************************************
void onHouse1Change()  {
  
  if (house1==1) { 		//Fire transistor for  the left house door, print status msg on display
      //Serial.println("Left");
      digitalWrite(houseOnePin, HIGH);
      delay(1000);
      digitalWrite(houseOnePin, LOW);
      Words("left");
  }
    
}

void onHouse2Change()  {		//Fire transistor for  the right house door, print status msg on display
  if (house2==1) {
     // Serial.println("Right");
      digitalWrite(houseTwoPin, HIGH);
      delay(1000);
      digitalWrite(houseTwoPin, LOW);
      Words("Right");
  }
  
}
void onMiddleChange()  {		//Send LoRa command to the middle door, print status msg on display
  if (middle==1) {
      Serial1.print("AT+SEND=2,6,MIDDLE\r\n");
      Serial.println(Serial1.readString());
      //Serial.println("Middle");
      delay(300);
      Words("Middle");
  }
  
}

void onNorthChange()  {			//Send LoRa command to the north door, print status msg on display
  if (north==1) {
      Serial1.print("AT+SEND=2,5,NORTH\r\n");
      Serial.println(Serial1.readString());
      //Serial.println("North");
      delay(300);
      Words("North");
  }
  
}

void onSouthChange()  {		//Send LoRa command to the south door, print status msg on display
  if (south==1) {
      Serial1.print("AT+SEND=2,5,SOUTH\r\n");
      Serial.println(Serial1.readString());
     // Serial.println("South");
      delay(300);
      Words("South");
  }
  
}
