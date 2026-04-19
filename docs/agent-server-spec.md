# Agent Command Server — Spec

**Status:** DRAFT v0.3 — all OQs closed; milestones 0–3 verified on macOS + Linux (Podman/Xvfb)  
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

**OQ-1 CLOSED:** Standalone flag. `-headless -agentserver` must compose freely
for CI use without implying debug verbosity.

---

## 3. Architecture

```
AI Agent (Claude / script)
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
│  - renders screenshot on demand      │
└──────────────────────────────────────┘
```

**Threading model:** The HTTP server runs on a dedicated thread. Commands are
posted to a `std::queue` protected by a mutex. The main thread drains the queue
once per frame and resolves the associated `std::promise`. The HTTP thread blocks
on the future until the main thread fulfills it (timeout: 5 s).

**OQ-2 CLOSED:** `cpp-httplib` (single header, MIT, no new CMake targets).
Vendored at `lib/cpp-httplib/httplib.h`. No extra link flags needed on any
platform.

---

## 4. Protocol

- **Transport:** HTTP/1.1 over TCP, `localhost:7777`
- **Format:** JSON request body + JSON response body
- **Encoding:** UTF-8
- All requests use `Content-Type: application/json`
- All responses include `Content-Type: application/json`

### Response envelope

```json
{
  "ok": true,
  "data": { ... }
}
```

On error:

```json
{
  "ok": false,
  "error": "human-readable description"
}
```

HTTP status codes mirror `ok`: `200` on success, `4xx`/`5xx` on error.

---

## 5. API Reference

### 5.1 `GET /state`

Returns the current UI state: which screen is active and what top-level widgets
are visible.

**Response `data`:**

```json
{
  "screen": "PlayerSelect",
  "widgets": [
    { "id": "ButtonOK",       "type": "button", "label": "OK",     "enabled": true  },
    { "id": "EditPlayerName", "type": "edit",   "value": "Player", "enabled": true  },
    { "id": "ListPlayers",    "type": "list",   "items": ["Claude"], "selected": 0  }
  ]
}
```

Widget types: `button`, `edit`, `list`, `label`, `slider`, `check`.

**OQ-3 CLOSED:** Stable string names via a registry function
`EventTypeToWidgetId(EventType) → std::string` in
`colobot-base/src/app/agent_server.cpp`.  
Controls not in the registry fall back to `"evt:N"` (N = numeric EventType).
The registry ships with all controls needed for Milestones 0–3.  
Canonical widget IDs are listed in §6.

### 5.2 `POST /click`

Clicks a widget by its `id`.

**Request:**

```json
{ "id": "ButtonOK" }
```

**Response `data`:**

```json
{ "clicked": "ButtonOK" }
```

Fails with `404` if widget not found or not visible; `409` if disabled.

### 5.3 `POST /select`

Selects an item in a list widget.

**Request:**

```json
{ "id": "ListPlayers", "item": "Claude" }
```

OR by index:

```json
{ "id": "ListPlayers", "index": 0 }
```

### 5.4 `POST /type`

Types text into the currently focused edit widget, replacing its contents.

**Request:**

```json
{ "id": "EditPlayerName", "text": "Claude" }
```

Bypasses OS input entirely — directly calls the widget's internal set-value
method. No reliance on keyboard event injection.

### 5.5 `POST /key`

Sends a named key to the game (for actions without a widget, e.g. F1, Escape).

**Request:**

```json
{ "key": "F1" }
```

Supported keys (v1): `F1`–`F12`, `Escape`, `Return`, `Space`, `Tab`.

### 5.6 `GET /screenshot`

Returns the current framebuffer as a PNG.

**Response `data`:**

```json
{ "png": "<base64-encoded PNG>" }
```

The screenshot is taken at the start of the next rendered frame after the
request is received.

**OQ-4 CLOSED:** Next rendered frame. The command is enqueued; the main thread
captures via `CEngine::GetDevice()->GetFrameBufferPixels()` after `SDL_GL_SwapWindow`
and encodes PNG in-memory before resolving the promise. Latency ~1 frame (~16 ms).

### 5.7 `GET /health`

Liveness check. Returns immediately without touching the main thread.

**Response `data`:**

```json
{ "status": "ok", "version": "0.3.0-alpha" }
```

---

## 6. Widget Addressing

Colobot's UI controls are registered via `CInterface::CreateControl(...)` with
an `EventType` that uniquely identifies them within a screen. The server exposes
these as string IDs.

Mapping strategy:
1. New file `colobot-base/src/ui/agent_widget_registry.h` — maps `EventType`
   → stable string name for all controls that an agent needs to address.
2. Controls not in the registry are still visible in `/state` but addressed by
   their `EventType` numeric value as a fallback string (`"evt:42"`).

**Open question OQ-3 (continued):** The registry approach is preferred.
Prototype must confirm that all relevant screens' widgets are reachable via
`EventType`.

---

## 7. Cross-Platform Requirements

| Requirement | macOS | Linux | Windows |
|-------------|-------|-------|---------|
| Server binds to 127.0.0.1 | ✓ | ✓ | ✓ |
| No OS accessibility APIs used | ✓ | ✓ | ✓ |
| No X11 / AppKit / WinAPI in server code | ✓ | ✓ | ✓ |
| HTTP library compiles without extra deps | ✓ | ✓ | TBD |
| Screenshot works without GPU readback | ✓ | ✓ | TBD |

**OQ-5 CLOSED:** Screenshot on Linux works via Xvfb + Mesa llvmpipe
(`LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe`). The game must be launched
**without** `-headless` so SDL2 creates a real window and GL context; Xvfb
provides the virtual display. `import -window root` (ImageMagick) captures the
Xvfb framebuffer. Verified 2026-04-19 on Ubuntu 24.04 arm64 (Podman/libkrun).
See `.devcontainer/test-headless.sh` for the repeatable test.

---

## 8. Test Plan

Tests are run by a **human** on each target platform before a milestone closes.

### Milestone 0 — Server starts and `/health` responds

```sh
./colobot -agentserver -headless &
sleep 2
curl -s http://127.0.0.1:7777/health | python3 -m json.tool
```

Expected: `{"ok": true, "data": {"status": "ok", ...}}`

Platforms: macOS, Linux  
Sign-off: human

### Milestone 1 — `/state` reflects the Player Select screen

```sh
./colobot -agentserver &
sleep 4
curl -s http://127.0.0.1:7777/state | python3 -m json.tool
```

Expected: response includes `"screen": "PlayerSelect"` and widget list with
`EditPlayerName` and `ButtonOK`.

Platforms: macOS, Linux  
Sign-off: human

### Milestone 2 — Set player name to "Claude" and click OK via API

```sh
curl -s -X POST http://127.0.0.1:7777/type \
  -H 'Content-Type: application/json' \
  -d '{"id":"EditPlayerName","text":"Claude"}'

curl -s -X POST http://127.0.0.1:7777/click \
  -H 'Content-Type: application/json' \
  -d '{"id":"ButtonOK"}'
```

Expected: game advances past Player Select screen.  
Verify with: `GET /state` showing the main menu screen.

Platforms: macOS, Linux  
Sign-off: human

### Milestone 3 — Navigate to a level and open SatCom help

```sh
curl -X POST http://127.0.0.1:7777/click -d '{"id":"ButtonExercises"}'
curl -X POST http://127.0.0.1:7777/click -d '{"id":"ButtonPlay"}'
sleep 5
curl -X POST http://127.0.0.1:7777/key   -d '{"key":"F1"}'
curl         http://127.0.0.1:7777/screenshot | python3 -c "
import sys,json,base64; d=json.load(sys.stdin)
open('/tmp/satcom.png','wb').write(base64.b64decode(d['data']['png']))"
```

Expected: `/tmp/satcom.png` shows the SatCom help panel.

Platforms: macOS, Linux  
Sign-off: human

---

## 9. Integration Points (from code exploration)

| Concern | Location | Detail |
|---------|----------|--------|
| CLI flag | `app.cpp:242` `option options[]` | Add `OPT_AGENTSERVER` entry |
| Main-loop drain | `app.cpp:1138` (event drain loop) | Call `m_agentServer->DrainQueue()` before queue loop |
| Screenshot capture | `gl33_device.cpp` `GetFrameBufferPixels()` | Returns RGBA pixels; encode PNG with `libpng` (already linked) |
| Widget lookup | `interface.cpp:284` `SearchControl(EventType)` | Returns `CControl*`; cast to `CEdit`, `CButton`, `CList` as needed |
| Screen detection | `interface.cpp` — inspect which EventTypes are live | No explicit "current screen" API; infer from visible controls |
| Swap point for screenshot | `app.cpp:1220` (after render) | Set a flag; next `DrainQueue()` captures and fulfills promise |

---

## 10. Spec Evolution Log

| Version | Date | Change | Agent |
|---------|------|--------|-------|
| 0.1 | 2026-04-17 | Initial draft from exploratory session | analyst |
| 0.2 | 2026-04-18 | Closed OQ-1..4; added integration points table | analyst |
| 0.3 | 2026-04-19 | Closed OQ-5 (Linux/Xvfb verified); updated cross-platform table | analyst |

---

## 11. Open Questions Summary

| ID | Question | Resolution |
|----|----------|------------|
| OQ-1 | Standalone `-agentserver` flag vs gated under `-debug`? | **CLOSED** — standalone |
| OQ-2 | HTTP library choice? | **CLOSED** — cpp-httplib header-only |
| OQ-3 | Widget IDs: stable strings vs numeric fallback? | **CLOSED** — registry + `evt:N` fallback |
| OQ-4 | Screenshot: next frame vs last frame? | **CLOSED** — next frame |
| OQ-5 | Headless screenshot on Linux (Xvfb / offscreen GL)? | **CLOSED** — Xvfb + llvmpipe + imagemagick; no `-headless` flag |
