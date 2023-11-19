/*
  ESP8266 Blink by Simon Peter
  Blink the blue LED on the ESP-01 module
  This example code is in the public domain

  The blue LED on the ESP-01 module is connected to GPIO1
  (which is also the TXD pin; so we cannot use Serial.print() at the same time)

  Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
*/
#define tcp_port 6000
#define udp_port 6005
#define Relay_Pin D1     // GPIP12:D6
#define Wet_Pin D2
#define AP_STA_SEL_PIN 14 // GPIP14:D5
#define heartbeat_interval 30000
#define mqtt_server "broker.mqtt-dashboard.com"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "WiFiUdp.h"
#include <PubSubClient.h>

// const char *ssid = "117-20";
// const char *password = "0978027009";
const char *ssid = "hotspot";
const char *password = "19891110";
const char *SoftAp_ssid = "Esp_wemos_d1_layne";
const char *SoftAP_password = "12345678";
StaticJsonDocument<200> doc;
WiFiUDP Udp;
ESP8266WebServer server(80);
WiFiClient clients[5];
bool clientConnected[5] = {false, false, false, false, false};
int maxClients = 5;
WiFiServer server_tcp(tcp_port);
WiFiClient espClient_mqtt;
PubSubClient client_mqtt(espClient_mqtt);
unsigned long t1 = millis();
void updateFirmware(const char *url)
{
  WiFiClient client;

  t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);

  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    // Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
    break;
  case HTTP_UPDATE_NO_UPDATES:
    // Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;
  case HTTP_UPDATE_OK:
    // Serial.println("HTTP_UPDATE_OK");
    break;
  }
}

void set_gpio_callback()
{
  if (server.hasArg("cmd") && (server.arg("cmd") == "setOn"))
  {
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(Relay_Pin, HIGH);
    Serial.println("setOn");
    server.send(200, "text/plain", String("setOn"));
  }
  else if (server.hasArg("cmd") && (server.arg("cmd") == "setOff"))
  {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(Relay_Pin, LOW);
    Serial.println("setOff");
    server.send(200, "text/plain", String("setOff"));
  }
  else
  {
    server.send(200, "text/plain", String("setgpio fail"));
  }
}

void handleRoot()
{

  if (server.method() == HTTP_GET)
  {

    String message = "Hello Root ";
    server.send(200, "text/plain", message);
    Serial.println(message);
  }

  else if (server.method() == HTTP_POST)
  {
    String message;
    StaticJsonDocument<200> doc;
    deserializeJson(doc, server.arg("plain"));
    if (String(doc["cmd"]) == "UpdateFirmware")
    {
      message = "{\"result\":0,\"cmd\":\"UpdateFirmware\"}";
      server.send(200, "text/plain", message);
      updateFirmware((const char *)doc["url"]);
      // server.send(200, "text/plain", message);
    }
    else if (String(doc["cmd"]) == "restart")
    {
      message = "{\"result\":0,\"cmd\":\"restart\"}";
      server.send(200, "text/plain", message);
      Serial.println(message);
      delay(200);
      ESP.restart();
    }
    else
    {
      message = "{\"result\":1}";
      Serial.println(message);
      server.send(200, "text/plain", message);
    }
  }
}
void mqtt_init()
{

  client_mqtt.setServer(mqtt_server, 1883);
  client_mqtt.setCallback(callback);
}
void tcp_nonblock_task()
{
  // 接受新連線
  WiFiClient newClient = server_tcp.available();
  if (newClient)
  {
    // 尋找空閒位置
    int index = -1;
    for (int i = 0; i < maxClients; i++)
    {
      if (!clientConnected[i])
      {
        index = i;
        break;
      }
    }
    // 如果沒有空閒位置就拒絕連線
    if (index == -1)
    {
      // check if connected client is still connected
      for (int i = 0; i < maxClients; i++)
      {
        if (clientConnected[i] && !clients[i].connected())
        {
          clientConnected[i] = false;
          index = i;
        }
      }
      if (index == -1)
      {
        newClient.stop();
        Serial.println("Too many clients");
      }
    }
    else
    {
      // 新客戶端連線成功
      Serial.print("New client connected: ");
      Serial.println(newClient.remoteIP());
      clients[index] = newClient;
      clientConnected[index] = true;
    }
  }

  // 檢查現有客戶端是否有發送訊息
  for (int i = 0; i < maxClients; i++)
  {
    if (clientConnected[i] && clients[i].available())
    {
      Serial.print("Message from client ");
      Serial.print(i);
      Serial.print(": ");
      while (clients[i].available())
      {
        char c = clients[i].read();
        Serial.print(c);
      }
      Serial.println();
      // 將訊息回傳給客戶端
      clients[i].print("OK");
    }
  }
}
void tcp_task()
{
  // 等待客户端连接
  WiFiClient client = server_tcp.available();
  if (!client)
  {
    // return;
  }
  else
  {

    // 处理客户端请求
    // Serial.println("New client");
    while (client.connected())
    {
      if (client.available())
      {
        String request = client.readStringUntil('\r');
        Serial.println("req:" + request);
      }
      if (WiFi.getMode() == WIFI_STA)
      {
        mqtt_loop();
        udp_echo();
      }
      server.handleClient();
    }
  }
  // 关闭连接
  // delay(1);
  // client.stop();
  // Serial.println("Client disconnected");
}
void web_init()
{
  server.on("/", handleRoot);
  server.on("/setgpio", set_gpio_callback);
  server.on("/info", device_info);
  // server.on("/network_setting", network_setting);
  server.begin();
}
void device_info()
{
  StaticJsonDocument<200> doc;
  doc["ip"] = WiFi.localIP();
  byte mac[6];
  WiFi.macAddress(mac);
  char mac_s[20] = {};
  sprintf(mac_s, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1],
          mac[2], mac[3], mac[4], mac[5]);

  // 加入一個名稱為 "age" 的整數欄位
  doc["mac"] = mac_s;
  // 將 JSON 物件轉換成字串輸出
  String jsonString;
  serializeJson(doc, jsonString);
  Serial.println(jsonString);
  server.send(200, "text/html", jsonString);
}
void boot_up_blink(int blink_count,int interval_ms)
{
  digitalWrite(LED_BUILTIN, LOW);

  for(int i =0;i<blink_count;i++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(interval_ms);
    digitalWrite(LED_BUILTIN, LOW);
    delay(interval_ms);
  }
  
}
void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
  pinMode(Relay_Pin, OUTPUT);   // Initialize the LED_BUILTIN pin as an output
  pinMode(Wet_Pin, INPUT); 
  pinMode(AP_STA_SEL_PIN, INPUT_PULLUP);
  boot_up_blink(3,300);
  if (digitalRead(AP_STA_SEL_PIN))
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      // Serial.println("Connecting to WiFi...");
    }
    Serial.print("WIFI_STA(IP):");
    Serial.println(WiFi.localIP());
    mqtt_init();
  }
  else
  {
    boot_up_blink(5,100);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(SoftAp_ssid, SoftAP_password);
    Serial.print("WIFI_AP(IP):");
    Serial.println(WiFi.softAPIP());
  }
  server_tcp.begin();
  Udp.begin(udp_port);
  web_init();
}

void heartbeat()
{
  static long int cnt = 0;
  if (millis() - t1 > heartbeat_interval)
  {
    t1 = millis();
    char s[50];
    sprintf(s, "hello:%ld\n", cnt);
    client_mqtt.publish("msgTopic", s);
    Serial.print(String(s));
    cnt++;
    // delay(1000);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  String s;
  StaticJsonDocument<200> doc;
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    s += (char)payload[i];
  }
  deserializeJson(doc, s);
  Serial.println(s);

  // Switch on the LED if an 1 was received as first character
  if (String(doc["cmd"]) == "setOn")
  {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
    digitalWrite(Relay_Pin, HIGH);
  }
  else if (String(doc["cmd"]) == "setOff")
  {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
    digitalWrite(Relay_Pin, LOW);
  }
}
void udp_echo()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // 讀取封包資料
    char packetData[packetSize + 1];
    int len = Udp.read(packetData, packetSize);
    packetData[len] = 0;

    // 處理封包資料
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    // ReplyBuffer = "Ack From" + ip.toString() + " MAC:" + mac2String(mac);
    Udp.write(packetData);
    Udp.endPacket();
    Serial.println(packetData);
  }
}
void mqtt_loop()
{
  if (!client_mqtt.connected())
  {
    reconnect();
  }
  client_mqtt.loop();
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client_mqtt.connected())
  {
    // Serial.print("Attempting MQTT connection...");
    //  Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client_mqtt.connect(clientId.c_str()))
    {
      // Serial.println("connected");
      //  Once connected, publish an announcement...
      client_mqtt.publish("msgTopic", "hello world");
      // ... and resubscribe
      client_mqtt.subscribe("cmdTopic");
    }
    else
    {
      // Serial.print("failed, rc=");
      // Serial.print(client_mqtt.state());
      // Serial.println(" try again in 5 seconds");
      //  Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
// the loop function runs over and over again forever
void loop()
{

  if (WiFi.getMode() == WIFI_STA)
  {
    // tcp_task();
    tcp_nonblock_task();
    udp_echo();
    mqtt_loop();
    heartbeat();
    // client_mqtt.publish("esp8266/test", "Hello from ESP8266");
  }
  else if (WiFi.getMode() == WIFI_AP)
  {
    // tcp_task();
    tcp_nonblock_task();
    udp_echo();
    // mqtt_loop();
    // heartbeat();
  }
  server.handleClient();
}