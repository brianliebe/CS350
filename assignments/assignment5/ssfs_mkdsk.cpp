#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <cstdlib>
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
	ofstream disk_file(disk_file_name.c_str(), ios::binary | ios::out);
	disk_file.seekp((num_blocks * block_size) - 1);
	disk_file.write("", 1);

	// write superblock
	disk_file.seekp(0);
	disk_file.write((char*)&num_blocks, sizeof(num_blocks));
	disk_file.write((char*)&block_size, sizeof(block_size));

	// write inode map to file
	int start_location = 1 * block_size;
	disk_file.seekp(start_location);
	int max_entries = 256;
	int inode_map_blocks = (max_entries * 36) / block_size;
	if ((max_entries * 36) % block_size > 0) inode_map_blocks++;

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
	start_location = (1 + inode_map_blocks) * block_size;
	disk_file.seekp(start_location);

	int free_list_blocks = ((num_blocks) * 4) / block_size;
	if ((num_blocks * 4) % block_size > 0) free_list_blocks++;

	for (int i = 0; i < num_blocks; i++) 
	{
		// Write a 1 (free) for all blocks except the ones that have been allocated already
		int temp_int = 1;
		if (i < free_list_blocks + inode_map_blocks + 1)
		{
			temp_int = 0;
		}
		disk_file.write((char*)&temp_int, sizeof(temp_int));
	}

	return 0;
}