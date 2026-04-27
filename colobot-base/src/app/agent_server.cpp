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
#include "level/level_category.h"
#include "level/robotmain.h"
#include "object/interface/programmable_object.h"
#include "object/interface/program_storage_object.h"
#include "object/interface/shielded_object.h"
#include "object/interface/slotted_object.h"
#include "object/interface/task_executor_object.h"
#include "object/object.h"
#include "object/object_manager.h"
#include "object/task/task.h"
#include "script/cbottoken.h"
#include "script/script.h"
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
        // ── Decorative / background glints (all screens) ─────────────────────
        { static_cast<int>(EVENT_INTERFACE_GLINTl), "GlintLeft"          },
        { static_cast<int>(EVENT_INTERFACE_GLINTr), "GlintRight"         },
        { static_cast<int>(EVENT_INTERFACE_GLINTu), "GlintTop"           },
        { static_cast<int>(EVENT_INTERFACE_GLINTb), "GlintBottom"        },

        // ── Player-select screen ──────────────────────────────────────────────
        { static_cast<int>(EVENT_INTERFACE_NEDIT),  "EditPlayerName"     },
        { static_cast<int>(EVENT_INTERFACE_NLIST),  "ListPlayers"        },
        { static_cast<int>(EVENT_INTERFACE_NOK),    "ButtonOK"           },
        { static_cast<int>(EVENT_INTERFACE_NDELETE),"ButtonDelete"       },
        { static_cast<int>(EVENT_INTERFACE_NLABEL), "LabelPlayerName"    },
        { static_cast<int>(EVENT_INTERFACE_PERSO),  "ButtonCustomize"    },

        // ── Main menu ─────────────────────────────────────────────────────────
        { static_cast<int>(EVENT_INTERFACE_TRAINER),      "ButtonExercises"   },
        { static_cast<int>(EVENT_INTERFACE_DEFI),         "ButtonChallenges"  },
        { static_cast<int>(EVENT_INTERFACE_MISSION),      "ButtonMissions"    },
        { static_cast<int>(EVENT_INTERFACE_FREE),         "ButtonFreeGame"    },
        { static_cast<int>(EVENT_INTERFACE_CODE_BATTLES), "ButtonCodeBattles" },
        { static_cast<int>(EVENT_INTERFACE_PLUS),         "ButtonPlus"        },
        { static_cast<int>(EVENT_INTERFACE_MODS),         "ButtonMods"        },
        { static_cast<int>(EVENT_INTERFACE_USER),         "ButtonUserLevels"  },
        { static_cast<int>(EVENT_INTERFACE_NAME),         "ButtonPlayerName"  },
        { static_cast<int>(EVENT_INTERFACE_SETUP),        "ButtonSetup"       },
        { static_cast<int>(EVENT_INTERFACE_QUIT),         "ButtonQuit"        },
        { static_cast<int>(EVENT_INTERFACE_SATCOM),       "ButtonSatCom"      },

        // ── Level / mission select ────────────────────────────────────────────
        { static_cast<int>(EVENT_INTERFACE_CHAP),   "ListChapter"        },
        { static_cast<int>(EVENT_INTERFACE_LIST),   "ListLevel"          },
        { static_cast<int>(EVENT_INTERFACE_PLAY),   "ButtonPlay"         },
        { static_cast<int>(EVENT_INTERFACE_BACK),   "ButtonBack"         },
        { static_cast<int>(EVENT_INTERFACE_RESUME), "ButtonResume"       },

        // ── In-game pause / end dialog (maindialog.cpp) ───────────────────────
        { static_cast<int>(EVENT_DIALOG_OK),        "ButtonAbort"        },
        { static_cast<int>(EVENT_DIALOG_CANCEL),    "ButtonContinue"     },
        { static_cast<int>(EVENT_INTERFACE_AGAIN),  "ButtonAgain"        },
        { static_cast<int>(EVENT_INTERFACE_WRITE),  "ButtonSave"         },
        { static_cast<int>(EVENT_INTERFACE_READ),   "ButtonLoad"         },
        { static_cast<int>(EVENT_INTERFACE_ABORT),  "ButtonAbortConfirm" },

        // ── End-of-level cinematic (PHASE_WIN/PHASE_LOST with EndingFile) ─────
        { static_cast<int>(EVENT_BUTTON_OK),        "ButtonEndLevel"     },

        // ── In-game console (backtick toggle) ─────────────────────────────────
        { static_cast<int>(EVENT_CMD),              "EditConsole"        },
        { static_cast<int>(EVENT_SPEED),            "SpeedControl"       },

        // ── Setup — tab buttons ────────────────────────────────────────────────
        { static_cast<int>(EVENT_INTERFACE_SETUPd), "ButtonTabDisplay"   },
        { static_cast<int>(EVENT_INTERFACE_SETUPg), "ButtonTabGraphics"  },
        { static_cast<int>(EVENT_INTERFACE_SETUPp), "ButtonTabGameplay"  },
        { static_cast<int>(EVENT_INTERFACE_SETUPc), "ButtonTabControls"  },
        { static_cast<int>(EVENT_INTERFACE_SETUPs), "ButtonTabSound"     },
        { static_cast<int>(EVENT_INTERFACE_APPLY),  "ButtonApply"        },

        // ── Setup — Display tab ───────────────────────────────────────────────
        { static_cast<int>(EVENT_INTERFACE_DEVICE), "ListDevice"         },
        { static_cast<int>(EVENT_INTERFACE_RESOL),  "ListResolution"     },
        { static_cast<int>(EVENT_INTERFACE_FULL),   "CheckFullscreen"    },

        // ── Setup — Graphics tab ──────────────────────────────────────────────
        { static_cast<int>(EVENT_INTERFACE_SHADOW_SPOTS),            "CheckShadowSpots"          },
        { static_cast<int>(EVENT_INTERFACE_DIRTY),                   "CheckDirtyTextures"        },
        { static_cast<int>(EVENT_INTERFACE_PARTI),                   "CheckParticles"            },
        { static_cast<int>(EVENT_INTERFACE_CLIP),                    "SliderViewDistance"        },
        { static_cast<int>(EVENT_INTERFACE_PAUSE_BLUR),              "CheckPauseBlur"            },
        { static_cast<int>(EVENT_INTERFACE_RAIN),                    "CheckRain"                 },
        { static_cast<int>(EVENT_INTERFACE_GLINT),                   "CheckGlints"               },
        { static_cast<int>(EVENT_INTERFACE_FOG),                     "CheckFog"                  },
        { static_cast<int>(EVENT_INTERFACE_SHADOW_MAPPING),          "CheckShadowMapping"        },
        { static_cast<int>(EVENT_INTERFACE_SHADOW_MAPPING_QUALITY),  "CheckShadowQuality"        },
        { static_cast<int>(EVENT_INTERFACE_SHADOW_MAPPING_BUFFER),   "SliderShadowBuffer"        },
        { static_cast<int>(EVENT_INTERFACE_TEXTURE_FILTER),          "ListTextureFilter"         },
        { static_cast<int>(EVENT_INTERFACE_TEXTURE_MIPMAP),          "CheckTextureMipmap"        },
        { static_cast<int>(EVENT_INTERFACE_TEXTURE_ANISOTROPY),      "SliderAnisotropy"          },
        { static_cast<int>(EVENT_INTERFACE_MSAA),                    "SliderMSAA"                },
        { static_cast<int>(EVENT_INTERFACE_VSYNC),                   "CheckVSync"                },
        { static_cast<int>(EVENT_INTERFACE_MIN),                     "ButtonQualityMin"          },
        { static_cast<int>(EVENT_INTERFACE_NORM),                    "ButtonQualityNorm"         },
        { static_cast<int>(EVENT_INTERFACE_MAX),                     "ButtonQualityMax"          },

        // ── Setup — Gameplay tab ──────────────────────────────────────────────
        { static_cast<int>(EVENT_INTERFACE_LANGUAGE),  "ListLanguage"       },
        { static_cast<int>(EVENT_INTERFACE_TOOLTIP),   "CheckTooltips"      },
        { static_cast<int>(EVENT_INTERFACE_MOVIES),    "CheckCinematics"    },
        { static_cast<int>(EVENT_INTERFACE_SCROLL),    "CheckEdgeScroll"    },
        { static_cast<int>(EVENT_INTERFACE_INVERTX),   "CheckInvertMouseX"  },
        { static_cast<int>(EVENT_INTERFACE_INVERTY),   "CheckInvertMouseY"  },
        { static_cast<int>(EVENT_INTERFACE_EFFECT),    "CheckCameraShake"   },
        { static_cast<int>(EVENT_INTERFACE_BGPAUSE),   "CheckPauseMusic"    },
        { static_cast<int>(EVENT_INTERFACE_BLOOD),     "CheckBloodEffects"  },
        { static_cast<int>(EVENT_INTERFACE_SOLUCE),    "CheckShowSolution"  },
        { static_cast<int>(EVENT_INTERFACE_EDITMODE),  "CheckEditMode"      },
        { static_cast<int>(EVENT_INTERFACE_EDITVALUE), "SliderEditValue"    },
        { static_cast<int>(EVENT_INTERFACE_AUTOSAVE_ENABLE),   "CheckAutosave"        },
        { static_cast<int>(EVENT_INTERFACE_AUTOSAVE_INTERVAL), "SliderAutosaveInterval"},
        { static_cast<int>(EVENT_INTERFACE_AUTOSAVE_SLOTS),    "SliderAutosaveSlots"   },

        // ── Setup — Sound tab ─────────────────────────────────────────────────
        { static_cast<int>(EVENT_INTERFACE_VOLSOUND), "SliderSoundVolume"  },
        { static_cast<int>(EVENT_INTERFACE_VOLMUSIC), "SliderMusicVolume"  },

        // ── In-game HUD — gauges ─────────────────────────────────────────────
        { static_cast<int>(EVENT_OBJECT_GENERGY),   "GaugeEnergy"        },
        { static_cast<int>(EVENT_OBJECT_GSHIELD),   "GaugeShield"        },
        { static_cast<int>(EVENT_OBJECT_GRANGE),    "GaugeRange"         },
        { static_cast<int>(EVENT_OBJECT_GPROGRESS), "GaugeProgress"      },
        { static_cast<int>(EVENT_OBJECT_GRADAR),    "GaugeRadar"         },

        // ── In-game HUD — map ─────────────────────────────────────────────────
        { static_cast<int>(EVENT_OBJECT_MAP),       "HudMap"             },
        { static_cast<int>(EVENT_OBJECT_MAPZOOM),   "HudMapZoom"         },

        // ── In-game HUD — movement / action buttons ───────────────────────────
        { static_cast<int>(EVENT_OBJECT_LEFT),      "ButtonMoveLeft"     },
        { static_cast<int>(EVENT_OBJECT_RIGHT),     "ButtonMoveRight"    },
        { static_cast<int>(EVENT_OBJECT_UP),        "ButtonMoveForward"  },
        { static_cast<int>(EVENT_OBJECT_DOWN),      "ButtonMoveBack"     },
        { static_cast<int>(EVENT_OBJECT_GASUP),     "ButtonJetUp"        },
        { static_cast<int>(EVENT_OBJECT_GASDOWN),   "ButtonJetDown"      },
        { static_cast<int>(EVENT_OBJECT_HTAKE),     "ButtonTake"         },
        { static_cast<int>(EVENT_OBJECT_CAMERA),    "ButtonCamera"       },
        { static_cast<int>(EVENT_OBJECT_DELETE),    "ButtonDelete"       },

        // ── In-game HUD — robot selection ────────────────────────────────────
        { static_cast<int>(EVENT_OBJECT_DESELECT),        "ButtonDeselect"     },
        { static_cast<int>(EVENT_OBJECT_BHELP),           "ButtonSatComOpen"   },
        { static_cast<int>(EVENT_OBJECT_HELP),            "ButtonHelp"         },
        { static_cast<int>(EVENT_OBJECT_SHORTCUT_MODE),   "ButtonShortcutMode" },
        { static_cast<int>(EVENT_OBJECT_MOVIELOCK),       "IndicatorMovieLock" },
        { static_cast<int>(EVENT_OBJECT_EDITLOCK),        "IndicatorEditLock"  },
        { static_cast<int>(EVENT_OBJECT_SAVING),          "IndicatorSaving"    },

        // ── In-game HUD — build buttons ───────────────────────────────────────
        { static_cast<int>(EVENT_OBJECT_BDERRICK),  "ButtonBuildDerrick"      },
        { static_cast<int>(EVENT_OBJECT_BSTATION),  "ButtonBuildPowerStation" },
        { static_cast<int>(EVENT_OBJECT_BFACTORY),  "ButtonBuildBotFactory"   },
        { static_cast<int>(EVENT_OBJECT_BCONVERT),  "ButtonBuildConverter"    },
        { static_cast<int>(EVENT_OBJECT_BTOWER),    "ButtonBuildTower"        },
        { static_cast<int>(EVENT_OBJECT_BREPAIR),   "ButtonBuildRepairCenter" },
        { static_cast<int>(EVENT_OBJECT_BRESEARCH), "ButtonBuildResearchCenter"},
        { static_cast<int>(EVENT_OBJECT_BRADAR),    "ButtonBuildRadarStation" },
        { static_cast<int>(EVENT_OBJECT_BENERGY),   "ButtonBuildPowerCaptor"  },
        { static_cast<int>(EVENT_OBJECT_BLABO),     "ButtonBuildBioLab"       },
        { static_cast<int>(EVENT_OBJECT_BNUCLEAR),  "ButtonBuildNuclearPlant" },
        { static_cast<int>(EVENT_OBJECT_BPARA),     "ButtonBuildLightningRod" },
        { static_cast<int>(EVENT_OBJECT_BINFO),     "ButtonBuildInfoExchange" },
        { static_cast<int>(EVENT_OBJECT_BSAFE),     "ButtonBuildVault"        },

        // ── In-game HUD — research buttons ───────────────────────────────────
        { static_cast<int>(EVENT_OBJECT_RTANK),     "ButtonResearchTank"      },
        { static_cast<int>(EVENT_OBJECT_RFLY),      "ButtonResearchFly"       },
        { static_cast<int>(EVENT_OBJECT_RTHUMP),    "ButtonResearchThump"     },
        { static_cast<int>(EVENT_OBJECT_RCANON),    "ButtonResearchCannon"    },
        { static_cast<int>(EVENT_OBJECT_RTOWER),    "ButtonResearchTower"     },
        { static_cast<int>(EVENT_OBJECT_RPHAZER),   "ButtonResearchPhazer"    },
        { static_cast<int>(EVENT_OBJECT_RSHIELD),   "ButtonResearchShield"    },
        { static_cast<int>(EVENT_OBJECT_RATOMIC),   "ButtonResearchNuclear"   },

        // ── In-game HUD — program management ─────────────────────────────────
        { static_cast<int>(EVENT_OBJECT_PROGEDIT),    "ButtonOpenStudio"   },
        { static_cast<int>(EVENT_OBJECT_PROGLIST),    "ListPrograms"       },
        { static_cast<int>(EVENT_OBJECT_PROGRUN),     "ButtonRunProgram"   },
        { static_cast<int>(EVENT_OBJECT_PROGADD),     "ButtonAddProgram"   },
        { static_cast<int>(EVENT_OBJECT_PROGREMOVE),  "ButtonRemoveProgram"},
        { static_cast<int>(EVENT_OBJECT_PROGCLONE),   "ButtonCloneProgram" },
        { static_cast<int>(EVENT_OBJECT_PROGMOVEUP),  "ButtonProgMoveUp"   },
        { static_cast<int>(EVENT_OBJECT_PROGMOVEDOWN),"ButtonProgMoveDown" },
        { static_cast<int>(EVENT_OBJECT_PROGSTOP),    "ButtonStopProgram"  },

        // ── SatCom / in-game documentation viewer ────────────────────────────
        { static_cast<int>(EVENT_EDIT1),            "SatComContent"      },
        { static_cast<int>(EVENT_SATCOM_HUSTON),    "SatComHouston"      },
        { static_cast<int>(EVENT_SATCOM_SAT),       "SatComSat"          },
        { static_cast<int>(EVENT_SATCOM_LOADING),   "SatComLoading"      },
        { static_cast<int>(EVENT_SATCOM_PROG),      "SatComProg"         },
        { static_cast<int>(EVENT_SATCOM_SOLUCE),    "SatComSoluce"       },
        { static_cast<int>(EVENT_HYPER_PREV),       "SatComPrev"         },
        { static_cast<int>(EVENT_HYPER_NEXT),       "SatComNext"         },
        { static_cast<int>(EVENT_HYPER_HOME),       "SatComHome"         },
        { static_cast<int>(EVENT_HYPER_SIZE1),      "SatComSize1"        },
        { static_cast<int>(EVENT_HYPER_SIZE2),      "SatComSize2"        },
        { static_cast<int>(EVENT_HYPER_SIZE3),      "SatComSize3"        },
        { static_cast<int>(EVENT_HYPER_SIZE4),      "SatComSize4"        },
        { static_cast<int>(EVENT_HYPER_SIZE5),      "SatComSize5"        },
        { static_cast<int>(EVENT_OBJECT_INFOOK),    "SatComClose"        },

        // ── Script studio (in-game code editor) ──────────────────────────────
        { static_cast<int>(EVENT_STUDIO_LIST),      "ListStudioPrograms" },
        { static_cast<int>(EVENT_STUDIO_EDIT),      "StudioEdit"         },
        { static_cast<int>(EVENT_STUDIO_COMPILE),   "StudioCompile"      },
        { static_cast<int>(EVENT_STUDIO_RUN),       "StudioRun"          },
        { static_cast<int>(EVENT_STUDIO_OK),        "StudioOK"           },
        { static_cast<int>(EVENT_STUDIO_CANCEL),    "StudioCancel"       },
        { static_cast<int>(EVENT_STUDIO_CLONE),     "StudioClone"        },
        { static_cast<int>(EVENT_STUDIO_NEW),       "StudioNew"          },
        { static_cast<int>(EVENT_STUDIO_OPEN),      "StudioOpen"         },
        { static_cast<int>(EVENT_STUDIO_SAVE),      "StudioSave"         },
        { static_cast<int>(EVENT_STUDIO_UNDO),      "StudioUndo"         },
        { static_cast<int>(EVENT_STUDIO_CUT),       "StudioCut"          },
        { static_cast<int>(EVENT_STUDIO_COPY),      "StudioCopy"         },
        { static_cast<int>(EVENT_STUDIO_PASTE),     "StudioPaste"        },
        { static_cast<int>(EVENT_STUDIO_SIZE),      "StudioFontSize"     },
        { static_cast<int>(EVENT_STUDIO_HELP),      "StudioHelp"         },
        { static_cast<int>(EVENT_STUDIO_REALTIME),  "StudioRealtime"     },
        { static_cast<int>(EVENT_STUDIO_STEP),      "StudioStep"         },
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
        // Optional "action": "tap" (default), "down", or "up". Down/up enable
        // holding keys for continuous movement (e.g. walking the astronaut).
        std::string action = JsonGet(req.body, "action");
        if (action.empty()) action = "tap";
        if (action != "tap" && action != "down" && action != "up")
        {
            res.status = 400;
            res.set_content(ErrResponse("action must be 'tap', 'down' or 'up'"), "application/json");
            return;
        }
        try
        {
            std::string body = PostAndWait([this, key, action]() -> std::string {
                return DoKey(key, action);
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            res.status = 400;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Post("/click_pos", [this](const httplib::Request& req, httplib::Response& res) {
        std::string sx = JsonGet(req.body, "x");
        std::string sy = JsonGet(req.body, "y");
        if (sx.empty() || sy.empty())
        {
            res.status = 400;
            res.set_content(ErrResponse("missing 'x' or 'y'"), "application/json");
            return;
        }
        try
        {
            float x = std::stof(sx);
            float y = std::stof(sy);
            std::string body = PostAndWait([this, x, y]() -> std::string {
                return DoClickPos(x, y);
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            res.status = 400;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Post("/mouse_move", [this](const httplib::Request& req, httplib::Response& res) {
        std::string sx = JsonGet(req.body, "x");
        std::string sy = JsonGet(req.body, "y");
        if (sx.empty() || sy.empty())
        {
            res.status = 400;
            res.set_content(ErrResponse("missing 'x' or 'y'"), "application/json");
            return;
        }
        float x = std::stof(sx), y = std::stof(sy);
        std::string body = PostAndWait([this, x, y]() -> std::string {
            return DoMouseMove(x, y);
        });
        res.set_content(OkResponse(body), "application/json");
    });

    m_svr->Post("/drag", [this](const httplib::Request& req, httplib::Response& res) {
        std::string sfx = JsonGet(req.body, "from_x");
        std::string sfy = JsonGet(req.body, "from_y");
        std::string stx = JsonGet(req.body, "to_x");
        std::string sty = JsonGet(req.body, "to_y");
        if (sfx.empty() || sfy.empty() || stx.empty() || sty.empty())
        {
            res.status = 400;
            res.set_content(ErrResponse("missing from_x/from_y/to_x/to_y"), "application/json");
            return;
        }
        float fx = std::stof(sfx), fy = std::stof(sfy);
        float tx = std::stof(stx), ty = std::stof(sty);
        std::string ssteps  = JsonGet(req.body, "steps");
        std::string sbtn    = JsonGet(req.body, "button");
        int steps           = ssteps.empty()  ? 20          : std::stoi(ssteps);
        std::string button  = sbtn.empty()    ? "left"      : sbtn;
        if (steps < 1) steps = 1;
        std::string body = PostAndWait([this, fx, fy, tx, ty, steps, button]() -> std::string {
            return DoDrag(fx, fy, tx, ty, steps, button);
        });
        res.set_content(OkResponse(body), "application/json");
    });

    m_svr->Post("/mouse_button", [this](const httplib::Request& req, httplib::Response& res) {
        std::string action = JsonGet(req.body, "action");
        std::string sx     = JsonGet(req.body, "x");
        std::string sy     = JsonGet(req.body, "y");
        if (action.empty() || sx.empty() || sy.empty())
        {
            res.status = 400;
            res.set_content(ErrResponse("missing 'action', 'x', or 'y'"), "application/json");
            return;
        }
        if (action != "down" && action != "up")
        {
            res.status = 400;
            res.set_content(ErrResponse("action must be 'down' or 'up'"), "application/json");
            return;
        }
        float x = std::stof(sx), y = std::stof(sy);
        std::string sbtn   = JsonGet(req.body, "button");
        std::string button = sbtn.empty() ? "left" : sbtn;
        std::string body = PostAndWait([this, action, x, y, button]() -> std::string {
            return DoMouseButton(action, x, y, button);
        });
        res.set_content(OkResponse(body), "application/json");
    });

    m_svr->Get("/window", [this](const httplib::Request& req, httplib::Response& res) {
        std::string body = PostAndWait([this]() -> std::string {
            return DoWindow();
        });
        res.set_content(OkResponse(body), "application/json");
    });

    m_svr->Get("/objects", [this](const httplib::Request& req, httplib::Response& res) {
        try
        {
            std::string body = PostAndWait([this]() -> std::string {
                return DoObjects();
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            res.status = 500;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Post("/launch", [this](const httplib::Request& req, httplib::Response& res) {
        std::string cat  = JsonGet(req.body, "category");
        std::string chap = JsonGet(req.body, "chap");
        std::string rank = JsonGet(req.body, "rank");
        if (cat.empty() || chap.empty() || rank.empty())
        {
            res.status = 400;
            res.set_content(ErrResponse("missing 'category', 'chap', or 'rank'"), "application/json");
            return;
        }
        try
        {
            std::string body = PostAndWait([this, cat, chap, rank]() -> std::string {
                return DoLaunch(cat, std::stoi(chap), std::stoi(rank));
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            int code = std::string(e.what()).find("unknown category") != std::string::npos ? 400 : 500;
            res.status = code;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Get("/program", [this](const httplib::Request& req, httplib::Response& res) {
        int slot = -1;
        if (req.has_param("slot"))
        {
            try { slot = std::stoi(req.get_param_value("slot")); }
            catch (...) { res.status = 400; res.set_content(ErrResponse("invalid slot"), "application/json"); return; }
        }
        try
        {
            std::string body = PostAndWait([this, slot]() -> std::string {
                return DoGetProgram(slot);
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            int code = std::string(e.what()).find("not found") != std::string::npos ? 404 : 500;
            res.status = code;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Post("/program", [this](const httplib::Request& req, httplib::Response& res) {
        std::string source  = JsonGet(req.body, "source");
        std::string slotStr = JsonGet(req.body, "slot");
        std::string compStr = JsonGet(req.body, "compile");
        int slot = slotStr.empty() ? -1 : std::stoi(slotStr);
        bool doCompile = (compStr == "true" || compStr == "1");
        try
        {
            std::string body = PostAndWait([this, source, slot, doCompile]() -> std::string {
                return DoSetProgram(slot, source, doCompile);
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            int code = std::string(e.what()).find("not found") != std::string::npos ? 404 : 500;
            res.status = code;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Get("/diagnostics", [this](const httplib::Request& req, httplib::Response& res) {
        int slot = -1;
        if (req.has_param("slot"))
        {
            try { slot = std::stoi(req.get_param_value("slot")); }
            catch (...) { res.status = 400; res.set_content(ErrResponse("invalid slot"), "application/json"); return; }
        }
        try
        {
            std::string body = PostAndWait([this, slot]() -> std::string {
                return DoDiagnostics(slot);
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            int code = std::string(e.what()).find("not found") != std::string::npos ? 404 : 500;
            res.status = code;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Get("/speed", [this](const httplib::Request& req, httplib::Response& res) {
        try
        {
            std::string body = PostAndWait([]() -> std::string {
                auto* main = CRobotMain::GetInstancePointer();
                if (!main) throw std::runtime_error("game not running");
                return "{\"speed\":" + std::to_string(main->GetSpeed()) + "}";
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            res.status = 500;
            res.set_content(ErrResponse(e.what()), "application/json");
        }
    });

    m_svr->Post("/speed", [this](const httplib::Request& req, httplib::Response& res) {
        std::string speedStr = JsonGet(req.body, "speed");
        if (speedStr.empty())
        {
            res.status = 400;
            res.set_content(ErrResponse("missing 'speed'"), "application/json");
            return;
        }
        float speed;
        try { speed = std::stof(speedStr); }
        catch (...) { res.status = 400; res.set_content(ErrResponse("invalid speed value"), "application/json"); return; }
        if (speed <= 0.0f || speed > 100.0f)
        {
            res.status = 400;
            res.set_content(ErrResponse("speed must be in (0, 100]"), "application/json");
            return;
        }
        try
        {
            std::string body = PostAndWait([speed]() -> std::string {
                auto* main = CRobotMain::GetInstancePointer();
                if (!main) throw std::runtime_error("game not running");
                main->SetSpeed(speed);
                return "{\"speed\":" + std::to_string(main->GetSpeed()) + "}";
            });
            res.set_content(OkResponse(body), "application/json");
        }
        catch (const std::exception& e)
        {
            res.status = 500;
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

    if (has("EditPlayerName") && has("ListPlayers"))     return "PlayerSelect";
    if (has("ButtonExercises") && has("ButtonQuit"))     return "MainMenu";
    if (has("ListChapter") && has("ListLevel"))           return "LevelSelect";
    if (has("ButtonEndLevel"))                            return "LevelComplete";
    if (has("ListLanguage"))                              return "SetupGame";
    if (has("ButtonTabDisplay") && has("ButtonTabGraphics")) return "SetupDisplay";
    if (has("StudioEdit") && has("StudioRun"))            return "Studio";
    if (has("ButtonAbort") || has("ButtonAgain"))         return "InGameMenu";
    if (has("SatComContent") && has("SatComClose"))       return "SatCom";
    // In-game HUD: no menu-specific widgets present.
    // Loading state has ≤6 widgets; real in-game states have ≥7 or more.
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

std::string CAgentServer::DoKey(const std::string& key, const std::string& action)
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
        { "Backquote", SDL_SCANCODE_GRAVE },
        { "Up",    SDL_SCANCODE_UP    },
        { "Down",  SDL_SCANCODE_DOWN  },
        { "Left",  SDL_SCANCODE_LEFT  },
        { "Right", SDL_SCANCODE_RIGHT },
        { "Delete",    SDL_SCANCODE_DELETE    },
        { "Backspace", SDL_SCANCODE_BACKSPACE },
        // Letter keys for WASD-style movement and shortcuts.
        { "A", SDL_SCANCODE_A }, { "B", SDL_SCANCODE_B }, { "C", SDL_SCANCODE_C },
        { "D", SDL_SCANCODE_D }, { "E", SDL_SCANCODE_E }, { "F", SDL_SCANCODE_F },
        { "G", SDL_SCANCODE_G }, { "H", SDL_SCANCODE_H }, { "I", SDL_SCANCODE_I },
        { "J", SDL_SCANCODE_J }, { "K", SDL_SCANCODE_K }, { "L", SDL_SCANCODE_L },
        { "M", SDL_SCANCODE_M }, { "N", SDL_SCANCODE_N }, { "O", SDL_SCANCODE_O },
        { "P", SDL_SCANCODE_P }, { "Q", SDL_SCANCODE_Q }, { "R", SDL_SCANCODE_R },
        { "S", SDL_SCANCODE_S }, { "T", SDL_SCANCODE_T }, { "U", SDL_SCANCODE_U },
        { "V", SDL_SCANCODE_V }, { "W", SDL_SCANCODE_W }, { "X", SDL_SCANCODE_X },
        { "Y", SDL_SCANCODE_Y }, { "Z", SDL_SCANCODE_Z },
    };

    auto it = kKeys.find(key);
    if (it == kKeys.end())
        throw std::runtime_error("unsupported key: " + key);

    // Inject SDL key events. "down"/"up" enable holding keys for continuous
    // movement (e.g. walking the astronaut to a power cell). "tap" sends both
    // down and up immediately, which is the existing behaviour.
    auto pushEvent = [&](Uint32 type) {
        SDL_Event ev{};
        ev.type                  = type;
        ev.key.keysym.scancode   = it->second;
        ev.key.keysym.sym        = SDL_GetKeyFromScancode(it->second);
        SDL_PushEvent(&ev);
    };

    if (action == "down" || action == "tap")
        pushEvent(SDL_KEYDOWN);
    if (action == "up"   || action == "tap")
        pushEvent(SDL_KEYUP);

    return "{\"key\":\"" + JsonEscape(key) + "\",\"action\":\"" + JsonEscape(action) + "\"}";
}

std::string CAgentServer::DoClickPos(float x, float y)
{
    // x, y are interface coords in [0,1] (origin bottom-left, matching Colobot conventions).
    // Convert to SDL window pixels: SDL origin is top-left, so flip y.
    glm::ivec2 winSize = m_engine->GetWindowSize();
    int px = static_cast<int>(x * static_cast<float>(winSize.x));
    int py = static_cast<int>((1.0f - y) * static_cast<float>(winSize.y));

    SDL_Event motion{};
    motion.type         = SDL_MOUSEMOTION;
    motion.motion.x     = px;
    motion.motion.y     = py;
    SDL_PushEvent(&motion);

    SDL_Event down{};
    down.type           = SDL_MOUSEBUTTONDOWN;
    down.button.button  = SDL_BUTTON_LEFT;
    down.button.x       = px;
    down.button.y       = py;
    SDL_PushEvent(&down);

    SDL_Event up = down;
    up.type = SDL_MOUSEBUTTONUP;
    SDL_PushEvent(&up);

    return "{\"x\":" + std::to_string(x) + ",\"y\":" + std::to_string(y) + "}";
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

// ---------------------------------------------------------------------------
// Mouse helper: resolve button name to SDL constant
// ---------------------------------------------------------------------------
static Uint8 SdlButton(const std::string& name)
{
    if (name == "right")  return SDL_BUTTON_RIGHT;
    if (name == "middle") return SDL_BUTTON_MIDDLE;
    return SDL_BUTTON_LEFT;
}

// Convert interface coords [0..1] (bottom-left origin) to SDL window pixels (top-left origin).
static glm::ivec2 InterfaceToPixel(float x, float y, glm::ivec2 winSize)
{
    return {
        static_cast<int>(x * static_cast<float>(winSize.x)),
        static_cast<int>((1.0f - y) * static_cast<float>(winSize.y))
    };
}

static std::string PxJson(const std::string& prefix, float ix, float iy, glm::ivec2 winSize)
{
    auto px = InterfaceToPixel(ix, iy, winSize);
    return "\"" + prefix + "\":{\"x\":" + std::to_string(ix)
         + ",\"y\":" + std::to_string(iy)
         + ",\"px\":" + std::to_string(px.x)
         + ",\"py\":" + std::to_string(px.y) + "}";
}

// ---------------------------------------------------------------------------
// POST /mouse_move — move cursor without clicking
// ---------------------------------------------------------------------------
std::string CAgentServer::DoMouseMove(float x, float y)
{
    glm::ivec2 winSize = m_engine->GetWindowSize();
    auto p = InterfaceToPixel(x, y, winSize);

    SDL_Event motion{};
    motion.type       = SDL_MOUSEMOTION;
    motion.motion.x   = p.x;
    motion.motion.y   = p.y;
    SDL_PushEvent(&motion);

    return "{\"x\":" + std::to_string(x) + ",\"y\":" + std::to_string(y)
         + ",\"px\":" + std::to_string(p.x) + ",\"py\":" + std::to_string(p.y) + "}";
}

// ---------------------------------------------------------------------------
// POST /drag — button-down → N motion events → button-up
// ---------------------------------------------------------------------------
std::string CAgentServer::DoDrag(float fx, float fy, float tx, float ty,
                                  int steps, const std::string& button)
{
    glm::ivec2 winSize = m_engine->GetWindowSize();
    auto pf = InterfaceToPixel(fx, fy, winSize);
    auto pt = InterfaceToPixel(tx, ty, winSize);
    Uint8 btn = SdlButton(button);

    // Press at start
    SDL_Event down{};
    down.type           = SDL_MOUSEBUTTONDOWN;
    down.button.button  = btn;
    down.button.x       = pf.x;
    down.button.y       = pf.y;
    SDL_PushEvent(&down);

    // Intermediate motion events along straight-line path
    for (int i = 0; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        SDL_Event motion{};
        motion.type     = SDL_MOUSEMOTION;
        motion.motion.x = pf.x + static_cast<int>(static_cast<float>(pt.x - pf.x) * t);
        motion.motion.y = pf.y + static_cast<int>(static_cast<float>(pt.y - pf.y) * t);
        SDL_PushEvent(&motion);
    }

    // Release at end
    SDL_Event up{};
    up.type           = SDL_MOUSEBUTTONUP;
    up.button.button  = btn;
    up.button.x       = pt.x;
    up.button.y       = pt.y;
    SDL_PushEvent(&up);

    return "{" + PxJson("from", fx, fy, winSize)
         + "," + PxJson("to", tx, ty, winSize)
         + ",\"steps\":" + std::to_string(steps)
         + ",\"button\":\"" + button + "\"}";
}

// ---------------------------------------------------------------------------
// POST /mouse_button — independent press or release
// ---------------------------------------------------------------------------
std::string CAgentServer::DoMouseButton(const std::string& action,
                                         float x, float y,
                                         const std::string& button)
{
    glm::ivec2 winSize = m_engine->GetWindowSize();
    auto p = InterfaceToPixel(x, y, winSize);
    Uint8 btn = SdlButton(button);

    SDL_Event ev{};
    ev.type           = (action == "down") ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
    ev.button.button  = btn;
    ev.button.x       = p.x;
    ev.button.y       = p.y;
    SDL_PushEvent(&ev);

    return "{\"action\":\"" + action + "\","
         + PxJson("pos", x, y, winSize)
         + ",\"button\":\"" + button + "\"}";
}

// ---------------------------------------------------------------------------
// GET /window — actual window + display geometry
// ---------------------------------------------------------------------------
std::string CAgentServer::DoWindow()
{
    // SDL_Window is in CApplication's private pimpl; retrieve via the current GL context
    // (safe because DoWindow() always runs on the main thread via PostAndWait).
    SDL_Window* win = SDL_GL_GetCurrentWindow();
    if (!win)
        return "{}";

    int wx = 0, wy = 0;
    SDL_GetWindowPosition(win, &wx, &wy);

    int lw = 0, lh = 0;
    SDL_GetWindowSize(win, &lw, &lh);

    int dw = 0, dh = 0;
    SDL_GL_GetDrawableSize(win, &dw, &dh);

    float sx = (lw > 0) ? static_cast<float>(dw) / static_cast<float>(lw) : 1.0f;
    float sy = (lh > 0) ? static_cast<float>(dh) / static_cast<float>(lh) : 1.0f;

    int dispIdx = SDL_GetWindowDisplayIndex(win);
    if (dispIdx < 0) dispIdx = 0;

    SDL_Rect bounds{}, usable{};
    SDL_GetDisplayBounds(dispIdx, &bounds);
    SDL_GetDisplayUsableBounds(dispIdx, &usable);

    // Collect flag strings
    Uint32 flags = SDL_GetWindowFlags(win);
    std::string flagArr = "[";
    bool ff = true;
    auto addFlag = [&](Uint32 mask, const char* name) {
        if (!(flags & mask)) return;
        if (!ff) flagArr += ",";
        flagArr += "\""; flagArr += name; flagArr += "\"";
        ff = false;
    };
    addFlag(SDL_WINDOW_SHOWN,        "shown");
    addFlag(SDL_WINDOW_HIDDEN,       "hidden");
    addFlag(SDL_WINDOW_FULLSCREEN,   "fullscreen");
    addFlag(SDL_WINDOW_FULLSCREEN_DESKTOP, "fullscreen_desktop");
    addFlag(SDL_WINDOW_RESIZABLE,    "resizable");
    addFlag(SDL_WINDOW_MAXIMIZED,    "maximized");
    addFlag(SDL_WINDOW_MINIMIZED,    "minimized");
    addFlag(SDL_WINDOW_INPUT_FOCUS,  "input_focus");
    addFlag(SDL_WINDOW_MOUSE_FOCUS,   "mouse_focus");
    addFlag(SDL_WINDOW_ALLOW_HIGHDPI, "high_dpi");
    flagArr += "]";

    std::string out;
    out += "{";
    out += "\"x\":"           + std::to_string(wx);
    out += ",\"y\":"          + std::to_string(wy);
    out += ",\"logical_w\":"  + std::to_string(lw);
    out += ",\"logical_h\":"  + std::to_string(lh);
    out += ",\"drawable_w\":" + std::to_string(dw);
    out += ",\"drawable_h\":" + std::to_string(dh);
    out += ",\"scale_x\":"    + std::to_string(sx);
    out += ",\"scale_y\":"    + std::to_string(sy);
    out += ",\"display_index\":" + std::to_string(dispIdx);
    out += ",\"display_bounds\":{\"x\":"; out += std::to_string(bounds.x);
    out += ",\"y\":"; out += std::to_string(bounds.y);
    out += ",\"w\":"; out += std::to_string(bounds.w);
    out += ",\"h\":"; out += std::to_string(bounds.h); out += "}";
    out += ",\"display_usable\":{\"x\":"; out += std::to_string(usable.x);
    out += ",\"y\":"; out += std::to_string(usable.y);
    out += ",\"w\":"; out += std::to_string(usable.w);
    out += ",\"h\":"; out += std::to_string(usable.h); out += "}";
    out += ",\"flags\":" + flagArr;
    out += "}";
    return out;
}

// ---------------------------------------------------------------------------
// /objects — list game objects with projected screen positions and rich state.
// ---------------------------------------------------------------------------

// Returns the class name of a foreground task, e.g. "TaskGoto".
// Uses Itanium ABI demangling heuristic (digits prefix + leading 'C' stripped).
static std::string TaskClassName(CForegroundTask* task)
{
    if (!task) return "";
    const char* raw = typeid(*task).name();
    // Itanium ABI: "10CTaskGoto" → skip digits → "CTaskGoto" → skip 'C' → "TaskGoto"
    while (*raw && (*raw >= '0' && *raw <= '9')) ++raw;
    if (*raw == 'C') ++raw;
    return raw;
}

std::string CAgentServer::DoObjects()
{
    if (!CObjectManager::IsCreated())
        return "[]";

    std::string out = "[";
    bool first = true;

    for (CObject* obj : CObjectManager::GetInstance().GetAllObjects())
    {
        if (!obj) continue;

        ObjectType type = obj->GetType();
        const char* typeName = GetObjectName(type);
        if (!typeName || typeName[0] == '\0') continue;

        glm::vec3 worldPos  = obj->GetPosition();
        glm::vec3 worldRot  = obj->GetRotation();

        float sx = -1.0f, sy = -1.0f;
        glm::vec2 p2D;
        bool inFrustum = m_engine->WorldToInterface(p2D, worldPos);
        bool visible   = inFrustum && p2D.x >= 0.0f && p2D.x <= 1.0f
                                   && p2D.y >= 0.0f && p2D.y <= 1.0f;
        if (inFrustum) { sx = p2D.x; sy = p2D.y; }

        // Optional rich state — only populated for objects that implement the interface.
        float energy = -1.0f;
        if (GetObjectPowerCell(obj))
            energy = GetObjectEnergyLevel(obj);

        float shield = -1.0f;
        if (auto* shielded = dynamic_cast<CShieldedObject*>(obj))
            shield = shielded->GetShield();

        bool progRunning = false;
        if (auto* progObj = dynamic_cast<CProgrammableObject*>(obj))
            progRunning = progObj->IsProgram();

        std::string taskName;
        if (auto* te = dynamic_cast<CTaskExecutorObject*>(obj))
            if (te->IsForegroundTask())
                taskName = TaskClassName(te->GetForegroundTask());

        if (!first) out += ",";
        first = false;

        out += "{";
        out += "\"id\":"           + std::to_string(obj->GetID());
        out += ",\"type\":\""      + JsonEscape(typeName) + "\"";
        out += ",\"screen_x\":"    + std::to_string(sx);
        out += ",\"screen_y\":"    + std::to_string(sy);
        out += ",\"visible\":"     + std::string(visible ? "true" : "false");
        out += ",\"pos\":{\"x\":"  + std::to_string(worldPos.x)
             + ",\"y\":"           + std::to_string(worldPos.y)
             + ",\"z\":"           + std::to_string(worldPos.z) + "}";
        out += ",\"rotation\":{\"x\":" + std::to_string(worldRot.x)
             + ",\"y\":"               + std::to_string(worldRot.y)
             + ",\"z\":"               + std::to_string(worldRot.z) + "}";
        if (energy >= 0.0f)
            out += ",\"energy\":"  + std::to_string(energy);
        if (shield >= 0.0f)
            out += ",\"shield\":"  + std::to_string(shield);
        if (dynamic_cast<CProgrammableObject*>(obj))
            out += ",\"program_running\":" + std::string(progRunning ? "true" : "false");
        if (!taskName.empty())
            out += ",\"task\":\"" + JsonEscape(taskName) + "\"";
        out += "}";
    }

    out += "]";
    return out;
}

// ---------------------------------------------------------------------------
// /launch — directly start a named level without navigating menus.
// ---------------------------------------------------------------------------

std::string CAgentServer::DoLaunch(const std::string& category, int chap, int rank)
{
    static const std::unordered_map<std::string, LevelCategory> kCatMap = {
        { "Exercises",    LevelCategory::Exercises    },
        { "Challenges",   LevelCategory::Challenges   },
        { "Missions",     LevelCategory::Missions     },
        { "FreeGame",     LevelCategory::FreeGame     },
        { "CodeBattles",  LevelCategory::CodeBattles  },
    };
    auto it = kCatMap.find(category);
    if (it == kCatMap.end())
        throw std::runtime_error("unknown category: " + category);

    auto* main = CRobotMain::GetInstancePointer();
    if (!main) throw std::runtime_error("game not running");

    main->SetLevel(it->second, chap, rank);
    main->ChangePhase(PHASE_SIMUL);

    return "{\"category\":\"" + JsonEscape(category) + "\""
           ",\"chap\":"       + std::to_string(chap) +
           ",\"rank\":"       + std::to_string(rank) + "}";
}

// ---------------------------------------------------------------------------
// /program and /diagnostics — direct access to robot program source and state.
// ---------------------------------------------------------------------------

// Returns {storage, slot_count, program} for the selected robot.
// slot=-1 means "last slot". Throws if no robot selected or slot out of range.
static std::tuple<CProgramStorageObject*, int, Program*>
GetSelectedProgram(int slot)
{
    auto* main = CRobotMain::GetInstancePointer();
    if (!main) throw std::runtime_error("no game running");

    CObject* obj = main->GetSelect();
    if (!obj) throw std::runtime_error("no robot selected: not found");

    auto* storage = dynamic_cast<CProgramStorageObject*>(obj);
    if (!storage) throw std::runtime_error("selected object has no programs: not found");

    int count = storage->GetProgramCount();
    if (count == 0) throw std::runtime_error("robot has no program slots: not found");

    if (slot < 0) slot = count - 1;
    if (slot >= count) throw std::runtime_error("slot out of range: not found");

    Program* prog = storage->GetProgram(slot);
    if (!prog) throw std::runtime_error("null program at slot: not found");

    return { storage, count, prog };
}

// Build a JSON error object from a CScript's current error state, or "null".
static std::string ScriptErrorJson(CScript* script)
{
    if (script->GetError() == 0) return "null";
    std::string msg;
    script->GetError(msg);
    int c1 = script->GetCursor1();
    int c2 = script->GetCursor2();
    return "{\"message\":\"" + JsonEscape(msg) + "\""
           ",\"cursor_start\":" + std::to_string(c1) +
           ",\"cursor_end\":"   + std::to_string(c2) + "}";
}

std::string CAgentServer::DoGetProgram(int slot)
{
    auto [storage, count, prog] = GetSelectedProgram(slot);
    if (slot < 0) slot = count - 1;

    CScript* script = prog->script.get();
    std::string source = script ? script->GetSource() : "";
    bool compiled      = script ? script->GetCompile() : false;

    return "{\"slot\":"      + std::to_string(slot) +
           ",\"slot_count\":" + std::to_string(count) +
           ",\"source\":\""  + JsonEscape(source) + "\"" +
           ",\"compiled\":"  + (compiled ? "true" : "false") +
           ",\"runnable\":"  + (prog->runnable ? "true" : "false") +
           ",\"filename\":\"" + JsonEscape(prog->filename) + "\"}";
}

std::string CAgentServer::DoSetProgram(int slot, const std::string& source, bool doCompile)
{
    auto [storage, count, prog] = GetSelectedProgram(slot);
    if (slot < 0) slot = count - 1;

    CScript* script = prog->script.get();
    if (!script) throw std::runtime_error("program has no script object: not found");

    script->SetSource(source);

    if (!doCompile)
        return "{\"compiled\":false,\"error\":null}";

    bool ok = script->CompileScript();
    std::string errJson = ok ? "null" : ScriptErrorJson(script);
    return "{\"compiled\":" + std::string(ok ? "true" : "false") +
           ",\"error\":"    + errJson + "}";
}

std::string CAgentServer::DoDiagnostics(int slot)
{
    auto* main = CRobotMain::GetInstancePointer();
    if (!main)
        return "{\"selected_robot\":false,\"slot\":0,\"compiled\":false,\"running\":false,\"error\":null}";

    CObject* obj = main->GetSelect();
    if (!obj)
        return "{\"selected_robot\":false,\"slot\":0,\"compiled\":false,\"running\":false,\"error\":null}";

    auto* storage = dynamic_cast<CProgramStorageObject*>(obj);
    auto* prog_obj = dynamic_cast<CProgrammableObject*>(obj);
    if (!storage)
        return "{\"selected_robot\":true,\"slot\":0,\"compiled\":false,\"running\":false,\"error\":null}";

    int count = storage->GetProgramCount();
    if (count == 0)
        return "{\"selected_robot\":true,\"slot\":0,\"compiled\":false,\"running\":false,\"error\":null}";

    if (slot < 0) slot = count - 1;
    if (slot >= count) throw std::runtime_error("slot out of range: not found");

    Program* prog = storage->GetProgram(slot);
    CScript* script = prog ? prog->script.get() : nullptr;

    bool compiled = script ? script->GetCompile() : false;
    bool running  = prog_obj ? prog_obj->IsProgram() : false;
    std::string errJson = (script && script->GetError() != 0) ? ScriptErrorJson(script) : "null";

    return "{\"selected_robot\":true"
           ",\"slot\":"      + std::to_string(slot) +
           ",\"compiled\":"  + (compiled ? "true" : "false") +
           ",\"running\":"   + (running  ? "true" : "false") +
           ",\"error\":"     + errJson + "}";
}
