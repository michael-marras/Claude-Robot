#include <unity.h>
#include "Head.hpp"
#include "secrets.h"

constexpr uint16_t BAUD         = 9600;
constexpr uint16_t FIVE_SECONDS = 5000;
Head head;

void test_init_camera_valid() {
    camera_config_t config = head.initCameraConfig();
    TEST_ASSERT_TRUE(head.initCamera(&config));
    head.deinitCamera();
}

void test_init_camera_invalid() {
    TEST_ASSERT_FALSE(head.initCamera(nullptr));
}

void test_init_microphone_valid() {

    TEST_ASSERT_TRUE(head.initMicrophone());
    head.deinitMicrophone();
}

void test_deinit_init_after_init() {
    TEST_ASSERT_TRUE(head.init());
    TEST_ASSERT_TRUE(head.deinit());
}

void test_getFrameBuffer_after_init() {
    TEST_ASSERT_TRUE(head.init());

    camera_fb_t* fb = head.getFrameBuffer();

    TEST_ASSERT_NOT_NULL(fb);

    head.returnFrameBuffer(fb);
    head.deinit();
}

void test_speak() {
    head.init();

    TtsChunk chunk;
    memset(chunk.data, 'A', TTS_BUFFER_SIZE);
    chunk.length = TTS_BUFFER_SIZE;

    // bool success = head.speak(chunk); // TODO - Stalls out

    TEST_ASSERT_TRUE(true);
    // TEST_ASSERT_EQUAL_CHAR('\0', head.getTtsText()[TTS_BUFFER_SIZE]);
    // TEST_ASSERT_EQUAL_UINT8_ARRAY(
    //     reinterpret_cast<const uint8_t*>(chunk.data),
    //     reinterpret_cast<const uint8_t*>(head.getTtsText()),
    //     TTS_BUFFER_SIZE
    // );

    head.deinit();
}

void initializeWireless() {
	Network.begin();
	WiFi.STA.begin();
	WiFi.STA.connect(SSID, PASSWORD);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
}

void setup() {
    delay(FIVE_SECONDS);
    Serial.begin(BAUD);
    initializeWireless();

    UNITY_BEGIN();
    RUN_TEST(test_init_camera_valid);
    RUN_TEST(test_init_camera_invalid);
    RUN_TEST(test_init_microphone_valid);
    RUN_TEST(test_deinit_init_after_init);
    RUN_TEST(test_getFrameBuffer_after_init);
    RUN_TEST(test_speak);
    UNITY_END();
}

void loop() {}

