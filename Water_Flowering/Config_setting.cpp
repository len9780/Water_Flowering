#include "Config_setting.h"
#include <Arduino.h>
File file;
FSInfo fs_info;
char *write_config(File f, unsigned char rw, char *dat) {
  char *r = NULL;
  f.seek(0);
  if (rw == write) {
    Serial.print("write_config write data:");
    Serial.println(dat);
    f.println(dat);
    f.close();
    f = SPIFFS.open("/config.txt", "r+");
    return r;
  } else {
    int n = 0;
    int i = 0;
    char *s;
    while (f.available()) {
      f.read();
      n++;
    }
    s = (char *)malloc(n);
    // Serial.println("file content size:" + String(n));
    f.seek(0);

    Serial.print(String(f.readBytes(s, n)));
    return s;
  }
}
