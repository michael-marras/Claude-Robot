from server import *
import wave

WAV_PATH = "./out/audio/test.wav"
ADDRESS  = '0.0.0.0'
PORT     = 9997

def test_sanity():
    assert 1 + 1 == 2

def test_openWavFile():
    wavFile = openWavFile(WAV_PATH)
    assert(wavFile)
    assert(isinstance(wavFile, wave.Wave_write))
    wavFile.close()

def test_conditionAudio():
    audioBuffer = bytearray()
    for i in range(320):
        audioBuffer.extend(b'/x00')
    conditionedAudio = conditionAudio(audioBuffer)
    assert(conditionedAudio.all())

test_sanity()
test_openWavFile()
test_conditionAudio()