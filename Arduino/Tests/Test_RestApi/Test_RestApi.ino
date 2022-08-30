#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
//#include <TinyUPnP.h>
  
//Upnp
#define LISTEN_PORT 6001  // http://<IP>:<LISTEN_PORT>/?name=<your string>
//#define LEASE_DURATION 0  // seconds
//#define FRIENDLY_NAME "TEST_UPNP"  // this name will appear in your router port forwarding section

//TinyUPnP tinyUPnP(20000);  // -1 means blocking, preferably, use a timeout value (ms)
WebServer server(LISTEN_PORT);

//Send POST to update fan state
const char* servidor="http://192.168.2.131:7896/iot/json?i=fan001&k=4jggokgpepnvsb2uv4s40d59ov";// -->Agregar

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
    update_on();
  }
  
  if (off_ == 1){
    Serial.println("Se apaga el ventilador");
    update_off(); // -->Agregar
  }
  
  // Respond to the client
  server.send(200, "application/json", body); // -->Agregar
}

// -->Agregar
void update_on(){
  String quote = "\"";
  String mensaje="{" +quote +"s" +quote +":" + quote + "Encendido" + quote + "}";

  HTTPClient http;
  Serial.print("**PRUEBA DE ENVIO DE POST:\t");
  Serial.println("");
  // you’re connected now, so print out the data
  Serial.println("Starting connection to server…");
  WiFiClient client;
  http.begin(client, servidor);
  Serial.print("Enviando mensaje JSON: ");
  // Specify content-type header
  //http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  //http.addHeader("Content-Length",longitud);

  int  httpResponseCode2 = http.POST(mensaje);

  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode2);
  
  String payload2 = http.getString();
  
  Serial.println(payload2);
  Serial.println();
  http.end();
}

void update_off(){
  String quote = "\"";
  String mensaje="{" +quote +"s" +quote +":" + quote + "Apagado" + quote + "}";

  HTTPClient http;
  Serial.print("**PRUEBA DE ENVIO DE POST:\t");
  Serial.println("");
  // you’re connected now, so print out the data
  Serial.println("Starting connection to server…");
  WiFiClient client;
  http.begin(client, servidor);
  Serial.print("Enviando mensaje JSON: ");
  // Specify content-type header
  //http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  //http.addHeader("Content-Length",longitud);

  int  httpResponseCode2 = http.POST(mensaje);

  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode2);
  
  String payload2 = http.getString();
  
  Serial.println(payload2);
  Serial.println();
  http.end();
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
