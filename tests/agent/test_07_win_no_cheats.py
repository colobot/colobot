"""Win Exercise chapter 1, level 1 without cheats by programming the robot.

Level goal: "Kill three spiders with a small program."
Robot: WheeledShooter at pos(0,0) with camera=BACK.

Strategy:
  1. Click the robot to open the Studio.
  2. Replace its program with a spider-killing loop using radar/turn/fire.
  3. Click StudioRun — this compiles and starts execution in one step.
  4. Close the studio (StudioOK) so the robot keeps running.
  5. Wait up to 30 s for the win screen (InGameMenu with ButtonAgain).
"""

import time
import pytest
from conftest import navigate_to_main_menu


# CBot program: repeatedly radar the nearest AlienSpider, turn to face it, fire.
SPIDER_KILL_PROGRAM = (
    "extern void object::SpiderKill()\n"
    "{\n"
    "    object target;\n"
    "    int i;\n"
    "    for( i = 0; i < 3; i++ )\n"
    "    {\n"
    "        target = radar(AlienSpider);\n"
    "        if( target == null ) break;\n"
    "        turn(direction(target.position));\n"
    "        fire(1);\n"
    "        wait(0.5);\n"
    "    }\n"
    "}\n"
)


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

@pytest.fixture(scope="module")
def fresh_level(client):
    """Start exercise chapter 1 level 1 from MainMenu. Shared across the module."""
    navigate_to_main_menu(client)

    client.click("ButtonExercises")
    client.wait_for_screen("LevelSelect")
    client.select("ListChapter", index=0)
    time.sleep(0.3)
    client.select("ListLevel", index=0)
    time.sleep(0.3)
    client.click("ButtonPlay")
    client.wait_for_screen("InGame", timeout=30)
    time.sleep(1.5)   # let the level settle before any clicks
    yield
    # Teardown — return to MainMenu so the session ends cleanly.
    try:
        screen = client.state()["screen"]
        if screen == "Studio":
            client.click("StudioCancel")
            client.wait_for_screen("InGame", timeout=5)
            screen = "InGame"
        if screen == "LevelComplete":
            client.click("ButtonEndLevel")
            client.wait_for_screen("LevelSelect", timeout=10)
            screen = "LevelSelect"
        if screen == "LevelSelect":
            client.click("ButtonBack")
            client.wait_for_screen("MainMenu", timeout=10)
            return
        if screen == "InGame":
            client.key("Escape")
            client.wait_for_screen("InGameMenu", timeout=5)
            screen = "InGameMenu"
        if screen == "InGameMenu":
            client.click("ButtonAbort")
            client.wait_for_screen("MainMenu", timeout=10)
    except Exception:
        pass


@pytest.fixture(scope="module")
def studio_ready(client, fresh_level):
    """Select a programmable robot and open its Studio with a program selected."""
    if not client.open_studio():
        pytest.skip("Could not open Studio for any programmable robot.")
    yield


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

def test_level_reached_ingame(client, fresh_level):
    assert client.state()["screen"] == "InGame"


def test_studio_opens_on_robot_click(client, studio_ready):
    assert client.state()["screen"] == "Studio"


def test_studio_has_edit_and_run(client, studio_ready):
    state = client.state()
    ids = [w["id"] for w in state["widgets"]]
    assert "StudioEdit" in ids
    assert "StudioRun" in ids


def test_program_runs_and_wins(client, studio_ready):
    """Write the spider-kill program, run it, and expect the win screen."""
    # Replace whatever is in the editor with our program.
    client.type("StudioEdit", SPIDER_KILL_PROGRAM)
    time.sleep(0.2)

    # StudioRun compiles and starts execution in one click.
    client.click("StudioRun")
    time.sleep(0.3)

    # Close the studio so the robot keeps running without the UI in the way.
    client.click("StudioOK")

    # Wait for the win screen. Exercise levels with an EndingFile show a
    # LevelComplete cinematic; levels without show InGameMenu with ButtonAgain.
    deadline = time.monotonic() + 30
    win_screen = None
    while time.monotonic() < deadline:
        screen = client.state()["screen"]
        if screen in ("LevelComplete", "InGameMenu"):
            win_screen = screen
            break
        time.sleep(0.5)

    assert win_screen is not None, "Win screen not reached within 30s"
    if win_screen == "LevelComplete":
        assert client.find_widget("ButtonEndLevel") is not None
    else:
        widget_ids = [w["id"] for w in client.state()["widgets"]]
        assert "ButtonAgain" in widget_ids, f"Expected ButtonAgain, got: {widget_ids}"
