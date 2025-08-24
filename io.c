#include "io.h"
#include <arpa/inet.h>
#include <cstdio>
#include <unistd.h>

unsigned int read_u32_int(int fd) {
	char buffer[U32_INT_SIZE];
	ssize_t bytes_read = read(fd, buffer, U32_INT_SIZE);

	if (bytes_read < U32_INT_SIZE) {
		printf("File read error in read_u32_int.\n");

		return 0;
	}

	return *((unsigned int *) buffer);
}

unsigned int read_u32_int_be(int fd) {
	char buffer[U32_INT_SIZE];
	ssize_t bytes_read = read(fd, buffer, U32_INT_SIZE);

	if (bytes_read < U32_INT_SIZE) {
		printf("File read error in read_u32_int_be.\n");

		return 0;
	}

	unsigned int u32_int_be = *((unsigned int *) buffer);
	unsigned int u32_int_le = ntohl(u32_int_be);

	return u32_int_le;
}

unsigned int read_u32_int_from_buffer(char *buffer) {
	return *((unsigned int *) buffer);
}

unsigned int read_u24_int_be_as_u32_int(int fd) {
	char buffer_32[U32_INT_SIZE];

	for (unsigned int i = 0; i < U32_INT_SIZE; i++) {
		buffer_32[i] = 0;
	}

	char buffer_24[U24_INT_SIZE];
	ssize_t bytes_read = read(fd, buffer_24, U24_INT_SIZE);

	if (bytes_read < U24_INT_SIZE) {
		printf("File read error in read_u24_int_be_as_u32_int.\n");

		return 0;
	}

	for (unsigned int i = 0, j = U24_INT_SIZE - 1; i < U24_INT_SIZE; i++, j--) {
		buffer_32[i] = buffer_24[j];
	}

	return *((unsigned int *) buffer_32);
}
