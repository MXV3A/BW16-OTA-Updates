
#include <WiFi.h>
#include <AnchorOTA.h>

char ssid[] = "yourNetwork";
char pass[] = "yourPassword";

#define OTA_PORT 8082

// Address of device where this board can fetch its updates
IPAddress ip = {192,168,32,14};

void setup() {
  Serial.begin(115200);

  Serial.println("Try to connect to WiFi");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  os_thread_create_arduino(otaThread, NULL, OS_PRIORITY_REALTIME, 4096);
}

void otaThread(const void *){
  while(true){
    if (OTA.beginCloud(ip, OTA_PORT) < 0) {
    Serial.println("OTA failed!");
    }
    delay(10000);
  }
}

void loop() {
  // Heartbeat
  Serial.println(WiFi.localIP());
  delay(5000);
}