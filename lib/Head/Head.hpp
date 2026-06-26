#pragma once
#include <esp_camera.h>
#include <ESP_I2S.h>
#include <Arduino.h>
#include "camera_pins.h"

class Head {
    public:
        /**
         * @brief Initialize the camera and microphone
         */
        void init();

        /** 
         * @brief Print a camera frame and an audio sample in the Serial monitor
         */
        void printData();

    private:
        I2SClass i2S_; 

        /** 
        * @brief Initialize the OV3360 camera using ESP_Camera
        * 
        * @param Struct containing config for the camera
        */
        void initCamera(camera_config_t* cameraConfig);

        /** 
         * @brief Initialize the mircrophone using ESP_I2S api
         */
        void initMicrophone();

        /**
         * @brief Initialize the config for the camera
         * 
         * @returns The config for the camera to be initialized with
         */
        constexpr camera_config_t initCameraConfig();

        /**
         * @brief Print audio sample
         */
        void printSample();

        /**
         * @brief Print camera frame
         */
        void printFrame();

};