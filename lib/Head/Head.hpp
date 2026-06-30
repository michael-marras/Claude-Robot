#pragma once
#include <esp_camera.h>
#include <ESP_I2S.h>
#include <Arduino.h>
#include "camera_pins.h"
#include <WiFi.h>

static constexpr size_t AUDIO_SAMPLES = 512;   

class Head {
    public:
        /**
         * @brief Initialize the camera and microphone
         */
        void init();

        /**
         * @brief Start cameraTask, microphoneTask, and receiveCommandsTask for the scheduler
         */
        void startTasks();

        /**
         * @brief Print audio sample to the serial monitor
         * 
         */
        void printSample(int16_t sample);

        /**
         * @brief Print camera frame to the serial monitor
         * 
         */
        void printFrame(camera_fb_t* frameBuffer);

        /**
         * @brief Returns a pointer to the buffer containing the jpeg frames capture by the camera
         */
        camera_fb_t* getFrameBuffer();

        /** 
         * @brief Updates the buffer in the head instasnce of audio samples captured by the mic 
         * 
         * @param size Number of bytes held in buffer
         */
        void updateAudioBuffer(size_t size);

        /**
         * @brief Returns the Framebuffer pointer back to the camera to be used again
         */
        void returnFrameBuffer(camera_fb_t* frameBuffer);

        /**
         * @brief Sends the audio buffer as a packet to the local server over udp
         * 
         * @param size Number of bytes held in buffer
         */
        void sendAudio(size_t size);

        /**
         * @brief Sends the frame buffer as a packet to the local server over tcp
         * 
         * @param size Number of bytes held in buffer
         */
        void sendVideo(size_t size);

        /**
         * @brief If head is not initialized then a message saying so is printed to the serial monitor and then the progam halts
         */
        void checkInitialized();

    private:
        bool    headInitialized_ = false;
        char    audioBuffer_[AUDIO_SAMPLES] = {}; 

        I2SClass      i2S_; 
        NetworkUDP    udp_;
        NetworkClient tcp_;
        
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
        camera_config_t initCameraConfig();

        /**
         * @brief Entry point for camera task
         * 
         * @param pvParameters Pointer that will be used as the parameter for the task being created
         */
        static void cameraTaskEntry(void* pvParameters);  

        /**
         * @brief Entry point for microphone task
         * 
         * @param pvParameters Pointer that will be used as the parameter for the task being created
         */
        static void microphoneTaskEntry(void* pvParameters);

        /**
         * @brief Entry point for receive commands task
         * 
         * @param pvParameters Pointer that will be used as the parameter for the task being created
         */
        static void receiveCommandsTaskEntry(void* pvParameters);

        /**
         * @brief Task handled by scheduler in charge of capturing camera data and sending it to the companion server
         */
        void cameraTask();  

        /**
         * @brief Task handled by scheduler in charge of capturing microphone data and sending it to the companion server
         */
        void microphoneTask();

        /**
         * @brief Task handled by scheduler in charge of receiving commands from the companion server
         */
        void receiveCommandsTask();
};