#ifndef SSFS_FILE_H
#define SSFS_FILE_H

#include <vector>
#include <string>

typedef struct Inode {
	std::string file_name;
	int file_size;
	int direct_block_pointers[12];
	int indirect_block_pointer;
	int double_indirect_block_pointer;
	Inode(std::string filename, int size) 
	{
		file_name = filename;
		file_size = size;
		for (int i = 0; i < 12; i++) direct_block_pointers[i] = 0;
	}	
} Inode;

typedef struct Inode_Map {
	// each entry is 36 bytes
	std::vector<std::string> file_names;
	std::vector<int> inode_locations;
	
} Inode_Map;


typedef struct Command
{
	// The scheduler can ONLY read and write entire blocks
	std::string command; // "READ" or "WRITE"
	int job_id;
	int block_id;
	char *data;
} Command;

#endif
