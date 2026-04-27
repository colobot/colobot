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

    def key(self, key: str, action: str = "tap") -> dict:
        """Send a key event. action='tap' (default), 'down', or 'up'.
        Use 'down'/'up' to hold a key for continuous input (e.g. walking
        the astronaut). 'tap' sends down+up immediately.
        """
        body = {"key": key}
        if action != "tap":
            body["action"] = action
        return self._post("/key", body)

    def key_down(self, key: str) -> dict:
        """Press and hold a key. Must be paired with key_up later."""
        return self.key(key, "down")

    def key_up(self, key: str) -> dict:
        """Release a held key."""
        return self.key(key, "up")

    def key_hold(self, key: str, duration: float) -> dict:
        """Press a key, sleep for duration seconds, then release."""
        import time as _t
        self.key_down(key)
        _t.sleep(duration)
        return self.key_up(key)

    # ------------------------------------------------------------------
    # Astronaut (Me) walking — uses held arrow keys
    # ------------------------------------------------------------------
    #
    # Heading convention: rotation.y in objects() is the heading angle
    # in radians. Forward direction at heading h is (cos h, -sin h) in
    # the (x, z) plane. The Right arrow increases heading; Left decreases.
    # See test_10's ch3/lvl1 prep hook for the canonical usage example.

    def walk_me_to(self, tx: float, tz: float, *,
                   tolerance: float = 1.5,
                   max_iters: int = 40) -> bool:
        """Walk the currently-selected astronaut Me to (tx, tz) in world coords.
        Issues short Up / Left / Right key holds. Returns True on success."""
        import math as _m
        import time as _t
        for _ in range(max_iters):
            objs = self.objects()
            me = next((o for o in objs if o["type"] == "Me"), None)
            if me is None:
                return False
            x, z = me["pos"]["x"], me["pos"]["z"]
            dx, dz = tx - x, tz - z
            dist = _m.hypot(dx, dz)
            if dist < tolerance:
                return True
            target_ang = _m.atan2(-dz, dx)        # forward = (cos h, -sin h)
            cur_ang    = me["rotation"]["y"]
            diff = (target_ang - cur_ang + _m.pi) % (2 * _m.pi) - _m.pi
            if abs(diff) > 0.2:
                turn_key = "Right" if diff > 0 else "Left"
                self.key_hold(turn_key, min(abs(diff) * 0.18, 0.4))
            else:
                self.key_hold("Up", min(dist * 0.04, 0.4))
            _t.sleep(0.1)
        return False

    def me_pickup_and_install(self, item_type: str, robot_type: str) -> bool:
        """Walk Me to the nearest item of `item_type`, take it, walk to the
        rear of the named robot, and drop it on the robot's power slot.

        Returns True on success. Used to prepare levels where the trainer
        bot starts without a power cell (power=-1) and the astronaut must
        manually install one before any robot script can run.
        """
        import math as _m
        import time as _t

        # key_hold durations are real-time; reset to 1× for predictable movement
        # regardless of whatever simulation speed is active, then restore.
        try:
            _saved_speed = self.get_speed()
            if _saved_speed != 1.0:
                self.set_speed(1.0)
        except Exception:
            _saved_speed = 1.0

        try:
            # Select Me — its shortcut is the first event id in the 1500 range.
            # In all level layouts seen so far this is evt:1501.
            for evt in range(1501, 1510):
                if self.find_widget(f"evt:{evt}") is not None:
                    self.click(f"evt:{evt}")
                    _t.sleep(0.4)
                    if self.find_widget("ButtonTake") is not None:
                        break
            else:
                return False

            objs = self.objects()
            item = next((o for o in objs if o["type"] == item_type), None)
            bot  = next((o for o in objs if o["type"] == robot_type), None)
            if item is None or bot is None:
                return False

            # Step 1 — walk Me to the item and pick it up. Tolerance is generous
            # because turn-step convergence rarely lands exactly on target; 2.5
            # units is comfortably within ButtonTake's interaction range.
            self.walk_me_to(item["pos"]["x"], item["pos"]["z"], tolerance=2.5, max_iters=60)
            self.click("ButtonTake")
            _t.sleep(0.6)

            # Step 2 — compute a position behind the bot (along its -forward axis)
            bh = bot["rotation"]["y"]
            rear_x = bot["pos"]["x"] - _m.cos(bh) * 5.0
            rear_z = bot["pos"]["z"] + _m.sin(bh) * 5.0

            # Step 3 — walk to the rear and drop
            self.walk_me_to(rear_x, rear_z, tolerance=2.5, max_iters=60)
            self.click("ButtonTake")
            _t.sleep(0.6)

            # Verify the cell is now attached (its world position becomes a small
            # local offset relative to the bot, e.g. (1.7, -0.5, 1.1)).
            objs = self.objects()
            item = next((o for o in objs if o["type"] == item_type), None)
            if item is None:
                return False
            return abs(item["pos"]["x"]) < 10 and abs(item["pos"]["z"]) < 10
        finally:
            try:
                if _saved_speed != 1.0:
                    self.set_speed(_saved_speed)
            except Exception:
                pass

    def click_pos(self, x: float, y: float) -> dict:
        """Click at interface coordinates (x, y) in [0,1], origin bottom-left."""
        return self._post("/click_pos", {"x": x, "y": y})

    def objects(self) -> list:
        """Return a list of game objects with their projected screen positions."""
        return self._get("/objects")

    def find_object(self, type_name: str) -> Optional[dict]:
        """Return the first visible on-screen object with the given type name, or None."""
        return next(
            (o for o in self.objects() if o["type"] == type_name and o["visible"]),
            None,
        )

    # ------------------------------------------------------------------
    # Raw mouse input
    # ------------------------------------------------------------------

    def mouse_move(self, x: float, y: float) -> dict:
        """Move cursor to interface coords (x, y) in [0..1] without clicking."""
        return self._post("/mouse_move", {"x": x, "y": y})

    def drag(self, from_x: float, from_y: float,
             to_x: float, to_y: float,
             steps: int = 20, button: str = "left") -> dict:
        """Click-and-drag from (from_x, from_y) to (to_x, to_y) in interface coords."""
        return self._post("/drag", {
            "from_x": from_x, "from_y": from_y,
            "to_x":   to_x,   "to_y":   to_y,
            "steps":  steps,  "button": button,
        })

    def mouse_down(self, x: float, y: float, button: str = "left") -> dict:
        """Press a mouse button at interface coords. Must be followed by mouse_up."""
        return self._post("/mouse_button", {"action": "down", "x": x, "y": y, "button": button})

    def mouse_up(self, x: float, y: float, button: str = "left") -> dict:
        """Release a mouse button at interface coords."""
        return self._post("/mouse_button", {"action": "up", "x": x, "y": y, "button": button})

    def window(self) -> dict:
        """Return actual window geometry: logical size, drawable size, display usable area."""
        return self._get("/window")

    def launch(self, category: str, chap: int, rank: int) -> dict:
        """Start a level directly: category is 'Exercises'|'Challenges'|'Missions'|'FreeGame'."""
        return self._post("/launch", {"category": category, "chap": chap, "rank": rank})

    def set_speed(self, speed: float) -> dict:
        """Set the simulation speed multiplier (1.0=normal, 4.0=4× faster, max 100)."""
        return self._post("/speed", {"speed": speed})

    def get_speed(self) -> float:
        """Return the current simulation speed multiplier."""
        return self._get("/speed")["speed"]

    def get_program(self, slot: int = -1) -> dict:
        """Return source code and compile state for a program slot on the selected robot."""
        params = {} if slot < 0 else {"slot": slot}
        return self._get("/program", params=params)

    def set_program(self, source: str, slot: int = -1, compile: bool = False) -> dict:
        """Write source code to a program slot. If compile=True, compile and return errors."""
        body: dict = {"source": source}
        if slot >= 0:
            body["slot"] = slot
        if compile:
            body["compile"] = True
        return self._post("/program", body)

    def compile(self, source: str, slot: int = -1) -> Optional[dict]:
        """Write and compile source. Returns error dict on failure, None on success."""
        result = self.set_program(source, slot=slot, compile=True)
        return result.get("error")

    def diagnostics(self, slot: int = -1) -> dict:
        """Return compile and runtime state for the selected robot's program slot."""
        params = {} if slot < 0 else {"slot": slot}
        return self._get("/diagnostics", params=params)

    def run_program(self) -> None:
        """Click ButtonRunProgram on the selected robot's HUD."""
        self.click("ButtonRunProgram")

    def open_studio(self, expected_source: str = None) -> bool:
        """
        Select a programmable robot and open its Studio code editor.
        If expected_source is given, all program slots are scanned for one that
        is already runnable with matching source; if found it is selected in the
        Studio list so StudioRun is enabled without adding a duplicate slot.
        Skips robots that reject ButtonAddProgram (e.g. already-running bots).
        Returns True on success, False if no suitable robot found.
        """
        import time as _t
        for evt_id in range(1501, 1550):
            widget_id = f"evt:{evt_id}"
            if self.find_widget(widget_id) is None:
                continue
            try:
                self._post("/click", {"id": widget_id})
            except Exception:
                continue
            _t.sleep(0.3)
            if self.find_widget("ButtonOpenStudio") is None:
                continue

            # Scan all slots for an existing runnable slot with matching source.
            # This avoids adding a duplicate slot AND ensures we end up on a
            # runnable slot (the game may reset the active slot to the original
            # non-runnable one after a level reload, so we must select it explicitly).
            reuse_slot = None
            if expected_source is not None:
                try:
                    first = self.get_program(0)
                    slot_count = first.get("slot_count", 1)
                    for s in range(slot_count):
                        try:
                            p = self.get_program(s)
                            if p.get("runnable") and \
                                    p.get("source", "").strip() == expected_source.strip():
                                reuse_slot = s
                                break
                        except Exception:
                            pass
                except Exception:
                    pass

            new_slot = None
            if reuse_slot is None and self.find_widget("ButtonAddProgram") is not None:
                try:
                    # Remember old count so we can select the newly-added slot
                    # in Studio (the game may open on a pre-existing non-runnable
                    # slot instead of the one we just added).
                    try:
                        first = self.get_program(0)
                        old_count = first.get("slot_count", 1)
                    except Exception:
                        old_count = None
                    self.click("ButtonAddProgram")
                    _t.sleep(0.2)
                    if old_count is not None:
                        new_slot = old_count  # new slot is appended at the end
                except Exception:
                    continue  # Bot is busy/running, try next shortcut

            try:
                self.click("ButtonOpenStudio")
                self.wait_for_screen("Studio", timeout=5.0)
                # Always explicitly select the target slot so StudioRun is enabled.
                # reuse_slot: an existing runnable slot found during scan.
                # new_slot: the slot we just added via ButtonAddProgram.
                select_slot = reuse_slot if reuse_slot is not None else new_slot
                if select_slot is not None:
                    try:
                        self.select("ListStudioPrograms", index=select_slot)
                        _t.sleep(0.1)
                    except Exception:
                        pass
                return True
            except Exception:
                continue
        return False

    def select_programmable_robot(self, timeout: float = 3.0) -> bool:
        """
        Click through shortcut buttons (1501-1549) until ButtonOpenStudio appears,
        indicating a programmable robot is selected.
        Returns True if a programmable robot was selected, False otherwise.
        """
        for evt_id in range(1501, 1550):
            widget_id = f"evt:{evt_id}"
            if self.find_widget(widget_id) is None:
                continue
            try:
                self._post("/click", {"id": widget_id})
            except Exception:
                continue
            import time as _t; _t.sleep(0.3)
            if self.find_widget("ButtonOpenStudio") is not None:
                return True
        return False

    def console(self, command: str) -> None:
        """Open the in-game console, execute a command, and close it."""
        self.key("Backquote")
        import time; time.sleep(0.1)
        self.type("EditConsole", command)
        self.key("Return")

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
