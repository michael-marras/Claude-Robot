import socket
import wave

numChannels = 1
sampleWidthBytes = 2
wavFrameRateHz = 16000

server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server.bind(('0.0.0.0', 9999))

wavFile = wave.open("./out/audio/test.wav", 'wb')
wavFile.setnchannels(numChannels)
wavFile.setsampwidth(sampleWidthBytes)
wavFile.setframerate(wavFrameRateHz)
wavFileInitPos = 0
waveFileEndPos = 160000

print("server initiated")

while True:
    data, addr = server.recvfrom(512)
    print(data[0])
    wavFile.writeframes(data)
    if wavFile.tell() >= waveFileEndPos:
        wavFile.setpos(wavFileInitPos)
    