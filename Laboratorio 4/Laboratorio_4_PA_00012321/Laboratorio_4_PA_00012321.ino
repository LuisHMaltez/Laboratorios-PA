#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/
#define WLAN_SSID "ARTEFACTOS"
#define WLAN_PASS "ARTEFACTOS"

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883 // usar 8883 para SSL
#define AIO_USERNAME "LuisMaltez"
#define AIO_KEY "aio_pIoN76vcJO10Vy0mZSWeniZ22ozj"

/************ Global State ***************************************************/
// Cliente WiFi para MQTT
WiFiClient client;

// Cliente MQTT con credenciales
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ****************************************/
// Publicación
Adafruit_MQTT_Publish luminucidad = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Gauge");

// Suscripción
Adafruit_MQTT_Subscribe buton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/boton_led", MQTT_QOS_1);

/****************************** Pines ****************************************/
int pinLDR = 32;
int led = 34;

/*************************** Callbacks ***************************************/
void slidercallback(double x) {
  Serial.print("Hey, slider value: ");
  Serial.println(x);
}

void butoncallback(char *data, uint16_t len) {
  Serial.print("Hey, button value: ");
  Serial.println(data);

  String message = String(data);
  message.trim();

  if (message == "ON") {
    digitalWrite(led, HIGH);
  }
  if (message == "OFF") {
    digitalWrite(led, LOW);
  }
}

/*********************************************** Setup *****************************/
void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  delay(10);

  /************ Conexión WiFi ************/
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /************ Callbacks ************/
  buton.setCallback(butoncallback);

  /************ Suscripciones ************/
  mqtt.subscribe(&buton);
}

/************************************************* Loop *****************************/
void loop() {
  MQTT_connect();
  mqtt.processPackets(10000);

  if (!mqtt.ping()) {
    mqtt.disconnect();
  }

  int valorLDR = analogRead(pinLDR);

  Serial.print(F("\nEnviando valor a feed Gauge: "));
  Serial.print(valorLDR);
  Serial.print("...");
  if (!luminucidad.publish((int32_t)valorLDR)) {
    Serial.println(F(" Fallido!"));
  } else {
    Serial.println(F(" Éxito!"));
  }

  delay(500);
}

/********************* Función de conexión MQTT ******************************/
void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) { // 0 = conectado
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 10 seconds...");
    mqtt.disconnect();
    delay(10000);
    retries--;
    if (retries == 0) {
      while (1); // esperar reset por watchdog
    }
  }
  Serial.println("MQTT Connected!");
}
