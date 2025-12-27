#pragma once

#include <cstddef>
#include <sys/types.h>

/*
 * Socket
 * ------
 * Thin wrapper over a non-blocking socket fd.
 *
 * Responsibilities:
 * - Create sockets
 * - Set non-blocking mode
 * - Read / write wrappers
 * - Normalize syscall results
 *
 * Non-responsibilities:
 * - epoll
 * - accept loop
 * - buffers ownership
 * - protocol logic
 */

class Socket {
public:
    // Create TCP socket (AF_INET)
    static int create_tcp();

    // Set O_NONBLOCK on fd
    static bool set_nonblocking(int fd);

    // Read wrapper
    // Returns:
    //  >0 : bytes read
    //   0 : peer closed
    //  -1 : error (check errno outside)
    static ssize_t read(int fd, void* buf, size_t len);

    // Write wrapper
    // Returns:
    //  >0 : bytes written
    //  -1 : error (check errno outside)
    static ssize_t write(int fd, const void* buf, size_t len);
};
