#include <Arduino.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "mbedtls/md.h"
#include <ezTime.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include <LITTLEFS.h> //LittleFS.h when using 2.0.0 of the esp-core

//Sleep modus and wait/delay
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
//#define TIME_TO_SLEEP  60*4        /* Time ESP32 will go to sleep for minutes (in seconds) */
#define TIME_TO_SLEEP  60*1

unsigned long startMillis;  
unsigned long currentMillis;
const unsigned long period = 60000;

//wifi
const char* ssid       = "H369A6E844A";
const char* password   = "AC57CC39CF5D";

//time
Timezone MyTZ;

//SHA hashing
byte hmacResult[32];
mbedtls_md_context_t ctx;
mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;


////sensor Sample and Value variable
struct samples{
  float TempTotal;
  float HumTotal;
  float PresTotal;
  float ResTotal;
  float IaqTotal;
  float Co2Total;
  float VocTotal;
  int   SampleCount;
};
samples Samples;


struct value{
  String SampleId;
  String StationId;
  String parameter;
  String TimeAt;
  String TimeFor;
  String unit;
  String symbol;
  float  value;
};

value Value;

const int maxValues = 72; 
value Values[maxValues];
int Index = 0;
int BuffSize = 0;


//filesysem FS and Webserver
#define FORMAT_LITTLEFS_IF_FAILED false
AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  printHello();

  

 //init and get the time, Wifi
  startMillis = millis();
  
  WifiOn();
  setDebug(INFO);
  waitForSync();
  MyTZ.setLocation(F("Europe/Amsterdam"));
  printLocalTime();
  //WifiOff();


  BSECsetup(); //See BSECsensor.ino

  
  ReadSample();   //to prefent first false reading
  Samples.TempTotal = 0;
  Samples.HumTotal = 0;
  Samples.PresTotal = 0;
  Samples.ResTotal = 0;
  Samples.IaqTotal = 0;
  Samples.Co2Total = 0;
  Samples.VocTotal = 0;
  Samples.SampleCount = 0;


//Storage
  if(!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    } else {
      Serial.print("LittleFS Mounted, ");
    }
  if(LITTLEFS.exists("/index.html")) {
    Serial.println("INDEX file found");
//    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//      request->send(200, "text/plain", "Aleks, Hello from the ESP32 world");
//    });
    server.serveStatic("/", LITTLEFS, "/").setDefaultFile("index.html");
    
    server.onNotFound(notFound);

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    server.begin();
        
  }
}

void loop() { 
  

  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= period)  //test whether the period has elapsed
  {
    printLocalTime();
    startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state.
    SampleStore();
  }


  
  
  
  //wait or sleep
  ////delay(TIME_TO_SLEEP * 1000);
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // ESP32 wakes up timer
  //Serial.println("Going to light-sleep now");
  //Serial.flush(); 
  //esp_light_sleep_start();

}



//--------------------------------------------------------

value GetParameter(int parameter){
  value thisVal;
  
  thisVal.TimeFor   = getLocalTime();
  thisVal.TimeAt    = getUtcTime();
  thisVal.StationId = "BSEC-Meetstation";
  switch (parameter){
    case 0:
      thisVal.parameter = "temperature";
      thisVal.value     = Samples.TempTotal / Samples.SampleCount;
      thisVal.unit      = "C";
      thisVal.symbol    = "T";
      thisVal.SampleId  = SHA256(thisVal.StationId, thisVal.TimeAt,thisVal.parameter);
    break;
    case 1:
      thisVal.parameter = "pressure";
      thisVal.value     = Samples.PresTotal / Samples.SampleCount;
      thisVal.unit      = "hPa";
      thisVal.symbol    = "P";
      thisVal.SampleId  = SHA256(thisVal.StationId, thisVal.TimeAt,thisVal.parameter);
     break;
     case 2:
      thisVal.parameter = "humidity";
      thisVal.value     = Samples.HumTotal / Samples.SampleCount;
      thisVal.unit      = "%";
      thisVal.symbol    = "H";
      thisVal.SampleId  = SHA256(thisVal.StationId, thisVal.TimeAt,thisVal.parameter);
     break;
     case 3:
      thisVal.parameter = "air-resistance";
      thisVal.value     = Samples.ResTotal / Samples.SampleCount;
      thisVal.unit      = "KOhms";
      thisVal.symbol    = "r";
      thisVal.SampleId  = SHA256(thisVal.StationId, thisVal.TimeAt,thisVal.parameter);
     break;
     case 4:
      thisVal.parameter = "iaq";
      thisVal.value     = Samples.IaqTotal / Samples.SampleCount;
      thisVal.unit      = "";
      thisVal.symbol    = "IAQ";
      thisVal.SampleId  = SHA256(thisVal.StationId, thisVal.TimeAt,thisVal.parameter);
     break;
     case 5:
      thisVal.parameter = "co2";
      thisVal.value     = Samples.Co2Total / Samples.SampleCount;
      thisVal.unit      = "ppm";
      thisVal.symbol    = "Co2";
      thisVal.SampleId  = SHA256(thisVal.StationId, thisVal.TimeAt,thisVal.parameter);
     break;
     case 6:
      thisVal.parameter = "voc";
      thisVal.value     = Samples.VocTotal / Samples.SampleCount;
      thisVal.unit      = "ppm";
      thisVal.symbol    = "voc";
      thisVal.SampleId  = SHA256(thisVal.StationId, thisVal.TimeAt,thisVal.parameter);
     break;
    }
  return(thisVal);
}

void printLocalTime(){
  Serial.println(MyTZ.dateTime(COOKIE));
}

String getLocalTime() {
  String now;
  now = MyTZ.dateTime("Y-m-d~TH:i:00");
  return(now); 
}

String getUtcTime() {
  String now;
  now = UTC.dateTime("Y-m-d~TH:i:s");
  return(now); 
}

void printHello(){
  Serial.println("Hello World, Iam ESP32 Thing Plus");
}

void WifiOn(){

  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("esp32");
  WiFi.begin(ssid, password);
  //WiFi.begin();
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry <32) {
      delay(500);
      retry = retry + 1;
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
}


//void WifiOff(){
//  //disconnect WiFi as it's no longer needed
//  WiFi.disconnect(true);
//  WiFi.mode(WIFI_OFF);
//  Serial.println("DISCONNECTED");
//}

void WifiReconnect(){
  WiFi.reconnect();
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 16 ) {
      delay(500);
      //WiFi.reconnect();
      Serial.print(".");
      }
  delay(1000);
  Serial.println(" CONNECTED");
}



String SHA256(String StationId, String TimeFor, String Parameter){
  char part1[50];
  char part2[50];
  char part3[50];
  
  StationId.toCharArray(part1, 50);
  TimeFor.toCharArray(part2, 50); 
  Parameter.toCharArray(part3, 50);
  
 
  const size_t Length1 = strlen(part1);
  const size_t Length2 = strlen(part2);
  const size_t Length3 = strlen(part3);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char *) part1, Length1);
  mbedtls_md_hmac_update(&ctx, (const unsigned char *) part2, Length2);
  mbedtls_md_hmac_update(&ctx, (const unsigned char *) part3, Length3);
  mbedtls_md_hmac_finish(&ctx, hmacResult);
  mbedtls_md_free(&ctx);
 
  String result;
  for(int i= 0; i< sizeof(hmacResult); i++){
      char str[3];
      sprintf(str, "%02x", (int)hmacResult[i]);
      result = result + str;
      
  }
  return(result);
}

String makeJSON(){
  String JSONdoc;
  DynamicJsonDocument doc(29016);


  for (int i = 0; i < BuffSize; i++){
    JsonArray val = doc.createNestedArray();
    val.add(Values[i].SampleId);
    val.add(Values[i].StationId);
    val.add(Values[i].parameter);
    val.add(Values[i].TimeAt);
    val.add(Values[i].TimeFor);
    JsonObject val1 = val.createNestedObject();
    val1[Values[i].symbol] = Values[i].value;
    val.add(Values[i].unit);
  }

  //serializeJsonPretty(doc, Serial);
  Serial.print("JSON size: ");
  Serial.println(doc.memoryUsage());
  serializeJson(doc, JSONdoc);
  return(JSONdoc);
}


int send2Server(String JSONdoc){
  HTTPClient http;
  String url="http://rest:5000/data/";
  //String url="http://httpbin.org/anything";   //Test URL pings messages back
  String jsondata=JSONdoc;

  http.begin(url); 
  http.addHeader("Content-Type", "application/json"); 

  int httpResponseCode = http.POST(jsondata); //Send the actual POST request

  if(httpResponseCode>0){
    String response = http.getString();  //Get the response to the request
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);           //Print request answer
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);

    http.end();

 }
 return(httpResponseCode);
}
