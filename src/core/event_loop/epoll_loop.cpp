#include "epoll_loop.h"
#include <unistd.h>
#include <stdexcept>

EpollLoop::EpollLoop()
    : epoll_fd_(::epoll_create1(0)),
      events_(128),
      ready_(0) {

    if (epoll_fd_ < 0) {
        throw std::runtime_error("epoll_create1 failed");
    }
}

EpollLoop::~EpollLoop() {
    ::close(epoll_fd_);
}

void EpollLoop::add(int fd, uint32_t events, void* data) {
    epoll_event ev{};
    ev.events = events;
    ev.data.ptr = data;

    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        throw std::runtime_error("epoll_ctl ADD failed");
    }
}

void EpollLoop::modify(int fd, uint32_t events, void* data) {
    epoll_event ev{};
    ev.events = events;
    ev.data.ptr = data;

    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        throw std::runtime_error("epoll_ctl MOD failed");
    }
}

void EpollLoop::remove(int fd) {
    ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
}

int EpollLoop::wait(int timeout_ms) {
    ready_ = ::epoll_wait(
        epoll_fd_,
        events_.data(),
        static_cast<int>(events_.size()),
        timeout_ms
    );
    return ready_;
}

const epoll_event& EpollLoop::event_at(int i) const {
    return events_[i];
}

int EpollLoop::ready_count() const {
    return ready_;
}
