#include <MHZ19.h>  //Libreria de sensor MH-Z19B
#include <SoftwareSerial.h> //Libreria de comunicacion serial
#include <DHT.h> // Libreria para sensor de DHT22 de humedad y temperatura


#include <WiFi.h>
#include <time.h>

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

//Relee
#define RELAY_PIN 18

// Contador para temporizar las mediciones
unsigned long timer = 0;

//Timestamp

const char* ssid     = "IQ8I1SE 5991";
const char* password = "080#00Qn47n";

const char* NTP_SERVER = "ch.pool.ntp.org";
const char* TZ_INFO    = "CST6CDT";  // enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)

tm timeinfo;
time_t now;
long unsigned lastNTPtime;
long unsigned SetTiming;
unsigned long lastEntryTime;

void setup() {
  Serial.begin(9600);
  
  pinMode(RELAY_PIN, OUTPUT);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  configTime(0, 0, NTP_SERVER);
  // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
  setenv("TZ", TZ_INFO, 1);

  if (getNTPtime(10)) {  // wait up to 10sec to sync
  } else {
    Serial.println("Time not set");
    ESP.restart();
  }
  
  SetTiming = time(&now);
  lastEntryTime = millis();

  
  mySerial.begin(9600);
  myMHZ19.begin(mySerial);
  // Turn auto calibration ON (OFF autoCalibration(false))
  myMHZ19.autoCalibration(false);
  //Iniciar sensor DHT22
  sense.begin();

  
}

void loop() {

  digitalWrite(RELAY_PIN, HIGH);
  
  delay(2500);
  // Tomar mediciones cada 2 segundos
  digitalWrite(RELAY_PIN, LOW);
  delay(2500);

  lastNTPtime = time(&now);
  
  if (lastNTPtime-SetTiming == 10){
    Serial.println("Enviando consignas");
    SetTiming = lastNTPtime;
    sensorica();
    Serial.println("Enviado");
  }


}

void sensorica() {

  lastNTPtime = time(&now);

  //Estructura de tiempo
  showTime(timeinfo); 
    
  // Obtener la medición de CO2 actual como ppm
  int nivelCO2 = myMHZ19.getCO2();
  hume = sense.readHumidity();
  temp = sense.readTemperature();

  // Mostrar el nivel de CO2 en el monitor serie
  Serial.print("CO2 (ppm): ");                      
  Serial.println(nivelCO2);  

  // Mostrar la temperatura en el monitor serie
  Serial.print("Temperatura (C): ");                  
  Serial.println(temp); 

  Serial.print("Humedad (%): ");                  
  Serial.println(hume);

  Serial.println("-----------------");
    

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

void showTime(tm localTime) {
  Serial.print(localTime.tm_mday);
  Serial.print('/');
  Serial.print(localTime.tm_mon + 1);
  Serial.print('/');
  Serial.print(localTime.tm_year - 100);
  Serial.print('-');
  Serial.print(localTime.tm_hour);
  Serial.print(':');
  Serial.print(localTime.tm_min);
  Serial.print(':');
  Serial.print(localTime.tm_sec);
  Serial.print(" Day of Week ");
  if (localTime.tm_wday == 0)   Serial.println(7);
  else Serial.println(localTime.tm_wday);
}
