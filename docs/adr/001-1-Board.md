# Title
Using One dev Board

## Status
Pending

## Context
The robot needs to be able to smoothly move while making visual inference

## Decision
I'm proposing to use one board to handle sensory and motor functions. I've realized that since inference will be running locally on a laptop and all the robot will be doing is capturing audio/visual data and roundtripping that and llm commands, we should have enough overhead to control motion and audio/visual data smoothly.

## Consequences
- No UART comms between boards
- No extra wiring
- Heat could be a problem
- Stuttering could be an issue
