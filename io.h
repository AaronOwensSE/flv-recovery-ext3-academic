#ifndef IO_H
#define IO_H

// A set of functions for compound read/write operations on file descriptors and buffers.

#define U32_INT_SIZE 4
#define U24_INT_SIZE 3

unsigned int read_u32_int(int fd);
unsigned int read_u32_int_be(int fd);
unsigned int read_u32_int_from_buffer(char *buffer);
unsigned int read_u24_int_be_as_u32_int(int fd);

#endif
