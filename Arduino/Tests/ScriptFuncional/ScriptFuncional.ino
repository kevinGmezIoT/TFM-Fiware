
//Declaracion de librerias
#include <MHZ19.h> 
#include <SoftwareSerial.h> 
#include <DHT.h> 
#include <time.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <TinyUPnP.h>



#define LISTEN_PORT 6001  // http://<IP>:<LISTEN_PORT>/?name=<your string>
#define LEASE_DURATION 0  // seconds
#define FRIENDLY_NAME "TEST_UPNP"  // this name will appear in your router port forwarding section

TinyUPnP tinyUPnP(20000);  // -1 means blocking, preferably, use a timeout value (ms)
WebServer server(LISTEN_PORT);

// Relevador
#define RELAY_PIN 18

// JSON data buffer
StaticJsonDocument<250> jsonDocument;
char buffer[250];

//Servidor Timestamp
const char* NTP_SERVER = "ch.pool.ntp.org";
const char* TZ_INFO    = "CST6CDT";  // enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
tm timeinfo;
time_t now;
long unsigned lastNTPtime;
long unsigned SetTiming;

const char* TEMPserv2="http://ec2-54-87-3-62.compute-1.amazonaws.com:7896/iot/json?i=temp001&k=4jggokgpepnvsb2uv4s40d59ov";
const char* TEMPserv="http://ec2-54-87-3-62.compute-1.amazonaws.com:1026/v2/entities/urn:ngsi:Temp:001";

const char* HUMserv2="http://ec2-54-87-3-62.compute-1.amazonaws.com:7896/iot/json?i=hum001&k=4jggokgpepnvsb2uv4s40d59ov";
const char* HUMserv="http://ec2-54-87-3-62.compute-1.amazonaws.com:1026/v2/entities/urn:ngsi:Hum:001";

const char* CO2serv2="http://ec2-54-87-3-62.compute-1.amazonaws.com:7896/iot/json?i=co2001&k=4jggokgpepnvsb2uv4s40d59ov";
const char* CO2serv="http://ec2-54-87-3-62.compute-1.amazonaws.com:1026/v2/entities/urn:ngsi:CO2:001";  

const char* servidor="http://ec2-54-87-3-62.compute-1.amazonaws.com:7896/iot/json?i=fan001&k=4jggokgpepnvsb2uv4s40d59ov";

// Pin RX Arduino conectado al pin TX del MHZ19
#define RX_PIN 16
// Pin TX Arduino conectado al pin RX del MHZ19
#define TX_PIN 17
// Objeto para sensor MHZ19
MHZ19 myMHZ19;
// Serial requerido por el MHZ19
SoftwareSerial mySerial(RX_PIN, TX_PIN);

// para DHT22
int D5 = 5;
DHT sense(D5,DHT22);
float temp,hume;


//Conexion Wi-Fi

//const char* ssid     = "IQ8I1SE 5991";
//const char* password = "080#00Qn47n";


void setup() {
  Serial.begin(9600);

  connectToWiFi();

  configTime(0, 0, NTP_SERVER);
  // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
  setenv("TZ", TZ_INFO, 1);

  if (getNTPtime(10)) {  // wait up to 10sec to sync
  } else {
    Serial.println("Time not set");
    ESP.restart();
  }

  SetTiming = time(&now);
  
  pinMode(RELAY_PIN, OUTPUT);
  mySerial.begin(9600);
  myMHZ19.begin(mySerial);
  // Turn auto calibration ON (OFF autoCalibration(false))
  myMHZ19.autoCalibration(false);
  //Iniciar sensor DHT22
  sense.begin();

  config_upnp();
  setup_routing();
}

void loop() {
  
  tinyUPnP.updatePortMappings(600000);  // 10 minutes
  server.handleClient();
  delay(500);
  // Tomar mediciones cada 2 segundos  
    
  lastNTPtime = time(&now);
  
  if (lastNTPtime-SetTiming == 3600){
    Serial.println("Enviando datos a servidor...");
    SetTiming = lastNTPtime;
    sensorica();
  }  

}


void sensorica() {
    
  // Obtener la medición de CO2 actual como ppm
  int nivelCO2 = myMHZ19.getCO2();
  float hume = sense.readHumidity();
  float temp = sense.readTemperature();

  // Mostrar el nivel de CO2 en el monitor serie
  Serial.print("CO2 (ppm): ");                      
  Serial.println(nivelCO2);  

  // Mostrar la temperatura en el monitor serie
  Serial.print("Temperatura (C): ");                  
  Serial.println(temp); 

  Serial.print("Humedad (%): ");                  
  Serial.println(hume);

  sendhttp(nivelCO2,"c",CO2serv,CO2serv2);
  delay(4000);
  sendhttp(temp,"t",TEMPserv,TEMPserv2);
  delay(4000);
  sendhttp(hume,"h",HUMserv,HUMserv2);

  Serial.println("-----DATOS ENVIADOS-----");  

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

bool getNTPtime(int sec) {

  {
    uint32_t start = millis();
    do {
      time(&now);
      localtime_r(&now, &timeinfo);
      Serial.print(".");
      delay(10);
    } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
    if (timeinfo.tm_year <= (2016 - 1900)) return false;  // the NTP call was not successful
    Serial.print("now ");  Serial.println(now);
    char time_output[30];
    strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
    Serial.println(time_output);
    Serial.println();
  }
  return true;
}

void sendhttp(float valuesensor,String ind ,String servidor, String servidor2) {
  

  String quote = "\"";
  String mensaje="{" +quote +ind +quote +":" + quote + valuesensor + quote + "}";

  //calculo longitud del mensaje
  int  mm = mensaje.length()+38;
  String longitud= (String) mm;


  //prueba de GET
  HTTPClient http;
  // Your Domain name with URL path or IP address with path
  http.begin(servidor);
  http.addHeader("fiware-service", "openiot");
  http.addHeader("fiware-servicepath", "/");
  int httpResponseCode = http.GET(); //POST(salidajson);
  String payload = http.getString();
    
  Serial.println("respuesta devuelta al comando HTTP.getString( Payload): ");
  Serial.println(payload);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  Serial.println();
  http.end();

  Serial.print("**PRUEBA DE ENVIO DE POST:\t");
  Serial.println("");
    
  // you’re connected now, so print out the data
  Serial.println();
  Serial.println("Starting connection to server…");
  WiFiClient client;
  http.begin(client, servidor2);
  Serial.print("Enviando mensaje JSON: ");
  // Specify content-type header
  //http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  //http.addHeader("Content-Length",longitud);
    
  Serial.print("Longitud del mensaje=");
  Serial.println(longitud );
  Serial.println("Contenido del mensaje=");
  Serial.println(mensaje);
    
  int  httpResponseCode2 = http.POST(mensaje);
    
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode2);
      
  String payload2 = http.getString();
      
  Serial.println(payload2);
  Serial.println();
  http.end();
      
  //salidajson="";
  
}

void config_upnp(){
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
    Serial.println("Encendiendo ventilador");
    digitalWrite(RELAY_PIN, HIGH);
    update_x(1);
  }
  
  if (off_ == 1){
    Serial.println("Apagando ventilador");
    digitalWrite(RELAY_PIN, LOW);
    update_x(0);
  }
  
  // Respond to the client
  server.send(200, "application/json", body);
}

void update_x(int val){
  String textstat;
  
  if (val==0){
    textstat = "Apagado";
  }
  else {
    textstat = "Encendido";
  }
  
  String quote = "\"";
  String mensaje="{" +quote +"s" +quote +":" + quote + textstat + quote + "}";

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
