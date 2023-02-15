
#include <WiFi.h>
#include <AnchorOTA.h>

char ssid[] = "yourNetwork";
char pass[] = "yourPassword";

#define OTA_PORT 8082

void setup() {
  Serial.begin(115200);

  Serial.println("Try to connect to WiFi");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  // Send OTA Update to this IP
  Serial.println(WiFi.localIP());

  // Broadcast mDNS service at OTA_PORT that makes Arduino IDE find Ameba device
  OTA.beginArduinoMdnsService("Ameba Device", OTA_PORT);

  os_thread_create_arduino(otaThread, NULL, OS_PRIORITY_REALTIME, 4096);
}

void otaThread(const void *){
  while(true){
    if(OTA.beginLocal(OTA_PORT) < 0 ) {
      Serial.println("Failed OTA Update");
    }
    delay(10000);
  }
}

void loop() {
  // Heartbeat
  Serial.println(WiFi.localIP());
  delay(5000);
}