"""
test_08 — /launch, /program, /diagnostics, and rich /objects endpoints.

Verifies that an AI agent can:
  1. POST /launch — start a level directly without menu navigation.
  2. GET /objects  — read rich object state (energy, shield, rotation, etc.).
  3. GET /program  — read source and compile state.
  4. POST /program — write source and compile; check error on bad code.
  5. POST /program — write valid source and compile; check compiled=true.
  6. GET /diagnostics — verify state reflects compile result.
"""

import time
import sys
import os

sys.path.insert(0, os.path.dirname(__file__))
import pytest
from conftest import navigate_to_main_menu
from agent_client import AgentClient

VALID_SOURCE = "extern void object() { move(5); }\n"
BAD_SOURCE   = "extern void object() { move(5) }\n"  # missing semicolon


def _enter_exercise_with_robot(client: AgentClient):
    """Navigate to exercise 1-1, enter the level, select a robot."""
    navigate_to_main_menu(client)
    client.click("ButtonExercises")
    client.wait_for_screen("LevelSelect", timeout=10)
    client.select("ListChapter", index=0)
    time.sleep(0.3)
    client.select("ListLevel", index=0)
    time.sleep(0.3)
    client.click("ButtonPlay")
    client.wait_for_screen("InGame", timeout=30)
    time.sleep(1.0)
    assert client.select_programmable_robot(), "No programmable robot found in level"
    # Add a fresh runnable program slot
    if client.find_widget("ButtonAddProgram") is not None:
        client.click("ButtonAddProgram")
        time.sleep(0.3)


@pytest.fixture(autouse=True)
def return_to_menu(client):
    yield
    navigate_to_main_menu(client)


def test_launch_starts_level(client: AgentClient):
    """/launch transitions directly from MainMenu to InGame."""
    navigate_to_main_menu(client)
    client.launch("Exercises", 1, 1)
    client.wait_for_screen("InGame", timeout=30)


def test_objects_rich_state(client: AgentClient):
    """Objects in an active level expose pos, rotation, and optional energy/shield."""
    navigate_to_main_menu(client)
    client.launch("Exercises", 1, 1)
    client.wait_for_screen("InGame", timeout=30)
    time.sleep(1.0)

    objs = client.objects()
    assert len(objs) > 0, "No objects returned for active level"

    for obj in objs:
        assert "id" in obj
        assert "type" in obj
        assert "pos" in obj
        assert "x" in obj["pos"] and "y" in obj["pos"] and "z" in obj["pos"]
        assert "rotation" in obj
        assert "x" in obj["rotation"]

    # Programmable robots should expose program_running
    prog_objs = [o for o in objs if "program_running" in o]
    assert len(prog_objs) > 0, "No programmable objects found"


def test_window_geometry(client: AgentClient):
    """GET /window returns coherent window + display geometry."""
    info = client.window()

    # Basic shape
    assert "logical_w"  in info and "logical_h"  in info
    assert "drawable_w" in info and "drawable_h" in info
    assert "scale_x"    in info and "scale_y"    in info
    assert "display_bounds"  in info
    assert "display_usable"  in info
    assert "flags"           in info

    # Sanity checks
    assert info["logical_w"] > 0 and info["logical_h"] > 0
    assert info["drawable_w"] >= info["logical_w"]  # drawable ≥ logical (HiDPI)
    assert info["scale_x"] >= 1.0 and info["scale_y"] >= 1.0

    # Display usable area must fit inside display bounds
    usable  = info["display_usable"]
    bounds  = info["display_bounds"]
    assert usable["w"] <= bounds["w"] and usable["h"] <= bounds["h"], \
        "Usable area wider/taller than full display — OS dock data corrupt"

    # Window must fit within usable area (not clipped into dock/taskbar)
    assert info["logical_w"] <= bounds["w"], \
        f"Window logical_w ({info['logical_w']}) wider than display ({bounds['w']})"


def test_drag_noop(client: AgentClient):
    """POST /drag returns from/to coordinates without error."""
    # Just verify the endpoint responds correctly; visual drag effect is manual-test territory.
    result = client.drag(0.3, 0.8, 0.5, 0.7, steps=5)
    assert "from" in result and "to" in result
    assert result["steps"] == 5


def test_mouse_move(client: AgentClient):
    """POST /mouse_move resolves interface coords to pixel coords."""
    result = client.mouse_move(0.5, 0.5)
    info = client.window()
    # Center of screen should map to roughly half the window size
    assert abs(result["px"] - info["logical_w"] // 2) <= 2
    # y is flipped: interface y=0.5 → SDL py ≈ logical_h/2
    assert abs(result["py"] - info["logical_h"] // 2) <= 2


def test_get_program_requires_robot(client: AgentClient):
    """GET /program on main menu returns 404 (no robot selected)."""
    navigate_to_main_menu(client)
    import requests as req
    r = req.get(f"{client.url}/program", timeout=5)
    assert r.status_code == 404


def test_program_read_write_compile(client: AgentClient):
    """Full cycle: enter level, set source, compile, read back."""
    _enter_exercise_with_robot(client)

    # Initial read — slot should exist
    prog = client.get_program()
    assert "slot" in prog
    assert "slot_count" in prog
    assert prog["runnable"] is True
    assert isinstance(prog["source"], str)

    # Write bad source and compile — expect error
    result = client.set_program(BAD_SOURCE, compile=True)
    assert result["compiled"] is False
    assert result["error"] is not None
    assert "message" in result["error"]

    # Write valid source and compile — expect success
    result = client.set_program(VALID_SOURCE, compile=True)
    assert result["compiled"] is True
    assert result["error"] is None

    # Read back — source and compiled flag should match
    prog = client.get_program()
    assert prog["compiled"] is True
    assert prog["source"] == VALID_SOURCE


def test_diagnostics_reflects_compile(client: AgentClient):
    """GET /diagnostics shows compile state after set_program."""
    _enter_exercise_with_robot(client)

    # Before compile
    diag = client.diagnostics()
    assert diag["selected_robot"] is True
    assert diag["running"] is False

    # Compile bad code — diagnostics should report error
    client.set_program(BAD_SOURCE, compile=True)
    diag = client.diagnostics()
    assert diag["compiled"] is False
    assert diag["error"] is not None

    # Compile good code — diagnostics should clear error
    client.set_program(VALID_SOURCE, compile=True)
    diag = client.diagnostics()
    assert diag["compiled"] is True
    assert diag["error"] is None
    assert diag["running"] is False


def test_compile_helper(client: AgentClient):
    """compile() helper returns None on success, error dict on failure."""
    _enter_exercise_with_robot(client)

    err = client.compile(BAD_SOURCE)
    assert err is not None
    assert "message" in err

    err = client.compile(VALID_SOURCE)
    assert err is None
