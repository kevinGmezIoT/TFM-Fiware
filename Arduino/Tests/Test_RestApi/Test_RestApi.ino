#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
//#include <TinyUPnP.h>
  
//Upnp
#define LISTEN_PORT 6001  // http://<IP>:<LISTEN_PORT>/?name=<your string>
//#define LEASE_DURATION 0  // seconds
//#define FRIENDLY_NAME "TEST_UPNP"  // this name will appear in your router port forwarding section

//TinyUPnP tinyUPnP(20000);  // -1 means blocking, preferably, use a timeout value (ms)
WebServer server(LISTEN_PORT);

// JSON data buffer
StaticJsonDocument<250> jsonDocument;
char buffer[250];
 
void connectToWiFi() {
  WiFi.mode(WIFI_STA);  
  delay(4000); // Esperar para que se configure la consola.
  WiFiManager wm;
  bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }

  if(WiFi.status()== WL_CONNECTED){
       // Mostrar mensaje de exito y dirección IP asignada
       Serial.println();
       Serial.print("Conectado a:\t");
       Serial.println(WiFi.SSID());
       Serial.print("IP address:\t");
       Serial.println(WiFi.localIP());
       Serial.println();
  }
}



void setup_routing() {
  server.on("/fan", HTTP_POST, handlePost); 
       
  // start server    
  server.begin();    
}
 
void handlePost() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  
  // Get RGB components
  int on_ = jsonDocument["on"];
  int off_ = jsonDocument["off"];
  Serial.println("Receive: ");
  Serial.println(body);
  if (on_ == 1){
    Serial.println("Se enciende el ventilador");
  }
  
  if (off_ == 1){
    Serial.println("Se apaga el ventilador");
  }
  
  // Respond to the client
  server.send(200, "application/json", body);
}

void setup() {     
  Serial.begin(115200);         
    
  connectToWiFi();  
  //config_upnp();
  setup_routing();   
}    
       
void loop() {    
  //tinyUPnP.updatePortMappings(600000);  // 10 minutes
  server.handleClient();   
  delay(5);  
}
