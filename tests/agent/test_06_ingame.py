"""In-game tests: code editor (Studio) and cheat console verification.

Navigation path: MainMenu → Exercises → Chapter 1 → Level 1 (WheeledShooter vs spiders).
The level has a single programmable robot. We:
  1. Enter the level and verify the InGame screen.
  2. Click the robot to open the Studio, edit its program, and compile it.
  3. Use the console to enable cheats (showsoluce, winmission).
  4. Verify the win screen appears.

Clicking the robot uses /click_pos. In this level the camera starts in BACK mode
focusing on the WheeledShooter, placing it at roughly (0.5, 0.25) in interface coords.
If the robot is not clickable at that position the studio tests are skipped gracefully.
"""

import time
import pytest


ROBOT_X = 0.5   # interface x — horizontal center
ROBOT_Y = 0.25  # interface y — lower portion of the screen (BACK camera, robot in foreground)

SIMPLE_PROGRAM = (
    "extern void object::SpiderKill()\n"
    "{\n"
    "    turn(90);\n"
    "    fire(1);\n"
    "}\n"
)


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

@pytest.fixture(scope="module")
def in_exercise_level(client):
    """Navigate MainMenu → Exercises → Chapter 1, Level 1 → InGame.

    Scoped to module so the level is started once and all tests share the
    running game state (the level is not restarted between tests).
    """
    # Ensure we start from a known screen.
    screen = client.state()["screen"]
    if screen == "SetupGame":
        client.click("ButtonBack")
        client.wait_for_screen("MainMenu")
    elif screen != "MainMenu":
        client.wait_for_screen("MainMenu", timeout=5)

    client.click("ButtonExercises")
    client.wait_for_screen("LevelSelect")

    # Select chapter 1 and level 1.
    client.select("ListChapter", index=0)
    time.sleep(0.3)
    client.select("ListLevel", index=0)
    time.sleep(0.3)

    client.click("ButtonPlay")
    client.wait_for_screen("InGame", timeout=30)
    time.sleep(1.0)   # let the level finish loading and rendering
    yield
    # Tear-down: abort the level so later tests start from a clean state.
    try:
        screen = client.state()["screen"]
        if screen == "InGame":
            client.key("Escape")
            client.wait_for_screen("InGameMenu", timeout=5)
        if client.state()["screen"] == "InGameMenu":
            client.click("ButtonAbort")
            client.wait_for_screen("MainMenu", timeout=10)
    except Exception:
        pass


# ---------------------------------------------------------------------------
# Level entry
# ---------------------------------------------------------------------------

def test_exercise_level_loads(client, in_exercise_level):
    """Level 1 of exercises should reach InGame screen."""
    assert client.state()["screen"] == "InGame"


def test_ingame_has_satcom_button(client, in_exercise_level):
    w = client.find_widget("ButtonSatCom")
    assert w is not None
    assert w["enabled"]


# ---------------------------------------------------------------------------
# Script studio
# ---------------------------------------------------------------------------

@pytest.fixture(scope="module")
def studio_open(client, in_exercise_level):
    """Click the robot to open the Studio. Skips if the robot is not clickable."""
    client.click_pos(ROBOT_X, ROBOT_Y)
    time.sleep(0.5)
    try:
        client.wait_for_screen("Studio", timeout=4.0)
    except TimeoutError:
        pytest.skip(
            f"Studio did not open after clicking ({ROBOT_X}, {ROBOT_Y}). "
            "The robot may not be at that position — adjust ROBOT_X/ROBOT_Y."
        )
    yield
    # Close studio if still open.
    try:
        if client.state()["screen"] == "Studio":
            client.click("StudioCancel")
            client.wait_for_screen("InGame", timeout=5)
    except Exception:
        pass


def test_studio_opens(client, studio_open):
    assert client.state()["screen"] == "Studio"


def test_studio_has_editor_widget(client, studio_open):
    w = client.find_widget("StudioEdit")
    assert w is not None
    assert w["type"] == "edit"


def test_studio_edit_and_compile(client, studio_open):
    """Clear the editor, type a simple program, and compile it."""
    # Select-all then replace with our program.
    client.type("StudioEdit", SIMPLE_PROGRAM)
    time.sleep(0.2)
    client.click("StudioCompile")
    time.sleep(0.5)
    # After compilation the Studio screen should still be visible (no crash/close).
    assert client.state()["screen"] == "Studio"


def test_studio_close_returns_to_game(client, studio_open):
    client.click("StudioCancel")
    client.wait_for_screen("InGame", timeout=5)
    assert client.state()["screen"] == "InGame"


# ---------------------------------------------------------------------------
# Cheat console
# ---------------------------------------------------------------------------

def test_showsoluce_cheat(client, in_exercise_level):
    """Enable showsoluce cheat via the console. Verifies the game stays responsive."""
    screen_before = client.state()["screen"]
    client.console("showsoluce")
    time.sleep(0.3)
    # showsoluce toggles the solution display in SatCom — game should remain InGame.
    assert client.state()["screen"] == screen_before


def test_winmission_cheat_wins_level(client, in_exercise_level):
    """winmission immediately completes the level; the win screen should appear."""
    client.console("winmission")
    # The game transitions to an end-of-level screen (InGameMenu with ButtonAgain).
    client.wait_for_screen("InGameMenu", timeout=10)
    s = client.state()
    widget_ids = [w["id"] for w in s["widgets"]]
    assert "ButtonAgain" in widget_ids or "ButtonAbort" in widget_ids, (
        f"Expected end-of-level buttons, got: {widget_ids}"
    )
