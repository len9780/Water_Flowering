#include "CD74HC4067.h"
#include <Arduino.h>

#define S0 14 // GPIO14,D5
#define S1 12 // GPIO12,D6
#define S2 5 // GPIO5
#define S3 4 // GPIO4



void CD74HC4067_init() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
}
int get_val_from_CD74HC4067(int channel, int analog_read_pin) {
  digitalWrite(S0, (channel & 0x01)>0 ? HIGH : LOW);
  digitalWrite(S1, (channel & 0x02)>0 ? HIGH : LOW);
  digitalWrite(S2, (channel & 0x04)>0 ? HIGH : LOW);
  digitalWrite(S3, (channel & 0x08)>0 ? HIGH : LOW);
  return analogRead(analog_read_pin);
}

int get_soil_moisture_A0(){
  int r = get_val_from_CD74HC4067(1,A0);
  Serial.println("soil_moisture_A0:"+String(r));
  return r;
}
int get_soil_moisture_D0(){
int r = get_val_from_CD74HC4067(0,A0);
  Serial.println("soil_moisture_D0:"+String(r));
  return r;
}

int get_water_leve(){
  int r = get_val_from_CD74HC4067(2,A0);
  Serial.println("Water_Level:"+String(r));
  return r;
}