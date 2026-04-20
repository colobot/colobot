"""Milestone 1 — /state screen detection and widget listing."""

import pytest
from conftest import navigate_to_player_select


@pytest.fixture()
def on_player_select(client):
    navigate_to_player_select(client)
    client.wait_for_screen("PlayerSelect")


def test_state_screen_is_player_select(client, on_player_select):
    assert client.state()["screen"] == "PlayerSelect"


def test_state_has_required_widgets(client, on_player_select):
    ids = client.widget_ids()
    assert "EditPlayerName" in ids, f"EditPlayerName missing from {ids}"
    assert "ButtonOK" in ids,       f"ButtonOK missing from {ids}"
    assert "ListPlayers" in ids,    f"ListPlayers missing from {ids}"


def test_edit_widget_has_value(client, on_player_select):
    w = client.find_widget("EditPlayerName")
    assert w is not None
    assert w["type"] == "edit"
    assert "value" in w


def test_ok_button_is_enabled(client, on_player_select):
    w = client.find_widget("ButtonOK")
    assert w is not None
    assert w["enabled"]


def test_delete_button_exists(client, on_player_select):
    w = client.find_widget("ButtonDelete")
    assert w is not None
    assert "enabled" in w


def test_widgets_have_required_fields(client, on_player_select):
    for w in client.state()["widgets"]:
        assert "id" in w
        assert "type" in w
        assert "enabled" in w
