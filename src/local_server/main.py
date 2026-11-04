'''
Initializes TCP/UDP sockets, the Whisper transcription model, and the
YOLO26 object detection model, then spawns worker threads for audio
capture, video capture, speech transcription, object detection, and
the Claude-based robot agent loop.
'''
import multiprocessing
import threading

from server import (
    init_video_socket,
    init_audio_socket,
    Model,
    YOLO,
    FrameStore,
    run_audio_server,
    run_video_server,
    transcribe,
    detect_objects,
    robot_agent_thread,
)

ADDRESS = '0.0.0.0'
PORT    = 9997

print("Server Initializing")
audio_q = multiprocessing.Queue()
frame_q = multiprocessing.Queue()
tts_q   = multiprocessing.Queue()
socketTCP = init_video_socket(ADDRESS, PORT)
socketUDP = init_audio_socket(ADDRESS, PORT)

transcription_model = Model(
    model = 'small.en',
    print_realtime=False,
    print_progress=False,
    print_timestamps=False,
    single_segment=True,
    no_context=True,
    suppress_non_speech_tokens=True
)

object_detection_model = YOLO(
    model="yolo26m.pt",
    verbose=False
)

frame_store = FrameStore()

thread1 = threading.Thread(target=run_audio_server, args=(socketUDP, audio_q))
thread2 = threading.Thread(target=run_video_server, args=(socketTCP, frame_q, frame_store))
thread3 = threading.Thread(target=transcribe, args=(transcription_model, audio_q, tts_q))
thread4 = threading.Thread(target=detect_objects, args=(object_detection_model, frame_q))
thread5 = threading.Thread(target=robot_agent_thread, args=(tts_q, socketUDP, frame_store))
thread1.start()
thread2.start()
thread3.start()
thread4.start()
thread5.start()
