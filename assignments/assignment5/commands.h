#ifndef COMMANDS_H
#define COMMANDS_H

// CREATE <SSFS file name>
// IMPORT <SSFS file name> <unix file name>
// CAT <SSFS file name>
// DELETE <SSFS file name>
// WRITE <SSFS file name> <char> <start byte> <num bytes> 
// READ <SSFS file name> <start byte> <num bytes>
// LIST
// SHUTDOWN

#include <string>
#include <fstream>

void create_file(std::string ssfs_file_name, int num_blocks, int block_size)
{
	std::ofstream new_file;
	for(int j = 0; j < 32; j++)
	{
		disk_file.seekp(i);
		if(temp_file[j] == ssfs_file_name)
		{
			perror("File name already exists");
		}
		else
		{
			newFile.open(ssfs_file_name);
		}
	}
	new_file.close();
}

void import_file(char ssfs_file_name, char unix_file_name)
{
	ofstream existing_file;
	ifstream imported_file;
	existing_file.open(ssfs_file_name, ios::out);
	imported_file.open(unix_file_name, ios::in);
	
	char c;
	c = fgetc(existing_file);
}

void cat_file(char ssfs_file_name)
{

}

void delete_file(char ssfs_file_name)
{

}

void write_to_file(char ssfs_file_name, char character, int start_bytes, int num_bytes)
{

}

void read_from_file(char ssfs_file_name, int start_bytes, int num_bytes)
{

}

void list_files()
{

}

void shutdown_ssfs()
{

}

#endif