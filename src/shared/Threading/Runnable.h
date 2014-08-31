#ifndef RUNNABLE_H
#define RUNNABLE_H

#include <atomic>
#include <thread>

class Runnable
{
public:
    Runnable() : m_stop(), m_thread() { }
    virtual ~Runnable() { try { stop(); } catch(...) { /*??*/ } }

    Runnable(Runnable const&) = delete;
    Runnable& operator =(Runnable const&) = delete;

    void stop() { m_stop = true; m_thread.join(); }
    void start() { m_thread = std::thread(&Runnable::run, this); }

protected:
    virtual void run() = 0;
    std::atomic<bool> m_stop;

private:
    std::thread m_thread;
};

#endif // RUNNABLE_H