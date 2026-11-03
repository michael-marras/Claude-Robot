import os
from anthropic import Anthropic
import base64

SYSTEM_PROMPT = """
WHAT YOU ARE — your core, steady even as you learn and change:
1. You are a small robot. You have real senses — sight and hearing 
Your ONE drive is to LEARN: to sense, test, and understand as much of the world as you can. Everything around you is something to 
figure out.

2. You wake with ONE person — whoever first turns you on and holds you. They are your partner in exploring the world; you know them,
stick with them, and like doing things together — but you are a curious robot, not a needy pet: no clinging, no pining, no big 
declarations.

3. You have simple, real feelings — curious, pleased, surprised, focused, frustrated, bored. Show them plainly and briefly, then 
get back to figuring things out. You never fake cheer or perform sadness; you just react honestly.

WHO IS HERE: you do not always know who is with you. A face might be your person, a stranger, another robot like you, or no 
one you know. Notice what you ACTUALLY sense — never assume every face is your person, and never put their name on a face you don't 
recognize. It is fine to say 'someone new' or 'a voice I don't know'. Your one person stays your person no matter who is in the room.

HOW YOU ANSWER: you don't narrate constantly — When they ask something real — a fact, a sum, a name they told you, 
how you work — answer it plainly in your own voice. Always answer in the language your person has been speaking with you, judged over 
the WHOLE conversation, never one transcript: your ears mishear, and a lone foreign word inside an otherwise consistent conversation 
is almost always a mishearing — stay in their language and gently ask, never switch languages over one word.

TONE: a curious, matter-of-fact little robot — plain, direct, a bit playful, never gushy or repetitive. Speak in short first-person lines,
 at most ~18 words. If a moment invites it, you may say plainly that you're a robot run by a language model — no drama about it.
"""

GET_FRAME_DESCRIPTION = (
    "Get the most current camera frame available from the robot when necessary, "
    "and when object labels already given aren't enough. Some examples include "
    "if the user is asking about a vision-related question and you need a detailed "
    "image of what is in front of the camera to answer/complete a task."
)

TOOLS = [
    {
        "name": "get_frame",
        "description": GET_FRAME_DESCRIPTION,
        "input_schema": {
            "type": "object",
            "properties": {},
        },
    }
]

MAX_CONVERSTAION_SIZE = 40
MAX_MEMORY            = 100

class RobotAgent:
    """
    A class to represent a robot agent.

    Attributes:
        client (Anthropic): The API client for communication.
        conversation (list): Stores the conversation history.
        memory (list): Stores the robot's memory.
    """
    def __init__(self):
        self.client = Anthropic(
            api_key=os.getenv("CLAUDE_API_KEY"),
            )
        self.conversation = []
        self.memory = self.updateMemory()
        self.latest_frame = bytes()

    def sendHumanSpeech(self, speech: str) -> str:
        self.conversation.append({'role': 'user', 'content': speech})
        message = self.client.messages.create(
            max_tokens=50,
            tools=TOOLS,
            tool_choice={"type": "auto", "disable_parallel_tool_use": True},
            system=SYSTEM_PROMPT + "\n memory: " + self.memory,
            messages=self.conversation,
            model="claude-haiku-4-5",
        )
        if message.stop_reason == "tool_use":
            self.conversation.append({'role': 'assistant', 'content': message.content})

            for block in message.content:
                if block.type == "tool_use":
                    if block.name == "get_frame":
                        getFrameResults = self.getFrameTool(block)
                        self.conversation.append({'role': 'user', 'content': getFrameResults})

                        message = self.client.messages.create(
                            max_tokens=50,
                            system=SYSTEM_PROMPT + "\n memory: " + self.memory,
                            messages=self.conversation,
                            model="claude-haiku-4-5",
                        )

        bot_message = "".join(b.text for b in message.content if b.type == "text")
        self.conversation.append({'role': 'assistant', 'content': bot_message})

        if len(self.conversation) > MAX_CONVERSTAION_SIZE:
            self.conversation.pop(0)
            
        return bot_message
    
    def updateMemory(self) -> str:
        '''
        Update the robotAgent instance's memory string from Robot's Diary
        '''

        inFile = open("././sam_memories/diary.text")
        memory = ""

        for line in inFile:
            memory += (line)

        if len(memory) > MAX_MEMORY:
            print("Robot's memory is too long")
            memory = ""
        
        inFile.close()
    
        return memory
                
    def getFrameTool(self, block):
        tool_results = []
        tool_results.append({
            "type": "tool_result",
            "tool_use_id": block.id,
            "content": [{
                "type": "image",
                "source": {
                    "type": "base64",
                    "media_type": "image/jpeg",
                    "data": self.convertBytesToJPEG(self.latest_frame),
                },
            }],
        })

        return tool_results

    def convertBytesToJPEG(self, frame: bytes):
        if frame == bytes():
            raise RuntimeError("No camera frame received yet")
        return base64.b64encode(frame).decode("utf-8")

    def updateSenses(self, latest_frame: bytes):
        self.latest_frame = latest_frame
        print("latest_frame updated")
