#include <bme68x.h>
#include <Adafruit_BME680.h>
#include <bme68x_defs.h>



//#include <Adafruit_Sensor.h>

/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/

#include <Wire.h>

Adafruit_BME680 bme; 


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  delay(3000);
  WifiOn();
  SetTime();
  WifiOff();
  PrintTime();

  if (!bme.begin())
  {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  

  // Set up oversampling and filter initialization
  //bme.setTemperatureOversampling(BME680_OS_8X);
  //bme.setHumidityOversampling(BME680_OS_2X);
  //bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
  
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1500);                       // wait for a second

  bme.performReading();
  printHello();

  float temperature = (bme.temperature);
  float humidity    = (bme.humidity);
  float pressure    = (bme.pressure / 100.0);
  float gas         = (bme.gas_resistance / 1000.0);

  Serial.print(temperature);
  Serial.println(" *C");

  Serial.print(humidity);
  Serial.println(" %");

  Serial.print(pressure);
  Serial.println(" hPa");

  Serial.print(gas);
  Serial.println(" KOhms");

  Serial.println("------------------------------");

  delay(50000);

}
