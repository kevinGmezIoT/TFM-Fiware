/*  
  Note: This example includes the library EasyDDNS. You'll have to add this package using your Arduino Library Manager.
        The purpose of this package is to publish your dynamic IP to a DDNS service that will allocate a human readable
        address to your current IP. If you do not need that, you can remove this dependency.
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>
#include <EasyDDNS.h>
#include <TinyUPnP.h>

#define LISTEN_PORT 8888  // http://<IP>:<LISTEN_PORT>/?name=<your string>
#define LEASE_DURATION 0  // seconds
#define FRIENDLY_NAME "TEST_UPNP"  // this name will appear in your router port forwarding section
//#define DDNS_TOKEN "d2b9e916-ab53-4b1c-b8ed-fe8bbd17ca89"
//#define DDNS_DOMAIN "nivekurama2.duckdns.org"

TinyUPnP tinyUPnP(20000);  // -1 means blocking, preferably, use a timeout value (ms)
WebServer server(LISTEN_PORT);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp32!");
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

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

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  connectToWiFi();

  boolean portMappingAdded = false;
  tinyUPnP.addPortMappingConfig(WiFi.localIP(), LISTEN_PORT, RULE_PROTOCOL_TCP, LEASE_DURATION, FRIENDLY_NAME);
  while (!portMappingAdded) {
    portMappingAdded = tinyUPnP.commitPortMappings();
    Serial.println("");
  
    if (!portMappingAdded) {
      // for debugging, you can see this in your router too under forwarding or UPnP
      tinyUPnP.printAllPortMappings();
      Serial.println(F("This was printed because adding the required port mapping failed"));
      delay(30000);  // 30 seconds before trying again
    }
  }
  
  Serial.println("UPnP done");

  // DDNS
  //EasyDDNS.service("duckdns");
  //EasyDDNS.client(DDNS_DOMAIN, DDNS_TOKEN);

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {

  //EasyDDNS.update(300000);  // check for New IP
  
  tinyUPnP.updatePortMappings(600000);  // 10 minutes
  
  server.handleClient();
  
  delay(5);
}
