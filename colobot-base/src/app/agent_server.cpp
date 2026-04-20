// Agent Command Server — HTTP/JSON bridge for AI agent → Colobot interaction.
// See docs/agent-server-spec.md for the full specification.

// Suppress warnings from third-party header
#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Weverything"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wall"
#  pragma GCC diagnostic ignored "-Wextra"
#endif

#undef CPPHTTPLIB_OPENSSL_SUPPORT
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#include "../../../lib/cpp-httplib/httplib.h"
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

#include "agent_server.h"

#include "common/event.h"
#include "common/image.h"
#include "common/logger.h"
#include "graphics/core/device.h"
#include "graphics/core/framebuffer.h"
#include "graphics/engine/engine.h"
#include "graphics/opengl33/glutil.h"
#include "level/robotmain.h"
#include "ui/controls/button.h"
#include "ui/controls/control.h"
#include "ui/controls/edit.h"
#include "ui/controls/interface.h"
#include "ui/controls/list.h"
#include "ui/controls/window.h"

#include <SDL.h>
#include <png.h>

#include <algorithm>
#include <chrono>
#include <csetjmp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// ---------------------------------------------------------------------------
// Widget ID registry
// ---------------------------------------------------------------------------

static const std::unordered_map<int, std::string>& GetRegistry()
{
    static const std::unordered_map<int, std::string> kRegistry = {
        // Player select screen
        { static_cast<int>(EVENT_INTERFACE_NEDIT),  "EditPlayerName" },
        { static_cast<int>(EVENT_INTERFACE_NLIST),  "ListPlayers"    },
        { static_cast<int>(EVENT_INTERFACE_NOK),    "ButtonOK"       },
        { static_cast<int>(EVENT_INTERFACE_NDELETE),"ButtonDelete"   },
        // Main menu
        { static_cast<int>(EVENT_INTERFACE_TRAINER),"ButtonExercises"},
        { static_cast<int>(EVENT_INTERFACE_DEFI),   "ButtonChallenges"},
        { static_cast<int>(EVENT_INTERFACE_MISSION),"ButtonMissions" },
        { static_cast<int>(EVENT_INTERFACE_FREE),   "ButtonFreeGame" },
        { static_cast<int>(EVENT_INTERFACE_NAME),   "ButtonPlayerName"},
        { static_cast<int>(EVENT_INTERFACE_SETUP),  "ButtonSetup"    },
        { static_cast<int>(EVENT_INTERFACE_QUIT),   "ButtonQuit"     },
        // Level / mission select
        { static_cast<int>(EVENT_INTERFACE_CHAP),   "ListChapter"    },
        { static_cast<int>(EVENT_INTERFACE_LIST),   "ListLevel"      },
        { static_cast<int>(EVENT_INTERFACE_PLAY),   "ButtonPlay"     },
        { static_cast<int>(EVENT_INTERFACE_BACK),   "ButtonBack"     },
        { static_cast<int>(EVENT_INTERFACE_RESUME), "ButtonResume"   },
        // In-game
        { static_cast<int>(EVENT_INTERFACE_ABORT),  "ButtonAbort"    },
        { static_cast<int>(EVENT_INTERFACE_AGAIN),  "ButtonAgain"    },
        { static_cast<int>(EVENT_INTERFACE_WRITE),  "ButtonSave"     },
        { static_cast<int>(EVENT_INTERFACE_READ),   "ButtonLoad"     },
        { static_cast<int>(EVENT_INTERFACE_SATCOM), "ButtonSatCom"   },
    };
    return kRegistry;
}

static std::string EventTypeToWidgetId(EventType et)
{
    auto it = GetRegistry().find(static_cast<int>(et));
    if (it != GetRegistry().end())
        return it->second;
    return "evt:" + std::to_string(static_cast<int>(et));
}

static EventType WidgetIdToEventType(const std::string& id)
{
    for (auto& [k, v] : GetRegistry())
        if (v == id)
            return static_cast<EventType>(k);
    // Try evt:N fallback
    if (id.rfind("evt:", 0) == 0)
    {
        try { return static_cast<EventType>(std::stoi(id.substr(4))); }
        catch (...) {}
    }
    return EVENT_NULL;
}

// ---------------------------------------------------------------------------
// Tiny JSON helpers
// ---------------------------------------------------------------------------

static std::string JsonEscape(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + 4);
    for (unsigned char c : s)
    {
        if (c == 0) break; // null-terminate: Colobot strings may be null-padded
        switch (c)
        {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if (c < 0x20) { /* skip other control chars */ }
            else out += static_cast<char>(c);
            break;
        }
    }
    return out;
}

static std::string OkResponse(const std::string& dataJson)
{
    return "{\"ok\":true,\"data\":" + dataJson + "}";
}

static std::string ErrResponse(const std::string& msg)
{
    return "{\"ok\":false,\"error\":\"" + JsonEscape(msg) + "\"}";
}

// Naive key lookup in a flat JSON object string (no nested objects).
static std::string JsonGet(const std::string& body, const std::string& key)
{
    // Look for "key":"value" or "key":number or "key":true/false
    std::string needle = "\"" + key + "\"";
    auto pos = body.find(needle);
    if (pos == std::string::npos)
        return {};
    pos += needle.size();
    // Skip whitespace and colon
    while (pos < body.size() && (body[pos] == ' ' || body[pos] == ':'))
        ++pos;
    if (pos >= body.size())
        return {};
    if (body[pos] == '"')
    {
        // String value
        ++pos;
        std::string val;
        while (pos < body.size() && body[pos] != '"')
        {
            if (body[pos] == '\\' && pos + 1 < body.size())
            {
                ++pos;
                switch (body[pos])
                {
                case '"':  val += '"';  break;
                case '\\': val += '\\'; break;
                case 'n':  val += '\n'; break;
                case 'r':  val += '\r'; break;
                case 't':  val += '\t'; break;
                default:   val += body[pos]; break;
                }
            }
            else
            {
                val += body[pos];
            }
            ++pos;
        }
        return val;
    }
    else
    {
        // Number or boolean — read until delimiter
        std::string val;
        while (pos < body.size() && body[pos] != ',' && body[pos] != '}' && body[pos] != ' ')
            val += body[pos++];
        return val;
    }
}

// ---------------------------------------------------------------------------
// Base64 encoding
// ---------------------------------------------------------------------------

static const char kB64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string Base64Encode(const unsigned char* data, size_t len)
{
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3)
    {
        unsigned int b = static_cast<unsigned int>(data[i]) << 16;
        if (i + 1 < len) b |= static_cast<unsigned int>(data[i+1]) << 8;
        if (i + 2 < len) b |= static_cast<unsigned int>(data[i+2]);
        out += kB64Chars[(b >> 18) & 0x3f];
        out += kB64Chars[(b >> 12) & 0x3f];
        out += (i + 1 < len) ? kB64Chars[(b >> 6) & 0x3f] : '=';
        out += (i + 2 < len) ? kB64Chars[(b >> 0) & 0x3f] : '=';
    }
    return out;
}

// ---------------------------------------------------------------------------
// Widget type helpers
// ---------------------------------------------------------------------------

static Ui::CInterface* GetCurrentInterface()
{
    auto* main = CRobotMain::GetInstancePointer();
    if (!main) return nullptr;
    return main->GetInterface();
}

static std::string WidgetTypeString(Ui::CControl* ctrl)
{
    if (dynamic_cast<Ui::CEdit*>(ctrl))   return "edit";
    if (dynamic_cast<Ui::CList*>(ctrl))   return "list";
    if (dynamic_cast<Ui::CButton*>(ctrl)) return "button";
    return "unknown";
}

// Search for a control by EventType in interface's top-level controls AND inside CWindows.
static Ui::CControl* SearchControlDeep(Ui::CInterface* iface, EventType et)
{
    if (!iface) return nullptr;
    for (const auto& ctrl : iface->GetControls())
    {
        if (!ctrl) continue;
        if (ctrl->GetEventType() == et) return ctrl.get();
        if (auto* win = dynamic_cast<Ui::CWindow*>(ctrl.get()))
        {
            for (const auto& child : win->GetControls())
            {
                if (child && child->GetEventType() == et)
                    return child.get();
            }
        }
    }
    return nullptr;
}

// Append widget JSON entry for a single control.
static void AppendWidgetJson(std::string& out, bool& first, Ui::CControl* ctrl)
{
    // Note: not all controls set STATE_VISIBLE explicitly; include all for now.

    std::string wid  = EventTypeToWidgetId(ctrl->GetEventType());
    std::string type = WidgetTypeString(ctrl);
    bool enabled     = (ctrl->GetState() & Ui::STATE_ENABLE) != 0;

    if (!first) out += ",";
    first = false;

    out += "{";
    out += "\"id\":\"" + JsonEscape(wid) + "\"";
    out += ",\"type\":\"" + type + "\"";
    out += ",\"enabled\":" + std::string(enabled ? "true" : "false");

    if (auto* ed = dynamic_cast<Ui::CEdit*>(ctrl))
    {
        out += ",\"value\":\"" + JsonEscape(ed->GetText()) + "\"";
    }
    else if (auto* ls = dynamic_cast<Ui::CList*>(ctrl))
    {
        out += ",\"items\":[";
        int total = ls->GetTotal();
        for (int i = 0; i < total; ++i)
        {
            if (i) out += ",";
            out += "\"" + JsonEscape(ls->GetItemName(i)) + "\"";
        }
        out += "]";
        out += ",\"selected\":" + std::to_string(ls->GetSelect());
    }
    else
    {
        std::string name = ctrl->GetName();
        if (!name.empty())
            out += ",\"label\":\"" + JsonEscape(name) + "\"";
    }

    out += "}";
}

// ---------------------------------------------------------------------------
// CAgentServer
// ---------------------------------------------------------------------------

CAgentServer::CAgentServer(int port)
    : m_port(port)
{}

CAgentServer::~CAgentServer()
{
    Stop();
}

void CAgentServer::Start(Gfx::CEngine* engine, CEventQueue* eventQueue)
{
    m_engine     = engine;
    m_eventQueue = eventQueue;
    m_running   = true;
    m_thread    = std::thread(&CAgentServer::ServerThread, this);
}

void CAgentServer::Stop()
{
    if (!m_running)
        return;
    m_running = false;
    if (m_svr)
        m_svr->stop();
    if (m_thread.joinable())
        m_thread.join();
}

void CAgentServer::DrainQueue()
{
    std::queue<Command> local;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::swap(local, m_queue);
    }
    while (!local.empty())
    {
        Command& cmd = local.front();
        try
        {
            std::string result = cmd.execute();
            cmd.promise.set_value(std::move(result));
        }
        catch (...)
        {
            cmd.promise.set_exception(std::current_exception());
        }
        local.pop();
    }
}

std::string CAgentServer::PostAndWait(std::function<std::string()> fn)
{
    Command cmd;
    cmd.execute = std::move(fn);
    auto future = cmd.promise.get_future();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(cmd));
    }
    if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout)
        throw std::runtime_error("main thread timeout");
    return future.get(); // may rethrow exception from execute()
}

void CAgentServer::ServerThread()
{
    m_svr = std::make_unique<httplib::Server>();

    m_svr->Get("/health", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(OkResponse("{\"status\":\"ok\",\"version\":\"0.3.0-alpha\"}"),
                        "application/json");
    });

    m_svr->Get("/state", [this](const httplib::Request& req, httplib::Response& res) {
        try
        {
            std::string body = PostAndWait([this]() -> std::string {
                return BuildStateJson();
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            res.status = 500;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Post("/click", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = JsonGet(req.body, "id");
        if (id.empty())
        {
            res.status = 400;
            res.set_content(ErrResponse("missing 'id'"), "application/json");
            return;
        }
        try
        {
            std::string body = PostAndWait([this, id]() -> std::string {
                return DoClick(id);
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            int code = std::string(e.what()).find("not found") != std::string::npos ? 404 :
                       std::string(e.what()).find("disabled") != std::string::npos ? 409 : 500;
            res.status = code;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Post("/type", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id   = JsonGet(req.body, "id");
        std::string text = JsonGet(req.body, "text");
        if (id.empty())
        {
            res.status = 400;
            res.set_content(ErrResponse("missing 'id'"), "application/json");
            return;
        }
        try
        {
            std::string body = PostAndWait([this, id, text]() -> std::string {
                return DoType(id, text);
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            res.status = 404;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Post("/select", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id   = JsonGet(req.body, "id");
        std::string item = JsonGet(req.body, "item");
        std::string idx  = JsonGet(req.body, "index");
        if (id.empty())
        {
            res.status = 400;
            res.set_content(ErrResponse("missing 'id'"), "application/json");
            return;
        }
        try
        {
            std::string body = PostAndWait([this, id, item, idx]() -> std::string {
                return DoSelect(id, item, idx);
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            res.status = 404;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Post("/key", [this](const httplib::Request& req, httplib::Response& res) {
        std::string key = JsonGet(req.body, "key");
        if (key.empty())
        {
            res.status = 400;
            res.set_content(ErrResponse("missing 'key'"), "application/json");
            return;
        }
        try
        {
            std::string body = PostAndWait([this, key]() -> std::string {
                return DoKey(key);
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            res.status = 400;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Get("/screenshot", [this](const httplib::Request& req, httplib::Response& res) {
        try
        {
            bool useOS = req.has_param("source") && req.get_param_value("source") == "os";
            std::string body;

            if (useOS)
            {
                // OS screenshot: captures what actually appeared on screen including window
                // chrome, compositor effects, and HiDPI scaling. Useful for validating
                // platform-specific rendering behaviour. Runs on the HTTP thread directly.
                body = PostAndWait([this]() -> std::string { return DoScreenshotOS(); });
            }
            else
            {
                // GL framebuffer readback: captures the rendered frame directly from FBO 0.
                // No external tools required; works headless. Default for CI use.
                std::future<std::string> fut;
                {
                    std::lock_guard<std::mutex> lk(m_screenshotMutex);
                    m_screenshotPromise = std::promise<std::string>();
                    fut = m_screenshotPromise.get_future();
                }
                m_screenshotPending.store(true, std::memory_order_release);

                if (fut.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
                    throw std::runtime_error("screenshot timeout — main thread did not respond");

                body = fut.get();
            }

            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            res.status = 500;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    GetLogger()->Info("Agent server listening on 127.0.0.1:%%", m_port);
    m_svr->listen("127.0.0.1", m_port);
    GetLogger()->Info("Agent server stopped");
}

// ---------------------------------------------------------------------------
// Main-thread command implementations
// ---------------------------------------------------------------------------

// Detect which game screen is currently shown from the set of present widget IDs.
static std::string DetectScreen(const std::vector<std::string>& ids)
{
    auto has = [&](const char* id) {
        return std::find(ids.begin(), ids.end(), id) != ids.end();
    };

    if (has("EditPlayerName") && has("ListPlayers"))  return "PlayerSelect";
    if (has("ButtonExercises") && has("ButtonQuit"))  return "MainMenu";
    if (has("ListChapter") && has("ListLevel"))        return "LevelSelect";
    if (has("ButtonAbort") || has("ButtonAgain"))      return "InGameMenu";
    // In-game (HUD or SatCom): no menu-specific widgets present.
    // Loading state has ≤6 widgets; real in-game states have ≥7 (SatCom) or more.
    if (ids.size() >= 7 && !has("EditPlayerName") && !has("ButtonExercises") && !has("ListChapter"))
        return "InGame";
    return "unknown";
}

std::string CAgentServer::BuildStateJson()
{
    Ui::CInterface* iface = GetCurrentInterface();
    if (!iface)
        return "{\"screen\":\"unknown\",\"widgets\":[]}";

    std::string widgets = "[";
    bool first = true;
    std::vector<std::string> ids;

    for (const auto& ctrl : iface->GetControls())
    {
        if (!ctrl) continue;
        if (auto* win = dynamic_cast<Ui::CWindow*>(ctrl.get()))
        {
            // Recurse into window children — most game screens use a CWindow.
            for (const auto& child : win->GetControls())
            {
                if (!child) continue;
                ids.push_back(EventTypeToWidgetId(child->GetEventType()));
                AppendWidgetJson(widgets, first, child.get());
            }
        }
        else
        {
            ids.push_back(EventTypeToWidgetId(ctrl->GetEventType()));
            AppendWidgetJson(widgets, first, ctrl.get());
        }
    }
    widgets += "]";

    std::string screen = DetectScreen(ids);
    return "{\"screen\":\"" + screen + "\",\"widgets\":" + widgets + "}";
}

std::string CAgentServer::DoClick(const std::string& id)
{
    EventType et = WidgetIdToEventType(id);
    if (et == EVENT_NULL)
        throw std::runtime_error("widget not found: " + id);

    Ui::CControl* ctrl = SearchControlDeep(GetCurrentInterface(), et);
    if (!ctrl || !(ctrl->GetState() & Ui::STATE_VISIBLE))
        throw std::runtime_error("widget not found: " + id);
    if (!(ctrl->GetState() & Ui::STATE_ENABLE))
        throw std::runtime_error("widget disabled: " + id);

    // Synthesize a click event on the main event queue
    Event ev(et);
    m_eventQueue->AddEvent(std::move(ev));

    return "{\"clicked\":\"" + JsonEscape(id) + "\"}";
}

std::string CAgentServer::DoType(const std::string& id, const std::string& text)
{
    EventType et = WidgetIdToEventType(id);
    if (et == EVENT_NULL)
        throw std::runtime_error("widget not found: " + id);

    Ui::CControl* ctrl = SearchControlDeep(GetCurrentInterface(), et);
    if (!ctrl || !(ctrl->GetState() & Ui::STATE_VISIBLE))
        throw std::runtime_error("widget not found: " + id);

    auto* ed = dynamic_cast<Ui::CEdit*>(ctrl);
    if (!ed)
        throw std::runtime_error("widget is not an edit field: " + id);

    ed->SetText(text);
    return "{\"typed\":\"" + JsonEscape(id) + "\"}";
}

std::string CAgentServer::DoSelect(const std::string& id,
                                   const std::string& item,
                                   const std::string& indexStr)
{
    EventType et = WidgetIdToEventType(id);
    if (et == EVENT_NULL)
        throw std::runtime_error("widget not found: " + id);

    Ui::CControl* ctrl = SearchControlDeep(GetCurrentInterface(), et);
    if (!ctrl || !(ctrl->GetState() & Ui::STATE_VISIBLE))
        throw std::runtime_error("widget not found: " + id);

    auto* ls = dynamic_cast<Ui::CList*>(ctrl);
    if (!ls)
        throw std::runtime_error("widget is not a list: " + id);

    int idx = -1;
    if (!indexStr.empty())
    {
        idx = std::stoi(indexStr);
    }
    else if (!item.empty())
    {
        int total = ls->GetTotal();
        for (int i = 0; i < total; ++i)
        {
            if (ls->GetItemName(i) == item)
            {
                idx = i;
                break;
            }
        }
        if (idx < 0)
            throw std::runtime_error("item not found in list: " + item);
    }
    else
    {
        throw std::runtime_error("must provide 'item' or 'index'");
    }

    ls->SetSelect(idx);

    // Notify the game that the list selection changed
    Event ev(et);
    m_eventQueue->AddEvent(std::move(ev));

    return "{\"selected\":" + std::to_string(idx) + "}";
}

std::string CAgentServer::DoKey(const std::string& key)
{
    static const std::unordered_map<std::string, SDL_Scancode> kKeys = {
        { "Escape", SDL_SCANCODE_ESCAPE },
        { "Return", SDL_SCANCODE_RETURN },
        { "Space",  SDL_SCANCODE_SPACE  },
        { "Tab",    SDL_SCANCODE_TAB    },
        { "F1",  SDL_SCANCODE_F1  }, { "F2",  SDL_SCANCODE_F2  },
        { "F3",  SDL_SCANCODE_F3  }, { "F4",  SDL_SCANCODE_F4  },
        { "F5",  SDL_SCANCODE_F5  }, { "F6",  SDL_SCANCODE_F6  },
        { "F7",  SDL_SCANCODE_F7  }, { "F8",  SDL_SCANCODE_F8  },
        { "F9",  SDL_SCANCODE_F9  }, { "F10", SDL_SCANCODE_F10 },
        { "F11", SDL_SCANCODE_F11 }, { "F12", SDL_SCANCODE_F12 },
    };

    auto it = kKeys.find(key);
    if (it == kKeys.end())
        throw std::runtime_error("unsupported key: " + key);

    // Inject SDL key-down + key-up into SDL's event queue so the game sees them.
    SDL_Event down{};
    down.type         = SDL_KEYDOWN;
    down.key.keysym.scancode = it->second;
    down.key.keysym.sym      = SDL_GetKeyFromScancode(it->second);
    SDL_PushEvent(&down);

    SDL_Event up = down;
    up.type = SDL_KEYUP;
    SDL_PushEvent(&up);

    return "{\"key\":\"" + JsonEscape(key) + "\"}";
}

// ---------------------------------------------------------------------------
// GL framebuffer capture — called from main thread between Render and SwapBuffers
// ---------------------------------------------------------------------------

std::vector<unsigned char> CAgentServer::EncodeRGBAToPNG(
    const unsigned char* rgba, int width, int height)
{
    std::vector<unsigned char> out;

    png_structp pngPtr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!pngPtr)
        throw std::runtime_error("png_create_write_struct failed");

    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        png_destroy_write_struct(&pngPtr, nullptr);
        throw std::runtime_error("png_create_info_struct failed");
    }

    if (setjmp(png_jmpbuf(pngPtr)))
    {
        png_destroy_write_struct(&pngPtr, &infoPtr);
        throw std::runtime_error("PNG encoding error");
    }

    // Write into a memory buffer via custom write callback.
    png_set_write_fn(pngPtr, &out,
        [](png_structp p, png_bytep data, png_size_t len) {
            auto* v = static_cast<std::vector<unsigned char>*>(png_get_io_ptr(p));
            v->insert(v->end(), data, data + len);
        },
        [](png_structp) {});

    png_set_IHDR(pngPtr, infoPtr, width, height, 8,
        PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(pngPtr, infoPtr);

    // glReadPixels produces rows bottom-up; PNG expects top-down.
    // Build row pointers in reverse row order and convert RGBA→RGB on the fly.
    int stride = width * 4; // RGBA source stride
    std::vector<unsigned char> rgb(width * height * 3);
    for (int y = 0; y < height; ++y)
    {
        const unsigned char* src = rgba + (height - 1 - y) * stride;
        unsigned char* dst = rgb.data() + y * width * 3;
        for (int x = 0; x < width; ++x)
        {
            dst[x * 3 + 0] = src[x * 4 + 0];
            dst[x * 3 + 1] = src[x * 4 + 1];
            dst[x * 3 + 2] = src[x * 4 + 2];
        }
    }

    std::vector<png_bytep> rows(height);
    for (int y = 0; y < height; ++y)
        rows[y] = rgb.data() + y * width * 3;

    png_write_image(pngPtr, rows.data());
    png_write_end(pngPtr, infoPtr);
    png_destroy_write_struct(&pngPtr, &infoPtr);

    return out;
}

void CAgentServer::CaptureFrameIfPending(Gfx::CDevice* device, const glm::ivec2& size)
{
    if (!m_screenshotPending.load(std::memory_order_acquire))
        return;

    m_screenshotPending.store(false, std::memory_order_relaxed);

    std::string result;
    try
    {
        auto pixels = device->GetFrameBufferPixels();
        const auto* rgba = static_cast<const unsigned char*>(pixels->GetPixelsData());
        auto png = EncodeRGBAToPNG(rgba, size.x, size.y);
        std::string b64 = Base64Encode(png.data(), png.size());
        result = "{\"png\":\"" + b64 + "\"}";
    }
    catch (const std::exception& e)
    {
        // Fulfill with an error so the HTTP thread doesn't hang.
        std::lock_guard<std::mutex> lk(m_screenshotMutex);
        m_screenshotPromise.set_exception(std::current_exception());
        return;
    }

    std::lock_guard<std::mutex> lk(m_screenshotMutex);
    m_screenshotPromise.set_value(std::move(result));
}

// ---------------------------------------------------------------------------
// OS screenshot — captures the actual screen via platform screencapture tools.
// Use ?source=os to validate platform-specific rendering and compositor behaviour.
// ---------------------------------------------------------------------------

std::string CAgentServer::DoScreenshotOS()
{
    std::string tmpPath = "/tmp/colobot_agent_screenshot_os.png";

#if defined(__APPLE__)
    system("osascript -e 'tell application \"System Events\" to set frontmost of "
           "first process whose name contains \"colobot\" to true' 2>/dev/null");
    std::string cmd = "screencapture -x " + tmpPath;
    if (system(cmd.c_str()) != 0)
        throw std::runtime_error("screencapture failed");
#else
    std::string cmd = "import -window root " + tmpPath + " 2>/dev/null"
                      " || scrot " + tmpPath + " 2>/dev/null";
    if (system(cmd.c_str()) != 0)
        throw std::runtime_error(
            "OS screenshot failed — install imagemagick or scrot, "
            "and ensure a display server (Xvfb) is running");
#endif

    FILE* f = fopen(tmpPath.c_str(), "rb");
    if (!f)
        throw std::runtime_error("cannot open OS screenshot file");
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    std::vector<unsigned char> buf(sz);
    if (fread(buf.data(), 1, sz, f) != static_cast<size_t>(sz))
    {
        fclose(f);
        throw std::runtime_error("failed to read OS screenshot file");
    }
    fclose(f);

    std::string b64 = Base64Encode(buf.data(), buf.size());
    return "{\"png\":\"" + b64 + "\"}";
}
