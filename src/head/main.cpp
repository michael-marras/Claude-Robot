#include "Head.hpp"
#include "secrets.h"

constexpr uint32_t BAUD = 9600;
constexpr uint16_t FIVE_SECONDS = 5000;

Head head;

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
	Serial.begin(BAUD);
	delay(FIVE_SECONDS);
	initializeWireless();
	head.init();
	head.startTasks();
}

void loop() {
}

