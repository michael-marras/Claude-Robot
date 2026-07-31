import socket
import wave
import numpy as np 
import webrtcvad
import cv2
import espeak_ng
import ctypes

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
HEAD_ADDR          = "192.168.86.22"

def openWavFile(file_path):
    wavFile = wave.open(file_path, "wb")
    wavFile.setnchannels(NUM_CHANNELS)
    wavFile.setsampwidth(SAMPLE_WIDTH_BYTES)
    wavFile.setframerate(WAV_FRAME_RATE_HZ)
    return wavFile

def initAudioServer(address, port):
    server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server.bind((address, port))
    server.connect((HEAD_ADDR, PORT))
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
    
def transcribe(model, queue1, queue2):
    buffer = bytearray()
    while True:
        chunk = queue1.get()  # blocks until something arrives
        if chunk is None:
            if buffer:
                segments = model.transcribe(conditionAudio(buffer))
                for segment in segments:
                    print(segment.text)
                    queue2.put(segment.text)
                buffer.clear()  # reset for next utterance
        else:
            buffer.extend(chunk)

def textToSpeech(queue, socketUDP):
    espeak_ng.initialize(output=espeak_ng.espeak_AUDIO_OUTPUT.AUDIO_OUTPUT_RETRIEVAL)

    wf = wave.open("debug_tts.wav", "wb")
    wf.setnchannels(1)
    wf.setsampwidth(2)
    wf.setframerate(22050)

    def callback(wav, num_samples, event):
        try:
            if wav is not None and num_samples > 0:
                data = ctypes.string_at(wav, num_samples * 2)
                wf.writeframes(data)  # capture exactly what gets sent
                sent = socketUDP.send(data)
                if sent <= 0:
                    print("bytes not sent")
        except Exception as e:
            print(f"callback error: {e}")
        return 0
    
    espeak_ng.set_synth_callback(callback)
    while True:
        espeak_ng.synth(queue.get())

def detectObjects(model, queue):
    while True:
        frame = conditionFrame(queue.get())
        results = model.predict(
            source=frame,
            save=True,
            verbose=True,
            conf=0.5
        )

        # for result in results:
        #     print(result.verbose())
    
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
