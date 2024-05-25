#include "http_server.h"
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>


#include "Config_setting.h"
ESP8266WebServer http_server(80);

extern File file;
void http_server_task() {
  if (http_server.method() == HTTP_POST) {
    char s[50];
    StaticJsonDocument<200> doc;
    StaticJsonDocument<200> config_json;
    deserializeJson(doc, http_server.arg("plain"));
    if (doc["ssid"] && doc["passwd"]) {
      String output;
      sprintf(s, "ssid:%s\npasswd:%s", String(doc["ssid"]),
              String(doc["passwd"]));
      deserializeJson(config_json, write_config(file, read, NULL));
      config_json["ssid"] = String(doc["ssid"]);
      config_json["passwd"] = String(doc["passwd"]);
      serializeJson(config_json, output);
      write_config(file, write, (char *)output.c_str());
      http_server.send(200, "text/plain", (char *)output.c_str());
      // server.send(200, "text/plain", s);
      delay(200);
      ESP.restart();
      // server.send(200, "text/plain", message);
    } else {
      char message[50];
      sprintf(message, "%s", "{\"result\":1}");
      Serial.println(message);
      http_server.send(200, "text/plain", message);
    }
  }
}
// void http_server_task() {}