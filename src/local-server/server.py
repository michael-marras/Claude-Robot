import socket
import wave
import threading
import shutil
import numpy as np 
from pywhispercpp.model import Model

PORT               = 9998
HEADER             = 64
WAV_PATH           = "./out/audio/test.wav"
VAD_PATH           = "./models/ggml-silero-v6.2.0.bin"
TXT_PATH           = "./out/audio/test.txt"
NUM_FRAMES         = 16000
NUM_CHANNELS       = 1
SAMPLE_WIDTH_BYTES = 2
WAV_FRAME_RATE_HZ  = 16000

def initAudioServer():
    server1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server1.bind(('0.0.0.0', PORT))

    wavFile = wave.open(WAV_PATH, 'wb')
    wavFile.setnchannels(NUM_CHANNELS)
    wavFile.setsampwidth(SAMPLE_WIDTH_BYTES)
    wavFile.setframerate(WAV_FRAME_RATE_HZ)

    textFile = open(TXT_PATH, "w")

    return server1, wavFile, textFile

def initVideoServer():
    server2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server2.bind(('0.0.0.0', PORT))
    server2.listen(1)

    mjpegFile = open("./out/video/test.mjpeg", "wb")

    client, addr = server2.accept()
    return client, mjpegFile

def runInference(model, buffer, textFile):
    numpy_array = np.frombuffer(buffer, dtype = np.int16)
    array_normalized = numpy_array.astype(np.float32) / 32768.0
    segments = model.transcribe(array_normalized)
    for segment in segments:
        print(segment.text)

def runAudioServer(socketUDP, wavFile, model, textFile):
    buffer = bytearray()
    thread3 = None
    while True:
        data, addr = socketUDP.recvfrom(512)
        buffer.extend(data)
        wavFile.writeframes(data)
        if wavFile.tell() >= NUM_FRAMES:
            if thread3 is not None and thread3.is_alive():
                thread3.join()   
            buffer_copy = buffer.copy()
            buffer.clear()
            thread3 = threading.Thread(target = runInference, args = (model, buffer_copy, textFile))
            thread3.start()
            wavFile.close()
            shutil.copy(WAV_PATH, "chunk.wav")
            wavFile = wave.open(WAV_PATH, 'wb')
            wavFile.setnchannels(NUM_CHANNELS)
            wavFile.setsampwidth(SAMPLE_WIDTH_BYTES)
            wavFile.setframerate(WAV_FRAME_RATE_HZ)
            
def runVideoServer(socketTCP, file):
    while True:
        videoData = socketTCP.recv(1024)
        file.write(videoData)

print("Server Initializing")

socketUDP, wavFile, textFile = initAudioServer()
socketTCP, mjpegFile = initVideoServer()

model = Model(
    model = 'base.en', 
    print_realtime=False, 
    print_progress=False
)

thread1 = threading.Thread(target = runAudioServer, args = (socketUDP, wavFile, model, textFile))
thread2 = threading.Thread(target = runVideoServer, args = (socketTCP, mjpegFile))

thread1.start()
thread2.start()
    