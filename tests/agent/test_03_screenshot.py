"""Milestone 3 — Screenshot: GL readback and OS capture modes."""

import pytest
from conftest import navigate_to_main_menu

PNG_HEADER = b"\x89PNG\r\n\x1a\n"


# ---------------------------------------------------------------------------
# GL readback (default — works everywhere, no deps)
# ---------------------------------------------------------------------------

def test_gl_screenshot_returns_png(client):
    png = client.screenshot(source="gl")
    assert png[:8] == PNG_HEADER, "Response is not a valid PNG"


def test_gl_screenshot_is_not_empty(client):
    png = client.screenshot(source="gl")
    assert len(png) > 10_000, f"PNG too small ({len(png)} bytes) — likely a blank frame"


def test_gl_screenshot_default_is_gl(client):
    """Calling /screenshot with no params should use GL readback."""
    import base64
    import requests
    r = requests.get(f"{client.url}/screenshot", timeout=15)
    assert r.status_code == 200
    body = r.json()
    assert body["ok"]
    png = base64.b64decode(body["data"]["png"])
    assert png[:8] == PNG_HEADER


@pytest.fixture()
def on_main_menu(client):
    """Navigate to MainMenu — reachable from any screen without a game restart."""
    navigate_to_main_menu(client)
    client.wait_for_screen("MainMenu", timeout=10)


def test_gl_screenshot_visual_regression(client, on_main_menu, update_snapshots):
    """Compare MainMenu against a saved baseline (first run saves it)."""
    if update_snapshots:
        client.save_snapshot("main_menu")
        return
    # 10% tolerance: the main menu has an animated background that varies per frame.
    client.assert_snapshot("main_menu", tolerance=0.10)


# ---------------------------------------------------------------------------
# OS screenshot (?source=os) — requires display server + screencapture tool
# ---------------------------------------------------------------------------

def test_os_screenshot_returns_png_or_clear_error(client):
    """
    OS screenshot may fail if no display/tool is available.
    Accepts either a valid PNG or a clean error — never a malformed response.
    """
    import base64
    import requests
    r = requests.get(f"{client.url}/screenshot?source=os", timeout=15)
    body = r.json()
    if body["ok"]:
        png = base64.b64decode(body["data"]["png"])
        assert png[:8] == PNG_HEADER, "OS screenshot is not a valid PNG"
    else:
        assert "error" in body and body["error"]


@pytest.mark.os_screenshot
def test_os_screenshot_content(client):
    """
    Full OS screenshot validation. Run with -m os_screenshot only on platforms
    with a display server and screencapture tool installed.
    """
    png = client.screenshot(source="os")
    assert png[:8] == PNG_HEADER
    assert len(png) > 10_000
