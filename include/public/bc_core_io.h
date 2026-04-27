// SPDX-License-Identifier: MIT

#ifndef BC_CORE_IO_H
#define BC_CORE_IO_H

#include "bc_core_error.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct bc_core_writer {
    int fd;
    int error_latched;
    char* buffer;
    size_t capacity;
    size_t position;
} bc_core_writer_t;

bool bc_core_writer_init(bc_core_writer_t* writer, int fd, char* buffer, size_t capacity);
bool bc_core_writer_init_standard_error(bc_core_writer_t* writer, char* buffer, size_t capacity);
bool bc_core_writer_init_standard_output(bc_core_writer_t* writer, char* buffer, size_t capacity);
bool bc_core_writer_init_buffer_only(bc_core_writer_t* writer, char* buffer, size_t capacity);
bool bc_core_writer_flush(bc_core_writer_t* writer);
bool bc_core_writer_destroy(bc_core_writer_t* writer);
bool bc_core_writer_has_error(const bc_core_writer_t* writer);
bool bc_core_writer_buffer_data(const bc_core_writer_t* writer, const char** out_data, size_t* out_length);

bool bc_core_writer_write_bytes(bc_core_writer_t* writer, const void* data, size_t len);
bool bc_core_writer_write_char(bc_core_writer_t* writer, char value);
bool bc_core_writer_write_error_description(bc_core_writer_t* writer, bc_core_error_code_t code);

#define BC_CORE_WRITER_PUTS(writer, literal) \
    bc_core_writer_write_bytes((writer), (literal), sizeof(literal) - 1U)

typedef struct bc_core_reader {
    int fd;
    int eof_latched;
    int error_latched;
    char* buffer;
    size_t capacity;
    size_t read_position;
    size_t fill_position;
} bc_core_reader_t;

bool bc_core_reader_init(bc_core_reader_t* reader, int fd, char* buffer, size_t capacity);
bool bc_core_reader_destroy(bc_core_reader_t* reader);
bool bc_core_reader_has_error(const bc_core_reader_t* reader);
bool bc_core_reader_is_eof(const bc_core_reader_t* reader);
bool bc_core_reader_read(bc_core_reader_t* reader, void* out, size_t max_len, size_t* out_read);
bool bc_core_reader_read_exact(bc_core_reader_t* reader, void* out, size_t len);
bool bc_core_reader_read_line(bc_core_reader_t* reader, const char** out_line, size_t* out_length);

#endif /* BC_CORE_IO_H */
