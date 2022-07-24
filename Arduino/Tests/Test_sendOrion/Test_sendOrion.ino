#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SimpleTimer.h>
#include <DHT.h>

#define DHTTYPE DHT11   // DHT 11

const int DHTPin = 5;     // what digital pin we’re connected to

DHT dht(DHTPin, DHTTYPE);

const char* ssid = "MI_RED_WIFI";
const char* password ="CLAVE_WIFI";

const char* servidor2="http://192.168.1.66:1026/v2/entities/Practica2/attrs/";
const char* servidor="http://192.168.1.66:1026/v2/entities/";

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

 Serial.begin(115200);
  delay(4000); // Esperar para que se configure la consola.
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) { // Comprobar la conexión.

    delay(1000);

    Serial.println("Conectando a la red WiFi…");
  }

  Serial.print("Conectado a la red WiFi: ");
  Serial.println(ssid);
  Serial.println("DHT11 test!");

  dht.begin();

 Serial.println("DHT11 test OK !");

}

void loop() {

delay(2000);

 // Reading temperature or humidity takes about 250 milliseconds!

   float h = dht.readHumidity();

   float t = dht.readTemperature();

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

// medir nivel señal wifi
 dato_rssi = 0;
 for (int i=0;i < 10;i++){
  dato_rssi += WiFi.RSSI();
  delay(20);
 }

 dato_rssi = dato_rssi/10;
 
Serial.print("Nivel WIFI :");
Serial.println(dato_rssi);

 //sensor Hall
dato_hall=hallRead();

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
      http.begin(servidor2);
      Serial.print("Enviando mensaje JSON: ");
      // Specify content-type header
      http.addHeader("Accept", "application/json");
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Content-Length",longitud);
    
      Serial.print("Longitud del mensaje=");
      Serial.println(longitud );
      Serial.println("Contenido del mensaje=");
      Serial.println(mensaje );
    
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
