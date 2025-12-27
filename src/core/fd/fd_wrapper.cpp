#include "fd_wrapper.h"
#include <unistd.h>

FDWrapper::FDWrapper() noexcept
    : fd_(-1) {}

FDWrapper::FDWrapper(int fd) noexcept
    : fd_(fd) {}

FDWrapper::FDWrapper(FDWrapper&& other) noexcept
    : fd_(other.fd_) {
    other.fd_ = -1;
}

FDWrapper& FDWrapper::operator=(FDWrapper&& other) noexcept {
    if (this != &other) {
        reset();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

FDWrapper::~FDWrapper() {
    reset();
}

int FDWrapper::get() const noexcept {
    return fd_;
}

bool FDWrapper::valid() const noexcept {
    return fd_ >= 0;
}

int FDWrapper::release() noexcept {
    int old_fd = fd_;
    fd_ = -1;
    return old_fd;
}

void FDWrapper::reset(int new_fd) noexcept {
    if (fd_ >= 0) {
        ::close(fd_);
    }
    fd_ = new_fd;
}
