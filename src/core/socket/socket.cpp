#include "socket.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

int Socket::create_tcp() {
    return ::socket(AF_INET, SOCK_STREAM, 0);
}

bool Socket::set_nonblocking(int fd) {
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }
    return (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0);
}

ssize_t Socket::read(int fd, void* buf, size_t len) {
    return ::read(fd, buf, len);
}

ssize_t Socket::write(int fd, const void* buf, size_t len) {
    return ::write(fd, buf, len);
}
