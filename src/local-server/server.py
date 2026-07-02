from pywhispercpp import Model
import socket
import wave
import threading

HEADER = 64
NUM_CHANNELS = 1
SAMPLE_WIDTH_BYTES = 2
WAV_FRAME_RATE_HZ = 16000
PORT = 9999

def initAudioServer():
    server1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server1.bind(('0.0.0.0', PORT))

    wavFile = wave.open("./out/audio/test.wav", 'wb')
    wavFile.setnchannels(NUM_CHANNELS)
    wavFile.setsampwidth(SAMPLE_WIDTH_BYTES)
    wavFile.setframerate(WAV_FRAME_RATE_HZ)
    wavFileInitPos = 0
    waveFileEndPos = 160000

    return server1, wavFile

def initVideoServer():
    server2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server2.bind(('0.0.0.0', PORT))
    server2.listen(1)

    mjpegFile = open("./out/video/test.mjpeg", "wb")

    client, addr = server2.accept()
    return client, mjpegFile

def runAudioServer(socketUDP, file):
    while True:
        data, addr = socketUDP.recvfrom(512)
        file.writeframes(data)

def runVideoServer(socketTCP, file):
    while True:
        videoData = socketTCP.recv(1024)
        file.write(videoData)

print("Server Initializing")

socketUDP, wavFile = initAudioServer()
socketTCP, mjpegFile = initVideoServer()

thread1 = threading.Thread(target = runAudioServer, args = (socketUDP, wavFile))
thread2 = threading.Thread(target = runVideoServer, args = (socketTCP, mjpegFile))

thread1.start()
thread2.start()
    