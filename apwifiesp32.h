// apwifiesp32.h
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

extern String wifiSSID;
extern String wifiPassword;

WebServer server(80);

void handleRoot() {
  String html = "<html><body>";
  html += "<form method='POST' action='/wifi'>";
  html += "Red Wi-Fi: <input type='text' name='ssid'><br>";
  html += "Contraseña: <input type='password' name='password'><br>";
  html += "<input type='submit' value='Conectar'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleWifi() {
  wifiSSID = server.arg("ssid");
  wifiPassword = server.arg("password");

  Serial.print("Conectando a la red Wi-Fi ");
  Serial.println(wifiSSID);
  Serial.print("Clave Wi-Fi ");
  Serial.println(wifiPassword);
  Serial.print("...");

  WiFi.disconnect(); // Desconectar la red Wi-Fi anterior, si se estaba conectado
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str(),6);
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED and cnt < 8) {
    delay(1000);
    Serial.print(".");
    cnt++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conexión establecida");
    server.send(200, "text/plain", "Conexión establecida");
  } else {
    Serial.println("Conexión no establecida");
    server.send(200, "text/plain", "Conexión no establecida");
  }
}

void initAP(const char* apSsid,const char* apPassword) { // Nombre de la red Wi-Fi y  Contraseña creada por el ESP32
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid, apPassword);

  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);

  server.begin();
  Serial.print("Ip de esp32...");
  Serial.println(WiFi.softAPIP());
  //Serial.println(WiFi.localIP());
  Serial.println("Servidor web iniciado");
}

void loopAP() {
  server.handleClient();
}
