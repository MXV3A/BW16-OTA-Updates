/*
  This sketch shows how to enable OTA service and proceed your task at the same time

  For how to use OTA, please refer sketch "ota_basic".

*/

#include <WiFi.h>
#include <AnchorOTA.h>

char ssid[] = "yourNetwork";
char pass[] = "yourPassword";

#define MY_VERSION_NUMBER 1
#define OTA_PORT 8082
IPAddress ip = {192,168,32,14};


void setup() {
  Serial.begin(115200);
  Serial.println("This is version %d\r\n\r\n", MY_VERSION_NUMBER);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.println("Failed. Wait 1s and retry...\r\n");
    delay(1000);
  }
  Serial.println("Connected to %s\r\n", ssid);
  os_thread_create(ota_thread, NULL, OS_PRIORITY_REALTIME, 4096);
}

void loop() {
  delay(1000);
  Serial.println("Current system tick: %d\r\n", millis());
}

void ota_thread(const void *) {
  while(true){
    if (OTA.beginCloud(ip, OTA_PORT) < 0) {
      Serial.println("OTA failed!!\r\n");
    }
    delay(10000);
  }
}
