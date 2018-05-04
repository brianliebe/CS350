#ifndef SSFS_FILE_H
#define SSFS_FILE_H

#include <vector>
#include <string>

typedef struct Inode {
	std::string file_name;
	int file_size;
	int *direct_block_pointers;
	int indirect_block;
	int *indirect_block_pointers;
	//int double_indirect_block_pointer;
	Inode(std::string filename, int size, int block_size) 
	{
		file_name = filename;
		file_size = size;
		direct_block_pointers = new int[12];
		indirect_block = -1;
		indirect_block_pointers = new int[block_size / 4];		
		for (int i = 0; i < 12; i++) direct_block_pointers[i] = -1;
		for (int i = 0; i < block_size/4; i++) indirect_block_pointers[i] = -1;
	}
	Inode()
	{
		// nothing
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
	int thread_id;
	char *data;
	Command(std::string comm, int job, int id, char *d, int t_id) 
	{
		command = comm;
		job_id = job;
		block_id = id;
		data = d;
		thread_id = t_id;
	}
	Command(std::string comm, int id, char *d)
	{
		command = comm;
		block_id = id;
		data = d;
	}
} Command;

typedef struct Thread_Arg
{
	std::string filename;
	int thread_id;
	Thread_Arg(std::string f, int t)
	{
		filename = f;
		thread_id = t;
	}
} Thread_Arg;

#endif
