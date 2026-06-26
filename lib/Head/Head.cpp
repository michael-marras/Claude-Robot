#include "Head.hpp"

constexpr uint16_t SIXTEEN_KHZ      = 16000;
constexpr uint8_t  PDM_MIC_DATA_PIN = 41;
constexpr uint8_t  PDM_MIC_CLK_PIN  = 42;
constexpr uint32_t TWENTY_MHZ       = 20000000;

void Head::init() {
	Serial.println("Head Initializing!");
    camera_config_t cameraConfig = this -> initCameraConfig();
    this -> initCamera(&cameraConfig);
	this -> initMicrophone();
	Serial.println("Head Initialized Successfully!");
}

void Head::printData() {
	this -> printSample();
	this -> printFrame();
}

void Head::initCamera(camera_config_t* cameraConfig) {
	esp_err_t err = esp_camera_init(cameraConfig);
	if(err != ESP_OK) {
		Serial.println("Camera failed to initialize");
        while(1);
	}

	sensor_t* camera = esp_camera_sensor_get();
	camera -> set_vflip(camera, 1);
}

void Head::initMicrophone() {
	this -> i2S_.setPinsPdmRx(PDM_MIC_CLK_PIN, PDM_MIC_DATA_PIN);

	if(!this -> i2S_.begin(I2S_MODE_PDM_RX, SIXTEEN_KHZ, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
		Serial.println("Failed to initialize I2S!");
		while(1);
	}
}

constexpr camera_config_t Head::initCameraConfig() {
    camera_config_t config = {};

    // --- Clock (drives the sensor) ---
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer   = LEDC_TIMER_0;
	config.xclk_freq_hz = TWENTY_MHZ;     

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

    return config;
}

void Head::printSample() {
	int16_t sample = this -> i2S_.read();

	if (sample && sample != -1 && sample != 1) {
		Serial.println(sample);
	}
}

void Head::printFrame() {
	camera_fb_t* frameBuffer = esp_camera_fb_get();

	if (!frameBuffer) {
		Serial.println("Failed to get camera frame buffer");
		return;
	}

	Serial.write(frameBuffer -> buf, frameBuffer -> len);
	esp_camera_fb_return(frameBuffer);
}


