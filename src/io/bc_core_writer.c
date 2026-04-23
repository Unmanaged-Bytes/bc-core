// SPDX-License-Identifier: MIT

#include "bc_core_io.h"
#include "bc_core_memory.h"

#include <errno.h>
#include <unistd.h>

static bool raw_write_all(int fd, const void* data, size_t len)
{
    const unsigned char* remaining = (const unsigned char*)data;
    size_t left = len;
    while (left > 0) {
        ssize_t written = write(fd, remaining, left);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (written == 0) {
            return false;
        }
        remaining += (size_t)written;
        left -= (size_t)written;
    }
    return true;
}

bool bc_core_writer_init(bc_core_writer_t* writer, int fd, char* buffer, size_t capacity)
{
    if (fd < 0 || buffer == NULL || capacity == 0) {
        return false;
    }
    writer->fd = fd;
    writer->error_latched = 0;
    writer->buffer = buffer;
    writer->capacity = capacity;
    writer->position = 0;
    return true;
}

bool bc_core_writer_has_error(const bc_core_writer_t* writer)
{
    return writer->error_latched != 0;
}

bool bc_core_writer_flush(bc_core_writer_t* writer)
{
    if (writer->error_latched) {
        return false;
    }
    if (writer->position == 0) {
        return true;
    }
    if (!raw_write_all(writer->fd, writer->buffer, writer->position)) {
        writer->error_latched = 1;
        return false;
    }
    writer->position = 0;
    return true;
}

bool bc_core_writer_destroy(bc_core_writer_t* writer)
{
    bool ok = bc_core_writer_flush(writer);
    writer->fd = -1;
    writer->buffer = NULL;
    writer->capacity = 0;
    writer->position = 0;
    return ok;
}

bool bc_core_writer_write_bytes(bc_core_writer_t* writer, const void* data, size_t len)
{
    if (writer->error_latched) {
        return false;
    }
    if (len == 0) {
        return true;
    }

    size_t remaining = writer->capacity - writer->position;
    if (len <= remaining) {
        if (!bc_core_copy(writer->buffer + writer->position, data, len)) {
            writer->error_latched = 1;
            return false;
        }
        writer->position += len;
        return true;
    }

    if (!bc_core_writer_flush(writer)) {
        return false;
    }

    if (len < writer->capacity) {
        if (!bc_core_copy(writer->buffer, data, len)) {
            writer->error_latched = 1;
            return false;
        }
        writer->position = len;
        return true;
    }

    if (!raw_write_all(writer->fd, data, len)) {
        writer->error_latched = 1;
        return false;
    }
    return true;
}

bool bc_core_writer_write_char(bc_core_writer_t* writer, char value)
{
    if (writer->error_latched) {
        return false;
    }
    if (writer->position == writer->capacity) {
        if (!bc_core_writer_flush(writer)) {
            return false;
        }
    }
    writer->buffer[writer->position++] = value;
    return true;
}

bool bc_core_writer_write_uint64_dec(bc_core_writer_t* writer, uint64_t value)
{
    char scratch[21];
    size_t length = 0;
    if (!bc_core_fmt_uint64_dec(scratch, sizeof(scratch), value, &length)) {
        writer->error_latched = 1;
        return false;
    }
    return bc_core_writer_write_bytes(writer, scratch, length);
}

bool bc_core_writer_write_uint64_hex(bc_core_writer_t* writer, uint64_t value)
{
    char scratch[16];
    size_t length = 0;
    if (!bc_core_fmt_uint64_hex(scratch, sizeof(scratch), value, &length)) {
        writer->error_latched = 1;
        return false;
    }
    return bc_core_writer_write_bytes(writer, scratch, length);
}

bool bc_core_writer_write_uint64_hex_padded(bc_core_writer_t* writer, uint64_t value, size_t digits)
{
    char scratch[16];
    size_t length = 0;
    if (!bc_core_fmt_uint64_hex_padded(scratch, sizeof(scratch), value, digits, &length)) {
        writer->error_latched = 1;
        return false;
    }
    return bc_core_writer_write_bytes(writer, scratch, length);
}

bool bc_core_writer_write_int64(bc_core_writer_t* writer, int64_t value)
{
    char scratch[22];
    size_t length = 0;
    if (!bc_core_fmt_int64(scratch, sizeof(scratch), value, &length)) {
        writer->error_latched = 1;
        return false;
    }
    return bc_core_writer_write_bytes(writer, scratch, length);
}

bool bc_core_writer_write_double(bc_core_writer_t* writer, double value, int frac_digits)
{
    char scratch[64];
    size_t length = 0;
    if (!bc_core_fmt_double(scratch, sizeof(scratch), value, frac_digits, &length)) {
        writer->error_latched = 1;
        return false;
    }
    return bc_core_writer_write_bytes(writer, scratch, length);
}

bool bc_core_writer_write_bytes_human(bc_core_writer_t* writer, uint64_t bytes)
{
    char scratch[32];
    size_t length = 0;
    if (!bc_core_fmt_bytes_human(scratch, sizeof(scratch), bytes, &length)) {
        writer->error_latched = 1;
        return false;
    }
    return bc_core_writer_write_bytes(writer, scratch, length);
}

bool bc_core_writer_write_duration_ns(bc_core_writer_t* writer, uint64_t nanoseconds)
{
    char scratch[32];
    size_t length = 0;
    if (!bc_core_fmt_duration_ns(scratch, sizeof(scratch), nanoseconds, &length)) {
        writer->error_latched = 1;
        return false;
    }
    return bc_core_writer_write_bytes(writer, scratch, length);
}
