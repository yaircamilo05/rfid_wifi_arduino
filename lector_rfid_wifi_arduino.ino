#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

// Pines para el lector RFID (ajusta según tu configuración)
#define RST_PIN 2  // Pin de reset del RFID
#define SS_PIN  15  // Pin de selección del RFID

const char* ssid = "Familia Mailo";         // Nombre de la red WiFi
const char* password = "60367199";          // Contraseña de la red WiFi

const char* serverUrl = "http://192.168.1.7:3000/api/login";  // IP del servidor y endpoint

WiFiClient client;  // Crear un objeto WiFiClient
MFRC522 rfid(SS_PIN, RST_PIN);  // Crear un objeto para el lector RFID

void setup() {
  Serial.begin(115200); // Iniciar la comunicación serial
  SPI.begin();          // Iniciar comunicación SPI
  rfid.PCD_Init();      // Iniciar el lector RFID

  // Conectar a la red WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a WiFi");
}

void loop() {
  // Verificar si se detecta una tarjeta RFID
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // Obtener el ID de la tarjeta (UID)
    String rfidTag = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      rfidTag += String(rfid.uid.uidByte[i], HEX);  // Convertir el byte del UID a HEX
    }

    Serial.print("Tarjeta detectada con UID: ");
    Serial.println(rfidTag);

    // Llamar a la función que hace la solicitud POST al servidor
    enviarPostRequest(rfidTag);

    // Detener la lectura de la tarjeta para evitar múltiples lecturas del mismo ID
    rfid.PICC_HaltA();
  }

  delay(1000);  // Esperar 1 segundo antes de volver a comprobar
}

// Función para enviar la solicitud POST al servidor
void enviarPostRequest(String rfidTag) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Iniciar la conexión con el servidor usando WiFiClient
    http.begin(client, serverUrl);  // Usar WiFiClient y URL del servidor
    http.addHeader("Content-Type", "application/json");  // Encabezado para enviar JSON

    // JSON que vamos a enviar con el RFID
    String jsonData = "{\"rfid\":\"" + rfidTag + "\"}";  // Crear JSON con el UID de la tarjeta RFID

    // Hacer la solicitud POST y recibir la respuesta
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      // La solicitud fue exitosa, imprimir el código de respuesta y la respuesta del servidor
      String response = http.getString();
      Serial.print("Código de respuesta: ");
      Serial.println(httpResponseCode);
      Serial.print("Respuesta del servidor: ");
      Serial.println(response);
    } else {
      // Si la solicitud falló, imprimir el error
      Serial.print("Error en la solicitud POST, código: ");
      Serial.println(httpResponseCode);
    }

    http.end();  // Terminar la conexión con el servidor
  } else {
    Serial.println("Error al conectar a WiFi");
  }
}
