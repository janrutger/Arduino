#include <WiFi.h>
#include "time.h"

//wifi
const char* ssid       = "H369A6E844A";
const char* password   = "AC57CC39CF5D";

//time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


//----------------------------------------------------------


void printHello(){
  Serial.println("Hello World, Iam ESP32-S2 Thing Plus");
}


void WifiOn(){
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
}


void WifiOff(){
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("DISCONNECTED");
}

void SetTime(){
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time set");
  PrintTime();
}

void PrintTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
