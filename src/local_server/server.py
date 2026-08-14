'''
Local server for the robot's audio/video pipeline.

Handles a UDP audio stream (voice activity detection, buffering, and
transcription via whisper) and a TCP video stream (JPEG frame
reassembly and object detection via YOLO), then forwards recognized
speech and camera frames to a RobotAgent instance running in its own
thread.
'''
import os
import multiprocessing
import threading
import socket
import wave
import numpy as np
import webrtcvad
import cv2
from RobotAgent import RobotAgent

from pywhispercpp.model import Model
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
HEAD_ADDR          = "192.168.86.28"
FIVE_SECONDS       = 5

class FrameStore:
    '''
    Class to store a frame variable to be shared between the RobotAgent thread 
    and the runVideoServer thread

    Attributes:
        lock 
        frame
    '''
    def __init__(self):
        '''
        Constructor for FrameStore object
        '''
        self.lock = threading.Lock()
        self.frame = bytes()

    def set_frame(self, frame: bytes):
        '''
        Set frame stored in object
        '''
        with self.lock:
            self.frame = frame

    def get_frame(self) -> bytes:
        '''
        Get Frame stored in object
        '''
        with self.lock:
            return self.frame

def open_wav_file(file_path: str) -> wave.Wave_write:
    '''
    Open a write only wave file given a file path 

    Args:
        file_path: The file path to write to

    Returns:
        Wave_write object set to the given file path
    '''
    os.makedirs(os.path.dirname(file_path), exist_ok=True)
    wav_file = wave.open(file_path, "wb")
    wav_file.setnchannels(NUM_CHANNELS)
    wav_file.setsampwidth(SAMPLE_WIDTH_BYTES)
    wav_file.setframerate(WAV_FRAME_RATE_HZ)
    return wav_file

def init_audio_socket(address: str, port: int) -> socket:
    '''
    Initialize the socket used for the audio server

    Args:
        address: The address for which the socket to connect to
        port: The the port for which the socket to connect to
    
    Returns:
        Socket to use for the audio server
    '''
    audio_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    audio_socket.bind((address, port))
    audio_socket.connect((HEAD_ADDR, PORT))
    return audio_socket

def init_video_socket(address: str, port: int) -> socket:
    '''
    Initialize the socket used for the video server

    Args: 
        address: The address for which the socket to connect to
        port: The the port for which the socket to connect to

    Returns:
        Socket to use for the video server
    '''
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((address, port))
    server.listen(1)
    return server

def condition_audio(audio: bytearray) -> np.ndarray:
    '''
    Condition the audio to a normalized array

    Args:
        audio: the unnormalized bytearray
    
    Returns:
        Normalized numpy array
    '''
    audio = np.frombuffer(audio, dtype=np.int16)
    array_normalized = np.frombuffer(audio, dtype=np.int16).astype(np.float32) / 32768.0
    return array_normalized

def condition_frame(frame: bytes) -> np.ndarray:
    '''
    Conditions jpeg bytes into numpy array

    Args:
        frame: jpeg bytes to be converted

    Returns:
        Numpy array containing jpeg
    '''
    frame = np.frombuffer(frame, dtype=np.uint8)
    decoded = cv2.imdecode(frame, cv2.IMREAD_COLOR) # pylint: disable=no-member
    if decoded is None:
        raise ValueError("Invalid or incomplete JPEG data")

    return decoded

def recv_exact(sock: socket, n: int) -> bytes:
    """
    Read exactly n bytes from a socket, blocking until all bytes arrive.

    Args:
        sock: The connected socket to read from.
        n: The exact number of bytes to read.

    Returns:
        The bytes read, of length n.

    Raises:
        ConnectionError: If the socket closes before n bytes are received.
    """
    buf = bytearray()
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("socket closed")
        buf.extend(chunk)
    return bytes(buf)

def transcribe(model: Model, audio_queue: multiprocessing.Queue, text_queue: multiprocessing.Queue):
    '''
    Thread responsible for infering text from speech

    Args:
        model: The model used to make inference from
        audio_queue: The queue containing audio bytes for inference
        text_queue: The queue being loaded with infered text
    '''
    buffer = bytearray()
    while True:
        chunk = audio_queue.get()  # blocks until something arrives
        if chunk is None:
            if buffer:
                segments = model.transcribe(condition_audio(buffer))
                for segment in segments:
                    print(segment.text)
                    text_queue.put(segment.text)
                buffer.clear()  # reset for next utterance
        else:
            buffer.extend(chunk)

def detect_objects(model: YOLO, jpeg_queue: multiprocessing.Queue):
    '''
    Thread responsible for object detection

    Args:
        model: The computer vision model to detect objects in camera frame
        jpeg_queue: Queue containing jpeg bytes
    '''
    while True:
        frame = condition_frame(jpeg_queue.get())
        results = model.predict(
            source=frame,
            save=True,
            verbose=True,
            conf=0.5
        )

def run_audio_server(socket_udp: socket, audio_queue: multiprocessing.Queue):
    '''
    Thread to run local audio server

    Args:
        socket_udp: The socket communicating with the robot over udp protocol
        audio_queue: The queue to laod with audio bytes
    '''
    wav_file = open_wav_file(WAV_PATH)
    vad = webrtcvad.Vad(mode=2)
    silent_frames = 0
    while True:
        data = socket_udp.recvfrom(UDP_BYTES_RECV)[0]
        detection = vad.is_speech(data, WAV_FRAME_RATE_HZ)
        if detection:
            audio_queue.put(data)
            wav_file.writeframes(data)
            silent_frames = 0
        else:
            silent_frames += 1
            if silent_frames == SILENCE_LIMIT:
                audio_queue.put(None)  # sentinel: utterance ended

def run_video_server(socket_tcp: socket, jpeg_queue: multiprocessing.Queue, frame_store: FrameStore):
    '''
    Thread to run local video server on

    Args:
        socket_tcp: The socket communicating with the robot over tcp protocol
        jpeg_queue: The queue for which to load jpeg bytes captured from the robot
        frame_store: The object to update with the latest frame
    '''
    with open("./out/video/test.mjpeg", "wb") as mjpeg_file:
        while True:
            client_socket, addr = socket_tcp.accept()
            client_socket.settimeout(FIVE_SECONDS)
            print(f"video client connected: {addr}")
            while True:
                try:
                    length_bytes = recv_exact(client_socket, 4)
                    frame_len = int.from_bytes(length_bytes, byteorder="little")
                    frame = recv_exact(client_socket, frame_len)
                    frame_store.set_frame(frame)
                    jpeg_queue.put(frame)
                    mjpeg_file.write(frame)
                except (ConnectionError, OSError) as e:
                    print(f"video socket dropped: {e}")
                    client_socket.close()
                    break

def robot_agent_thread(queue: multiprocessing.Queue, socket_udp: socket, frame_store: FrameStore):
    '''
    Thread to run the llm agent responsible for controlling the robot

    Args:
        queue: The queue to get transcribed speech from
        socket_udp: The socket to send text from llm back to the robot over udp protocol
        frame_store: The object to pull the latest jpeg frame from
    '''
    robot_agent = RobotAgent()
    # test_prompt = ("Well hello there. I'm Sam, and here's a long sentence"
    # "for you. I think that one of the most fascinating things is that the"
    # " quick brown fox jumped over the wall")
    while True:
        speech=queue.get()
        robot_agent.updateSenses(frame_store.get_frame())
        response = robot_agent.sendHumanSpeech(speech=speech)
        print(response)
        packet_size = socket_udp.send(response.encode())
        print(packet_size)
        if  packet_size < len(response):
            print("full response not sent")

        # TESTING
        # socket_udp.send(test_prompt.encode())
        # time.sleep(10)
