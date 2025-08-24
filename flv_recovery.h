#ifndef FLV_RECOVERY_H
#define FLV_RECOVERY_H

// A set of functions for recovering an FLV file from an Ext3 partition.

#include "ext3_partition.h"

#define FLV_SIG_LEN 4
#define PROG_BAR_LEN 48

const char FLV_SIG[FLV_SIG_LEN] = {'F', 'L', 'V', (char) 1};

// Standalone Functions
bool get_first_flv_sig_block(ext3_partition &ext3_part, unsigned int &first_flv_sig_block);
queue get_flv_sig_blocks(ext3_partition &ext3_part);
void recover_flv(ext3_partition &ext3_part, const char *out_file_path, unsigned int first_block_num);

// Helper Functions
bool is_flv_sig_block(unsigned int fd, unsigned int block_num, unsigned int block_size);
bool is_flv_sig(char *str);

#endif
