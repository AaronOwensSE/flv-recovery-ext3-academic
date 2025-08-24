#ifndef FLV_ITERATOR_H
#define FLV_ITERATOR_H

// An iterator and associated functions for recovering an FLV file from an Ext3 partition.

#include "ext3_partition.h"
#include <sys/types.h>

#define FIRST_TAG_OFFSET 13
#define PAYLOAD_SIZE_REL_OFFSET 1
#define PAYLOAD_SIZE_LEN 3
#define TAG_HEADER_LEN 11
#define PREV_TAG_SIZE_LEN 4

struct flv_iterator {
	ext3_partition ext3_part;
	int out_fd;
	queue block_num_queue;
	unsigned int block_num;
	off64_t tag_offset;
	off64_t eof_offset;
	bool eof;
};

// Struct Functions
flv_iterator create_flv_iterator(ext3_partition &ext3_part, const char *out_file_path, unsigned int first_block_num);
void recover_flv(flv_iterator &flv_it);
void delete_flv_iterator(flv_iterator &flv_it);

// Helper Functions
queue get_direct_blocks(unsigned int first_block_num);
void recover_from_queue(flv_iterator &flv_it);
bool copy_block(flv_iterator &flv_it);
bool advance_tag_offset(flv_iterator &flv_it);
void truncate_file(flv_iterator &flv_it);
bool get_blocks_from_first_indirection(flv_iterator &flv_it);
bool is_matching_first_indirect_block(unsigned int first_indirect_block_num, flv_iterator &flv_it);
bool enqueue_from_first_indirect_block(unsigned int first_indirect_block_num, flv_iterator &flv_it);
bool get_blocks_from_second_indirection(flv_iterator &flv_it);
bool is_matching_second_indirect_block(unsigned int second_indirect_block_num, flv_iterator &flv_it);
bool enqueue_from_second_indirect_block(unsigned int second_indirect_block_num, flv_iterator &flv_it);
bool get_blocks_from_third_indirection(flv_iterator &flv_it);
bool is_matching_third_indirect_block(unsigned int third_indirect_block_num, flv_iterator &flv_it);
bool enqueue_from_third_indirect_block(unsigned int third_indirect_block_num, flv_iterator &flv_it);

#endif
