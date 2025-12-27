#pragma once

/*
 * FDWrapper
 * ----------
 * Sole owner of a file descriptor.
 *
 * Core rules:
 * - Exactly one owner for an fd
 * - RAII-based close()
 * - Non-copyable
 * - Movable
 * - No side effects (no logging, no exit, no exceptions)
 */

class FDWrapper {
public:
    // Construct with no ownership
    FDWrapper() noexcept;

    // Take ownership of an existing fd
    explicit FDWrapper(int fd) noexcept;

    // Disable copy
    FDWrapper(const FDWrapper&) = delete;
    FDWrapper& operator=(const FDWrapper&) = delete;

    // Enable move
    FDWrapper(FDWrapper&& other) noexcept;
    FDWrapper& operator=(FDWrapper&& other) noexcept;

    // Destructor releases ownership
    ~FDWrapper();

    // Return underlying fd (does not transfer ownership)
    int get() const noexcept;

    // Check if fd is valid
    bool valid() const noexcept;

    // Release ownership without closing
    int release() noexcept;

    // Replace owned fd (close old one if needed)
    void reset(int new_fd = -1) noexcept;

private:
    int fd_;
};
