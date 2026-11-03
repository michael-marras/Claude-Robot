"""
Unit tests for RobotAgent.

Mocks the Anthropic client (no real API calls) and the diary file
(no real filesystem access), so tests run in isolation.
"""
import base64
from unittest.mock import MagicMock, mock_open, patch

import pytest

import RobotAgent as robot_agent_module
from RobotAgent import RobotAgent, MAX_CONVERSTAION_SIZE, MAX_MEMORY


# ---------------------------------------------------------------------------
# Fixtures / helpers
# ---------------------------------------------------------------------------

@pytest.fixture
def mock_anthropic_client():
    """Patch the Anthropic class so RobotAgent never hits the real API."""
    with patch.object(robot_agent_module, "Anthropic") as mock_cls:
        yield mock_cls.return_value


@pytest.fixture
def agent(mock_anthropic_client):
    """A RobotAgent with the Anthropic client and diary file mocked out."""
    with patch("builtins.open", mock_open(read_data="short memory")):
        return RobotAgent()


def make_text_message(text, stop_reason="end_turn"):
    """Build a fake Anthropic message containing a single text block."""
    block = MagicMock()
    block.type = "text"
    block.text = text
    msg = MagicMock()
    msg.stop_reason = stop_reason
    msg.content = [block]
    return msg


def make_tool_use_message(tool_name="get_frame", tool_id="tool_1"):
    """Build a fake Anthropic message requesting a tool call."""
    block = MagicMock()
    block.type = "tool_use"
    block.name = tool_name
    block.id = tool_id
    msg = MagicMock()
    msg.stop_reason = "tool_use"
    msg.content = [block]
    return msg


# ---------------------------------------------------------------------------
# __init__
# ---------------------------------------------------------------------------

def test_init_creates_client_and_empty_conversation(agent, mock_anthropic_client):
    assert agent.client is mock_anthropic_client
    assert agent.conversation == []


def test_init_loads_memory_from_diary(agent):
    assert agent.memory == "short memory"


def test_init_sets_empty_latest_frame(agent):
    assert agent.latest_frame == bytes()


# ---------------------------------------------------------------------------
# updateMemory
# ---------------------------------------------------------------------------

def test_update_memory_reads_diary_contents(mock_anthropic_client):
    diary_text = "Robot remembers the kitchen.\n"
    with patch("builtins.open", mock_open(read_data=diary_text)):
        agent_obj = RobotAgent()
    assert agent_obj.memory == diary_text


def test_update_memory_clears_when_too_long(mock_anthropic_client):
    long_diary = "x" * (MAX_MEMORY + 1)
    with patch("builtins.open", mock_open(read_data=long_diary)):
        agent_obj = RobotAgent()
    assert agent_obj.memory == ""


# ---------------------------------------------------------------------------
# sendHumanSpeech
# ---------------------------------------------------------------------------

def test_send_human_speech_returns_text_and_updates_conversation(agent):
    agent.client.messages.create.return_value = make_text_message("Hello there.")

    result = agent.sendHumanSpeech("hi robot")

    assert result == "Hello there."
    assert agent.conversation[0] == {"role": "user", "content": "hi robot"}
    assert agent.conversation[-1] == {"role": "assistant", "content": "Hello there."}


def test_send_human_speech_handles_get_frame_tool_use(agent):
    tool_use_msg = make_tool_use_message()
    final_msg = make_text_message("Here's what I see.")
    agent.client.messages.create.side_effect = [tool_use_msg, final_msg]
    # getFrameTool is patched out here so this test only covers
    # sendHumanSpeech's control flow, independent of the attribute bug
    # noted above.
    agent.getFrameTool = MagicMock(return_value=[{"type": "tool_result"}])

    result = agent.sendHumanSpeech("what do you see?")

    assert result == "Here's what I see."
    assert agent.client.messages.create.call_count == 2
    agent.getFrameTool.assert_called_once()


def test_send_human_speech_pops_oldest_message_when_over_limit(agent):
    agent.conversation = [
        {"role": "user", "content": f"msg-{i}"} for i in range(MAX_CONVERSTAION_SIZE)
    ]
    agent.client.messages.create.return_value = make_text_message("ok")

    agent.sendHumanSpeech("new message")

    # Two messages are appended (user + assistant) then one is popped,
    # so the conversation grows by one and the oldest entry is dropped.
    assert len(agent.conversation) == MAX_CONVERSTAION_SIZE + 1
    assert agent.conversation[0]["content"] == "msg-1"


# ---------------------------------------------------------------------------
# convertBytesToJPEG
# ---------------------------------------------------------------------------

def test_convert_bytes_to_jpeg_returns_base64_string(agent):
    raw = b"fake-jpeg-bytes"
    encoded = agent.convertBytesToJPEG(raw)
    assert encoded == base64.b64encode(raw).decode("utf-8")


def test_convert_bytes_to_jpeg_raises_on_empty_frame(agent):
    with pytest.raises(RuntimeError):
        agent.convertBytesToJPEG(bytes())


# ---------------------------------------------------------------------------
# updateSenses
# ---------------------------------------------------------------------------

def test_update_senses_updates_latest_frame(agent):
    agent.updateSenses(b"new-frame-bytes")
    assert agent.latest_frame == b"new-frame-bytes"
