"""HTTP client wrapper for the Colobot agent server."""

import base64
import time
from pathlib import Path
from typing import Optional

import requests

SNAPSHOTS_DIR = Path(__file__).parent / "snapshots"


class AgentClient:
    def __init__(self, url: str = "http://127.0.0.1:7777"):
        self.url = url.rstrip("/")
        self._session = requests.Session()

    # ------------------------------------------------------------------
    # Core API
    # ------------------------------------------------------------------

    def health(self) -> dict:
        return self._get("/health")

    def state(self) -> dict:
        return self._get("/state")

    def click(self, widget_id: str) -> dict:
        return self._post("/click", {"id": widget_id})

    def type(self, widget_id: str, text: str) -> dict:
        return self._post("/type", {"id": widget_id, "text": text})

    def select(self, widget_id: str, *, item: str = None, index: int = None) -> dict:
        body = {"id": widget_id}
        if item is not None:
            body["item"] = item
        elif index is not None:
            body["index"] = index
        return self._post("/select", body)

    def key(self, key: str) -> dict:
        return self._post("/key", {"key": key})

    def screenshot(self, source: str = "gl") -> bytes:
        """Returns raw PNG bytes. source='gl' (default) or 'os'."""
        params = {} if source == "gl" else {"source": source}
        data = self._get("/screenshot", params=params)
        return base64.b64decode(data["png"])

    # ------------------------------------------------------------------
    # Convenience helpers
    # ------------------------------------------------------------------

    def wait_for_screen(self, screen: str, timeout: float = 15.0, poll: float = 0.5) -> dict:
        """Poll /state until the named screen is active. Returns the state dict."""
        deadline = time.monotonic() + timeout
        last_screen = None
        while time.monotonic() < deadline:
            try:
                s = self.state()
                last_screen = s["screen"]
                if last_screen == screen:
                    return s
            except Exception:
                pass
            time.sleep(poll)
        raise TimeoutError(
            f"Screen '{screen}' not reached within {timeout}s (last: '{last_screen}')"
        )

    def widget_ids(self, state: Optional[dict] = None) -> list[str]:
        """Return the list of widget IDs from the current (or given) state."""
        s = state or self.state()
        return [w["id"] for w in s["widgets"]]

    def find_widget(self, widget_id: str, state: Optional[dict] = None) -> Optional[dict]:
        s = state or self.state()
        return next((w for w in s["widgets"] if w["id"] == widget_id), None)

    # ------------------------------------------------------------------
    # Visual regression
    # ------------------------------------------------------------------

    def save_snapshot(self, name: str, source: str = "gl") -> Path:
        """Capture and save a baseline screenshot. Returns the saved path."""
        SNAPSHOTS_DIR.mkdir(exist_ok=True)
        path = SNAPSHOTS_DIR / f"{name}.png"
        path.write_bytes(self.screenshot(source=source))
        return path

    def assert_snapshot(self, name: str, source: str = "gl", tolerance: float = 0.02,
                        update: bool = False):
        """
        Compare a screenshot against the saved baseline.
        tolerance: max allowed fraction of pixels that differ (default 2%).
        Saves the actual screenshot next to the baseline as <name>.actual.png on failure.
        If no baseline exists, saves one and passes (first-run behaviour).
        """
        try:
            from PIL import Image, ImageChops
        except ImportError:
            raise ImportError("pillow is required for visual regression: pip install pillow")

        import io
        import math

        baseline_path = SNAPSHOTS_DIR / f"{name}.png"
        actual_bytes = self.screenshot(source=source)

        update = update or getattr(self, "_update_snapshots", False)
        if update or not baseline_path.exists():
            baseline_path.write_bytes(actual_bytes)
            return  # First run or explicit update: save baseline, don't fail

        baseline = Image.open(baseline_path).convert("RGB")
        actual = Image.open(io.BytesIO(actual_bytes)).convert("RGB")

        if baseline.size != actual.size:
            actual_path = SNAPSHOTS_DIR / f"{name}.actual.png"
            actual_path.write_bytes(actual_bytes)
            raise AssertionError(
                f"Snapshot '{name}': size mismatch {baseline.size} vs {actual.size}"
            )

        diff = ImageChops.difference(baseline, actual)
        total = baseline.width * baseline.height
        differing = sum(1 for px in diff.getdata() if any(c > 10 for c in px))
        ratio = differing / total

        if ratio > tolerance:
            actual_path = SNAPSHOTS_DIR / f"{name}.actual.png"
            actual_path.write_bytes(actual_bytes)
            raise AssertionError(
                f"Snapshot '{name}': {ratio:.1%} pixels differ (tolerance {tolerance:.1%}). "
                f"Actual saved to {actual_path}. "
                f"Run with --update-snapshots to accept the new baseline."
            )

    # ------------------------------------------------------------------
    # Internal
    # ------------------------------------------------------------------

    def _get(self, path: str, params: dict = None) -> dict:
        r = self._session.get(self.url + path, params=params, timeout=10)
        r.raise_for_status()
        body = r.json()
        if not body["ok"]:
            raise RuntimeError(f"Agent server error on GET {path}: {body['error']}")
        return body["data"]

    def _post(self, path: str, payload: dict) -> dict:
        r = self._session.post(self.url + path, json=payload, timeout=10)
        r.raise_for_status()
        body = r.json()
        if not body["ok"]:
            raise RuntimeError(f"Agent server error on POST {path}: {body['error']}")
        return body["data"]
