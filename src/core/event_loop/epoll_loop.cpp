#include "epoll_loop.h"

#include <sys/epoll.h>
#include <unistd.h>

EpollLoop::EpollLoop()
    : epoll_fd_(::epoll_create1(0)) {}

EpollLoop::~EpollLoop() {
    if (epoll_fd_ >= 0) {
        ::close(epoll_fd_);
    }
}

bool EpollLoop::add(int fd, uint32_t events, void* user_data) {
    epoll_event ev{};
    ev.events = events;
    ev.data.ptr = user_data;

    return (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == 0);
}

bool EpollLoop::modify(int fd, uint32_t events, void* user_data) {
    epoll_event ev{};
    ev.events = events;
    ev.data.ptr = user_data;

    return (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == 0);
}

bool EpollLoop::remove(int fd) {
    return (::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == 0);
}

int EpollLoop::wait(int timeout_ms) {
    ready_events_.clear();

    constexpr int MAX_EVENTS = 1024;
    epoll_event events[MAX_EVENTS];

    int n = ::epoll_wait(epoll_fd_, events, MAX_EVENTS, timeout_ms);
    if (n <= 0) {
        return n;
    }

    ready_events_.reserve(n);
    for (int i = 0; i < n; ++i) {
        Event ev{};
        ev.fd = events[i].data.fd;          // not used, but kept for clarity
        ev.events = events[i].events;
        ev.user_data = events[i].data.ptr;
        ready_events_.push_back(ev);
    }

    return n;
}

const std::vector<EpollLoop::Event>& EpollLoop::events() const {
    return ready_events_;
}
