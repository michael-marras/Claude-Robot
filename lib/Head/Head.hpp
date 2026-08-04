#pragma once
#include <esp_camera.h>
#include <ESP_I2S.h>
#include <Arduino.h>
#include "camera_pins.h"
#include <WiFi.h>
#include <ESP8266SAM.h>
#include <AudioOutputI2S.h>

static constexpr size_t NUM_BYTES = 320;   
static constexpr size_t TTS_BUFFER_SIZE = 2944;

struct TtsChunk {
    uint8_t data[TTS_BUFFER_SIZE];
    size_t  length;
};

class Head {
    public:
        /**
         * @brief Initialize the camera, microphone, and tcp/udp connections
         * 
         * @return true on success
         */
        bool init();

        /**
         * @brief deinitialize the camera, microphone, and tcp/udp connections
         * 
         * @return true on success
         */
        bool deinit();

        /** 
        * @brief Initialize the OV3360 camera using ESP_Camera
        * 
        * @param Struct containing config for the camera
        * @return true on success
        */
        bool initCamera(camera_config_t* cameraConfig);

        /** 
        * @brief Dinitialize the camera
        * 
        * @return true on success
        */
        bool deinitCamera();

        /** 
        * @brief Initialize the mircrophone
        * 
        * @return true on success
        */
        bool initMicrophone();

        /**
         * @brief Deinitialize the microphone
         * 
         * @return true on success
         */
        bool deinitMicrophone();

        /**
         * @brief Initialize the SAM text synthesizer
         * 
         * @return true on success
         */
        bool initSAM();

        /**
         * @brief Initialize the config for the camera
         * 
         * @returns The config for the camera to be initialized with
         */
        camera_config_t initCameraConfig();

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
         * @brief Produce SAM speech
         */
        bool speak(TtsChunk chunk);

        /**
         * @brief Get TtsText_ buffer
         */
        const char* getTtsText();

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
         * 
         * @return true on success
         */
        bool returnFrameBuffer(camera_fb_t* frameBuffer);

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
        void sendVideo(camera_fb_t* frameBuffer);

        /**
         * @brief If head is not initialized then a message saying so is printed to the serial monitor and then the progam halts
         */
        void checkInitialized();

        /**
         * @brief Checks whether shutdown has been requested via the event group.
         * @return true if ABORT_BIT is currently set.
         */
        bool shutdownRequested();

    private:
        bool headInitialized_              = false;
        bool tasksStarted_                 = false;
        char audioBuffer_[NUM_BYTES]       = {}; 
        char ttsText_[TTS_BUFFER_SIZE + 1] = {};
        
        EventGroupHandle_t eventGroup_;
        I2SClass           i2S_; 
        AudioOutputI2S*    samOut_;
        ESP8266SAM*        sam_;
        NetworkUDP         udp_;
        NetworkClient      tcp_;

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
        static void recvSpeechTaskEntry(void* pvParameters);

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
        void recvSpeechTask();
};