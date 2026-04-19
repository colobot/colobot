"""
pytest fixtures for the Colobot agent server test suite.

Two modes:
  1. Connect to a running server (local dev):
       COLOBOT_AGENT_URL=http://localhost:7777 pytest tests/agent/

  2. Auto-start Colobot (CI / devcontainer):
       pytest tests/agent/ --colobot-bin=./build/colobot --colobot-datadir=./build/data
"""

import os
import platform
import subprocess
import time
from typing import Generator

import pytest
import requests

from agent_client import AgentClient


# ---------------------------------------------------------------------------
# CLI options
# ---------------------------------------------------------------------------

def pytest_addoption(parser):
    parser.addoption("--colobot-bin",     default=None, help="Path to colobot binary")
    parser.addoption("--colobot-datadir", default=None, help="Path to colobot data dir")
    parser.addoption("--colobot-port",    default=7777,  type=int, help="Agent server port")
    parser.addoption("--update-snapshots", action="store_true",
                     help="Overwrite visual regression baselines with current screenshots")


# ---------------------------------------------------------------------------
# Shared option: --update-snapshots
# ---------------------------------------------------------------------------

@pytest.fixture(scope="session")
def update_snapshots(request) -> bool:
    return request.config.getoption("--update-snapshots")


# ---------------------------------------------------------------------------
# Game server lifecycle
# ---------------------------------------------------------------------------

@pytest.fixture(scope="session")
def game_server(request) -> Generator[str, None, None]:
    """
    Yields the base URL of a running agent server.

    If COLOBOT_AGENT_URL is set, connects to it directly (no process management).
    Otherwise starts Colobot using --colobot-bin / --colobot-datadir.
    """
    url = os.environ.get("COLOBOT_AGENT_URL")
    if url:
        # Connect to an already-running instance — caller owns the process.
        _wait_for_server(url, timeout=5)
        yield url
        return

    binary = request.config.getoption("--colobot-bin")
    datadir = request.config.getoption("--colobot-datadir")
    port = request.config.getoption("--colobot-port")

    if not binary or not datadir:
        pytest.skip(
            "No running server found and --colobot-bin/--colobot-datadir not set. "
            "Set COLOBOT_AGENT_URL or pass --colobot-bin and --colobot-datadir."
        )

    xvfb = _start_xvfb()
    env = _build_env(xvfb)
    proc = _start_colobot(binary, datadir, port, env)

    base_url = f"http://127.0.0.1:{port}"
    try:
        _wait_for_server(base_url, timeout=30, process=proc)
        yield base_url
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
        if xvfb:
            xvfb.terminate()


@pytest.fixture(scope="session")
def client(game_server, update_snapshots) -> AgentClient:
    """Provides a ready AgentClient for the session."""
    c = AgentClient(game_server)
    c._update_snapshots = update_snapshots  # propagate flag for assert_snapshot
    return c


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _start_xvfb():
    """Start Xvfb on Linux if no display is available. Returns the process or None."""
    if platform.system() != "Linux":
        return None
    if os.environ.get("DISPLAY"):
        return None  # display already available
    try:
        proc = subprocess.Popen(
            ["Xvfb", ":99", "-screen", "0", "1024x768x24"],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
        )
        os.environ["DISPLAY"] = ":99"
        time.sleep(1)
        return proc
    except FileNotFoundError:
        pytest.skip("Xvfb not found — install xvfb or set DISPLAY")


def _build_env(xvfb) -> dict:
    env = os.environ.copy()
    env["LIBGL_ALWAYS_SOFTWARE"] = "1"
    env["GALLIUM_DRIVER"] = "llvmpipe"
    return env


def _start_colobot(binary: str, datadir: str, port: int, env: dict) -> subprocess.Popen:
    cmd = [
        binary,
        f"-agentserver", str(port),
        "-datadir", datadir,
        "-glversion", "3.3",
        "-glprofile", "core",
        "-resolution", "640x480",
    ]
    return subprocess.Popen(
        cmd, env=env,
        stdin=subprocess.DEVNULL,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


def _wait_for_server(url: str, timeout: float, process: subprocess.Popen = None):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if process and process.poll() is not None:
            pytest.fail(f"Colobot exited early (code {process.returncode})")
        try:
            r = requests.get(f"{url}/health", timeout=1)
            if r.status_code == 200 and r.json().get("ok"):
                return
        except requests.RequestException:
            pass
        time.sleep(0.5)
    pytest.fail(f"Agent server at {url} did not become ready within {timeout}s")
