#include "Body.hpp"

// Constructor definition
Body::Body(uint8_t aIn1, 
           uint8_t aIn2,
           uint8_t bIn1,
           uint8_t bIn2,
           uint8_t pwma,
           uint8_t pwmb,
           uint8_t stdby,
           uint16_t pwm_freq,
           uint16_t pwm_res)
    : motorLeft_(aIn1, aIn2, pwma, pwm_freq, pwm_res),  // Initialize motorLeft
      motorRight_(bIn1, bIn2, pwmb, pwm_freq, pwm_res), // Initialize motorRight
      stdby_(stdby)               // Initialize standby pin
{}

bool Body::initialize() {
    pinMode(stdby_, OUTPUT);
    digitalWrite(stdby_, HIGH);
    motorLeft_.initialize();
    motorRight_.initialize();
    return true;
}

bool Body::move(int16_t distance) {
    if (distance < 0) {
        motorLeft_.setSpeed(-120);
        motorRight_.setSpeed(-120);
        return true;
    }
    else if (distance > 0) {
        motorLeft_.setSpeed(120);
        motorRight_.setSpeed(120);
        return true;
    }
    else {
        motorLeft_.setSpeed(0);
        motorRight_.setSpeed(0);
        return true;
    }


}