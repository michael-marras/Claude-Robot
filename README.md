# Claude Robot
## An autonomous robot powered by claude Haiku 4.5
This is a fully autonomous robot that utilizes computer vision, and speech-to-text/text-to-speech to send
processed data for claude to direct outputs of the robot

## Project Status
- In Progress

## Architecture
### Parts Used
- Head
  - Seeed Studio Xiao Esp32S3 Sense
  - MAX98357A
    - mini 4-8 ohm Speaker
  - servo motor
  - 0.96" SSD1306 I2C OLED
- Body (Quadruped)
  - ESP32-DevKitC-32 Development Board
  - 8x MG90S all-metal micro servos
  - Buck converter (5V out, ~5A) — steps the 7.4V battery down to ~5V for the servos. Required: 7.4V would damage the MG90S (rated 4.8–6V). Sized at 5A for headroom with 8 servos.
  - Battery connector/pigtail — match the battery's connector (XT30 or JST); don't cut the stock leads.
  - 1000µF electrolytic capacitor (10V+) — across the buck output to smooth servo current surges and prevent brownout resets.
  - Silicone wire — 22AWG for power/ground, 30AWG for signal leads.
  
### Technologies Used
- C++ (Arduino framework)
- PlatformIO (build/flash)
- Ultralytics YOLOv11 (vision)
- OpenAI Whisper (speech-to-text)
- Anthropic Claude API (reasoning + tool-calling)

### Flow of Data
![Wiring diagram](docs/images/data-flow.jpg)

## How To Contribute
1. Read the [style guide](docs/style-guide.md)
2. Create a branch (`feat/your-feature`, `fix/your-fix`, `docs your-doc`)
3. Make your changes
4. Ensure all new functions/methods include Doxygen comments
5. Open a PR and fill out the checklist
6. PRs require review before merging into main

## Build & Test
### Prerequisites
- [PlatformIO Core](https://platformio.org/install/cli) (CLI) or the PlatformIO VS Code extension
    - **Github CI and native tests will not work with only PlatformIo Core**

### Connect esp32 boards to computer
- When initially connecting either esp32 boards via usb to your computer, you might find that flashing will fail or your device isn't detected. In that case you want to do the following

1. Unplug usb cable from computer
2. Hold boot button on s3
3. While holding boot button, reinsert usb and hold for 2 mississippi
4. Release boot button
5. If these steps don't work initially, retry from step 1

### Compile and flash the code
```bash
pio run -e <env_name> -t upload
```

### Run tests
## Running tests on the seeed xiao esp32 s3 sense

- Unit tests on the S3 are bugged so you gotta follow this weird workaround to run tests on the board
1. Build and flash
```bash
pio test -e test_head --without-testing
```

2. Run this command, and it should stall out
```bash
pio device monitor -e test_head --rts 1 --dtr 1
```

3. Click the reset button on S3

4. Should give stalled output, but in the output you'll find test results
```bash
test/test_head_head/test_head_head.cpp:21:test_camera_init_valid:PASS
test/test_head_head/test_head_head.cpp:22:test_init_valid:PASS
-----------------------
2 Tests 0 Failures 0 Ignored 
OK
```

### Test output files
- the s3 will deliver audio and video data to the server and may write to files in /out
```bash
ffplay <relative-path-to-mjpeg-file>
```

