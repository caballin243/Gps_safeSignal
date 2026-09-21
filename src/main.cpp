#include <Arduino.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <PubSubClient.h>

//-----wifi-----
const char* wifi_name = "LAPTOP_Caballin243";
const char* wifi_pasword = "12345678";
WiFiClient clienteWifi;

//-----gps-----
#define rx2 16
#define tx2 17
TinyGPSPlus gps;
HardwareSerial ss(2);
unsigned long ultimoEnvio = 0;

//-----mqtt-----
const char* topic_Ubicacion_Vehiculo = "Vehiculos/Espacio_1";
const int puerto = 1883;
const char* host = "192.168.137.1";
const char* id_vehiculo = "id_bomberos";
PubSubClient clientmqtt(clienteWifi);

void reconnectar_wifi() {
  WiFi.begin(wifi_name,wifi_pasword);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("Reconectando");
    delay(5000);
      if(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
      }
  }
  Serial.println("");
  Serial.println("Conexion lograda");
}

void reconectar_mqtt(){
  while (!clientmqtt.connected()) {
    Serial.print("conectandose...");
    if (clientmqtt.connect(id_vehiculo)) {
      Serial.println("Conectado. ");
    } else {
      Serial.print("ERROR, rc=");
      Serial.print(clientmqtt.state());
      Serial.println(" Intentando de nuevo en 5 seg");
      delay(5000);
    }
  }
}

void enviarJson(double lat, double lon) {
    String json = "{\"lat\":" + String(lat, 6) + ",\"lon\":" + String(lon, 6) + "}";
    
    Serial.println(json);
    clientmqtt.publish(topic_Ubicacion_Vehiculo,json.c_str());
}


void setup(){
  Serial.begin(115200);
  ss.begin(9600,SERIAL_8N1,rx2,tx2);
  Serial.println("Buscando Satelites gps...");
  Serial.println("salga al exterior para mejor exactitud...");

//iniciar wifi
  WiFi.begin(wifi_name,wifi_pasword);
  clientmqtt.setServer(host,puerto);
}

void loop(){
  clientmqtt.loop();
  if (WiFi.status() != WL_CONNECTED){
    reconnectar_wifi();
  }
  if (!clientmqtt.connected()){
    reconectar_mqtt();
  }
  
  

  while (ss.available() > 0) {
    char ubi = ss.read();
    if (gps.encode(ubi)){
      if(gps.location.isValid() && millis()-ultimoEnvio >=2000){
        double latitud = gps.location.lat();
        double longitud = gps.location.lng();

        Serial.print("Latitud: ");
        Serial.println(latitud,6);
        Serial.println("Longitud: ");
        Serial.println(longitud,6);

        Serial.print("Satélites detectados: ");
        Serial.println(gps.satellites.value());
        Serial.println("---------------------------");
        enviarJson(latitud,longitud);

        ultimoEnvio = millis();
      }
      else{
        Serial.println("datos invalidos");
      }
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("Error: No se detecta el módulo GPS. Revisa tus cables.");
    delay(2000);
  }
}
