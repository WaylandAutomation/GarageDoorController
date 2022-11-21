#include <NTPClient_Generic.h>
#include <WiFiUdp.h>

// Code generated by Arduino IoT Cloud, DO NOT EDIT.

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>


const char SSID[]     = SECRET_SSID;    // Network SSID (name)
const char PASS[]     = SECRET_PASS;    // Network password (use for WPA, or use as key for WEP)

void onHouse1Change();
void onHouse2Change();
void onMiddleChange();
void onNorthChange();
void onSouthChange();

String temperature;
 bool house1;
 bool house2;
 bool lpos;
 bool middle;
 bool mPos;
 bool north;
 bool nPos;
 bool rpos;
 bool south;
 bool sPos;


void initProperties(){

  ArduinoCloud.addProperty(temperature, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(house1, READWRITE, ON_CHANGE, onHouse1Change);
  ArduinoCloud.addProperty(house2, READWRITE, ON_CHANGE, onHouse2Change);
  ArduinoCloud.addProperty(lpos, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(middle, READWRITE, ON_CHANGE, onMiddleChange, 1);
  ArduinoCloud.addProperty(mPos, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(north, READWRITE, ON_CHANGE, onNorthChange);
  ArduinoCloud.addProperty(nPos, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(rpos, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(south, READWRITE, ON_CHANGE, onSouthChange);
  ArduinoCloud.addProperty(sPos, READ, ON_CHANGE, NULL);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);