#include "Head.hpp"
#include "camera_pins.h"
#include <Arduino.h>

void Head::init() {
    camera_config_t* cameraConfig = this -> initCameraConfig();
    this -> initCamera(cameraConfig);
}

camera_config_t* Head::initCameraConfig() {
    camera_config_t config;

    // --- Clock (drives the sensor) ---
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer   = LEDC_TIMER_0;
	config.xclk_freq_hz = 20000000;        // 20 MHz

	// --- Data pins (DVP parallel bus) ---
	config.pin_d0 = Y2_GPIO_NUM;
	config.pin_d1 = Y3_GPIO_NUM;
	config.pin_d2 = Y4_GPIO_NUM;
	config.pin_d3 = Y5_GPIO_NUM;
	config.pin_d4 = Y6_GPIO_NUM;
	config.pin_d5 = Y7_GPIO_NUM;
	config.pin_d6 = Y8_GPIO_NUM;
	config.pin_d7 = Y9_GPIO_NUM;

	// --- Clock/sync pins ---
	config.pin_xclk  = XCLK_GPIO_NUM;
	config.pin_pclk  = PCLK_GPIO_NUM;
	config.pin_vsync = VSYNC_GPIO_NUM;
	config.pin_href  = HREF_GPIO_NUM;

	// --- SCCB control pins ---
	config.pin_sccb_sda = SIOD_GPIO_NUM;
	config.pin_sccb_scl = SIOC_GPIO_NUM;

	// --- Power/reset (unused on XIAO, -1) ---
	config.pin_pwdn  = PWDN_GPIO_NUM;
	config.pin_reset = RESET_GPIO_NUM;

	// --- Image format ---
	config.pixel_format = PIXFORMAT_JPEG;   // pre-compressed, forward as-is
	config.frame_size   = FRAMESIZE_VGA;    // 640x480 — good for YOLO
	config.jpeg_quality = 12;               // 0–63, lower = better/bigger

	// --- Frame buffers ---
	config.fb_count    = 2;
	config.fb_location = CAMERA_FB_IN_PSRAM;
	config.grab_mode   = CAMERA_GRAB_LATEST; // newest frame, best for live

    return &config;
}

esp_err_t Head::initCamera(camera_config_t* cameraConfig) {
	esp_err_t err = esp_camera_init(cameraConfig);
	if(err != ESP_OK) {
		Serial.println("Camera failed to initialize");
        return err;
	}

	sensor_t* camera = esp_camera_sensor_get();
	camera -> set_vflip(camera, 1);
    return ESP_OK;
}