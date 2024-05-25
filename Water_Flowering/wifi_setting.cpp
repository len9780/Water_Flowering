
#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "wifi_setting.h"
void startap(char *ssid, char *passwd) {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, passwd);
  Serial.print("WIFI_AP(IP):");
  Serial.println(WiFi.softAPIP());
}
void startsta(char *ssid, char *passwd) {
  WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    // Serial.println("Connecting to WiFi...");
  }
  Serial.print("WIFI_STA(IP):");
  Serial.println(WiFi.localIP());
}
