"""Language options verification — navigates to Setup > Game and checks all language entries."""

import time
import pytest
from conftest import navigate_to_main_menu

EXPECTED_LANGUAGES = [
    "[System default]",
    "English",
    "French",
    "German",
    "Polish",
    "Russian",
    "Czech",
    "Brazilian Portuguese",
    "Magyar",
]


@pytest.fixture()
def on_setup_game(client):
    """Navigate to Setup > Gameplay tab from any screen."""
    navigate_to_main_menu(client)
    client.click("ButtonSetup")
    time.sleep(0.5)
    # The setup screen may open on any tab — navigate to gameplay tab explicitly.
    client.click("ButtonTabGameplay")
    client.wait_for_screen("SetupGame", timeout=10)


def test_setup_game_screen_reachable(client, on_setup_game):
    assert client.state()["screen"] == "SetupGame"


def test_setup_game_has_language_list(client, on_setup_game):
    w = client.find_widget("ListLanguage")
    assert w is not None
    assert w["type"] == "list"


def test_language_list_item_count(client, on_setup_game):
    w = client.find_widget("ListLanguage")
    items = w.get("items", [])
    assert len(items) == len(EXPECTED_LANGUAGES), (
        f"Expected {len(EXPECTED_LANGUAGES)} languages, got {len(items)}: {items}"
    )


def test_all_languages_present(client, on_setup_game):
    w = client.find_widget("ListLanguage")
    items = w.get("items", [])
    missing = [lang for lang in EXPECTED_LANGUAGES if lang not in items]
    assert not missing, f"Missing languages: {missing} (list: {items})"


def test_language_order(client, on_setup_game):
    w = client.find_widget("ListLanguage")
    items = w.get("items", [])
    assert items == EXPECTED_LANGUAGES, f"Language order differs:\n  got:      {items}\n  expected: {EXPECTED_LANGUAGES}"


def test_can_select_language(client, on_setup_game):
    """Verify the language list accepts a selection."""
    client.select("ListLanguage", item="English")
    w = client.find_widget("ListLanguage")
    assert w is not None
    assert w.get("selected", -1) >= 0


def test_back_returns_to_main_menu(client, on_setup_game):
    client.click("ButtonBack")
    client.wait_for_screen("MainMenu")
    assert client.state()["screen"] == "MainMenu"
