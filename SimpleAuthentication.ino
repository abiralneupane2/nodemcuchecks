#include "WiFi.h"
#include "ESPAsyncWebServer.h"

 
#define STASSID "home_wifi_wlink"
#define STAPSK  "0305098076"


const char* ssid = STASSID;
const char* password = STAPSK;
 
AsyncWebServer server(80);
 
void setup(){
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());
 
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World");
  });
 
  server.begin();
}
 
void loop(){}
