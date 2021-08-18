#include <ArduinoJson.h>

void setup() {
  // Initialize Serial port
  Serial.begin(115200);
  while (!Serial) continue;

  // Allocate the JSON document
  //
  DynamicJsonDocument doc(3072);

  JsonArray doc_0 = doc.createNestedArray();
  doc_0.add("ca26a4a25da9077b0055962386aff5d29b344bf6953c622f16dcb575b47aab68");
  doc_0.add("Amsterdam");
  doc_0.add("temperatureCPU");
  doc_0.add("2021-06-11T07:26:44");
  doc_0.add("2021-06-11T09:26:00");
  JsonObject doc_0_5 = doc_0.createNestedObject();
  doc_0_5["T"] = 55.504;
  doc_0.add("c");

for (int index = 0; index < 20; index++){
  JsonArray doc_1 = doc.createNestedArray();
  doc_1.add("ca26a4a25da9077b0055962386aff5d29b344bf6953c622f16dcb575b47aab68");
  doc_1.add("Amsterdam");
  doc_1.add("temperatureCPU");
  doc_1.add("2021-06-11T07:26:44");
  doc_1.add("2021-06-11T09:26:00");
  JsonObject doc_1_5 = doc_1.createNestedObject();
  doc_1_5["T"] = 55.504;
  doc_1.add("c");
}

  // Generate the minified JSON and send it to the Serial port.
  //
  serializeJson(doc, Serial);
  // The above line prints:
  // {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}

  // Start a new line
  Serial.println();

  // Generate the prettified JSON and send it to the Serial port.
  //
  serializeJsonPretty(doc, Serial);
  // The above line prints:
  // {
  //   "sensor": "gps",
  //   "time": 1351824120,
  //   "data": [
  //     48.756080,
  //     2.302038
  //   ]
  // }
}

void loop() {
  // not used in this example
}
