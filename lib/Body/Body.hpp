#pragma once
#include <Arduino.h>

class Body {
    public:
        Body(uint8_t aIn1, 
             uint8_t aIn2,
             uint8_t bIn1,
             uint8_t bIn2,
             uint8_t pwma,
             uint8_t pwmb,
             uint8_t stdby,
             uint16_t pwm_freq,
             uint16_t pwm_res);

        bool initialize();

        bool deinitialize();

        bool move(int16_t distance);

        bool turn(int16_t angle);

        bool isInitialized();

    private:
        class Motor {
            public:
                Motor(uint8_t in1, uint8_t in2, uint8_t pwm, int pwm_freq, int pwm_res)
                    : in1_(in1), in2_(in2), pwm_(pwm), pwmFreq_(pwm_freq), pwmRes_(pwm_res) {}
                    
                void initialize() {
                    pinMode(in1_, OUTPUT);
                    pinMode(in2_, OUTPUT);
                    pinMode(pwm_, OUTPUT);

                    ledcAttach(pwm_, pwmFreq_, pwmRes_);
                }
        
                void setSpeed(int16_t speed) {
                    digitalWrite(in1_, speed > 0 ? HIGH : LOW);
                    digitalWrite(in2_, speed < 0 ? HIGH : LOW);
                    ledcWrite(pwm_, constrain(abs(speed), 0, 255));
                }

            private:
                uint8_t in1_;
                uint8_t in2_;
                uint8_t pwm_;
                uint16_t pwmFreq_;
                uint16_t pwmRes_;
        };

        Motor motorLeft_;
        Motor motorRight_;
        uint8_t stdby_;
};