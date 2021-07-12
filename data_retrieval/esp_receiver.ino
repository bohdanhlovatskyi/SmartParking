#include <SPI.h> 
#include <nRF24L01.h> 
#include <RF24.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define PIN_CE  2
#define PIN_CSN 15

const byte address[6] = "00001";

RF24 radio(PIN_CE, PIN_CSN);

float rc_data[3] = {0};
bool received = false; // SHOULD BE FALSE

void setup() {

  Serial.begin(9600);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  // !WiFi.begin("SAMASUNG", "07050378")
  while (!WiFi.begin("SAMASUNG", "07050378")) {
    Serial.println("There was a problem with wifi connection");
    delay(10000);
  }
  WiFi.mode(WIFI_STA); 

  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
    delay(500);
    Serial.println("Waiting for connection to wifi");
  }

  while (!radio.begin()) {
    Serial.println("There was a problem with nrf connection");
    delay(10000);
  }

  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  while(radio.available())
  {
    radio.read(&rc_data, sizeof(rc_data));
    for (int i = 0; i < 3; i++) {
      Serial.print(rc_data[i]); Serial.print("; ");
    }
    Serial.println();
    received = true;
  }

    if ((WiFi.status() == WL_CONNECTED) && received) { //Check WiFi connection status
      Serial.println("Sending data");
      WiFiClient wifiClient;
      HTTPClient http;
 
      StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
      JsonObject& JSONencoder = JSONbuffer.createObject(); 
 
      JSONencoder["id"] = "some id";
      JSONencoder["value"] = rc_data[1]; // y value
      JSONencoder["timestamp"] = "10:30";

      /*
      JsonArray& values = JSONencoder.createNestedArray("values"); //JSON array
      values.add(20); //Add value to array
 
      JsonArray& timestamps = JSONencoder.createNestedArray("timestamps"); //JSON array
      timestamps.add("10:10"); //Add value to array
      */
  
      char JSONmessageBuffer[300];
      JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
      Serial.println(JSONmessageBuffer);

      // http.begin("http://anteph.pythonanywhere.com/postjson");      //Specify request destination
      http.begin(wifiClient, "http://192.168.0.103:5000/postjson");  
      // http.begin(wifiClient, "http://10.10.225.18:5000/postjson");  
      http.addHeader("Content-Type", "application/json");  //Specify content-type header
 
      int httpCode = http.POST(JSONmessageBuffer);   //Send the request
      String payload = http.getString();                                        //Get the response payload
 
      Serial.println(httpCode);   //Print HTTP return code
      Serial.println(payload);    //Print request response payload
 
      http.end();  //Close connection
 
      } else {
        Serial.println("Error in WiFi connection");
      }
      received = false;
      delay(1000);  //Send a request every 30 seconds
}

 