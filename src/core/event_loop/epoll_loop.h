#pragma once

#include <vector>
#include <sys/epoll.h>

class EpollLoop {
public:
    EpollLoop();
    ~EpollLoop();

    void add(int fd, uint32_t events, void* user_data);
    void modify(int fd, uint32_t events, void* user_data);
    void remove(int fd);

    int wait(int timeout_ms);

    struct Event {
        int fd;
        uint32_t events;
        void* user_data;
    };

    const std::vector<Event>& events() const;

private:
    int epoll_fd_;
    std::vector<epoll_event> raw_events_;
    std::vector<Event> events_;
};
