#pragma once

#include <cstdint>
#include <vector>

/*
 * EpollLoop
 * ---------
 * Thin RAII wrapper over epoll.
 *
 * Responsibilities:
 * - Create / destroy epoll instance
 * - Register / modify / remove FDs
 * - Wait for readiness events
 *
 * Non-responsibilities:
 * - Reading or writing
 * - Connection logic
 * - State machines
 * - Error handling policy
 */

class EpollLoop {
public:
    EpollLoop();
    ~EpollLoop();

    EpollLoop(const EpollLoop&) = delete;
    EpollLoop& operator=(const EpollLoop&) = delete;

    // Register fd with interest mask
    bool add(int fd, uint32_t events, void* user_data);

    // Modify interest mask
    bool modify(int fd, uint32_t events, void* user_data);

    // Remove fd
    bool remove(int fd);

    // Wait for events
    int wait(int timeout_ms);

    struct Event {
        int fd;
        uint32_t events;
        void* user_data;
    };

    // Access collected events after wait()
    const std::vector<Event>& events() const;

private:
    int epoll_fd_;
    std::vector<Event> ready_events_;
};
