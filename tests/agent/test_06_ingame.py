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
from conftest import navigate_to_main_menu



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
    navigate_to_main_menu(client)

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
    # Tear-down: return to MainMenu from wherever the level left things.
    try:
        screen = client.state()["screen"]
        if screen == "LevelComplete":
            client.click("ButtonEndLevel")
            client.wait_for_screen("LevelSelect", timeout=10)
            screen = "LevelSelect"
        if screen == "Studio":
            client.click("StudioCancel")
            client.wait_for_screen("InGame", timeout=5)
            screen = "InGame"
        if screen == "InGame":
            client.key("Escape")
            client.wait_for_screen("InGameMenu", timeout=5)
            screen = "InGameMenu"
        if screen == "InGameMenu":
            client.click("ButtonAbort")
            client.wait_for_screen("MainMenu", timeout=10)
        elif screen == "LevelSelect":
            client.click("ButtonBack")
            client.wait_for_screen("MainMenu", timeout=10)
    except Exception:
        pass


# ---------------------------------------------------------------------------
# Level entry
# ---------------------------------------------------------------------------

def test_exercise_level_loads(client, in_exercise_level):
    """Level 1 of exercises should reach InGame screen."""
    assert client.state()["screen"] == "InGame"


def test_ingame_hud_has_widgets(client, in_exercise_level):
    """Verify the InGame HUD exposes at least the console and speed widgets."""
    state = client.state()
    ids = [w["id"] for w in state["widgets"]]
    # The console edit field is always present in-game (hidden until backtick pressed).
    assert "EditConsole" in ids, f"EditConsole missing from InGame HUD: {ids}"


# ---------------------------------------------------------------------------
# Script studio
# ---------------------------------------------------------------------------

@pytest.fixture(scope="module")
def studio_open(client, in_exercise_level):
    """Select a programmable robot and open its Studio with a program selected."""
    if not client.open_studio():
        pytest.skip("Could not open Studio for any programmable robot.")
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
    """winmission triggers the win ending scene (LevelComplete) with a continue button."""
    client.console("winmission")
    # Levels with EndingFile show a cinematic ending scene before returning to LevelSelect.
    client.wait_for_screen("LevelComplete", timeout=10)
    assert client.find_widget("ButtonEndLevel") is not None
    # Dismiss the ending screen so the session returns to a clean state.
    client.click("ButtonEndLevel")
    client.wait_for_screen("LevelSelect", timeout=10)
