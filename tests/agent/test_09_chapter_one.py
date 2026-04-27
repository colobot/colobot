"""
test_09 — Complete every level in Exercises Chapter 1 without cheats.

For each of the 7 levels the test:
  1. Launches the level directly via /launch.
  2. Opens the Studio (adds a fresh runnable program slot).
  3. Writes the official solution script (sourced from the game's *.txt files).
  4. Clicks StudioRun (compile + start in one action) then StudioOK.
  5. Waits for the LevelComplete screen (all chapter-1 exercises have EndingFile).

Solutions come from the level data files, e.g.
  data/levels/exercises/chapter001/level001/tspid1.txt
They are reproduced here verbatim so the test has no filesystem dependency.

Each test is independent — /launch resets the level so there is no shared state.
"""

import time
import sys
import os

import pytest

sys.path.insert(0, os.path.dirname(__file__))
from conftest import navigate_to_main_menu
from agent_client import AgentClient

# ---------------------------------------------------------------------------
# Official solution scripts (verbatim from the game data *.txt files)
# ---------------------------------------------------------------------------

# level001 — Spiders 1  (tspid1.txt)
# Aim at each of the three alien spiders in turn and fire.
TSPID1 = """extern void object::Example()
{
\taim(0);
\tfire(1);

\tturn(90);
\tfire(1);

\tturn(-180);
\tfire(1);
}
"""

# level002 — Power Cell 1  (tcell1.txt)
# Pick up a power cell from one robot and give it to another.
TCELL1 = """extern void object::Example()
{
\tgrab();
\tturn(90);
\tdrop();

\tturn(-180);

\tgrab();
\tturn(90);
\tdrop();
}
"""

# level003 — Titanium 1  (ttit1.txt)
# Drive forward, collect titanium ore, return and deposit.
TTIT1 = """extern void object::Example()
{
\tmove(20);
\tgrab();

\tturn(180);
\tmove(30);
\tdrop();
\tmove(-3);
}
"""

# level004 — Titanium 2  (ttit2.txt)
# Use radar to find ore and converter; navigate and deposit.
TTIT2 = """extern void object::Example()
{
\tobject    item;

\titem = radar(TitaniumOre);
\tgoto(item.position);
\tgrab();

\titem = radar(Converter);
\tgoto(item.position);
\tdrop();
\tmove(-3);
}
"""

# level005 — Power Cell 2  (tcell2.txt)
# Continuously find power cells and ferry them to winged shooters.
TCELL2 = """extern void object::Example()
{
\tobject    item;

\twhile(true)
\t{
\t\titem = radar(PowerCell);
\t\tgoto(item.position);
\t\tgrab();

\t\titem = radar(WingedShooter);
\t\tgoto(item.position);
\t\tdrop();
\t}
}
"""

# level006 — Spiders 2  (tspid2.txt)
# Continuously radar nearest spider, turn to face it, and fire.
TSPID2 = """extern void object::Example()
{
\tobject    item;

\twhile (true)
\t{
\t\titem = radar(AlienSpider);
\t\tturn(direction(item.position));
\t\tfire(1);
\t}
}
"""

# level007 — Spiders 3  (tspid3.txt)
# Radar spider, turn, advance to firing range, then fire.
TSPID3 = """extern void object::Example()
{
\tobject    item;

\twhile (true)
\t{
\t\titem = radar(AlienSpider);
\t\tturn(direction(item.position));
\t\tmove(distance(position, item.position)-40);
\t\tfire(1);
\t}
}
"""

# ---------------------------------------------------------------------------
# Test parameters: (rank, title, source, timeout_seconds)
# Timeout is generous — CI machines can be slow and tcell2 loops until
# all four WingedShooters are fuelled.
# ---------------------------------------------------------------------------

LEVELS = [
    (1, "Spiders 1",    TSPID1, 40),
    (2, "Power Cell 1", TCELL1, 40),
    (3, "Titanium 1",   TTIT1,  40),
    (4, "Titanium 2",   TTIT2,  90),   # goto() pathfinds around quartz clusters; converter delay adds ~13s after drop
    (5, "Power Cell 2", TCELL2, 90),   # while(true) loop; ends when all bots fuelled
    (6, "Spiders 2",    TSPID2, 45),
    (7, "Spiders 3",    TSPID3, 60),
]


# ---------------------------------------------------------------------------
# Helper
# ---------------------------------------------------------------------------

def _await_win(client: AgentClient, timeout: float, label: str) -> str:
    """Poll /state until LevelComplete; fail on timeout."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        screen = client.state()["screen"]
        if screen == "LevelComplete":
            return screen
        time.sleep(0.2)
    pytest.fail(f"{label}: LevelComplete not reached within {timeout}s "
                f"(last screen: {client.state()['screen']})")


def _teardown(client: AgentClient):
    """Drive back to MainMenu from any end-of-level screen."""
    try:
        client.set_speed(1.0)
    except Exception:
        pass
    try:
        screen = client.state()["screen"]
        if screen == "LevelComplete":
            client.click("ButtonEndLevel")
            client.wait_for_screen("LevelSelect", timeout=10)
            screen = "LevelSelect"
        if screen in ("LevelSelect", "InGameMenu"):
            client.click("ButtonBack" if screen == "LevelSelect" else "ButtonAbort")
            client.wait_for_screen("MainMenu", timeout=10)
        elif screen == "InGame":
            client.key("Escape")
            client.wait_for_screen("InGameMenu", timeout=5)
            client.click("ButtonAbort")
            client.wait_for_screen("MainMenu", timeout=10)
    except Exception:
        pass


# ---------------------------------------------------------------------------
# Parametrized test
# ---------------------------------------------------------------------------

@pytest.mark.parametrize(
    "rank,title,source,win_timeout",
    LEVELS,
    ids=[f"level{r:03d}-{t.lower().replace(' ', '_')}" for r, t, _, _ in LEVELS],
)
def test_exercise_chapter1_complete(
    client: AgentClient,
    rank: int,
    title: str,
    source: str,
    win_timeout: int,
):
    """Launch the level, inject the official solution, and verify it wins."""
    label = f"Exercises/ch1/lvl{rank} ({title})"

    # ── 1. Enter the level ────────────────────────────────────────────────
    navigate_to_main_menu(client)
    client.launch("Exercises", 1, rank)
    client.wait_for_screen("InGame", timeout=30)
    time.sleep(1.2)   # let physics/spawn settle before issuing robot commands
    client.set_speed(16.0)

    # ── 2. Open Studio, reusing an existing matching slot when possible ────
    if not client.open_studio(expected_source=source):
        _teardown(client)
        pytest.skip(f"{label}: no programmable robot found")

    assert client.state()["screen"] == "Studio", f"{label}: Studio did not open"

    # ── 3. Write official solution (only if editor content differs) and run ─
    edit = client.find_widget("StudioEdit")
    if not edit or edit.get("value", "").strip() != source.strip():
        client.type("StudioEdit", source)
        time.sleep(0.2)
    client.click("StudioRun")    # compile + start execution in one click
    time.sleep(0.3)
    client.click("StudioOK")    # close editor; robot keeps executing

    # ── 4. Wait for win ────────────────────────────────────────────────────
    win_screen = _await_win(client, win_timeout, label)

    # ── 5. Assert and clean up ─────────────────────────────────────────────
    assert win_screen == "LevelComplete", f"{label}: unexpected screen '{win_screen}'"
    assert client.find_widget("ButtonEndLevel") is not None, \
        f"{label}: LevelComplete reached but ButtonEndLevel missing"

    _teardown(client)
