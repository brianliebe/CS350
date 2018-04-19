#include <iostream>
#include <string>
#include <cstring>
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

	// write inode map to file
	disk_file.seekp(0);
	int max_entries = 256;
	printf("%d\n", max_entries);

	for (int i = 0; i < max_entries; i++)
	{
		char temp_file[32];
		strcpy(temp_file, "NULL_FILE");
		disk_file.write((char*)&temp_file, sizeof(temp_file));
	}
	for (int i = 0; i < max_entries; i++)
	{
		int temp_int = -1;
		disk_file.write((char*)&temp_int, sizeof(temp_int));
	}

	// write free block list to file
	disk_file.seekp(72 * block_size);
	int blocks_in_use = ((num_blocks) * 4) / block_size;
	printf("%d\n", blocks_in_use);

	for (int i = blocks_in_use + 72; i < num_blocks; i++) 
	{
		// start at the first block after the free block list
		disk_file.write((char*)&i, sizeof(i));
	}
	int final_entry = -2;
	disk_file.write((char*)&final_entry, sizeof(final_entry));

	return 0;
}











