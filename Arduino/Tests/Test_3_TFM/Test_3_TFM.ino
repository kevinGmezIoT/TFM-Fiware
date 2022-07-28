#include <MHZ19.h>  //Libreria de sensor MH-Z19B
#include <SoftwareSerial.h> //Libreria de comunicacion serial
#include <DHT.h> // Libreria para sensor de DHT22 de humedad y temperatura
 
// Mensajeria
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>


//const char* servidor="ec2-54-234-212-21.compute-1.amazonaws.com";

const char* servidor2="http://ec2-54-234-212-21.compute-1.amazonaws.com:7896/iot/json?i=temp001&k=4jggokgpepnvsb2uv4s40d59ov";
const char* servidor="http://ec2-54-234-212-21.compute-1.amazonaws.com:1026/v2/entities/urn:ngsi-ld:Temp:001";

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

const char* ssid     = "IQ8I1SE 5991";
const char* password = "080#00Qn47n";


void setup() {
  Serial.begin(9600);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");


  
  mySerial.begin(9600);
  myMHZ19.begin(mySerial);
  // Turn auto calibration ON (OFF autoCalibration(false))
  myMHZ19.autoCalibration(false);
  //Iniciar sensor DHT22
  sense.begin();

  
}

void loop() {
  
  delay(10000);
  // Tomar mediciones cada 2 segundos  
    
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

  //sendhttp(nivelCO2,"CO2",servidor,servidor2);
  sendhttp(temp,"t",servidor,servidor2);
  //sendhttp(hume,"h",servidor);
  




Serial.println("-----------------");  

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
  delay(10000);
    
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
      
  
  delay(10000);
  
}
