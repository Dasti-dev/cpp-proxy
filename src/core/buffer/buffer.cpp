#include "buffer.h"
#include <algorithm>

Buffer::Buffer(size_t initial_capacity)
    : data_(initial_capacity),
      read_offset_(0),
      write_offset_(0) {}

Buffer::Buffer(Buffer&& other) noexcept
    : data_(std::move(other.data_)),
      read_offset_(other.read_offset_),
      write_offset_(other.write_offset_) {
    other.read_offset_ = 0;
    other.write_offset_ = 0;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        data_ = std::move(other.data_);
        read_offset_ = other.read_offset_;
        write_offset_ = other.write_offset_;

        other.read_offset_ = 0;
        other.write_offset_ = 0;
    }
    return *this;
}

char* Buffer::write_ptr() {
    return data_.data() + write_offset_;
}

size_t Buffer::writable_bytes() const {
    return data_.size() - write_offset_;
}

const char* Buffer::read_ptr() const {
    return data_.data() + read_offset_;
}

size_t Buffer::readable_bytes() const {
    return write_offset_ - read_offset_;
}

void Buffer::commit(size_t bytes) {
    write_offset_ += bytes;
}

void Buffer::consume(size_t bytes) {
    read_offset_ += bytes;

    // Compact if buffer is empty
    if (read_offset_ == write_offset_) {
        read_offset_ = 0;
        write_offset_ = 0;
    }
}

void Buffer::clear() {
    read_offset_ = 0;
    write_offset_ = 0;
}

void Buffer::ensure_capacity(size_t additional) {
    if (writable_bytes() >= additional) {
        return;
    }

    // First try compaction
    if (read_offset_ > 0) {
        size_t readable = readable_bytes();
        std::move(data_.begin() + read_offset_,
                  data_.begin() + write_offset_,
                  data_.begin());
        read_offset_ = 0;
        write_offset_ = readable;
    }

    // Grow if still insufficient
    if (writable_bytes() < additional) {
        data_.resize(write_offset_ + additional);
    }
}
