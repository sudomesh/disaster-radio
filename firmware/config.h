#ifndef CONFIG_H
#define CONFIG_H

// secrets in a separate file so we don't accidentally commit credentials
#if __has_include("secrets.h")
#include "secrets.h"
#endif

#define CLOCK_SPEED 240
#define LORA_FREQ 915E6

#ifdef TTGO_LORA_V1
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_CS 18
#define LORA_RST 14
#define LORA_IRQ 26
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#endif

#ifdef TTGO_LORA_V2
#define SD_SCK 14
#define SD_MISO 2
#define SD_MOSI 15
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST NOT_A_PIN
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#endif

#ifdef TTGO_TBEAM
// #define LORA_SCK 14
// #define LORA_MISO 2
// #define LORA_MOSI 15
// #define LORA_CS 18
// #define LORA_RST 23
// #define LORA_IRQ 26
// #define SD_SCK 14
// #define SD_MISO 2
// #define SD_MOSI 15
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST NOT_A_PIN
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// https://github.com/cyberman54/ESP32-Paxcounter/blob/825520a20f1a1b7a9ac5e725484c2704bcb8117b/src/hal/ttgobeam.h
#define GPS_SERIAL 9600, SERIAL_8N1, GPIO_NUM_12, GPIO_NUM_15

#endif

#ifdef HELTEC
#define LORA_IRQ 26
#define LORA_IO1 35
#define LORA_IO2 34
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_RST 14
#define LORA_CS 18
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#endif

#ifdef SPARKFUN
#define LORA_IRQ 26
#define LORA_IO1 33
#define LORA_IO2 32
#define LORA_SCK 14 SCK
#define LORA_MISO 12
#define LORA_MOSI 13
#define LORA_RST NOT_A_PIN
#define LORA_CS 16
//#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST NOT_A_PIN
#define OLED_WIDTH 128
#define OLED_HEIGHT 128
#endif

#ifdef LOPY4
// all necessary pins are defined in the arduino-esp32 variants
// https://github.com/espressif/arduino-esp32/blob/master/variants/lopy4/pins_arduino.h
// any additional board specifc settings should be added here
#define LORA_IRQ 23
#define LORA_CS 18
// to add second LoRa module connect it to these pins and specific DUAL_LORA as a build flag
#define LORA2_IRQ 26
#define LORA2_RST NOT_A_PIN
#define LORA2_CS 25
#endif

#endif
