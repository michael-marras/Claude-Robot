# Title
Run Inference On Computer

## Status
Proposed

## Context
The Xiao Esp32S3 cannot handle ultralyitics yolov11 and open ai's optimized whisper. 

## Decision
I'm proposing to stream visual and audio data from the Xiao Esp32S3 over wifi to a computer that runs the audio/visual inference.

## Consequences
- Leaves more overhead for Xiao Esp32S3 to handle motor/sensory data alone
- Could add latency from wifi communication between laptop and Xiao Esp32S3 
- Cheaper than buying a devboard that can handle the inference