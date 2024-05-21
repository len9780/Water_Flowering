// #ifndef CONFIG_SETTING_H
// #define CONFIG_SETTING_H
// void CD74HC4067_init();
#define firm_ver "1.0.0"
#include <FS.h>
typedef enum rw { read = 0, write } rw;
char *write_config(File f, unsigned char rw, char *dat);
// #endif