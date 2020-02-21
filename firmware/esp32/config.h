#ifndef CONFIG_H
#define CONFIG_H

// secrets in a separate file so we don't accidentally commit credentials
#if __has_include("secrets.h")
#include "secrets.h"
#endif

#define CLOCK_SPEED 240

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
#define LORA_IRQ DIO0
#define LORA_IO1 DIO1
#define LORA_IO2 DIO2
#define LORA_SCK SCK
#define LORA_MISO MISO
#define LORA_MOSI MOSI
#define LORA_RST RST_LoRa
#define LORA_CS SS
#define OLED_SDA SDA_OLED
#define OLED_SCL SCL_OLED
#define OLED_RST RST_OLED
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
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST NOT_A_PIN
#define OLED_WIDTH 128
#define OLED_HEIGHT 128
#endif

#endif
