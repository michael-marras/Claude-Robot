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
         * 
         */
        void startTasks();

        /**
         * @brief Print audio sample
         */
        void printSample(int16_t sample);

        /**
         * @brief Print camera frame to the serial monitor
         */
        void printFrame(camera_fb_t* frameBuffer);

        /**
         * @brief Get the frame buffer pointer
         */
        camera_fb_t* getFrameBuffer();

        /** 
         * @brief Get the audio sample
         * 
         * @returns amplitude of audio sample
         */
        int16_t getAudioSample();

        /**
         * @brief Returns the Framebuffer pointer back to the camera to be used again
         */
        void returnFrameBuffer(camera_fb_t* frameBuffer);

    private:
        bool        headInitialized_ = false;
        I2SClass    i2S_; 

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
         * @brief Task in charge of capturing camera data and sending it to the companion server
         */
        static void cameraTaskEntry(void* arg);  

        /**
         * Task in charge of capturing microphone data and sending it to the companion server
         */
        static void microphoneTaskEntry(void* arg);

        /**
         * Task in charge of receiving commands from the companion server
         */
        static void receiveCommandsTaskEntry(void* arg);

        /**
         * @brief Task in charge of capturing camera data and sending it to the companion server
         */
        void cameraTask();  

        /**
         * Task in charge of capturing microphone data and sending it to the companion server
         */
        void microphoneTask();

        /**
         * Task in charge of receiving commands from the companion server
         */
        void receiveCommandsTask();
};