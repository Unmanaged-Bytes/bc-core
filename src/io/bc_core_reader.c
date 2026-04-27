// SPDX-License-Identifier: MIT

#include "bc_core_io.h"
#include "bc_core_memory.h"

#include <errno.h>
#include <unistd.h>

bool bc_core_reader_init(bc_core_reader_t* reader, int fd, char* buffer, size_t capacity)
{
    if (fd < 0 || buffer == NULL || capacity == 0) {
        return false;
    }
    reader->fd = fd;
    reader->eof_latched = 0;
    reader->error_latched = 0;
    reader->buffer = buffer;
    reader->capacity = capacity;
    reader->read_position = 0;
    reader->fill_position = 0;
    return true;
}

bool bc_core_reader_destroy(bc_core_reader_t* reader)
{
    reader->fd = -1;
    reader->buffer = NULL;
    reader->capacity = 0;
    reader->read_position = 0;
    reader->fill_position = 0;
    return true;
}

bool bc_core_reader_has_error(const bc_core_reader_t* reader)
{
    return reader->error_latched != 0;
}

bool bc_core_reader_is_eof(const bc_core_reader_t* reader)
{
    return reader->eof_latched != 0 && reader->read_position == reader->fill_position;
}

static bool reader_refill(bc_core_reader_t* reader)
{
    if (reader->read_position == reader->fill_position) {
        reader->read_position = 0;
        reader->fill_position = 0;
    } else if (reader->read_position > 0) {
        size_t buffered = reader->fill_position - reader->read_position;
        if (!bc_core_copy(reader->buffer, reader->buffer + reader->read_position, buffered)) {
            reader->error_latched = 1;
            return false;
        }
        reader->read_position = 0;
        reader->fill_position = buffered;
    }

    if (reader->eof_latched) {
        return true;
    }

    if (reader->fill_position == reader->capacity) {
        return true;
    }

    while (true) {
        ssize_t received = read(reader->fd, reader->buffer + reader->fill_position, reader->capacity - reader->fill_position);
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            reader->error_latched = 1;
            return false;
        }
        if (received == 0) {
            reader->eof_latched = 1;
            return true;
        }
        reader->fill_position += (size_t)received;
        return true;
    }
}

bool bc_core_reader_read(bc_core_reader_t* reader, void* out, size_t max_len, size_t* out_read)
{
    if (reader->error_latched) {
        return false;
    }
    if (max_len == 0) {
        *out_read = 0;
        return true;
    }

    if (reader->read_position == reader->fill_position) {
        if (!reader_refill(reader)) {
            return false;
        }
        if (reader->read_position == reader->fill_position) {
            *out_read = 0;
            return true;
        }
    }

    size_t available = reader->fill_position - reader->read_position;
    size_t to_copy = max_len < available ? max_len : available;

    if (!bc_core_copy(out, reader->buffer + reader->read_position, to_copy)) {
        reader->error_latched = 1;
        return false;
    }
    reader->read_position += to_copy;
    *out_read = to_copy;
    return true;
}

bool bc_core_reader_read_exact(bc_core_reader_t* reader, void* out, size_t len)
{
    if (reader->error_latched) {
        return false;
    }

    unsigned char* destination = (unsigned char*)out;
    size_t remaining = len;

    while (remaining > 0) {
        size_t read_bytes = 0;
        if (!bc_core_reader_read(reader, destination, remaining, &read_bytes)) {
            return false;
        }
        if (read_bytes == 0) {
            return false;
        }
        destination += read_bytes;
        remaining -= read_bytes;
    }
    return true;
}

bool bc_core_reader_read_line(bc_core_reader_t* reader, const char** out_line, size_t* out_length)
{
    if (reader->error_latched) {
        return false;
    }

    while (true) {
        size_t available = reader->fill_position - reader->read_position;
        if (available > 0) {
            size_t newline_offset = 0;
            if (bc_core_find_byte(reader->buffer + reader->read_position, available, '\n', &newline_offset)) {
                *out_line = reader->buffer + reader->read_position;
                *out_length = newline_offset;
                reader->read_position += newline_offset + 1U;
                return true;
            }
        }

        if (reader->eof_latched) {
            if (available == 0) {
                return false;
            }
            *out_line = reader->buffer + reader->read_position;
            *out_length = available;
            reader->read_position = reader->fill_position;
            return true;
        }

        if (reader->read_position == 0 && reader->fill_position == reader->capacity) {
            reader->error_latched = 1;
            return false;
        }

        if (!reader_refill(reader)) {
            return false;
        }
    }
}
