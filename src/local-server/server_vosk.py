import socket
import wave
import threading
import numpy as np 
import multiprocessing
import webrtcvad
import json
import collections

from vosk import Model, KaldiRecognizer

PORT               = 9997
HEADER             = 64
WAV_PATH           = "./out/audio/test.wav"
VAD_PATH           = "./models/ggml-silero-v6.2.0.bin"
TXT_PATH           = "./out/audio/test.txt"
NUM_FRAMES         = 40000
NUM_CHANNELS       = 1
SAMPLE_WIDTH_BYTES = 2
WAV_FRAME_RATE_HZ  = 16000
TARGET_BYTES       = 320000
COMMANDS           = ["claude", "walk", "move", "forward", "backward", "left", "right", "stop", "sit", "[unk]"]

def openWavFile(file_path, mode):
    wavFile = wave.open(file_path, mode)
    wavFile.setnchannels(NUM_CHANNELS)
    wavFile.setsampwidth(SAMPLE_WIDTH_BYTES)
    wavFile.setframerate(WAV_FRAME_RATE_HZ)
    return wavFile

def initAudioServer():
    server1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server1.bind(('0.0.0.0', PORT))

    return server1

def initVideoServer():
    server2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server2.bind(('0.0.0.0', PORT))
    server2.listen(1)

    mjpegFile = open("./out/video/test.mjpeg", "wb")
    client, addr = server2.accept()
    
    return client, mjpegFile

def conditionAudio(audio):
    audio = np.frombuffer(audio, dtype=np.int16)
    audio = (audio - audio.mean()).astype(np.int16)
    return audio.tobytes()
    
def transcribe(model, queue):
    rec = KaldiRecognizer(model, WAV_FRAME_RATE_HZ, json.dumps(COMMANDS))

    while True:
        if not queue.empty():
            if (rec.AcceptWaveform(queue.get())):
                print(rec.Result())
        
def runAudioServer(socketUDP, queue):
    wavFile = openWavFile(WAV_PATH, "wb")

    while True:
        data, addr = socketUDP.recvfrom(320)
        data = conditionAudio(data)
        queue.put(data)
        wavFile.writeframes(data)
            
def runVideoServer(socketTCP, file):
    while True:
        videoData = socketTCP.recv(1024)
        file.write(videoData)

# Entry Point
queue = multiprocessing.Queue()

print("Server Initializing")

socketUDP = initAudioServer()
socketTCP, mjpegFile = initVideoServer()

model = Model(model_name = "vosk-model-en-us-0.42-gigaspeech")

thread1 = threading.Thread(target=runAudioServer, args=(socketUDP, queue))
thread2 = threading.Thread(target=runVideoServer, args=(socketTCP, mjpegFile))
thread3 = threading.Thread(target=transcribe, args=(model, queue))

thread1.start()
thread2.start()
thread3.start()