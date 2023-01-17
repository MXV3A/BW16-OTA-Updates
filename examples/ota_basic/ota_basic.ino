
#include <WiFi.h>
#include <AnchorOTA.h>

char ssid[] = "yourNetwork";
char pass[] = "yourPassword";

#define MY_VERSION_NUMBER 1
#define OTA_PORT 8082


void setup() {
  Serial.begin(115200);
  Serial.println("This is version %d\r\n\r\n", MY_VERSION_NUMBER);

  Serial.println("Try to connect to %s\r\n", ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.println("Failed. Wait 1s and retry...\r\n");
    delay(1000);
  }
  Serial.println("Connected to %s\r\n", ssid);

  // Broadcast mDNS service at OTA_PORT that makes Arduino IDE find Ameba device
  OTA.beginArduinoMdnsService("MyAmeba", OTA_PORT);

  // Listen at OTA_PORT and wait for client (Eq. Arduino IDE). Client would send OTA image and make a update.
  if (OTA.beginLocal(OTA_PORT) < 0) {
    Serial.println("OTA failed!!\r\n");
  }
}

void loop() {
	Serial.println(".");
  delay(1000);
}
