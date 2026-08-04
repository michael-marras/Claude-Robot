#include "Head.hpp"
#include "../../include/secrets.h"

constexpr double SAM_GAIN = 0.1;

constexpr uint8_t  PDM_MIC_DATA_PIN = 41;
constexpr uint8_t  PDM_MIC_CLK_PIN  = 42;
constexpr uint8_t  CPU_CORE         = 1;
constexpr uint8_t  HEADER_SIZE      = 4;

constexpr uint8_t  MIC_TASK_PRIORITY = 4;
constexpr uint8_t  RCV_TASK_PRIORITY = 3;
constexpr uint8_t  CAM_TASK_PRIORITY = 3;

constexpr uint16_t SIXTEEN_KHZ  	= 16000;
constexpr uint16_t CAMERA_DELAY     = 1000;
constexpr uint16_t TCP_RECONN_DELAY = 500;
constexpr uint16_t PORT         	= 9997;
constexpr uint16_t SHUTDOWN_WAIT_MS = 1000;

constexpr uint32_t MIC_TASK_STACK_BYTES = 4096;
constexpr uint32_t RCV_TASK_STACK_BYTES = 8192;
constexpr uint32_t CAM_TASK_STACK_BYTES = 8192;

constexpr uint32_t TWENTY_MHZ = 20000000;

constexpr EventBits_t DEINIT_BIT      = BIT0;
constexpr EventBits_t CAM_DONE_BIT    = BIT1;
constexpr EventBits_t MIC_DONE_BIT    = BIT2;
constexpr EventBits_t SPEECH_DONE_BIT = BIT3;
constexpr EventBits_t ALL_DONE_BITS = CAM_DONE_BIT | MIC_DONE_BIT | SPEECH_DONE_BIT;

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
constexpr const char* MESSAGE_RECV_SPEECH_INIT  = "ReceiveSpeechTask initialized";
constexpr const char* MESSAGE_PACKET_RECVD      = "Packet received";
constexpr const char* MESSAGE_UDP_READ_FAILED   = "Read failed";
constexpr const char* MESSAGE_ROBOT_SPEAKING    = "Speech Produced";
constexpr const char* MESSAGE_HEADER_TRUNCATED  = "Length header truncated";
constexpr const char* MESSAGE_FRAME_TRUNCATED   = "Frame truncated";
constexpr const char* MESSAGE_SPEECH_FAILED     = "Speech failed to produce";
constexpr const char* MESSAGE_GROUP_FAIL        = "The event group was not created because there was insufficient heap available";

bool Head::init() {
	Serial.println(MESSAGE_INIT);
    camera_config_t cameraConfig = this -> initCameraConfig();

    if(!this -> initCamera(&cameraConfig)) {
		Serial.println(MESSAGE_INIT_ERROR);
		return false;
	}

	if(!this -> initMicrophone()) {
		Serial.println(MESSAGE_INIT_ERROR);
		return false;
	}

	if (!this->initSAM()) {
		Serial.println(MESSAGE_INIT_ERROR);
		return false;
	}

	eventGroup_ = xEventGroupCreate();
	if (eventGroup_ == NULL) {
		Serial.println(MESSAGE_GROUP_FAIL);
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

	if (eventGroup_ != nullptr && tasksStarted_) {
		xEventGroupSetBits(eventGroup_, DEINIT_BIT);

		EventBits_t bits = xEventGroupWaitBits(
			eventGroup_,
			ALL_DONE_BITS,
			pdFALSE,                            
			pdTRUE,                              
			portMAX_DELAY 
		);
	}

	if (esp_camera_deinit() != ESP_OK) {
		Serial.println(MESSAGE_CAM_DEINIT_ERROR);
		return false;
	}

	if (!i2S_.end()) {
		Serial.println(MESSAGE_MIC_DEINIT_ERROR);
		return false;
	}

	if (samOut_ != nullptr) {
		samOut_->stop();
		delete samOut_;
		samOut_ = nullptr;
	}
	delete sam_;
	sam_ = nullptr;

	udp_.stop();
	tcp_.stop();

	headInitialized_ = false;
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

bool Head::initSAM() {
	samOut_ = new AudioOutputI2S();
	if (!samOut_->SetPinout(D0, D1, D2)) {
		return false;
	}
	samOut_->SetGain(SAM_GAIN);
	sam_ = new ESP8266SAM();
	return true;
}

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

	xTaskCreatePinnedToCore(
		recvSpeechTaskEntry, 
		"receiving speech from companion code",
		RCV_TASK_STACK_BYTES,
		this,
		RCV_TASK_PRIORITY,
		nullptr,
		CPU_CORE
	);

	xTaskCreatePinnedToCore(
		cameraTaskEntry, 
		"capturing, processing, and sending images",
		CAM_TASK_STACK_BYTES,
		this,
		CAM_TASK_PRIORITY,
		nullptr,
		CPU_CORE
	);

	tasksStarted_ = true;
}

void Head::printSample(int16_t sample) {
	if (sample && sample != -1 && sample != 1) {
		Serial.println(sample);
	}
}

void Head::printFrame(camera_fb_t* frameBuffer) {
	Serial.write(frameBuffer -> buf, frameBuffer -> len);
}

bool Head::speak(TtsChunk chunk) {
	checkInitialized();

	memcpy(ttsText_, chunk.data, chunk.length);
	ttsText_[chunk.length] = '\0';

	bool success = sam_->Say(samOut_, ttsText_);
	samOut_->flush();
	samOut_->stop();

	return success;
}

const char* Head::getTtsText() {
	return ttsText_;
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
		Serial.println(MESSAGE_HEADER_TRUNCATED);
		tcp_.stop();
		return;
	}
	if(tcp_.write(frameBuffer -> buf, frameBuffer -> len) < frameBuffer -> len) {
		Serial.println(MESSAGE_FRAME_TRUNCATED);
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

bool Head::shutdownRequested() {
	return (xEventGroupGetBits(eventGroup_) & DEINIT_BIT) != 0;
}

void Head::cameraTaskEntry(void* pvParameters) {
	static_cast<Head*>(pvParameters) -> cameraTask(); 
}

void Head::microphoneTaskEntry(void* pvParameters) {
	static_cast<Head*>(pvParameters) -> microphoneTask(); 
}

void Head::recvSpeechTaskEntry(void* pvParameters) {
	static_cast<Head*>(pvParameters) -> recvSpeechTask(); 
}

void Head::cameraTask() {
	TickType_t lastUnblock = xTaskGetTickCount();
	while(!shutdownRequested()) {
		while (!tcp_.connected() && !shutdownRequested()) {
			tcp_.connect(IPAddress(IP_ADDRESS), PORT);
			vTaskDelay(pdMS_TO_TICKS(TCP_RECONN_DELAY));
		}

		camera_fb_t* frameBuffer = this -> getFrameBuffer();
		this -> sendVideo(frameBuffer);
		this -> returnFrameBuffer(frameBuffer);
		xTaskDelayUntil(&lastUnblock, pdMS_TO_TICKS(CAMERA_DELAY));
	}

	xEventGroupSetBits(eventGroup_, CAM_DONE_BIT);
	vTaskDelete(nullptr);
}

void Head::microphoneTask() {
	constexpr size_t bufferSize = sizeof(audioBuffer_);
	while(!shutdownRequested()) {
		// Serial.printf("Free heap: %u\n", ESP.getFreeHeap()); // Use this to check for memory leaks
		this -> updateAudioBuffer(bufferSize);
		this -> sendAudio(bufferSize);
	}

	xEventGroupSetBits(eventGroup_, MIC_DONE_BIT);
	vTaskDelete(nullptr);
}

void Head::recvSpeechTask() {
	Serial.println(MESSAGE_RECV_SPEECH_INIT);
	TtsChunk chunk;
	while(!shutdownRequested()) {
		int packetSize = udp_.parsePacket();
		if (packetSize > 0) {
			Serial.println(MESSAGE_PACKET_RECVD);
			int16_t len = udp_.read(chunk.data, TTS_BUFFER_SIZE);
			if (len <= 0) {
				Serial.println(MESSAGE_UDP_READ_FAILED);
			}
			else {
				Serial.println(packetSize);
				Serial.println(MESSAGE_ROBOT_SPEAKING);
				chunk.length = static_cast<size_t>(len);

				if (!speak(chunk)) {
					Serial.println(MESSAGE_SPEECH_FAILED);
				}
			}
		}
	}

	xEventGroupSetBits(eventGroup_, SPEECH_DONE_BIT);
	vTaskDelete(nullptr);
}
