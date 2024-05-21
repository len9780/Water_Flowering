// #ifndef CD74HC4067_H
// #define CD74HC4067_H

typedef enum {
  CH0 = 0,
  CH1,
  CH2,
  CH3,
  CH4,
  CH5,
  CH6,
  CH7,
  CH8,
  CH9,
  CH10,
  CH11,
  CH12,
  CH13,
  CH14,
  CH15
} channel_number;
void CD74HC4067_init();
int get_val_from_CD74HC4067(int channel, int analog_read_pin);
// #endif