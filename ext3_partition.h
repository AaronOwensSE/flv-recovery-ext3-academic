#ifndef EXT3_PARTITION_H
#define EXT3_PARTITION_H

// A representation of a Linux Ext3 file partition.

#include "queue.h"

#define SB0_OFFSET 1024
#define NUM_BLOCKS_IN_PARTITION_REL_OFFSET 4
#define BLOCK_SIZE_EXP_REL_OFFSET 24
#define BLOCK_SIZE_BASE 2
#define BLOCK_SIZE_UNIT 1024
#define BLOCK_ADDR_LEN 4
#define NUM_DIRECT_BLOCKS 12
#define MAX_JUMPS 10
#define PROG_BAR_LEN 48

struct ext3_partition {
	int fd;
	unsigned int num_blocks;
	unsigned int block_size;
	queue indirect_blocks;
	queue first_indirect_blocks;
	queue second_indirect_blocks;
	queue third_indirect_blocks;
};

// Struct Functions
ext3_partition create_ext3_partition(char *path);
void get_indirect_blocks(ext3_partition &ext3_part);
void get_first_indirect_blocks(ext3_partition &ext3_part);
void get_second_indirect_blocks(ext3_partition &ext3_part);
void get_third_indirect_blocks(ext3_partition &ext3_part);
unsigned int get_longest_indirect_block_num(queue &indirect_blocks, ext3_partition &ext3_part);
void delete_ext3_partition(ext3_partition &ext3_part);

// Helper Functions
unsigned int get_num_blocks_in_partition(int fd);
unsigned int get_block_size(int fd);
unsigned int get_block_size_exp(int fd);
bool is_indirect_block(int fd, unsigned int block_num, unsigned int block_size);
bool is_first_indirect_block(int fd, unsigned int block_num, unsigned int block_size, queue &indirect_blocks);
bool is_second_indirect_block(int fd, unsigned int block_num, unsigned int block_size, queue &indirect_blocks);
bool is_third_indirect_block(int fd, unsigned int block_num, unsigned int block_size, queue &indirect_blocks);
unsigned int get_indirect_block_len(int fd, unsigned int indirect_block_num, unsigned int block_size);

#endif
