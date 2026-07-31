#include "Head.hpp"
#include "secrets.h"
#include <ESP8266SAM.h>
#include <AudioOutputI2S.h>

constexpr uint32_t BAUD = 9600;
constexpr uint16_t FIVE_SECONDS = 5000;

Head head;
AudioOutputI2S *out = NULL;
ESP8266SAM     *sam;

void initializeWireless() {
	Network.begin();
	WiFi.STA.begin();
	WiFi.STA.connect(SSID, PASSWORD);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println();
	Serial.print("IP address: ");
	Serial.println(WiFi.STA.localIP());
}

void setup() {
	delay(FIVE_SECONDS);
	Serial.begin(BAUD);
	
	out = new AudioOutputI2S();
	out->SetPinout(D0,D1,D2);
	out->begin();
	sam = new ESP8266SAM;
	sam->Say(out, "Youtube");
	out->flush();

	delay(FIVE_SECONDS);

	initializeWireless();

	if(head.init()) {
		head.startTasks();
	}
}

void loop() {}

