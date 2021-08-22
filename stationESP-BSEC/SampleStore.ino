void SampleStore() {
  Serial.print("Read Sample #");
  ReadSample();
  //Serial.println(Samples.TempTotal);
  Serial.println(Samples.SampleCount);

  if (Samples.SampleCount >= 17){
    //WifiReconnect();    //After light_sleep, not as server
    events();
    Serial.println("--------------------------");

    for (int num = 0; num <= 3; num++){
    Value = GetParameter(num);
    Serial.print("Ringbuffer Address : ");
    Serial.println(Index % maxValues);
    Values[Index % maxValues] = Value;
    Index++;
    }

    if (Index < maxValues){
      BuffSize = Index;
    } else {
      BuffSize = maxValues;
    }

 //   if (Index+4 <= maxValues) {
 //     for (int num = 0; num <= 3; num++){
 //     Value = GetParameter(num);
 //     Values[Index] = Value;
 //     Index++;
 //     }
 //   }
  
//    for (int Current = 0; Current < Index; Current++){
    for (int Current = 0; Current < BuffSize; Current++){
      Serial.println(Values[Current].SampleId + " " + Values[Current].StationId + " " + Values[Current].TimeAt + " " + Values[Current].TimeFor + " " + Values[Current].parameter + " " + Values[Current].symbol + " " + Values[Current].value + " " + Values[Current].unit);
    }

    String JSONdoc;
    int responseCode;
    
    JSONdoc = makeJSON();
    Serial.println(JSONdoc);
  
    responseCode = send2Server(JSONdoc);
    if(responseCode == 200){
      Index = 0;
    }
    Samples.TempTotal = 0;
    Samples.HumTotal = 0;
    Samples.PresTotal = 0;
    Samples.ResTotal = 0;
    Samples.SampleCount = 0; 
  }

  
}
