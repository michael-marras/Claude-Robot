# Title
Using Two dev Boards

## Status
Pending

## Context
The robot needs to be able to smoothly move while making visual inference as well as have enough gpio pins

## Decision
I'm proposing to use two boards to handle sensory and motor functions seperately.

## Consequences
- More Gpio pins for expandability
- More processing power
- Have to setup UART communication between the boards

