#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <SimpleTimer.h>

const char* servidor2="http://192.168.2.131:7896/iot/json?i=temp001&k=4jggokgpepnvsb2uv4s40d59ov";
const char* servidor="http://192.168.2.131:1026/v2/entities/urn:ngsi-ld:Temp:001";

float dato_temp = 0.0;
float dato_rssi = 0.0;
int dato_conor = 0;
float dato_hall = 0.0;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.

unsigned long lastTime = 0;

// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)

unsigned long timerDelay = 5000;

// ledPin refers to ESP32 GPIO 23

const int ledPin = 2;

#define LED 2
#ifdef __cplusplus

extern "C" {
  #endif
  uint8_t temprature_sens_read();
  #ifdef __cplusplus
}

#endif

uint8_t temprature_sens_read();

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
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
}

void loop() {

delay(2000);

 // Reading temperature or humidity takes about 250 milliseconds!

   float h = random(1000)/1000.*100; //dht.readHumidity();

   float t = random(300)/1000.*100;//dht.readTemperature();

 if (isnan(h) || isnan(t)) {
     Serial.println("!Fallo al leer sensor  DHT !");
      //return;
    }
   else
   {
      Serial.print("Temperatura exterior: ");
      Serial.println(t);
      Serial.print("Humedad exterior:");
      Serial.println(h);
   }
   
  //tomar valor de la temperatura interior
  // Convierte de F a ºC

  dato_temp=(temprature_sens_read()-32)/1.8;
   Serial.print("Temperatura interior :");
  Serial.println(dato_temp);

/*
String quote = "\"";
String mensaje="{" +quote +"Humedad" +quote +":{" +quote +"type" +quote +":" +quote +"*" +quote +"," +quote +"value" +quote +":";

mensaje +=h;
mensaje +="," +quote +"metadata" +quote +":{" +quote +"accuracy" +quote +":{" +quote +"type" +quote +":" +quote +"*" +quote +"," +quote +"value" +quote +":9}}}," +quote +"Temperatura" +quote +":{" +quote +"type" +quote +":" +quote +"*" +quote +"," +quote +"value" +quote +":";
mensaje +=  t;
mensaje += "," +quote +"metadata" +quote +":{}}," +quote +"Temperaturaint" +quote +":{" +quote +"type" +quote +":" +quote +"*" +quote +"," +quote +"value" +quote +":";
mensaje += dato_temp;
mensaje +="," +quote +"metadata" +quote +":{}}," +quote +"Wifi" +quote +":{" +quote +"type" +quote +":" +quote +"*" +quote +"," +quote +"value" +quote +":";
mensaje += dato_rssi;
mensaje += "," +quote +"metadata" +quote +":{}}}";
*/
String quote = "\"";
String mensaje="{" +quote +"t" +quote +":" + quote + dato_temp + quote + "}";

//calculo longitud del mensaje
int  mm= mensaje.length()+38;
String longitud= (String) mm;

 //Send an HTTP POST request every 10 minutes

if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
       // Mostrar mensaje de exito y dirección IP asignada
       Serial.println();
       Serial.print("Conectado a:\t");
       Serial.println(WiFi.SSID());
       Serial.print("IP address:\t");
       Serial.println(WiFi.localIP());
       Serial.println();
       Serial.print("**PRUEBA DE ENVIO DE GET:\t");
       Serial.println("");
    
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
      delay(10000);
    
      Serial.print("**PRUEBA DE ENVIO DE POST:\t");
      Serial.println("");
      digitalWrite(ledPin, HIGH);
    
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
      
      digitalWrite(ledPin, LOW);
      delay(10000);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
