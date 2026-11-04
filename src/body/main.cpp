#include <Arduino.h>
#include "Body.hpp"

constexpr uint8_t AIN1 = 18;
constexpr uint8_t AIN2 = 19;
constexpr uint8_t BIN1 = 21;
constexpr uint8_t BIN2 = 15;
constexpr uint8_t PWMA = 23;
constexpr uint8_t PWMB = 14;
constexpr uint8_t STBY = 4;
constexpr uint8_t ENC_C1 = 32;
constexpr uint8_t ENC_C2 = 33;

constexpr int PWM_FREQ = 20000;  // 20 kHz, above audible range
constexpr int PWM_RES = 8;       // 0-255

Body body(AIN1, AIN2, BIN1, BIN2, PWMA, PWMB, STBY, PWM_FREQ, PWM_RES);

void run() {
	while (true) {
		body.move(10);
		delay(1000);
		body.move(0);
		delay(1000);
	}
}

void setup() {
	if (body.initialize()) {
		run();
	};
}

void loop() {}