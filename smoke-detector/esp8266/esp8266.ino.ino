/*
    > Project name: DEMBoost DEMWatch
    > Description:  Micro-service (REST API) that exposes the state of the smoke sensor, i.e. its analog values taken
    > Usage:        in browser http://Raspberrypi_IP:80/ VS in terminal #curl -X GET  http://Raspberrypi_IP:80/ 

    > Version:      2.0.1
    > Maintainer:   Nanamou
    > Last update:  06/08/2021
    > Status:       Test environnement

    > Pre-requisites:   GNU/Linux 
                        sudo apt-get update -y
                        sudo apt-get install python3.6 -y
                        sudo apt install python3-pip
                        sudo pip install Flask -y
                        sudo pip install Flask-RestPlus -y
                        pinout
                
    > Test command:     curl -X GET http://IP_raspberry:1234/


*/


//Importation des bibliothèques
#include "Arduino.h"
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

//Definition des entrées/sorties
#define PIN_MQ2_ANALOG A0 //n'a pas de correspondance GPIO sur la Raspberry Pi, on le récupère directement par PIN_MQ2_ANALOG 
#define PIN_MQ2_DIGITAL 5 //correspondance : la pin digitale D1 de l'ESP8266 = la pin 5 (GPIO 5) de la Raspberry Pi

//Informations de connexion (à changer par celles du Point d'Accès WiFi)
const char * ssid = "NomDeMonWiFi";
const char * password = "MotDePasseDeMonWifi";

//Déclaration des variables et prototypes des fonctions
void getSensorState();
void onConnected(const WiFiEventStationModeConnected& event);
void onGotIP(const WiFiEventStationModeGotIP& event);

const char * Username_CapteurLabo1 = "CapteurLabo1";
const char * Password_CapteurLabo1 = "CapteurLabo1Passord!";
ESP8266WebServer server(80);

//Fonction qui retourne les valeurs du capteur par HTTP
void getSensorState() {
  char responseMessage[255];
  sprintf(responseMessage, "{\"value\": \"%d\"}", analogRead(PIN_MQ2_ANALOG));
  server.send(200, "text/json", responseMessage);
}


// Routing de l'API
void restServerRouting() {
  server.on("/api", HTTP_GET, []() {
    server.send(200, F("text/html"),
                F("Documentation de notre API sur le /api ..."));
  });
  server.on(F("/"), HTTP_GET, getSensorState);
}

// server.args() rensvoyant une valeur differente à chaque fois, ,maintenir ce code commenté créera une petite latente ...
/*
  //Traitement des erreurs sur l'URL
  void handleNotFound() {
  String message = "Fichier non trouvé\n\n";
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
  }
*/

void setup() {
  //Mise en place d'une liaison serie
  Serial.begin(115200);
  Serial.println("");

  //Définition du nom de l'objet sur le réseau. Ce nom répresente l'IP de l'objet. On peut ainsi faire par exemple :
  //ping CapteurLabo1
  //http://CapteurLabo1:80
  //WiFi.hostname(Username_CapteurLabo1);
  WiFi.hostname("ESP-host"); 
  
  // Mode de connexion
  WiFi.mode(WIFI_STA);


  //Connexion au WiFi avec IP statique (choisir une IP non utilisée dans le réseau pour éviter les collision)
  //IPAddress ip(192, 168, 1, 102 );
  //IPAddress gateway(192, 168, 1, 1);
  //IPAddress subnet(255, 255, 255, 0);

  // Démarrage de la connexion
  //WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);

  //Methodes de gestion d'évènements liés au WiFI
  static WiFiEventHandler onConnectedHandler = WiFi.onStationModeConnected(onConnected);
  static WiFiEventHandler onGotIPHandler = WiFi.onStationModeGotIP(onGotIP);

  // Activation du mDNS pour pouvoir se connecter à l'ESP8266 via le local DNS hostmane: http://esp8266.local
  if (MDNS.begin("esp8266")) {
    Serial.print("L'ESP8266 est entrain de se connecter au WiFi : ");
    Serial.println(ssid);
    Serial.print("dont le mot de passe est : ");
    Serial.println(password);
  }

  // Set server routing
  restServerRouting();
  //Set not found response
  //server.onNotFound(handleNotFound);
  //Start server
  server.begin();
  //Serial.println("Le serveur HTTP a démarré");

  //Configuration des E/S
  pinMode(PIN_MQ2_ANALOG, INPUT);
  //pinMode(PIN_MQ2_DIGITAL, INPUT);

  //Information pour téléverser un programme en WiFi (Over The Air)
  ArduinoOTA.setHostname(Username_CapteurLabo1);
  ArduinoOTA.setPassword(Password_CapteurLabo1);
  ArduinoOTA.begin();
}

void loop() {
  //Téléverser un programme en WiFi (Over The Air)
  ArduinoOTA.handle();

  server.handleClient();
}

void onConnected(const WiFiEventStationModeConnected& event) {
  Serial.println("L'ESP8266 s'est connecté avec succès au Wi-Fi.");
}

void onGotIP(const WiFiEventStationModeGotIP& event) {
  //Affichage des infosrmations
  Serial.println("");
  Serial.print("Utilisez cette adresse URL pour vous connecter à l'ESP8266 : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP().toString());
  Serial.println(":80/");
  Serial.println("");
  Serial.println("");

  Serial.println("Informations sur le réseau");
  Serial.println("-------------------------------------------");

  Serial.print(" > Nom du capteur      : ");
  Serial.println(WiFi.hostname());
  Serial.print(" > Adresse MAC         : ");
  Serial.println(WiFi.macAddress());
  Serial.println(" > Adresse IP          : " + WiFi.localIP().toString());
  Serial.print(" > Masque Réseau       : ");
  Serial.println(WiFi.subnetMask());
  Serial.println(" > Passerelle          : " + WiFi.gatewayIP().toString());
  Serial.println(" > DNS                 : " + WiFi.dnsIP().toString());
  Serial.print(" > Canal               : ");
  Serial.println(WiFi.channel());
  Serial.print(" > Force du signal     : ");
  Serial.println(WiFi.RSSI());
}
