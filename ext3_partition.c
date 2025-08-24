#include "ext3_partition.h"
#include "io.h"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

ext3_partition create_ext3_partition(char *path) {
	ext3_partition ext3_part;
	ext3_part.fd = open(path, O_RDONLY);

	if (ext3_part.fd < 0) {
		printf("File open error in create_ext3_partition.\n");

		return ext3_part;
	}

	ext3_part.num_blocks = get_num_blocks_in_partition(ext3_part.fd);
	ext3_part.block_size = get_block_size(ext3_part.fd);

	// Populate these separately so the user has the option of quickly recovering small files without them.
	ext3_part.indirect_blocks = create_queue();
	ext3_part.first_indirect_blocks = create_queue();
	ext3_part.second_indirect_blocks = create_queue();
	ext3_part.third_indirect_blocks = create_queue();

	return ext3_part;
}

void get_indirect_blocks(ext3_partition &ext3_part) {
	printf("Searching for indirect blocks.\n");

	for (unsigned int i = 0; i < PROG_BAR_LEN; i++) {
		printf("_");
	}

	printf("\n");
	unsigned int blocks_per_prog_update = ext3_part.num_blocks / PROG_BAR_LEN;

	for (unsigned int i = 0; i < ext3_part.num_blocks; i++) {
		if (is_indirect_block(ext3_part.fd, i, ext3_part.block_size)) {
			enqueue(i, ext3_part.indirect_blocks);
		}

		if ((i + 1) % blocks_per_prog_update == 0) {
			printf(">");
			fflush(stdout);
		}
	}

	printf("\n");
}

void get_first_indirect_blocks(ext3_partition &ext3_part) {
	for (node *curr = ext3_part.indirect_blocks.head; curr; curr = curr->next) {
		if (is_first_indirect_block(ext3_part.fd, curr->value, ext3_part.block_size, ext3_part.indirect_blocks)) {
			enqueue(curr->value, ext3_part.first_indirect_blocks);
		}
	}
}

void get_second_indirect_blocks(ext3_partition &ext3_part) {
	for (node *curr = ext3_part.indirect_blocks.head; curr; curr = curr->next) {
		if (is_second_indirect_block(ext3_part.fd, curr->value, ext3_part.block_size, ext3_part.indirect_blocks)) {
			enqueue(curr->value, ext3_part.second_indirect_blocks);
		}
	}
}

void get_third_indirect_blocks(ext3_partition &ext3_part) {
	for (node *curr = ext3_part.indirect_blocks.head; curr; curr = curr->next) {
		if (is_third_indirect_block(ext3_part.fd, curr->value, ext3_part.block_size, ext3_part.indirect_blocks)) {
			enqueue(curr->value, ext3_part.third_indirect_blocks);
		}
	}
}

unsigned int get_longest_indirect_block_num(queue &indirect_blocks, ext3_partition &ext3_part) {
	unsigned int longest_indirect_block_num = 0;
	unsigned int longest_indirect_block_len = 0;
	unsigned int curr_indirect_block_len;

	for (node *curr = indirect_blocks.head; curr; curr = curr->next) {
		curr_indirect_block_len = get_indirect_block_len(ext3_part.fd, curr->value, ext3_part.block_size);

		if (curr_indirect_block_len > longest_indirect_block_len) {
			longest_indirect_block_num = curr->value;
			longest_indirect_block_len = curr_indirect_block_len;
		}
	}

	return longest_indirect_block_num;
}

void delete_ext3_partition(ext3_partition &ext3_part) {
	int close_return = close(ext3_part.fd);

	if (close_return < 0) {
		printf("File close error in delete_ext3_partition.\n");
	}

	delete_queue(ext3_part.indirect_blocks);
	delete_queue(ext3_part.first_indirect_blocks);
	delete_queue(ext3_part.second_indirect_blocks);
	delete_queue(ext3_part.third_indirect_blocks);
}

unsigned int get_num_blocks_in_partition(int fd) {
	off64_t offset = (off64_t) SB0_OFFSET + (off64_t) NUM_BLOCKS_IN_PARTITION_REL_OFFSET;	// Typecast needed?
	off64_t pos = lseek64(fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in get_num_blocks_in_partition.\n");

		return 0;
	}

	unsigned int num_blocks_in_partition = read_u32_int(fd);

	return num_blocks_in_partition;
}

unsigned int get_block_size(int fd) {
	unsigned int block_size_exp = get_block_size_exp(fd);
	unsigned int block_size = 1;

	for (unsigned int i = 0; i < block_size_exp; i++) {
		block_size *= BLOCK_SIZE_BASE;
	}

	block_size *= BLOCK_SIZE_UNIT;

	return block_size;
}

unsigned int get_block_size_exp(int fd) {
	off64_t offset = (off64_t) SB0_OFFSET + (off64_t) BLOCK_SIZE_EXP_REL_OFFSET;	// Typecast needed?
	off64_t pos = lseek64(fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in get_block_size_exp.\n");

		return 0;
	}

	unsigned int block_size_exp = read_u32_int(fd);

	return block_size_exp;
}

bool is_indirect_block(int fd, unsigned int block_num, unsigned int block_size) {
	off64_t offset = (off64_t) block_num * (off64_t) block_size;	// Typecast needed?
	off64_t pos = lseek64(fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in is_indirect_block.\n");

		return false;
	}

	char block[block_size];
	ssize_t bytes_read = read(fd, block, block_size);

	if (bytes_read < block_size) {
		printf("File read error in is_indirect_block.\n");

		return false;
	}

	unsigned int prev = read_u32_int_from_buffer(&block[0]);

	if (prev == 0) {
		return false;
	}

	unsigned int curr;
	bool zero_encountered = false;
	unsigned int num_jumps = 0;

	for (unsigned int i = BLOCK_ADDR_LEN; i < block_size; i += BLOCK_ADDR_LEN) {
		curr = read_u32_int_from_buffer(&block[i]);

		if (curr == 0) {
			zero_encountered = true;
		} else {
			if (zero_encountered) {
				return false;
			}

			if (curr != prev + 1) {
				num_jumps++;

				if (num_jumps > MAX_JUMPS) {
					return false;
				}
			}
		}

		prev = curr;
	}

	return true;
}

bool is_first_indirect_block(int fd, unsigned int indirect_block_num, unsigned int block_size, queue &indirect_blocks) {
	if (find(indirect_block_num, indirect_blocks) < 0) {
		return false;
	}

	off64_t offset = (off64_t) indirect_block_num * (off64_t) block_size;	// Typecast needed?
	off64_t pos = lseek64(fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in is_first_indirect_block.\n");

		return false;
	}

	unsigned int first_block_addr = read_u32_int(fd);

	return find(first_block_addr, indirect_blocks) < 0;
}

bool is_second_indirect_block(int fd, unsigned int indirect_block_num, unsigned int block_size, queue &indirect_blocks)
{
	if (find(indirect_block_num, indirect_blocks) < 0) {
		return false;
	}

	off64_t offset = (off64_t) indirect_block_num * (off64_t) block_size;	// Typecast needed?
	off64_t pos = lseek64(fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in is_second_indirect_block.\n");

		return false;
	}

	unsigned int first_block_addr = read_u32_int(fd);

	return is_first_indirect_block(fd, first_block_addr, block_size, indirect_blocks);
}

bool is_third_indirect_block(int fd, unsigned int indirect_block_num, unsigned int block_size, queue &indirect_blocks) {
	if (find(indirect_block_num, indirect_blocks) < 0) {
		return false;
	}

	off64_t offset = (off64_t) indirect_block_num * (off64_t) block_size;	// Typecast needed?
	off64_t pos = lseek64(fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in is_third_indirect_block.\n");

		return false;
	}

	unsigned int first_block_addr = read_u32_int(fd);

	return is_second_indirect_block(fd, first_block_addr, block_size, indirect_blocks);
}

unsigned int get_indirect_block_len(int fd, unsigned int indirect_block_num, unsigned int block_size) {
	off64_t offset = (off64_t) indirect_block_num * (off64_t) block_size;	// Typecast needed?
	off64_t pos = lseek64(fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in get_indirect_block_len.\n");

		return 0;
	}

	char block[block_size];
	ssize_t bytes_read = read(fd, block, block_size);

	if (bytes_read < block_size) {
		printf("File read error in get_indirect_block_len.\n");

		return 0;
	}

	unsigned int block_len = 0;

	for (unsigned int i = 0; i < block_size; i += BLOCK_ADDR_LEN) {
		if (read_u32_int_from_buffer(&block[i]) == 0) {
			break;
		}

		block_len++;
	}

	return block_len;
}
