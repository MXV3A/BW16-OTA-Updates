
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

  // Connect to OTA server OTA_PORT and wait for image (Eq. Arduino IDE). Server would send OTA image and make a update.
  
  IPAddress ip = {192,168,32,14};
  if (OTA.beginCloud(ip, OTA_PORT) < 0) {
    Serial.println("OTA failed!!\r\n");
  }
}

void loop() {
  delay(1000);
  Serial.println(".");
}
