import socket
import wave
import numpy as np 
import webrtcvad
import cv2

from ultralytics import YOLO

PORT               = 9997
WAV_PATH           = "./out/audio/test.wav"
TXT_PATH           = "./out/audio/test.txt"
NUM_CHANNELS       = 1
SAMPLE_WIDTH_BYTES = 2
WAV_FRAME_RATE_HZ  = 16000
SILENCE_LIMIT      = 20  
UDP_BYTES_RECV     = 320
TCP_BYTES_RECV     = 4096

def openWavFile(file_path):
    wavFile = wave.open(file_path, "wb")
    wavFile.setnchannels(NUM_CHANNELS)
    wavFile.setsampwidth(SAMPLE_WIDTH_BYTES)
    wavFile.setframerate(WAV_FRAME_RATE_HZ)
    return wavFile

def initAudioServer(address, port):
    server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server.bind((address, port))
    return server

def initVideoServer(address, port):
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((address, port))
    server.listen(1)
    server, addr = server.accept()
    return server

def conditionAudio(audio):
    audio = np.frombuffer(audio, dtype=np.int16)
    array_normalized = np.frombuffer(audio, dtype=np.int16).astype(np.float32) / 32768.0
    return array_normalized

def conditionFrame(frame):
    frame = np.frombuffer(frame, dtype=np.uint8)
    decoded = cv2.imdecode(frame, cv2.IMREAD_COLOR)
    if decoded is None:
        raise ValueError("Invalid or incomplete JPEG data")
    
    return decoded

def recvExact(sock, n):
    buf = bytearray()
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("socket closed")
        buf.extend(chunk)
    return bytes(buf)
    
def transcribe(model, queue):
    buffer = bytearray()
    while True:
        chunk = queue.get()  # blocks until something arrives
        if chunk is None:
            if buffer:
                segments = model.transcribe(conditionAudio(buffer))
                for segment in segments:
                    print(segment.text)
                buffer.clear()  # reset for next utterance
        else:
            buffer.extend(chunk)

def detectObjects(model, queue):
    while True:
        frame = conditionFrame(queue.get())
        results = model.predict(
            source=frame,
            save=True
        )

        for result in results:
            print(result.verbose())
    
def runAudioServer(socketUDP, queue):
    wavFile = openWavFile(WAV_PATH)
    vad = webrtcvad.Vad(mode=2)
    silentFrames = 0
    while True:
        data, addr = socketUDP.recvfrom(UDP_BYTES_RECV)
        detection = vad.is_speech(data, WAV_FRAME_RATE_HZ)
        if detection:
            queue.put(data)
            wavFile.writeframes(data)
            silentFrames = 0
        else:
            silentFrames += 1
            if silentFrames == SILENCE_LIMIT:
                queue.put(None)  # sentinel: utterance ended

def runVideoServer(socketTCP, queue):
    mjpegFile = open("./out/video/test.mjpeg", "wb")
    while True:
        lengthBytes = recvExact(socketTCP, 4)
        frameLen = int.from_bytes(lengthBytes, byteorder="little")
        frame = recvExact(socketTCP, frameLen)
        queue.put_nowait(frame)
        mjpegFile.write(frame)
