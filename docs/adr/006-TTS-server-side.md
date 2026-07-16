# Title
TTS Run on Server

## Status
Pending

## Context
TTS could be run on either of the esp32 boards or the server

## Decision
Run the TTS on the server side

## Consequences
- Server now has to deal with additional work load
- TTS engine on the esp32 s3 have reported blocking issues with freertos multitaksing
- Do not have to deal with changing frameworks from aurdino to arduino and espidf
