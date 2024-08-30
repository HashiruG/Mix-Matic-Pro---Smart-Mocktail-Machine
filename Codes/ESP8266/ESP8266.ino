#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>

SoftwareSerial espSerial(3, 1); 

const char* ssid = "th";
const char* password = "12345678";


const char* serverUrl = "https://192.168.43.172:3000/api";

WiFiClient client;


String previousData = "";

void setup() {
  Serial.begin(115200);
  espSerial.begin(9600);
 
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Attempting to connect");
  }
  
  Serial.println("\nConnected to WiFi");
  
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    Serial.println("Making HTTP request...");
    http.begin(client, serverUrl);
    int httpResponseCode = http.GET();
    
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println("Received payload:");
      Serial.println(payload);
      
      if (payload != previousData) {
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
          Serial.print("JSON parsing failed: ");
          Serial.println(error.c_str());
          return;
        }
        
        if (doc["success"]) {
          JsonArray dataArray = doc["data"];
          
          DynamicJsonDocument limitedDoc(1024);
          limitedDoc["success"] = true;
          JsonArray limitedData = limitedDoc.createNestedArray("data");
          
          for (int i = 0; i < min((int)dataArray.size(), 3); i++) {
            limitedData.add(dataArray[i]);
          }
          
          String limitedJsonString;
          serializeJson(limitedDoc, limitedJsonString);
          
          Serial.println("Sending to Arduino:");
          Serial.println(limitedJsonString);
          
          espSerial.println(limitedJsonString);
         
          previousData = payload;
        }
      } else {
        Serial.println("Data unchanged. Skipping send to Arduino.");
      }
    }
    http.end();
  }
  
  delay(1000); 
}
