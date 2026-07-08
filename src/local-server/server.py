import socket
import wave
import threading
import shutil
import numpy as np 
import queue

from pywhispercpp.model import Model

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
GAIN               = 3

def openWavFile(file_path, mode):
    wavFile = wave.open(file_path, mode)
    wavFile.setnchannels(NUM_CHANNELS)
    wavFile.setsampwidth(SAMPLE_WIDTH_BYTES)
    wavFile.setframerate(WAV_FRAME_RATE_HZ)
    return wavFile

def initAudioServer():
    server1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server1.bind(('0.0.0.0', PORT))

    wavFile = openWavFile(WAV_PATH, "wb")
    textFile = open(TXT_PATH, "w")

    return server1, wavFile, textFile

def initVideoServer():
    server2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server2.bind(('0.0.0.0', PORT))
    server2.listen(1)

    mjpegFile = open("./out/video/test.mjpeg", "wb")
    client, addr = server2.accept()
    
    return client, mjpegFile

def conditionAudio(audio):
    numpy_array = np.frombuffer(audio, dtype = np.int16)
    array_normalized = numpy_array.astype(np.float32) / 32768.0

    return GAIN * array_normalized

def transcribe(model, queue):
    # Condition the audio from queue
    while True:
        array_normalized = conditionAudio(queue.get())

        # Transcribe and print
        segments = model.transcribe(array_normalized)
        for segment in segments:
            print(segment.text)

def runAudioServer(socketUDP, wavFile, textFile, audioQ):
    buffer = bytearray()

    while True:
        data, addr = socketUDP.recvfrom(512)
        buffer.extend(data)
        wavFile.writeframes(data)

        if wavFile.tell() >= NUM_FRAMES:
            buffer_copy = buffer.copy()
            audioQ.put(buffer_copy)
            buffer.clear()
            wavFile.close()
            shutil.copy(WAV_PATH, "chunk.wav")
            wavFile = openWavFile(WAV_PATH, "wb")
            
def runVideoServer(socketTCP, file):
    while True:
        videoData = socketTCP.recv(1024)
        file.write(videoData)

# Entry Point
audioQ = queue.Queue()

print("Server Initializing")

socketUDP, wavFile, textFile = initAudioServer()
socketTCP, mjpegFile = initVideoServer()

model = Model(
    model = 'small.en', 
    print_realtime=False, 
    print_progress=False,
    audio_ctx=192,
    suppress_blank=True,
    single_segment=True
)

thread1 = threading.Thread(target = runAudioServer, args = (socketUDP, wavFile, textFile, audioQ))
thread2 = threading.Thread(target = runVideoServer, args = (socketTCP, mjpegFile))
thread3 = threading.Thread(target = transcribe, args = (model, audioQ))

thread1.start()
thread2.start()
thread3.start()