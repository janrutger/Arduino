#include "bsec.h"


// Create an object of the class Bsec
Bsec iaqSensor;

String output; //to print the error messages

// Entry point for the setup-loop
void BSECsetup(void)
{

  Wire.begin();

  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  
  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();

}


void ReadSample(){
  if (iaqSensor.run()) { // If new data is available
    Samples.TempTotal   = Samples.TempTotal + iaqSensor.temperature;
    Samples.HumTotal    = Samples.HumTotal  + iaqSensor.humidity;
    Samples.PresTotal   = Samples.PresTotal + iaqSensor.pressure / 100.0;
    Samples.ResTotal    = Samples.ResTotal  + iaqSensor.gasResistance / 1000.0;
    Samples.IaqTotal    = Samples.IaqTotal  + iaqSensor.iaq;
    Samples.Co2Total    = Samples.Co2Total  + iaqSensor.co2Equivalent;
    Samples.VocTotal    = Samples.VocTotal  + iaqSensor.breathVocEquivalent;
    Samples.SampleCount = Samples.SampleCount + 1;
    Serial.print("Accuracy: ");
    Serial.println(iaqSensor.iaqAccuracy);
    } else {
    checkIaqSensorStatus();
  }
}



// Helper function definitions
void checkIaqSensorStatus(void)
{
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      output = "BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    }
  }
}

void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}
