#include "CD74HC4067.h"
#include <Arduino.h>

#define S0 1 // GPIO1
#define S1 3 // GPIO3
#define S2 5 // GPIO5
#define S3 4 // GPIO4



void CD74HC4067_init() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
}
int get_val_from_CD74HC4067(int channel, int analog_read_pin) {
  digitalWrite(S0, (channel & 0x01) ? HIGH : LOW);
  digitalWrite(S1, (channel & 0x02) ? HIGH : LOW);
  digitalWrite(S2, (channel & 0x04) ? HIGH : LOW);
  digitalWrite(S3, (channel & 0x08) ? HIGH : LOW);
  return analogRead(analog_read_pin);
}