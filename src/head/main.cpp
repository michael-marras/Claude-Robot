#include "Head.hpp"
#include <WiFi.h>

constexpr uint16_t BAUD = 9600;

Head head;

void setup() {
	Serial.begin(BAUD);
	head.init();
}

void loop() {
}	
