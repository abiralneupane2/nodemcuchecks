#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "ESPAsyncWebServer.h"

#define STASSID "Aditi"
#define STAPSK  "0305098076"
 
const char* ssid = STASSID;
const char* password =  STAPSK;
 
AsyncWebServer server(80);

bool is_authenticated(AsyncWebServerRequest *request) {
  Serial.println("Enter is_authenticated");
  if(request->hasHeader("Cookie")){
      AsyncWebHeader* h = request->getHeader("Cookie");
      String cookie = h->value().c_str();
      Serial.println(cookie);
      if (cookie.indexOf("ESPSESSIONID=1") != -1) {
        Serial.println("Authentication Successful");
      return true;
      }
    }
  Serial.println("Authentication Failed");
  return false;
}

void loginHandler(AsyncWebServerRequest *request){
  if(is_authenticated(request)){
    Serial.println("Alreaey authenticated");
    AsyncWebServerResponse *response = request->beginResponse(200); //Sends 404 File Not Found
    response->addHeader("Location","/dataHandler");
    request->send(response);
    return;
  }
  if(request->hasArg("USERNAME") && request->hasArg("PASSWORD")){
    String username = request -> arg("USERNAME");
    String password = request -> arg("PASSWORD");
    Serial.println(username);
    if(username == "admin" && password == "admin"){
      Serial.println("User online");
      AsyncWebServerResponse *response = request->beginResponse(200); 
      response->addHeader("Location","/dataHandler");
      response->addHeader("Set-Cookie","/ESPSESSIONID=1");
      request->send(response);
      return;
    }
    
  }
  request->send(404, "text/plain", "wrong credentials");
      
}

void dataHandler(AsyncWebServerRequest *request){
  if(is_authenticated(request)){
    Serial.println("Handle data");
    request->send(200, "text/plain", "works");
    return;
  }
  AsyncWebServerResponse *response = request->beginResponse(404);
  response->addHeader("Location","/login");
  request->send(response);
}
void currentDataHandler(AsyncWebServerRequest *request){
      request->send(200, "text/plain", "works");
    }



void controlHandler(AsyncWebServerRequest *request){
      request->send(200, "text/plain", "works");
    }

void setup(){
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi.
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println(".");
    }
 
  Serial.println(WiFi.localIP());
  server.on("/login", HTTP_POST, loginHandler);
  server.on("/data", HTTP_GET, dataHandler);
  server.on("/currentdata", HTTP_POST, currentDataHandler);
  server.on("/control", HTTP_POST, controlHandler);
  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("logout");
      AsyncWebServerResponse *response = request->beginResponse(200);
      response->addHeader("Set-Cookie","/ESPSESSIONID=0");
      request->send(response);
    });
  
 
  server.begin();
}
 
void loop(){}
