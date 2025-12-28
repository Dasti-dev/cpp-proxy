#include "epoll_loop.h"

#include <unistd.h>
#include <stdexcept>

EpollLoop::EpollLoop()
    : epoll_fd_(::epoll_create1(0)),
      raw_events_(1024) {
    if (epoll_fd_ < 0)
        throw std::runtime_error("epoll_create1 failed");
}

EpollLoop::~EpollLoop() {
    ::close(epoll_fd_);
}

void EpollLoop::add(int fd, uint32_t events, void* user_data) {
    epoll_event ev{};
    ev.events = events;
    ev.data.ptr = user_data;   // ✅ ONLY ptr

    ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
}

void EpollLoop::modify(int fd, uint32_t events, void* user_data) {
    epoll_event ev{};
    ev.events = events;
    ev.data.ptr = user_data;

    ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
}

void EpollLoop::remove(int fd) {
    ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
}

int EpollLoop::wait(int timeout_ms) {
    int n = ::epoll_wait(
        epoll_fd_,
        raw_events_.data(),
        raw_events_.size(),
        timeout_ms
    );

    events_.clear();
    for (int i = 0; i < n; ++i) {
        events_.push_back({
            /* fd */ -1,                    // ❌ DO NOT USE
            raw_events_[i].events,
            raw_events_[i].data.ptr         // ✅ Connection*
        });
    }

    return n;
}

const std::vector<EpollLoop::Event>& EpollLoop::events() const {
    return events_;
}
