#include "Head.hpp"
#include "../../include/secrets.h"

constexpr uint16_t SIXTEEN_KHZ      = 16000;
constexpr uint8_t  PDM_MIC_DATA_PIN = 41;
constexpr uint8_t  PDM_MIC_CLK_PIN  = 42;
constexpr uint32_t TWENTY_MHZ       = 20000000;
constexpr uint16_t PORT             = 9997;
constexpr uint8_t  CPU_CORE         = 1;
constexpr uint8_t  HEADER_SIZE      = 4;
constexpr uint16_t CAMERA_DELAY     = 1000;
constexpr uint8_t  TTS_QUEUE_LENGTH = 32;

constexpr uint8_t  MIC_TASK_PRIORITY    = 4;
constexpr uint8_t  SPEECH_TASK_PRIORITY = 5;
constexpr uint8_t  RCV_TASK_PRIORITY    = 3;
constexpr uint8_t  CAM_TASK_PRIORITY    = 2;

constexpr uint32_t MIC_TASK_STACK_BYTES    = 4096;
constexpr uint32_t RCV_TASK_STACK_BYTES    = 8192;
constexpr uint32_t CAM_TASK_STACK_BYTES    = 8192;
constexpr uint32_t SPEECH_TASK_STACK_BYTES = 8192;

constexpr const char* MESSAGE_INIT_SUCCESS = "Head Initialized Successfully";
constexpr const char* MESSAGE_INIT_ERROR   = "Error initalizing head";
constexpr const char* MESSAGE_INIT         = "Head Initializing";
constexpr const char* MESSAGE_DEINIT       = "Head Deinitializing";
constexpr const char* MESSAGE_UNINIT_ERROR = "Head not Initialized";
constexpr const char* MESSAGE_CAMERA_INIT_ERROR = "Camera failed to initialize";
constexpr const char* MESSAGE_I2S_INIT_ERROR    = "Failed to initialize I2S!";
constexpr const char* MESSAGE_MIC_DEINIT_ERROR  = "Camera failed to deinit";
constexpr const char* MESSAGE_CAM_DEINIT_ERROR  = "Camera failed to deinit";
constexpr const char* MESSAGE_RETURN_FB_ERROR   = "Failed to return framebuffer";

bool Head::init() {
	Serial.println(MESSAGE_INIT);
    camera_config_t cameraConfig = this -> initCameraConfig();

    if(!this -> initCamera(&cameraConfig)) {
		Serial.println(MESSAGE_INIT_ERROR);
		return false;
	}
	else if(!this -> initMicrophone()) {
		Serial.println(MESSAGE_INIT_ERROR);
		return false;
	}
	// else if(!this -> initSpeaker()) {
	// 	Serial.println("speaker is a bitch");
	// 	return false;
	// }

	ttsQueue_ = xQueueCreate(TTS_QUEUE_LENGTH, sizeof(TtsChunk));
	if (ttsQueue_ == nullptr) {
		Serial.println("Failed to create TTS queue");
		return false;
	}

	udp_.begin(PORT);
	tcp_.connect(IPAddress(IP_ADDRESS), PORT);
	headInitialized_ = true;
	Serial.println(MESSAGE_INIT_SUCCESS);

	return true;
}

bool Head::deinit() {
	Serial.println(MESSAGE_DEINIT);

	if (esp_camera_deinit() != ESP_OK) {
		Serial.println(MESSAGE_CAM_DEINIT_ERROR);
		return false;
	}

	if (!i2S_.end()) {
		Serial.println(MESSAGE_MIC_DEINIT_ERROR);
		return false;
	}

	udp_.stop();
	tcp_.stop();
	return true;
}

bool Head::initCamera(camera_config_t* cameraConfig) {
	if (!cameraConfig) {
		Serial.println(MESSAGE_CAMERA_INIT_ERROR);
		return false;
	}

	esp_err_t err = esp_camera_init(cameraConfig);

	if(err != ESP_OK) {
		Serial.println(MESSAGE_CAMERA_INIT_ERROR);
        return false;
	}

	sensor_t* camera = esp_camera_sensor_get();
	camera -> set_vflip(camera, 1);
	return true;
}

bool Head::deinitCamera() {
	if (esp_camera_deinit() != ESP_OK) {
		Serial.println(MESSAGE_CAM_DEINIT_ERROR);
		return false;
	}

	return true;
}

bool Head::initMicrophone() {
	i2S_.setPinsPdmRx(PDM_MIC_CLK_PIN, PDM_MIC_DATA_PIN);

	if(!i2S_.begin(I2S_MODE_PDM_RX, SIXTEEN_KHZ, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
		Serial.println(MESSAGE_I2S_INIT_ERROR);
		return false;
	}

	return true;
}

bool Head::deinitMicrophone() {
	if (!i2S_.end()) {
		Serial.println(MESSAGE_MIC_DEINIT_ERROR);
		return false;
	}

	return true;
}

// bool Head::initSpeaker() {
// 	bool speakerBooted;
// 	i2sSpeaker_.setPins(D0, D1, D2);
// 	speakerBooted = i2sSpeaker_.begin(I2S_MODE_STD, 22050, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_RIGHT);

// 	return speakerBooted;
// }

camera_config_t Head::initCameraConfig() {
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
	config.frame_size   = FRAMESIZE_VGA; 
	config.jpeg_quality = 12;               // 0–63, lower = better/bigger

	// --- Frame buffers ---
	config.fb_count    = 2;
	config.fb_location = CAMERA_FB_IN_PSRAM;
	config.grab_mode   = CAMERA_GRAB_LATEST; // newest frame, best for live

    return config;
}

void Head::startTasks() {
	this -> checkInitialized();

	xTaskCreatePinnedToCore(
		microphoneTaskEntry, 
		"capturing, processing, and sending audio",
		MIC_TASK_STACK_BYTES,
		this,
		MIC_TASK_PRIORITY,
		nullptr,
		CPU_CORE
	);

	// xTaskCreatePinnedToCore(
	// 	receiveSpeechTaskEntry, 
	// 	"receiving speech from companion code",
	// 	RCV_TASK_STACK_BYTES,
	// 	this,
	// 	RCV_TASK_PRIORITY,
	// 	nullptr,
	// 	CPU_CORE
	// );

	xTaskCreatePinnedToCore(
		cameraTaskEntry, 
		"capturing, processing, and sending images",
		CAM_TASK_STACK_BYTES,
		this,
		CAM_TASK_PRIORITY,
		nullptr,
		CPU_CORE
	);

	// xTaskCreatePinnedToCore(
	// 	speechTaskEntry,
	// 	"producting speech",
	// 	SPEECH_TASK_STACK_BYTES,
	// 	this,
	// 	SPEECH_TASK_PRIORITY,
	// 	nullptr,
	// 	CPU_CORE
	// );
}

void Head::printSample(int16_t sample) {
	if (sample && sample != -1 && sample != 1) {
		Serial.println(sample);
	}
}

void Head::printFrame(camera_fb_t* frameBuffer) {
	Serial.write(frameBuffer -> buf, frameBuffer -> len);
}

camera_fb_t* Head::getFrameBuffer() {
	this -> checkInitialized();
	return esp_camera_fb_get();
}

void Head::updateAudioBuffer(size_t size) {
	this -> checkInitialized();
	i2S_.readBytes(audioBuffer_, size);
}

bool Head::returnFrameBuffer(camera_fb_t* frameBuffer) {
	if (!frameBuffer) {
		Serial.println(MESSAGE_RETURN_FB_ERROR);
		return false;
	}

	this -> checkInitialized();
	esp_camera_fb_return(frameBuffer);
	return true;
}

void Head::sendAudio(size_t size) {
	udp_.beginPacket(IPAddress(IP_ADDRESS), PORT);
	udp_.write(reinterpret_cast<const uint8_t*>(audioBuffer_), size);
	udp_.endPacket();
}

void Head::sendVideo(camera_fb_t* frameBuffer) {
	uint32_t len = frameBuffer->len;
	if (tcp_.write(reinterpret_cast<uint8_t*>(&len), HEADER_SIZE) < HEADER_SIZE) {
		Serial.println("length header truncated");
		tcp_.stop();
		return;
	}
	if(tcp_.write(frameBuffer -> buf, frameBuffer -> len) < frameBuffer -> len) {
		Serial.println("frame truncated");
		tcp_.stop();
		return;
	}
}

void Head::checkInitialized() {
	if (!headInitialized_) {
		Serial.println(MESSAGE_UNINIT_ERROR);
		while(1);
	}
}

void Head::cameraTaskEntry(void* pvParameters) {
	static_cast<Head*>(pvParameters) -> cameraTask(); 
}

void Head::microphoneTaskEntry(void* pvParameters) {
	static_cast<Head*>(pvParameters) -> microphoneTask(); 
}

// void Head::receiveSpeechTaskEntry(void* pvParameters) {
// 	static_cast<Head*>(pvParameters) -> receiveSpeechTask(); 
// }

// void Head::speechTaskEntry(void* pvParameters) {
// 	static_cast<Head*>(pvParameters) -> speechTask();
// }

void Head::cameraTask() {
	TickType_t lastUnblock = xTaskGetTickCount();
	while(1) {
		if (!tcp_.connected()) {
			tcp_.connect(IPAddress(IP_ADDRESS), PORT);
		}

		camera_fb_t* frameBuffer = this -> getFrameBuffer();
		this -> sendVideo(frameBuffer);
		this -> returnFrameBuffer(frameBuffer);
		xTaskDelayUntil(&lastUnblock, pdMS_TO_TICKS(CAMERA_DELAY));
	}
}

void Head::microphoneTask() {
	constexpr size_t bufferSize = sizeof(audioBuffer_);
	while(1) {
		// Serial.printf("Free heap: %u\n", ESP.getFreeHeap()); // Use this to check for memory leaks
		this -> updateAudioBuffer(bufferSize);
		this -> sendAudio(bufferSize);
	}
}

// void Head::receiveSpeechTask() {
// 	Serial.println("receiveSpeechTask initialized");
// 	TtsChunk chunk;
// 	while(1) {
// 		int packetSize = udp_.parsePacket();
// 		if (packetSize > 0) {
// 			Serial.println("packet received");
// 			int16_t len = udp_.read(chunk.data, TTS_BUFFER_SIZE);
// 			if (len <= 0) {
// 				Serial.println("read failed");
// 			}
// 			else {
// 				Serial.println(packetSize);
// 				chunk.length = static_cast<size_t>(len);
// 				if (xQueueSend(ttsQueue_, &chunk, portMAX_DELAY) != pdTRUE) {
// 					Serial.println("queue full, dropped");
// 				}
// 			}
// 		}
// 	}
// }

// void Head::speechTask() {
// 	TtsChunk chunk;
// 	static uint32_t lastWrite = millis();
// 	while(1){
// 		if (xQueueReceive(ttsQueue_, &chunk, portMAX_DELAY) == pdTRUE) {
// 			Serial.printf("writing chunk len=%u\n", chunk.length);
// 			Serial.printf("gap since last write: %lu ms\n", millis() - lastWrite);
// 			lastWrite = millis();
// 			size_t written = i2sSpeaker_.write(chunk.data, chunk.length);
// 			if (written < chunk.length) {
// 				Serial.printf("i2s write truncated: %u/%u\n", written, chunk.length);
// 			}
//         }
// 		Serial.println("speech");
// 		Serial.printf("queue depth: %u\n", uxQueueMessagesWaiting(ttsQueue_));
// 	}
// }
