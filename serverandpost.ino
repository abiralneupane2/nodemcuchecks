#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "ArduinoJson.h"
#include "DHTesp.h"
#include <ESP8266HTTPClient.h>


DHTesp dht;
#define STASSID "Aditi"
#define STAPSK  "0305098076"
String humidityValue, temperatureValue;
const int dhtPin=5;
String ssid = STASSID;
///"Nerdware";
String password =  STAPSK;
///"4320C1728B";
int device_num = 0;
const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";
AsyncWebServer server(80);
DynamicJsonDocument state(1024);
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
  <br>
  %ID_PASSWORDPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  console.log("checked");
  if(element.checked){ xhr.open("GET", "/state?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/state?output="+element.id+"&state=0", true); }
  xhr.send();
};
function changeWifiCredentials(){
  var xhr = new XMLHttpRequest();
  var ssid = document.getElementById("ssid").value;
  var password = document.getElementById("password").value;
  console.log(ssid);
  xhr.open("GET", "/state?ssid="+ssid+"&password="+password, true);
  xhr.send();
  };
</script>
</body>
</html>
)rawliteral";


String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}
String processor(const String& var){
  Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
//    buttons += "<h4>Output - GPIO 5</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"5\" " + outputState(5) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 4</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  if(var=="ID_PASSWORDPLACEHOLDER"){
    String fields = "";
    fields += "<input type=\"text\" id=\"ssid\" placeholder= " + ssid + "><br>";
    fields += "<input type=\"text\" id=\"password\" placeholder= " + password + "><br>";
    fields += "<button onclick=\"changeWifiCredentials()\">Change</button>";
    return fields;
    }
  return String();
}

void handleLogin(AsyncWebServerRequest *request) {
  String msg;
  if(request->hasHeader("Cookie")){
      AsyncWebHeader* h = request->getHeader("Cookie");
      String cookie = h->value().c_str();
      Serial.println(cookie);
      if (cookie.indexOf("ESPSESSIONID=1") != -1) {
        Serial.println("Authentication Successful");
        AsyncWebServerResponse *response = request->beginResponse(200); 
        request->redirect("/state");
        request->send(response);
        return;
      }
    }
  if(request->hasArg("USERNAME") && request->hasArg("PASSWORD")){
    String username = request -> arg("USERNAME");
    String password = request -> arg("PASSWORD");
    Serial.println(password);
    if(username == "Nerdware" && password == "admin"){
      Serial.println("User online");
      AsyncWebServerResponse *response = request->beginResponse(200); 
      response->addHeader("Set-Cookie","/ESPSESSIONID=1");
      request->send(response);
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><body><form action='/login' method='POST'>To log in, please use : admin/admin<br>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  request->send(200, "text/html", content);
}


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
  AsyncWebServerResponse *response = request->beginResponse(401);
  Serial.println("redirect to login");
  request->redirect("/login");
  request->send(response);
}
void loginPage(AsyncWebServerRequest *request){
  String content = "<html><body><form action='/login' method='POST'>To log in, please use : admin/admin<br>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>";
  request->send(200, "text/html", content);
  return;
}




void dataHandler(AsyncWebServerRequest *request){
  if(is_authenticated(request)){
    Serial.println("Handle data");
    
    
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasArg(PARAM_INPUT_1) && request->hasArg(PARAM_INPUT_2)) {
      String inputMessage1;
      String inputMessage2;
      inputMessage1 = request->arg(PARAM_INPUT_1);
      inputMessage2 = request->arg(PARAM_INPUT_2);
      Serial.println(inputMessage1 + ":" + inputMessage2);
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
      request->send(200);
      return;
    }
    else if (request->hasArg("ssid") && request->hasArg("password")){
      ssid = request->arg("ssid");
      password = request->arg("password");
      Serial.println(ssid+":"+password);
      request->send(200);
      return;
      }
    else {
      request->send_P(200, "text/html", index_html, processor);
      return;
    }
    request->send(200, "text/plain", "OK");
    return;
  }
  
}




void configurationServer(){
  Serial.println(WiFi.localIP());
  server.on("/login", handleLogin);

  server.on("/state", dataHandler);
  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("logout");
      AsyncWebServerResponse *response = request->beginResponse(200);
      response->addHeader("Set-Cookie","/ESPSESSIONID=0");
      response->addHeader("Location","/login");
      request->send(response);
    });
  
 
  server.begin();
  }

void mserver(){

  }
void setup(){
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  delay(1000);
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }
  dht.setup(dhtPin, DHTesp::DHT11);
  Serial.println(dht.getMinimumSamplingPeriod());
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  configurationServer();

  
  
}
void loop(){
  humidityValue = String(dht.getHumidity());
  temperatureValue = String(dht.getTemperature());
  Serial.println("Temperature = " + temperatureValue + "\t Humidity = " + humidityValue );
  delay(3000);
}
