from server import *

ADDRESS = '0.0.0.0'
PORT    = 9997

# Entry Point 
print("Server Initializing")
audioQ = multiprocessing.Queue()
frameQ = multiprocessing.Queue()
ttsQ   = multiprocessing.Queue()
socketTCP = init_video_socket(ADDRESS, PORT)
socketUDP = init_audio_socket(ADDRESS, PORT)

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

frame_store = FrameStore()

thread1 = threading.Thread(target=run_audio_server, args=(socketUDP, audioQ))
thread2 = threading.Thread(target=run_video_server, args=(socketTCP, frameQ, frame_store))
thread3 = threading.Thread(target=transcribe, args=(transcriptionModel, audioQ, ttsQ))
thread4 = threading.Thread(target=detect_objects, args=(objectDetectionModel, frameQ))
thread5 = threading.Thread(target=robot_agent_thread, args=(ttsQ, socketUDP, frame_store))
thread1.start()
thread2.start()
thread3.start()
thread4.start()
thread5.start()
