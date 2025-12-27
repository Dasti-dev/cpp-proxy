#pragma once

#include <cstddef>
#include <vector>

/*
 * Buffer
 * ------
 * Linear byte buffer with explicit read/write offsets.
 *
 * Core rules:
 * - No assumptions about full reads or writes
 * - Tracks consumed vs available bytes
 * - Never blocks
 * - No syscalls
 *
 * Typical usage:
 * - Read data into writable region
 * - Consume bytes after processing
 * - Write from readable region
 */
class Buffer {
public:
    explicit Buffer(size_t initial_capacity = 4096);

    // Disable copy (buffers are stateful and large)
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // Movable
    Buffer(Buffer&&) noexcept;
    Buffer& operator=(Buffer&&) noexcept;

    // Pointer to writable region
    char* write_ptr();
    size_t writable_bytes() const;

    // Pointer to readable region
    const char* read_ptr() const;
    size_t readable_bytes() const;

    // Commit bytes written into buffer
    void commit(size_t bytes);

    // Consume bytes from buffer
    void consume(size_t bytes);

    // Clear buffer completely
    void clear();

private:
    void ensure_capacity(size_t additional);

    std::vector<char> data_;
    size_t read_offset_;
    size_t write_offset_;
};
