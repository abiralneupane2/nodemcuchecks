#include <command.h>
//#include <ESP8266AVRISP.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include "DHTesp.h"
#include <ArduinoJson.h>

#define Led_OnBoard 2


int led1 = 4;

int dhtPin = 5;
int dhtSamplingPeriod;
String switchStatus;
String humidityValue, temperatureValue, postData;

DHTesp dht;

HTTPClient http;
const char* ssid = "Nerdware";                  // Your wifi Name       
const char* password = "4320C1728B";          // Your wifi Password
const String api = "/api";

const String host = "192.168.1.65"; //Your pc or server (database) IP, example : 192.168.0.0 , if you are a windows os user, open cmd, then type ipconfig then look at IPv4 Address.

void setup() {
  // put your setup code here, to run once:
  delay(1000);
  pinMode(Led_OnBoard, OUTPUT);       // Initialize the Led_OnBoard pin as an output
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(Led_OnBoard, LOW);
    delay(250);
    Serial.print(".");
    digitalWrite(Led_OnBoard, HIGH);
    delay(250);
    
  }

  digitalWrite(Led_OnBoard, HIGH);
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.println("Connected to Network/SSID");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  dht.setup(dhtPin, DHTesp::DHT11);
  dhtSamplingPeriod = dht.getMinimumSamplingPeriod();
//  /Serial.println(dhtSamplingPeriod);

  
}

void loop() {
  Serial.println("start");
  // put your main code here, to run repeatedly:
      //Declare object of class HTTPClient
 
  humidityValue = String(dht.getHumidity());
  temperatureValue = String(dht.getTemperature());
//  /int ldrvalue=analogRead(A0);  //Read Analog value of LDR
//  /LdrValueSend = String(ldrvalue);   //String to interger conversion
 
  //Post Data
  String postData = "temperature=" + temperatureValue;// + "&switch=" + 12;
  
  //http.begin("http://httpbin.org/post");
  http.begin("http://192.168.1.65:8000/api/arduino/");              //Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
 
  int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload
  
  //Serial.println("LDR Value=" + ldrvalue);
  Serial.println(httpCode);   //Print HTTP return code
  Serial.println("Temperature = " + temperatureValue + "\t Humidity = " + humidityValue );
  Serial.println("payload");
  Serial.println(deserialize(payload, "switch"));
  http.end();
  
//  setDevice(payload);
  digitalWrite(Led_OnBoard, LOW);
  delay(3000);
  digitalWrite(Led_OnBoard, HIGH);
  Serial.println("end");
}

String djangodeserealize(String data, String argument){
  const size_t capacity = JSON_ARRAY_SIZE(2)+JSON_ARRAY_SIZE(1)+30;
  DynamicJsonDocument doc(capacity);
  String json = data;
  
  deserializeJson(doc, json);
  
  String val = doc[0][argument]; // 22
  return val;
}


String deserialize(String data, String argument){
  String json = data;

  DynamicJsonDocument doc(32);
  deserializeJson(doc, json);

  String val = doc[argument];
  return val;
}

void setDevice(String payload){
  String switchValue=deserialize(payload, "switch");
  setSwitch(switchValue);
//  Serial.println("switchValue="+switchValue);
}
void setSwitch(String temp){
  int val = temp.toInt();
  int s[2];
  for(int i=0;i>=0;i++){
    s[i] = val%2;
    val/=2;
    if(val==0)
      break;
  }
//  /digitalWrite(led1, s[0]);
  digitalWrite(Led_OnBoard, !s[0]);
}
