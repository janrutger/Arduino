#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "mbedtls/md.h"
#include <ezTime.h>
#include <WiFi.h>
//#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Wire.h>
#include "FS.h"
#include <LittleFS.h>

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


//sensor
struct samples{
  float TempTotal;
  float HumTotal;
  float PresTotal;
  float ResTotal;
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

Adafruit_BME680 bme;

#define FORMAT_LITTLEFS_IF_FAILED false
//AsyncWebServer server(80);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  printHello();

  

 //init and get the time, Wifi
  Wire.begin();
  startMillis = millis();
  
  WifiOn();
  setDebug(INFO);
  waitForSync();
  MyTZ.setLocation(F("Europe/Amsterdam"));
  printLocalTime();
  //WifiOff();

  //init sensor
  if (!bme.begin())
  {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  
  ReadSample();   //to prefent first false reading
  Samples.TempTotal = 0;
  Samples.HumTotal = 0;
  Samples.PresTotal = 0;
  Samples.ResTotal = 0;
  Samples.SampleCount = 0;


//Storage
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    } else {
      Serial.print("LittleFS Mounted, ");
    }
  if(LittleFS.exists("/index.html")) {
    Serial.println("INDEX file found");
//    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//      request->send(200, "text/plain", "Hello, world");
//
//    server.onNotFound(notFound);

//    server.begin();
    
//    });
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
void ReadSample(){
  bme.performReading();

  Samples.TempTotal   = Samples.TempTotal + bme.temperature;
  Samples.HumTotal    = Samples.HumTotal  + bme.humidity;
  Samples.PresTotal   = Samples.PresTotal + bme.pressure / 100.0;
  Samples.ResTotal    = Samples.ResTotal  + bme.gas_resistance / 1000.0;
  Samples.SampleCount = Samples.SampleCount + 1;
}


value GetParameter(int parameter){
  value thisVal;
  //bme.performReading();


  
  thisVal.TimeFor   = getLocalTime();
  thisVal.TimeAt    = getUtcTime();
  thisVal.StationId = "ESP-Meetstation";
  switch (parameter){
    case 0:
      thisVal.parameter = "temperature";
      //thisVal.value     = bme.temperature;
      thisVal.value     = Samples.TempTotal / Samples.SampleCount;
      thisVal.unit      = "C";
      thisVal.symbol    = "T";
      thisVal.SampleId  = SHA256(thisVal.StationId, thisVal.TimeAt,thisVal.parameter);
    break;
    case 1:
      thisVal.parameter = "pressure";
      //thisVal.value     = bme.pressure / 100.0;
      thisVal.value     = Samples.PresTotal / Samples.SampleCount;
      thisVal.unit      = "hPa";
      thisVal.symbol    = "P";
      thisVal.SampleId  = SHA256(thisVal.StationId, thisVal.TimeAt,thisVal.parameter);
     break;
     case 2:
      thisVal.parameter = "humidity";
      //thisVal.value     = bme.humidity;
      thisVal.value     = Samples.HumTotal / Samples.SampleCount;
      thisVal.unit      = "%";
      thisVal.symbol    = "H";
      thisVal.SampleId  = SHA256(thisVal.StationId, thisVal.TimeAt,thisVal.parameter);
     break;
     case 3:
      thisVal.parameter = "air-quality";
      //thisVal.value     = bme.gas_resistance / 1000.0;
      thisVal.value     = Samples.ResTotal / Samples.SampleCount;
      thisVal.unit      = "KOhms";
      thisVal.symbol    = "r";
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
  Serial.println("Hello World, Iam ESP32-S2 Thing Plus");
}

void WifiOn(){

  Serial.printf("Connecting to %s ", ssid);
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
      retry = retry + 1;
      //WiFi.reconnect();
      Serial.print(".");
      }
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
  DynamicJsonDocument doc(17408);


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
