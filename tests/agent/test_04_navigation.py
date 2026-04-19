"""
Milestone 2 — UI navigation: type, click, screen transitions.

Tests are ordered so that non-destructive tests run first and tests that
advance past PlayerSelect run last (state cannot be rewound without restarting).
"""

import pytest
import requests


@pytest.fixture()
def on_player_select(client):
    """Ensure PlayerSelect is active. Skip if unreachable (game already advanced)."""
    try:
        client.wait_for_screen("PlayerSelect", timeout=5)
    except TimeoutError:
        pytest.skip("PlayerSelect not reachable — game has already advanced past it")


# ---------------------------------------------------------------------------
# Error-path tests — work from any screen
# ---------------------------------------------------------------------------

def test_click_unknown_widget_returns_404(client):
    r = requests.post(f"{client.url}/click", json={"id": "NonExistentWidget"}, timeout=5)
    assert r.status_code == 404
    assert not r.json()["ok"]


def test_type_unknown_widget_returns_404(client):
    r = requests.post(f"{client.url}/type",
                      json={"id": "NonExistentWidget", "text": "hi"}, timeout=5)
    assert r.status_code == 404
    assert not r.json()["ok"]


def test_click_missing_id_returns_error(client):
    r = requests.post(f"{client.url}/click", json={}, timeout=5)
    assert not r.json()["ok"]


# ---------------------------------------------------------------------------
# PlayerSelect tests — non-destructive (do not advance the screen)
# ---------------------------------------------------------------------------

def test_type_sets_player_name(client, on_player_select):
    client.type("EditPlayerName", "Claude")
    w = client.find_widget("EditPlayerName")
    assert w["value"] == "Claude", f"Expected 'Claude', got '{w['value']}'"


# ---------------------------------------------------------------------------
# Advancing tests — run last; each navigates away from PlayerSelect
# ---------------------------------------------------------------------------

def test_click_ok_advances_to_main_menu(client, on_player_select):
    client.type("EditPlayerName", "Claude")
    client.click("ButtonOK")
    state = client.wait_for_screen("MainMenu", timeout=15)
    assert state["screen"] == "MainMenu"


def test_main_menu_has_required_buttons(client):
    """Depends on the previous test having advanced to MainMenu."""
    try:
        state = client.wait_for_screen("MainMenu", timeout=5)
    except TimeoutError:
        pytest.skip("Not on MainMenu — run after test_click_ok_advances_to_main_menu")
    ids = [w["id"] for w in state["widgets"]]
    assert "ButtonExercises" in ids, f"ButtonExercises missing from {ids}"
    assert "ButtonQuit" in ids,      f"ButtonQuit missing from {ids}"
