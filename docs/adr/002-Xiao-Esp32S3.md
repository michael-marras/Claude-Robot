# Title 
Xiao Esp32S3

## Status
Pending

## Context
We need to be able to send visual and audio data to the llm. So we need a camera and a microphone to be on our robot.

## Decision
I'm proposing that we use the Seed Studio Xiao Esp32S3 as the eyes and ears of our robot as it is an esp32 board with a built in camera and microphone with wifi/bluetooth connectivity. 

## Consequences
- Don't need to setup external camera and microphone
- Onboard Jpeg compression making data transfer over wifi faster
- Small form factor