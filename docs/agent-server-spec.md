# Agent Command Server — Spec

**Status:** DRAFT v0.4 — all OQs closed; automated test suite shipped  
**Approach:** spec-first; analyst updates this doc as prototype clarifies unknowns  
**Agents:**
- `analyst` — owns this spec; refines it as prototype runs
- `coder` — implements strictly from the finalized spec
- `verifier` — checks implementation against spec; writes test results
- `human` — validates on macOS and Linux; signs off each milestone

---

## 1. Goal

Enable an AI agent to interact with a running Colobot instance without relying on
OS-specific accessibility APIs or coordinate-based mouse automation.  
The server must work identically on macOS, Linux, and Windows.

### Non-goals (v1)

- Authentication / multi-client support
- Replaying recorded sessions
- Modifying game data (levels, save files)
- Production use — dev/test mode only

---

## 2. Activation

The server is **opt-in** and never runs in normal gameplay.

```
./colobot -agentserver [port]
```

- Default port: `7777`
- Binds to `127.0.0.1` only (never exposed to network)
- Server starts before the main window appears
- Process exits normally when Colobot quits; server tears down cleanly

**Note:** `-headless` skips the SDL window, removing the GL context the agent server
needs for rendering and screenshots. When `-agentserver` is present the engine
always creates a window (hidden if `-headless` is also set). For CI use without a
physical display, start Xvfb first and omit `-headless`; the test fixtures handle
this automatically.

---

## 3. Architecture

```
AI Agent / test script
        │  HTTP JSON  (localhost:7777)
        ▼
┌──────────────────────────────────────┐
│  CAgentServer  (new class)           │
│  - thin HTTP server on a thread      │
│  - queues commands into main thread  │
│  - returns results synchronously     │
└───────────────┬──────────────────────┘
                │  command queue + result future
                ▼
┌──────────────────────────────────────┐
│  Colobot main thread                 │
│  - executes UI actions each frame    │
│  - reads widget tree from CInterface │
│  - captures GL framebuffer pre-swap  │
└──────────────────────────────────────┘
```

**Threading model:** The HTTP server runs on a dedicated thread. Commands are
posted to a `std::queue` protected by a mutex. The main thread drains the queue
once per frame and resolves the associated `std::promise`. The HTTP thread blocks
on the future until the main thread fulfills it (timeout: 5 s).

**Screenshot threading:** Screenshot requests use a separate atomic flag +
promise. `CApplication::Render()` calls `CaptureFrameIfPending()` between
`m_engine->Render()` and `SDL_GL_SwapWindow()`, when FBO 0 holds the final
composited frame. This avoids undefined backbuffer contents after swap.

---

## 4. Protocol

- **Transport:** HTTP/1.1 over TCP, `localhost:7777`
- **Format:** JSON request body + JSON response body
- **Encoding:** UTF-8
- All requests use `Content-Type: application/json`
- All responses include `Content-Type: application/json`

### Response envelope

```json
{ "ok": true,  "data": { ... } }
{ "ok": false, "error": "human-readable description" }
```

HTTP status codes mirror `ok`: `200` on success, `4xx`/`5xx` on error.

---

## 5. API Reference

### 5.1 `GET /state`

Returns the current UI state.

**Response `data`:**

```json
{
  "screen": "PlayerSelect",
  "widgets": [
    { "id": "ButtonOK",       "type": "button", "label": "OK",     "enabled": true },
    { "id": "EditPlayerName", "type": "edit",   "value": "Player", "enabled": true },
    { "id": "ListPlayers",    "type": "list",   "items": ["Claude"], "selected": 0 }
  ]
}
```

Widget types: `button`, `edit`, `list`, `label`, `slider`, `check`.  
Known screens: `PlayerSelect`, `MainMenu`, `LevelSelect`, `InGameMenu`, `InGame`, `unknown`.

### 5.2 `POST /click`

Clicks a widget by its `id`. Fails `404` if not found/visible; `409` if disabled.

```json
{ "id": "ButtonOK" }
```

### 5.3 `POST /select`

Selects a list item by value or index.

```json
{ "id": "ListPlayers", "item": "Claude" }
{ "id": "ListPlayers", "index": 0 }
```

### 5.4 `POST /type`

Sets edit widget value directly (bypasses OS input).

```json
{ "id": "EditPlayerName", "text": "Claude" }
```

### 5.5 `POST /key`

Sends a named key. Supported: `F1`–`F12`, `Escape`, `Return`, `Space`, `Tab`.

```json
{ "key": "F1" }
```

### 5.6 `GET /screenshot`

Returns a PNG of the current frame.

**Query parameters:**

| Parameter | Values | Default | Description |
|-----------|--------|---------|-------------|
| `source`  | `gl`, `os` | `gl` | Capture method (see below) |

**`source=gl` (default):** GL framebuffer readback via `glReadPixels` on FBO 0,
captured pre-swap. No external tools. Works headless (with Xvfb). Captures exactly
what the renderer produced. Use for CI, regression tests, game-logic validation.

**`source=os`:** OS screencapture tool (`screencapture` on macOS,
`import -window root` / `scrot` on Linux). Captures what appeared on screen
including window chrome, compositor scaling, HiDPI effects. Requires a visible
window and display server. Use for platform-specific visual validation.

**Response `data`:**
```json
{ "png": "<base64-encoded PNG>" }
```

### 5.7 `GET /health`

Liveness check. Returns immediately without touching the main thread.

```json
{ "status": "ok", "version": "0.3.0-alpha" }
```

---

## 6. Widget Addressing

Controls are identified by stable string IDs registered in `agent_server.cpp`
(`EventTypeToWidgetId`). Unregistered controls fall back to `"evt:N"`.

### Registered widget IDs (Milestones 0–3)

| Screen | Widget ID | Type |
|--------|-----------|------|
| PlayerSelect | `EditPlayerName` | edit |
| PlayerSelect | `ListPlayers` | list |
| PlayerSelect | `ButtonOK` | button |
| PlayerSelect | `ButtonDelete` | button |
| MainMenu | `ButtonExercises` | button |
| MainMenu | `ButtonChallenges` | button |
| MainMenu | `ButtonMissions` | button |
| MainMenu | `ButtonFreeGame` | button |
| MainMenu | `ButtonQuit` | button |
| LevelSelect | `ListChapter` | list |
| LevelSelect | `ListLevel` | list |
| LevelSelect | `ButtonPlay` | button |
| LevelSelect | `ButtonBack` | button |
| InGameMenu | `ButtonAbort` | button |
| InGameMenu | `ButtonAgain` | button |

---

## 7. Cross-Platform Requirements

| Requirement | macOS | Linux | Windows |
|-------------|-------|-------|---------|
| Server binds to 127.0.0.1 | ✓ | ✓ | ✓ |
| No OS accessibility APIs used | ✓ | ✓ | ✓ |
| GL screenshot (`source=gl`) | ✓ | ✓ | TBD |
| OS screenshot (`source=os`) | ✓ | ✓ (Xvfb+imagemagick) | TBD |
| HTTP library compiles without extra deps | ✓ | ✓ | TBD |

---

## 8. Automated Test Suite

Tests live in `tests/agent/` and run with **pytest**. They cover the same four
milestones previously validated manually and serve as CI regression gates.

### 8.1 Architecture

```
tests/agent/
├── conftest.py          # fixtures: game_server, client
├── agent_client.py      # HTTP wrapper + wait_for_screen helper
├── test_health.py       # M0 — /health
├── test_state.py        # M1 — /state screen detection
├── test_navigation.py   # M2 — type + click, screen transitions
├── test_screenshot.py   # M3 — GL and OS screenshot paths
└── snapshots/           # baseline PNGs for visual regression
```

### 8.2 Running tests

**Prerequisites:** Colobot binary built, `pytest` installed (`pip install pytest requests pillow`).

**Against an already-running server (local dev):**
```sh
COLOBOT_AGENT_URL=http://localhost:7777 pytest tests/agent/ -v
```

**Auto-start with Xvfb + llvmpipe (CI or devcontainer):**
```sh
pytest tests/agent/ -v \
  --colobot-bin=./build/colobot \
  --colobot-datadir=./build/data
```

**Update visual regression baselines:**
```sh
pytest tests/agent/ --update-snapshots
```

### 8.3 CI integration

The GitHub Actions workflow (`.github/workflows/agent-server-tests.yml`) runs the
full pytest suite on every push to `dev` or `claude/**` and on PRs targeting `dev`.
Screenshots are uploaded as artifacts on every run.

### 8.4 Adding new tests

1. Add a test file `tests/agent/test_<feature>.py`
2. Use the `client` fixture — it provides a ready `AgentClient` connected to a live server
3. Call `client.wait_for_screen("ScreenName")` before asserting widget state
4. Use `client.screenshot()` for GL capture; `client.screenshot(source="os")` for platform tests
5. Save baselines with `client.save_snapshot("name")` and assert with `client.assert_snapshot("name")`

---

## 9. Integration Points

| Concern | Location | Detail |
|---------|----------|--------|
| CLI flag | `app.cpp` `option options[]` | `OPT_AGENTSERVER` |
| Window creation | `app.cpp` | Always created when `-agentserver` set, even with `-headless`; `SDL_WINDOW_HIDDEN` in headless mode |
| Software GL | `app.cpp` `CreateVideoSurface()` | `SDL_GL_ACCELERATED_VISUAL` skipped in headless mode |
| Main-loop drain | `app.cpp` after `Render()` | `m_agentServer->DrainQueue()` |
| Pre-swap capture | `app.cpp` `Render()` | `CaptureFrameIfPending()` between `m_engine->Render()` and `SDL_GL_SwapWindow()` |
| GL readback | `gl33_device.cpp` `GetFrameBufferPixels()` | `glReadPixels` on FBO 0; rows flipped + RGBA→RGB + libpng encode |
| Widget lookup | `interface.cpp` `SearchControl(EventType)` | Returns `CControl*`; cast to `CEdit`, `CButton`, `CList` |
| Screen detection | `agent_server.cpp` `DetectScreen()` | Infers screen from visible widget IDs; threshold ≥7 widgets for `InGame` |

---

## 10. Spec Evolution Log

| Version | Date | Change |
|---------|------|--------|
| 0.1 | 2026-04-17 | Initial draft from exploratory session |
| 0.2 | 2026-04-18 | Closed OQ-1..4; added integration points table |
| 0.3 | 2026-04-19 | Closed OQ-5 (Linux/Xvfb verified); CI pipeline (Phase 1) |
| 0.4 | 2026-04-19 | GL framebuffer screenshot (Phase 2); dual screenshot modes; automated pytest suite |

---

## 11. Open Questions

All OQs closed. No open questions remaining for v1 scope.
