#pragma
#include "esp_camera.h"

class Head {
    public:
        /**
         * @brief Initialize the camera and microphone
         */
        void init();

        /** 
         * @brief Capture 1 video frame and record wav file
         */
        void captureData();

    private:
        /** 
        * @brief Initialize the OV3360 camera
        * 
        * @return ESP_OK if successfull
        */
        esp_err_t initCamera(camera_config_t* cameraConfig);

        /**
         * @brief Initialize the config for the camera
         * 
         * @returns The config for the camera to be initialized with
         */
        camera_config_t* initCameraConfig();

        /**
         * @brief captures 1 frame from the camera
         */
        void captureFrame();

        /**
         * 
         */
        void captureAudio();


};