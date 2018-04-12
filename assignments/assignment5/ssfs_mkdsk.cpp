#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "ssfs_file.h"

using namespace std;

int main (int argc, char **argv)
{
	int num_blocks, block_size;
	string disk_file_name = "DISK";

	if (argc < 3 || argc > 4) {
		cout << "Usage: ./ssfs_mkdsk <num blocks> <block size> <disk file name>" << endl;
		return 0;
	}
	else {
		num_blocks = atoi(argv[1]); // num of disk blocks that the DISK holds
		block_size = atoi(argv[2]); // size of each block
		if (argc == 4) disk_file_name = string(argv[3]);
	}
	if (num_blocks < 1024 || num_blocks > 128 * 1024) {
		cout << "Number of blocks must be between 1024 and 128K." << endl;
		return 0;
	}
	if (block_size < 128 || block_size > 512) {
		cout << "Block size must be between 128 and 512." << endl;
		return 0;
	}

	// create empty disk
	ofstream disk_file(disk_file_name, ios::binary | ios::out);
	disk_file.seekp((num_blocks * block_size) - 1);
	disk_file.write("", 1);

	return 0;
}
