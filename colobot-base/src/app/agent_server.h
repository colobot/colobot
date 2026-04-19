#pragma once

#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace httplib { class Server; }
namespace Gfx { class CEngine; }
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

    // Call once per frame from the main thread, after rendering.
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
    std::string DoType      (const std::string& id, const std::string& text);
    std::string DoSelect    (const std::string& id, const std::string& item, const std::string& index);
    std::string DoKey       (const std::string& key);
    std::string DoScreenshot();

    int             m_port;
    bool            m_running = false;

    // Owned by the server thread
    std::unique_ptr<httplib::Server> m_svr;
    std::thread     m_thread;

    std::mutex      m_mutex;
    std::queue<Command> m_queue;

    Gfx::CEngine*   m_engine     = nullptr;
    CEventQueue*    m_eventQueue = nullptr;
};
