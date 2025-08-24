#include "flv_recovery.h"
#include "flv_iterator.h"
#include <cstdio>
#include <unistd.h>

bool get_first_flv_sig_block(ext3_partition &ext3_part, unsigned int &first_flv_sig_block) {
	printf("Searching for FLV signature block.\n");

	for (unsigned int i = 0; i < PROG_BAR_LEN; i++) {
		printf("_");
	}

	printf("\n");

	unsigned int blocks_per_prog_update = ext3_part.num_blocks / PROG_BAR_LEN;

	for (unsigned int i = 0; i < ext3_part.num_blocks; i++) {
		if (is_flv_sig_block(ext3_part.fd, i, ext3_part.block_size)) {
			printf("*\n");

			first_flv_sig_block = i;

			return true;
		}

		if ((i + 1) % blocks_per_prog_update == 0) {
			printf(">");
			fflush(stdout);
		}
	}

	printf("\n");

	return false;
}

queue get_flv_sig_blocks(ext3_partition &ext3_part) {
	queue flv_sig_blocks = create_queue();

	printf("Searching for FLV signature blocks.\n");

	for (unsigned int i = 0; i < PROG_BAR_LEN; i++) {
		printf("_");
	}

	printf("\n");

	unsigned int blocks_per_prog_update = ext3_part.num_blocks / PROG_BAR_LEN;

	for (unsigned int i = 0; i < ext3_part.num_blocks; i++) {
		if (is_flv_sig_block(ext3_part.fd, i, ext3_part.block_size)) {
			enqueue(i, flv_sig_blocks);
		}

		if ((i + 1) % blocks_per_prog_update == 0) {
			printf(">");
			fflush(stdout);
		}
	}

	printf("\n");

	return flv_sig_blocks;
}

void recover_flv(ext3_partition &ext3_part, const char *out_file_path, unsigned int first_block_num) {
	flv_iterator flv_it = create_flv_iterator(ext3_part, out_file_path, first_block_num);
	recover_flv(flv_it);
	delete_flv_iterator(flv_it);
}

bool is_flv_sig_block(unsigned int fd, unsigned int block_num, unsigned int block_size) {
	off64_t offset = (off64_t) block_num * (off64_t) block_size;	// Typecast needed?
	off64_t pos = lseek64(fd, offset, SEEK_SET);

	if (pos < 0) {
		printf("File seek error in is_flv_sig_block.\n");

		return false;
	}

	char flv_sig[FLV_SIG_LEN];
	ssize_t bytes_read = read(fd, flv_sig, FLV_SIG_LEN);

	if (bytes_read < FLV_SIG_LEN) {
		printf("File read error in is_flv_sig_block.\n");

		return false;
	}

	return is_flv_sig(flv_sig);
}

bool is_flv_sig(char *str) {
	for (unsigned int i = 0; i < FLV_SIG_LEN; i++) {
		if (str[i] != FLV_SIG[i]) {
			return false;
		}
	}

	return true;
}
