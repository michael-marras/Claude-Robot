# Claude Robot
## An esp32 robot programmed in c++ using espidf that is controlled by Claude opus 4.7
This is a fully autonomous robot that utilizes computer vision, and speech-to-text/text-to-speech to send
processed data for claude to direct outputs of the robot

## Project Status
- In Progress

## Architecture
### Parts Used
- Head
  - Seed Studio Xiao Esp32S3
  - MAX98357A
    - mini 4-8 ohm Speaker
  - servo motor
  - 0.96" SSD1306 I2C OLED
- body
  - ESP32-DevKitC-32 Development Board
  - 8x MG90S all-metal micro servos
  - Buck converter (5V out, ~5A) — steps the 7.4V battery down to ~5V for the servos. Required: 7.4V would damage the MG90S (rated 4.8–6V). Sized at 5A for headroom with 8 servos.
  - Battery connector/pigtail — match the battery's connector (XT30 or JST); don't cut the stock leads.
  - 1000µF electrolytic capacitor (10V+) — across the buck output to smooth servo current surges and prevent brownout resets.
  - Silicone wire — 22AWG for power/ground, 30AWG for signal leads.
  
### Technologies Used
- Ultralytics Yolov11 image classifier
- Open AI Whisper
- espidf

## How To Contribute
1. Read the [style guide](docs/style-guide.md)
2. Make a branch

## Build & Test
### Prerequisites
### Compile and flash the code
### Run tests

## Usage
