#pragma once
#include <cstdint>

// XIAO ESP32S3 Sense camera pinout
constexpr int8_t  PWDN_GPIO_NUM  = -1;
constexpr int8_t  RESET_GPIO_NUM = -1;
constexpr uint8_t XCLK_GPIO_NUM  = 10;
constexpr uint8_t SIOD_GPIO_NUM  = 40;
constexpr uint8_t SIOC_GPIO_NUM  = 39;

constexpr uint8_t Y9_GPIO_NUM    = 48;
constexpr uint8_t Y8_GPIO_NUM    = 11;
constexpr uint8_t Y7_GPIO_NUM    = 12;
constexpr uint8_t Y6_GPIO_NUM    = 14;
constexpr uint8_t Y5_GPIO_NUM    = 16;
constexpr uint8_t Y4_GPIO_NUM    = 18;
constexpr uint8_t Y3_GPIO_NUM    = 17;
constexpr uint8_t Y2_GPIO_NUM    = 15;
constexpr uint8_t VSYNC_GPIO_NUM = 38;
constexpr uint8_t HREF_GPIO_NUM  = 47;
constexpr uint8_t PCLK_GPIO_NUM  = 13;

constexpr uint8_t LED_GPIO_NUM   = 21;