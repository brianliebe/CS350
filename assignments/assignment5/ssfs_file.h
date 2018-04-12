#ifndef SSFS_FILE_H
#define SSFS_FILE_H

typedef struct indirect_block {
	int d_size;
	std::vector<int> block_numbers;
} indirect_block;

typedef struct double_indirect_block {
	int d_size;
	std::vector<int> block_numbers;
} double_indirect_block;

typedef struct inode {
	std::string file_name;
	int file_size;
	int direct_block_pointers[12];
	indirect_block *indirect_block_pointer;
	double_indirect_block *double_indirect_block_pointer;	
} inode;

#endif
