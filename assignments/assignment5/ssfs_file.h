#ifndef SSFS_FILE_H
#define SSFS_FILE_H

#include <vector>
#include <string>

typedef struct indirect_block {
	int **direct_blocks;
} indirect_block;

typedef struct double_indirect_block {
	int **indirect_blocks;
} double_indirect_block;

typedef struct inode {
	std::string file_name;
	int file_size;
	int direct_block_pointers[12];
	int indirect_block_pointer;
	int double_indirect_block_pointer;	
} inode;

typedef struct Inode_Map {
	// each entry is 36 bytes
	std::vector<std::string> file_names;
	std::vector<int> inode_locations;
} Inode_Map;

#endif
