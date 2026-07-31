import multiprocessing
import threading
from pywhispercpp.model import Model

from server import *

ADDRESS = '0.0.0.0'
PORT    = 9997

# Entry Point 
print("Server Initializing")
audioQ = multiprocessing.Queue()
frameQ = multiprocessing.Queue()
ttsQ   = multiprocessing.Queue()
socketTCP = initVideoServer(ADDRESS, PORT)
socketUDP = initAudioServer(ADDRESS, PORT)


transcriptionModel = Model(
    model = 'small.en', 
    print_realtime=False, 
    print_progress=False,
    print_timestamps=False,
    single_segment=True,
    no_context=True,
    suppress_non_speech_tokens=True
)

objectDetectionModel = YOLO(
    model="yolo26m.pt",
    verbose=False
)

thread1 = threading.Thread(target=runAudioServer, args=(socketUDP, audioQ))
thread2 = threading.Thread(target=runVideoServer, args=(socketTCP, frameQ))
thread3 = threading.Thread(target=transcribe, args=(transcriptionModel, audioQ, ttsQ))
thread4 = threading.Thread(target=detectObjects, args=(objectDetectionModel, frameQ))
thread5 = threading.Thread(target=sendLLMResponse, args=(ttsQ, socketUDP))
thread1.start()
thread2.start()
thread3.start()
thread4.start()
thread5.start()
