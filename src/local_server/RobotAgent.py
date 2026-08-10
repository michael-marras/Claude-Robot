import os
from anthropic import Anthropic

SYSTEM_PROMPT = "You are going to be receiving speech from a person who is conversing with you. " \
"Pretend to be a sentient Robot." \
"Also you're name is Sam. Keep your responses concise and play along with the user." \
"Keep sentences under 15 words because the text synthesizer can only handle 60 phonemes." \
"Also try to break up long sentences with a comma often"

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

    def sendHumanSpeech(self, speech: str):
        self.conversation.append({'role': 'user', 'content': speech})
        message = self.client.messages.create(
            max_tokens=50,
            system=SYSTEM_PROMPT + "\n memory: " + self.memory,
            messages=self.conversation,
            model="claude-haiku-4-5",
        )
        bot_message = message.content[0].text
        self.conversation.append({'role': 'assistant', 'content': bot_message})

        if len(self.conversation) > MAX_CONVERSTAION_SIZE:
            self.conversation.pop(0)
        return bot_message
    
    def updateMemory(self):
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