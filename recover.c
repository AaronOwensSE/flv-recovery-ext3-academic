// A program for recovering an FLV file from an Ext3 partition.

#include "flv_recovery.h"
#include <cstdio>

bool args_valid(int argc, char **argv);

int main(int argc, char **argv) {
	printf("FLV FILE RECOVERY PROGRAM\n");

	if (!args_valid(argc, argv)) {
		printf("Invalid argument.\n");

		return 1;
	}

	ext3_partition ext3_part = create_ext3_partition(argv[1]);

	if (ext3_part.fd < 0) {
		return 1;
	}

	const char *out_file_path = "recovered_file.flv";

	unsigned int flv_sig_block_num;

	if (!get_first_flv_sig_block(ext3_part, flv_sig_block_num)) {
		printf("FLV signature block not found.\n");

		return 1;
	}

	recover_flv(ext3_part, out_file_path, flv_sig_block_num);

	delete_ext3_partition(ext3_part);

	return 0;
}

bool args_valid(int argc, char **argv) {
	return argc == 2;
}
