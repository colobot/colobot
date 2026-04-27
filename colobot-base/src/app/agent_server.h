#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <glm/glm.hpp>

namespace httplib { class Server; }
namespace Gfx
{
class CEngine;
class CDevice;
}
class CEventQueue;

class CAgentServer
{
public:
    explicit CAgentServer(int port = 7777);
    ~CAgentServer();

    // engine and eventQueue must outlive the server.
    // The interface is obtained dynamically via CRobotMain each request.
    void Start(Gfx::CEngine* engine, CEventQueue* eventQueue);
    void Stop();

    // Call once per frame from the main thread, AFTER rendering but BEFORE SwapBuffers.
    // Fulfills any pending screenshot request by reading from the current GL framebuffer.
    void CaptureFrameIfPending(Gfx::CDevice* device, const glm::ivec2& size);

    // Call once per frame from the main thread, after SwapBuffers.
    void DrainQueue();

    bool IsRunning() const { return m_running; }

private:
    struct Command
    {
        std::function<std::string()> execute;
        std::promise<std::string>    promise;
    };

    // Posts a command to the main thread and blocks until it completes (or times out).
    // Returns JSON result string. Throws std::runtime_error on timeout.
    std::string PostAndWait(std::function<std::string()> fn);

    void ServerThread();

    // Main-thread command implementations (called via PostAndWait)
    std::string BuildStateJson();
    std::string DoClick     (const std::string& id);
    std::string DoClickPos  (float x, float y);
    std::string DoType      (const std::string& id, const std::string& text);
    std::string DoSelect    (const std::string& id, const std::string& item, const std::string& index);
    std::string DoKey       (const std::string& key, const std::string& action = "tap");
    std::string DoMouseMove    (float x, float y);
    std::string DoDrag         (float fx, float fy, float tx, float ty, int steps, const std::string& button);
    std::string DoMouseButton  (const std::string& action, float x, float y, const std::string& button);
    std::string DoWindow       ();
    std::string DoObjects      ();
    std::string DoLaunch       (const std::string& category, int chap, int rank);
    std::string DoGetProgram   (int slot);
    std::string DoSetProgram   (int slot, const std::string& source, bool doCompile);
    std::string DoDiagnostics  (int slot);

    // Encodes raw RGBA pixels (bottom-up, from glReadPixels) as a PNG in memory.
    static std::vector<unsigned char> EncodeRGBAToPNG(
        const unsigned char* rgba, int width, int height);

    // OS-level screenshot via platform screencapture tools (?source=os).
    std::string DoScreenshotOS();

    int             m_port;
    bool            m_running = false;

    // Owned by the server thread
    std::unique_ptr<httplib::Server> m_svr;
    std::thread     m_thread;

    std::mutex      m_mutex;
    std::queue<Command> m_queue;

    // Screenshot capture: set by HTTP thread, fulfilled by main thread pre-swap.
    std::atomic<bool>              m_screenshotPending{false};
    std::mutex                     m_screenshotMutex;
    std::promise<std::string>      m_screenshotPromise;

    Gfx::CEngine*   m_engine     = nullptr;
    CEventQueue*    m_eventQueue = nullptr;
};
