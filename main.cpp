#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>
#include <UniversalTelegramBot.h> 
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h> // Incluimos la librería para hacer solicitudes HTTP
#include "apwifiesp32.h"

// Variables para las credenciales de Wi-Fi
String wifiSSID = "";
String wifiPassword = "";

// Telegram Bot credenciales
#define BOTtoken "7107533091:AAGerbYeymm4uvcHp2pAzj0uBk3RorqjXZI"
#define CHAT_ID "6317403904"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Servo configuracion
Servo myservo;
static const int servoPin = 13;

// Sensor configuracion
const int trigPin = 32;
const int echoPin = 33;
const int pirPin = 25;
const int ledPin = 26;

volatile bool pirDetected = false;
volatile bool checkDistanceFlag = false;

// Timer configuracion
hw_timer_t *timer = NULL;

// LCD configuracion
LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);

// variables
bool showMarissa = true;
unsigned long lastPirDetection = 0;
unsigned long lastDistanceCheck = 0;
//const unsigned long pirInterval = 6000; // 6 segundos
const unsigned long distanceInterval = 15000; // 15 segundos
const unsigned long marissaInterval = 10000; // 10 segundos

unsigned long feedStartTime = 0;
bool feeding = false;

// Ubidots configuracion
const char* ubidotsToken = "BBUS-5jIRvXpKvIdBMCqHpHA5ssWucWzFH4"; // tu token de Ubidots
const char* deviceLabel = "device1"; // dispositivo en Ubidots
const char* variableLabel = "veces_comida"; // variable en Ubidots

// Funcion para enviar a Ubidots
void sendToUbidots(int veces_comida) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://industrial.api.ubidots.com/api/v1.6/devices/" + String(deviceLabel);
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Auth-Token", ubidotsToken);

    String payload = "{\"" + String(variableLabel) + "\": " + String(veces_comida) + "}";

    int httpResponseCode = http.POST(payload);
    
    if (httpResponseCode > 0) {
      Serial.println("Dato enviado a ubidots" ); //+ String(httpResponseCode)
    } else {
      Serial.println("Error enviando info a Ubidots");
    }

    http.end();
  } else {
    Serial.println("Error conexion WiFi");
  }
}

// contador del comando "/comida"
int comidaCounter = 0;

// Funcion de alimentar
void feed() {
  feeding = true;
  feedStartTime = millis();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alimentando");
  digitalWrite(ledPin, HIGH);
  bot.sendMessage(CHAT_ID, "Alimentando", "");
  myservo.write(90);
  delay(2000);
  myservo.write(0);
  digitalWrite(ledPin, LOW);
  lcd.clear();
  // Incrementa contador para enviar a ubidots
  comidaCounter++;
  sendToUbidots(comidaCounter);
}

// Function to handle new messages
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Usuario no autorizado", "");
      continue;
    }

    String text = bot.messages[i].text;

    if (text == "/comida" && !feeding) {
      feed();
      showMarissa = false; // no muestra "MARISSA" durante la funcion de alimentar
      

    }
  }
}



// Function para leer la distancia del ultrasonico
float readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;

  return distance;
}
// Interrupt service routine for Timer
void IRAM_ATTR onTimer() {
  checkDistanceFlag = true;
}
// Interrupt service routine for PIR sensor
void IRAM_ATTR pirISR() {
  pirDetected = true;
}


void setup() {
  // Sensor pin configuraciones
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pirPin, INPUT_PULLUP);

  // LED configuracion
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);
  myservo.attach(servoPin);

  // Inicializa LCD
  while (lcd.begin(16, 2) != 1) {
    Serial.println(F("LCDI2C no está conectado o la declaración de pines LCD es incorrecta."));
    delay(5000);
  }

  // Inicializa el AP y espera a que las credenciales se configuren
  initAP("espAP", "123456789");
  while (wifiSSID == "" || wifiPassword == "") {
    loopAP(); // Maneja solicitudes entrantes para obtener credenciales
    delay(1000); // Espera 1 segundo antes de intentar de nuevo
  }

  // Connectar a WiFi
  Serial.print("Conectando a ");
  Serial.println(wifiSSID);
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado.");
  bot.sendMessage(CHAT_ID, "Bot iniciado", "");

  // Primer mensaje de LCD
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Bot iniciado");
  lcd.setCursor(4, 1);
  lcd.print("Mel & Emi");
  
 // Attach interrupt to PIR sensor
  attachInterrupt(digitalPinToInterrupt(pirPin), pirISR, RISING);

  // Timer configuration for distance check
  timer = timerBegin(0, 80, true); // 80 prescaler for 1MHz timer frequency
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 25000000, true); // 20 segundos intervalo
  timerAlarmEnable(timer);
 
}

void loop() {
  unsigned long currentMillis = millis();

  // Handle new messages
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
  // Muestra el nombre solo si ha pasado suficiente tiempo
  static unsigned long lastMarissaDisplay = 0;
  if (showMarissa && (currentMillis - lastMarissaDisplay >= marissaInterval)) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("MARISSA VET");
    lcd.setCursor(6, 1);
    lcd.print("^.^");
    lastMarissaDisplay = currentMillis; // Actualiza el último momento en que se mostró Marissa
  }
  // Handle PIR detection
  if (pirDetected) {
    pirDetected = false;
   // if (currentMillis - lastPirDetection >= pirInterval) {
    bot.sendMessage(CHAT_ID, "Tu Michi está aquí, dale de comer", "");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Michi detectado");
    //lastPirDetection = currentMillis; // Actualiza ultima detección
    //}
  }

  // Evaluar distancia
  if (checkDistanceFlag) {
    checkDistanceFlag = false;
    float distance = readDistance();
    if (distance > 7) {
      if (currentMillis - lastDistanceCheck >= distanceInterval) {
        bot.sendMessage(CHAT_ID, "Rellenar recipiente", "");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("AVISO!!!");
        lcd.setCursor(0, 1);
        lcd.print("Poca comida!");
        delay(3000);
        lastDistanceCheck = currentMillis; // actualiza el ultimo chequeo de distancia
      }
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Detecta comida a:");
      lcd.setCursor(0, 1);
      lcd.print(distance);
      lcd.print(" cm");
    }
  }

  // Manejo de la función de alimentación
  if (feeding) {
    if (currentMillis - feedStartTime >= 1000) { // Espera 2 segundos para completar la acción
      showMarissa = true; // muestra "MARISSA" después de la función de alimentar
      feeding = false; // Termina la alimentación
      
    }
  }


}
