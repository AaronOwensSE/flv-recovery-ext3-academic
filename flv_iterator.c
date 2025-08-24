#include "flv_iterator.h"
#include "io.h"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

flv_iterator create_flv_iterator(ext3_partition &ext3_part, const char *out_file_path, unsigned int first_block_num) {
	flv_iterator flv_it;
	flv_it.ext3_part = ext3_part;
	flv_it.out_fd = open(out_file_path, O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (flv_it.out_fd < 0) {
		printf("File open error in create_flv_iterator.\n");

		return flv_it;
	}

	flv_it.block_num_queue = get_direct_blocks(first_block_num);
	flv_it.block_num = 0;
	flv_it.tag_offset = FIRST_TAG_OFFSET;
	flv_it.eof_offset = 0;
	flv_it.eof = false;
	
	return flv_it;
}

void recover_flv(flv_iterator &flv_it) {
	recover_from_queue(flv_it);

	if (flv_it.eof) {
		truncate_file(flv_it);
		printf("FLV recovered from direct blocks.\n");

		return;
	}

	bool more_blocks = get_blocks_from_first_indirection(flv_it);

	if (!more_blocks) {
		truncate_file(flv_it);
		printf("FLV recovered from direct blocks after checking for first indirect block.\n");

		return;
	}
	
	recover_from_queue(flv_it);

	if (flv_it.eof) {
		truncate_file(flv_it);
		printf("FLV recovered from first indirect block.\n");

		return;
	}
	
	more_blocks = get_blocks_from_second_indirection(flv_it);

	if (!more_blocks) {
		truncate_file(flv_it);
		printf("FLV recovered from first indirect block after checking for second indirect block.\n");

		return;
	}

	recover_from_queue(flv_it);

	if (flv_it.eof) {
		truncate_file(flv_it);
		printf("FLV recovered from second indirect block.\n");

		return;
	}

	more_blocks = get_blocks_from_third_indirection(flv_it);

	if (!more_blocks) {
		truncate_file(flv_it);
		printf("FLV recovered from second indirect block after checking for third indirect block.\n");

		return;
	}

	recover_from_queue(flv_it);
	truncate_file(flv_it);
	printf("FLV recovered from third indirect block.\n");
}

void delete_flv_iterator(flv_iterator &flv_it) {
	// We did not create ext3_part, so we will not delete it. It is being used externally.
	int close_return = close(flv_it.out_fd);

	if (close_return < 0) {
		printf("File close error in delete_flv_iterator.\n");
	}

	delete_queue(flv_it.block_num_queue);
}

queue get_direct_blocks(unsigned int first_block_num) {
	queue direct_blocks = create_queue();

	for (unsigned int i = 0; i < NUM_DIRECT_BLOCKS; i++) {
		enqueue(first_block_num + i, direct_blocks);
	}

	return direct_blocks;
}

void recover_from_queue(flv_iterator &flv_it) {
	while (!flv_it.eof && flv_it.block_num_queue.len > 0) {
		copy_block(flv_it);

		while (advance_tag_offset(flv_it));
	}
}

bool copy_block(flv_iterator &flv_it) {
	if (flv_it.block_num_queue.len <= 0) {
		return false;
	}
	
	flv_it.block_num = dequeue(flv_it.block_num_queue);
	off64_t offset = (off64_t) flv_it.block_num * (off64_t) flv_it.ext3_part.block_size;	// Typecast needed?
	off64_t pos = lseek64(flv_it.ext3_part.fd, offset, SEEK_SET);
	
	if (pos < 0) {
		printf("File seek error in copy_block.\n");
		
		return false;
	}
	
	char buffer[flv_it.ext3_part.block_size];
	ssize_t bytes_read = read(flv_it.ext3_part.fd, buffer, flv_it.ext3_part.block_size);
	
	if (bytes_read < flv_it.ext3_part.block_size) {
		printf("File read error in copy_block.\n");
		
		return false;
	}
	
	ssize_t bytes_written = write(flv_it.out_fd, buffer, flv_it.ext3_part.block_size);
	
	if (bytes_written < flv_it.ext3_part.block_size) {
		printf("File write error in copy_block.\n");
		
		return false;
	}
	
	flv_it.eof_offset += (off64_t) flv_it.ext3_part.block_size;	// Typecast needed?
	
	return true;
}

bool advance_tag_offset(flv_iterator &flv_it) {
	// Typecast needed?
	if (flv_it.tag_offset + (off64_t) PAYLOAD_SIZE_REL_OFFSET + (off64_t) PAYLOAD_SIZE_LEN >= flv_it.eof_offset) {
		return false;
	}
	
	off64_t offset = flv_it.tag_offset + (off64_t) PAYLOAD_SIZE_REL_OFFSET;	// Typecast needed?
	off64_t pos = lseek64(flv_it.out_fd, offset, SEEK_SET);
	
	if (pos < 0) {
		printf("File seek error in advance_tag_offset.\n");
		
		return false;
	}
	
	unsigned int payload_len = read_u24_int_be_as_u32_int(flv_it.out_fd);
	
	// Typecast needed?
	if (flv_it.tag_offset + (off64_t) TAG_HEADER_LEN + (off64_t) payload_len + (off64_t) PREV_TAG_SIZE_LEN
		>= flv_it.eof_offset)
	{
		return false;
	}
	
	offset = flv_it.tag_offset + (off64_t) TAG_HEADER_LEN + (off64_t) payload_len;	// Typecast needed?
	pos = lseek64(flv_it.out_fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in advance_tag_offset.\n");
		
		return false;
	}
	
	unsigned int tag_len = read_u32_int_be(flv_it.out_fd);
	
	if (payload_len + TAG_HEADER_LEN != tag_len) {
		flv_it.eof = true;
		printf("End of file found.\n");
		
		return false;
	}
	
	flv_it.tag_offset += (off64_t) tag_len + (off64_t) PREV_TAG_SIZE_LEN;	// Typecast needed?
	
	return true;
}

void truncate_file(flv_iterator &flv_it) {
	if (flv_it.eof) {
		ftruncate(flv_it.out_fd, flv_it.tag_offset);
	}
}

bool get_blocks_from_first_indirection(flv_iterator &flv_it) {
	get_indirect_blocks(flv_it.ext3_part);
	get_first_indirect_blocks(flv_it.ext3_part);
	queue matching_first_indirect_blocks = create_queue();

	for (node *curr = flv_it.ext3_part.first_indirect_blocks.head; curr; curr = curr->next) {
		if (is_matching_first_indirect_block(curr->value, flv_it)) {
			enqueue(curr->value, matching_first_indirect_blocks);
		}
	}

	if (matching_first_indirect_blocks.len == 0) {
		printf("First indirect block not found.\n");

		return false;
	}

	unsigned int longest_indirect_block_num = get_longest_indirect_block_num(matching_first_indirect_blocks,
		flv_it.ext3_part);
	delete_queue(matching_first_indirect_blocks);
	printf("First indirect block matched.\n");
	bool blocks_enqueued = enqueue_from_first_indirect_block(longest_indirect_block_num, flv_it);

	return blocks_enqueued;
}

bool is_matching_first_indirect_block(unsigned int first_indirect_block_num, flv_iterator &flv_it) {
	off64_t offset = (off64_t) first_indirect_block_num * (off64_t) flv_it.ext3_part.block_size;	// Typecast needed?
	off64_t pos = lseek64(flv_it.ext3_part.fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in is_matching_first_indirect_block.\n");

		return false;
	}

	unsigned int first_block_num = read_u32_int(flv_it.ext3_part.fd);

	return first_block_num == flv_it.block_num + 1;
}

bool enqueue_from_first_indirect_block(unsigned int first_indirect_block_num, flv_iterator &flv_it) {
	off64_t offset = (off64_t) first_indirect_block_num * (off64_t) flv_it.ext3_part.block_size;	// Typecast needed?
	off64_t pos = lseek64(flv_it.ext3_part.fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in enqueue_from_first_indirect_block.\n");

		return false;
	}

	char block[flv_it.ext3_part.block_size];
	ssize_t bytes_read = read(flv_it.ext3_part.fd, block, flv_it.ext3_part.block_size);

	if (bytes_read < flv_it.ext3_part.block_size) {
		printf("File read error in enqueue_from_first_indirect_block.\n");

		return false;
	}

	unsigned int block_num;

	for (unsigned int i = 0; i < flv_it.ext3_part.block_size; i += BLOCK_ADDR_LEN) {
		block_num = read_u32_int_from_buffer(&block[i]);

		if (block_num == 0) {
			break;
		}
		
		enqueue(block_num, flv_it.block_num_queue);
	}

	return true;
}

bool get_blocks_from_second_indirection(flv_iterator &flv_it) {
	get_second_indirect_blocks(flv_it.ext3_part);
	queue matching_second_indirect_blocks = create_queue();

	for (node *curr = flv_it.ext3_part.second_indirect_blocks.head; curr; curr = curr->next) {
		if (is_matching_second_indirect_block(curr->value, flv_it)) {
			enqueue(curr->value, matching_second_indirect_blocks);
		}
	}

	if (matching_second_indirect_blocks.len == 0) {
		printf("Second indirect block not found.\n");

		return false;
	}

	unsigned int longest_indirect_block_num = get_longest_indirect_block_num(matching_second_indirect_blocks,
		flv_it.ext3_part);
	delete_queue(matching_second_indirect_blocks);
	printf("Second indirect block matched.\n");
	bool blocks_enqueued = enqueue_from_second_indirect_block(longest_indirect_block_num, flv_it);

	return blocks_enqueued;
}

bool is_matching_second_indirect_block(unsigned int second_indirect_block_num, flv_iterator &flv_it) {
	off64_t offset = (off64_t) second_indirect_block_num * (off64_t) flv_it.ext3_part.block_size;	// Typecast needed?
	off64_t pos = lseek64(flv_it.ext3_part.fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in is_matching_second_indirect_block.\n");

		return false;
	}

	unsigned int first_block_num = read_u32_int(flv_it.ext3_part.fd);

	return is_matching_first_indirect_block(first_block_num, flv_it);
}

bool enqueue_from_second_indirect_block(unsigned int second_indirect_block_num, flv_iterator &flv_it) {
	off64_t offset = (off64_t) second_indirect_block_num * (off64_t) flv_it.ext3_part.block_size;	// Typecast needed?
	off64_t pos = lseek64(flv_it.ext3_part.fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in enqueue_from_second_indirect_block.\n");

		return false;
	}

	char block[flv_it.ext3_part.block_size];
	ssize_t bytes_read = read(flv_it.ext3_part.fd, block, flv_it.ext3_part.block_size);

	if (bytes_read < flv_it.ext3_part.block_size) {
		printf("File read error in enqueue_from_second_indirect_block.\n");

		return false;
	}

	unsigned int block_num;

	for (unsigned int i = 0; i < flv_it.ext3_part.block_size; i += BLOCK_ADDR_LEN) {
		block_num = read_u32_int_from_buffer(&block[i]);

		if (block_num == 0) {
			break;
		}

		enqueue_from_first_indirect_block(block_num, flv_it);
	}

	return true;
}

bool get_blocks_from_third_indirection(flv_iterator &flv_it) {
	get_third_indirect_blocks(flv_it.ext3_part);
	queue matching_third_indirect_blocks = create_queue();

	for (node *curr = flv_it.ext3_part.third_indirect_blocks.head; curr; curr = curr->next) {
		if (is_matching_third_indirect_block(curr->value, flv_it)) {
			enqueue(curr->value, matching_third_indirect_blocks);
		}
	}

	if (matching_third_indirect_blocks.len == 0) {
		printf("Third indirect block not found.\n");

		return false;
	}

	unsigned int longest_indirect_block_num = get_longest_indirect_block_num(matching_third_indirect_blocks,
		flv_it.ext3_part);
	delete_queue(matching_third_indirect_blocks);
	printf("Third indirect block matched.\n");
	bool blocks_enqueued = enqueue_from_third_indirect_block(longest_indirect_block_num, flv_it);

	return blocks_enqueued;
}

bool is_matching_third_indirect_block(unsigned int third_indirect_block_num, flv_iterator &flv_it) {
	off64_t offset = (off64_t) third_indirect_block_num * (off64_t) flv_it.ext3_part.block_size;	// Typecast needed?
	off64_t pos = lseek64(flv_it.ext3_part.fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in is_matching_third_indirect_block.\n");

		return false;
	}

	unsigned int first_block_num = read_u32_int(flv_it.ext3_part.fd);

	return is_matching_second_indirect_block(first_block_num, flv_it);
}

bool enqueue_from_third_indirect_block(unsigned int third_indirect_block_num, flv_iterator &flv_it) {
	off64_t offset = (off64_t) third_indirect_block_num * (off64_t) flv_it.ext3_part.block_size;	// Typecast needed?
	off64_t pos = lseek64(flv_it.ext3_part.fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in enqueue_from_third_indirect_block.\n");

		return false;
	}

	char block[flv_it.ext3_part.block_size];
	ssize_t bytes_read = read(flv_it.ext3_part.fd, block, flv_it.ext3_part.block_size);

	if (bytes_read < flv_it.ext3_part.block_size) {
		printf("File read error in enqueue_from_third_indirect_block.\n");

		return false;
	}

	unsigned int block_num;

	for (unsigned int i = 0; i < flv_it.ext3_part.block_size; i += BLOCK_ADDR_LEN) {
		block_num = read_u32_int_from_buffer(&block[i]);

		if (block_num == 0) {
			break;
		}

		enqueue_from_second_indirect_block(block_num, flv_it);
	}

	return true;
}
