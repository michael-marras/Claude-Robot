#include "Head.hpp"
#include <WiFi.h>

constexpr uint32_t BAUD = 9600;
constexpr uint16_t FIVE_SECONDS = 5000;

Head head;

void setup() {
	Serial.begin(BAUD);
	delay(FIVE_SECONDS);
	head.init();
}

void loop() {
	head.printData();
}	
