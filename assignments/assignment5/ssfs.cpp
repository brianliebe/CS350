#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <fstream>
#include <cstring>
#include <sstream>
#include <algorithm>

#include "ssfs_file.h"

using namespace std;

/* GLOBAL VARIABLES */

vector<Command*> commands;
vector<Inode*> inodes;
Inode_Map *inode_map;
int *free_block_list;

mutex command_mutex, data_mutex, freeblock_mutex, map_mutex, inode_mutex; // make sure we have locks surrounding all thread-accessible data
condition_variable cond;
int threads_finished = 0;
int total_threads = 0;

int num_blocks = 0;
int block_size = 0;
bool force_close = false;
string disk_name = "DISK";

/*
	Returns an int location for the first free block in the free_block_list
*/
int getFreeBlockNumber() 
{
	unique_lock<mutex> lck(freeblock_mutex);
	for (int i = 0; i < num_blocks; i++) 
	{
		if (free_block_list[i] != 0)
		{
			free_block_list[i] = 0;
			lck.unlock();
			return i;
		}
	}
	lck.unlock();
	return -1;
}

void freeBlock(int id)
{
	unique_lock<mutex> lck(freeblock_mutex);
	free_block_list[id] = 1;
	lck.unlock();
}

void addCommandToQueue(Command *bc) 
{
	unique_lock<mutex> lck(command_mutex);
	commands.push_back(bc);
	lck.unlock();
}

void addCommandToQueue(Command **bc, int number_of_commands)
{
	unique_lock<mutex> lck(command_mutex);
	for (int i = 0; i < number_of_commands; i++)
	{
		commands.push_back(bc[i]);	
	}
	lck.unlock();
}

bool check_existence(string name)
 {
	unique_lock<mutex> lck(map_mutex);
	for (unsigned int i = 0; i < inode_map->file_names.size(); i++)
	{
		if (inode_map->file_names[i] == name)
		{
			lck.unlock();
			return true;
		}
	}
	lck.unlock();
	return false;
}

void create_file(string filename)
{
	if(inode_map->file_names.size() < 256)
	{
		if(check_existence(filename) == true)
		{
			perror("File name already exists!");
		}

		else{
		
			Inode *createInode = new Inode(filename, 0, block_size);
			inodes.push_back(createInode);
			inode_map->file_names.push_back(filename);
			inode_map->inode_locations.push_back(getFreeBlockNumber());
		}
		
	}
	else{
		perror("Not enough space");
	}
		
}

 

void import_file(string ssfs_file, string unix_file){
	/*ifstream unixFile;
	ofstream ssfsFile;
	unixFile.open(unix_file);
	ssfsFile.open(ssfs_file); // this is wrong, the ssfs_file is the new file we're creating within DISK
	
	char ch;
	while(!unixFile.eof()){
		unixFile.get(ch);
		ssfsFile<<ch;
	}
	unixFile.close();
	ssfsFile.close();	*/
	/*
	create a bunch of Commands that hold all the data from unix_file and then add those commands to the queue
	*/
	
}
//Outputing the file to the stdout 
void cat_file(string filename)
{
	/*
	for(int i = 0; i < inodes.size(); i++){
		if(inodes[i]->file_name == filename){
			
	
			
			for(int i = 0; i < (sizeof(inodes[i]->direct_block_pointers)/sizeof(inodes[i]->direct_block_pointers[0])); i++){
				char *buf = new char[block_size];
				disk.seekg(inodes[i]->direct_block_pointers[i] * block_size);
				disk.read(buf, block_size);
				disk.close();
				cout << buf << endl;
			}
		}
	}
		
		
		
*/		

}
void delete_file(string filename)
{
	//remove the inode
	//free block
	//search the inode map for the file
	for(int i = 0; i < inode_map->file_names.size(); i++){
		if(inode_map->file_names[i] == filename){
			unique_lock<mutex> lck(map_mutex);
			// freeBlock(inode_map->inode_locations[i]);
			inode_map->file_names.erase(inode_map->file_names.begin() + i);
			inode_map->inode_locations.erase(inode_map->inode_locations.begin() + i);
			lck.unlock();		
		}
	}
	for(int j = 0; j < inodes.size(); j++){
		if(inodes[j]->file_name == filename){
			Inode *inode = inodes[j];
			unique_lock<mutex> lck(inode_mutex);
			inodes.erase(inodes.begin() + j);
			lck.unlock();
			
			for (int i = 0; i < 12; i++) 
			{
				if (inode->direct_block_pointers[i] != -1) 
				{
					freeBlock(inode->direct_block_pointers[i]);
				}
			}

			for (int i = 0; i < block_size / 4 && inode->indirect_block != -1; i++) 
			{
				if (inode->indirect_block_pointers[i] != -1) 
				{
					freeBlock(inode->indirect_block_pointers[i]);
				}
			}
		}
	}
			
		
	
	
}
void write_to_file(string filename, char letter, int start_byte, int num_bytes)
{

}
void read_from_file(string filename, int start_byte, int num_bytes)
{

}
//Unless there's something I'm missing, I don't think we have a direct link between the inode map
//and individual inodes. I think we can just put the inodes that are created into an array
// and add/delete to it accordingly.
void list_files()
{	
	for(int i = 0; i < inodes.size(); i++){
		cout << "File name: " << inodes[i]->file_name << "," << "Size: " << inodes[i]->file_size << endl;
	}
}
void shutdown_ssfs()
{
	force_close = true; // make sure this is set
	this_thread::sleep_for(chrono::seconds(2));
	// need to copy the inode map back in
	// need to copy the inodes back in
	// data should already be set properly
	fstream disk(disk_name.c_str(), ios::binary | ios::out | ios::in);
	// disk.seekp(command->block_id * block_size, ios::beg);
	// disk.write(command->data, block_size);

	// write inode map to file
	int start_location = 1 * block_size;
	disk.seekp(start_location);
	int max_entries = 256;
	int inode_map_blocks = (max_entries * 36) / block_size;
	if ((max_entries * 36) % block_size > 0) inode_map_blocks++;

	for (int i = 0; i < inode_map->file_names.size(); i++) 
	{
		char temp_file[32];
		strcpy(temp_file, inode_map->file_names[i].c_str());
		disk.write((char*)temp_file, sizeof(temp_file));
	}
	for (int i = inode_map->file_names.size(); i < max_entries; i++)
	{
		char temp_file[32];
		strcpy(temp_file, "NULL_FILE");
		disk.write((char*)&temp_file, sizeof(temp_file));
	}
	for (int i = 0; i < inode_map->inode_locations.size(); i++)
	{
		disk.write((char*)&inode_map->inode_locations[i], sizeof(inode_map->inode_locations[i]));
	}
	for (int i = inode_map->inode_locations.size(); i < max_entries; i++)
	{
		int temp_int = -1;
		disk.write((char*)&temp_int, sizeof(temp_int));
	}

	// write free block list to file
	start_location = (1 + inode_map_blocks) * block_size;
	disk.seekp(start_location);
	
	for (int i = 0; i < num_blocks; i++) 
	{
		disk.write((char*)&free_block_list[i], sizeof(int));
	}

	// write inodes to file 
	for (int i = 0; i < inodes.size(); i++)
	{
		int block_number = 0;
		for (int j = 0; j < inode_map->file_names.size(); j++)
		{
			if (inode_map->file_names[j] == inodes[i]->file_name)
			{
				block_number = inode_map->inode_locations[j];
				break;
			}
		}
		if (block_number != 0)
		{
			disk.seekp(block_number * block_size);
			disk.write((char*)&inodes[i]->file_name, sizeof(inodes[i]->file_name));
			disk.write((char*)&inodes[i]->file_size, sizeof(inodes[i]->file_size));
			for (int j = 0; j < 12; j++) disk.write((char*)&inodes[i]->direct_block_pointers[j], sizeof(int));
			disk.write((char*)&inodes[i]->indirect_block, sizeof(int));
			// disk.write((char*)&inodes[i]->double_indirect_block_pointer, sizeof(int));
			disk.seekp(inodes[i]->indirect_block * block_size);
			for (int j = 0; j < block_size / 4; j++)
			{
				disk.write((char*)&inodes[i]->indirect_block_pointers[j], sizeof(int));
			}
		}
	}

	disk.close();
}

void execute_commands(string disk_name) 
{
	while (!force_close)
	{
		unique_lock<mutex> lck(command_mutex); // acquire the lock

		while (!commands.size()) 
		{
			if (threads_finished == total_threads)
			{
				// if all the data has been read in AND the command queue is empty, return
				lck.unlock();
				return;
			}
			// otherwise, wait for the condition variable (or 100 ms to recheck)
			cond.wait_for(lck, chrono::milliseconds(100));
		}
		Command *command = commands.at(0);
		commands.erase(commands.begin()); // remove it from queue
		lck.unlock();
		
		if (command->command == "READ")
		{
			cout << "START: READ for job " << command->job_id << endl;
			ifstream disk(disk_name.c_str(), ios::binary | ios::in);
			char *buffer = new char [block_size];
			disk.seekg(command->block_id * block_size);
			disk.read(buffer, block_size);
			disk.close();
			
			cout << buffer << endl;

			cout << "STOP: READ for job " << command->job_id << endl;
		}
		if (command->command == "WRITE")
		{
			cout << "START: WRITE for job " << command->job_id << endl;
			fstream disk(disk_name.c_str(), ios::binary | ios::out | ios::in);
			disk.seekp(command->block_id * block_size, ios::beg);
			disk.write(command->data, block_size);
			disk.close();
			cout << "STOP: WRITE for job " << command->job_id << endl;
			delete command->data;
		}
		delete command;
	}
}

/*
	Okay, so the idea here is that this (threaded) function will call create_file/import_file/etc 
	and those functions WON'T actually do anything. They'll just create Commands (holds block,
	WRITE/READ, and the data) which will be performed by the scheduler thread. So each "command" function
	will just add the Commands to a queue, and the other thread performs them. So each "command" function
	should "figure out" what blocks need to be written to based on what it's being asked to do.
*/
void read_thread_ops(string filename)
{
	ifstream op_file(filename.c_str());
	string line;
	while (getline(op_file, line) && !force_close) 
	{
		string command = line.substr(0, line.find(" "));
		if (command == "CREATE")
		{
			istringstream iss(line);
			string a, b;
			if (!(iss >> a >> b)) { break; }
			create_file(b);
		}
		else if (command == "IMPORT") 
		{
			istringstream iss(line);
			string a, b, c;
			if (!(iss >> a >> b >> c)) { break; }
			import_file(b, c);
		}
		else if (command == "CAT")
		{
			istringstream iss(line);
			string a, b;
			if (!(iss >> a >> b)) { break; }
			cat_file(b);
		}
		else if (command == "DELETE")
		{
			istringstream iss(line);
			string a, b;
			if (!(iss >> a >> b)) { break; }
			delete_file(b);
		}
		else if (command == "WRITE")
		{
			istringstream iss(line);
			string a, b;
			char c;
			int d, e;
			if (!(iss >> a >> b >> c >> d >> e)) { break; }
			write_to_file(b, c, d, e);
		}
		else if (command == "READ") 
		{
			istringstream iss(line);
			string a, b;
			int c, d;
			if (!(iss >> a >> b >> c >> d)) { break; }
			read_from_file(b, c, d);
		}
		else if (command == "LIST")
		{
			list_files();
		}
		else if (command == "SHUTDOWN")
		{
			shutdown_ssfs();
		}
		else
		{
			cout << "Error reading line from OP file :\"" << line << "\"" << endl;
		}
	}
	op_file.close();
	unique_lock<mutex> lck(command_mutex);
	threads_finished++;
	lck.unlock();
}

int main(int argc, char **argv) 
{
	string op1, op2, op3, op4;
	switch(argc) 
	{
		case 1:
			printf("USAGE: ./ssfs <disk file name> <op1> <op2> <op3>\n");
			return 0;
		case 2:
			printf("USAGE: ./ssfs <disk file name> <op1> <op2> <op3>\n");
			return 0;
		case 3:
			disk_name = string(argv[1]);
			op1 = string(argv[2]);
			total_threads = 1;
			break;
		case 4:
			disk_name = string(argv[1]);
			op1 = string(argv[2]);
			op2 = string(argv[3]);
			total_threads = 2;
			break;
		case 5:
			disk_name = string(argv[1]);
			op1 = string(argv[2]);
			op2 = string(argv[3]);
			op3 = string(argv[4]);
			total_threads = 3;
			break;
		case 6:
			disk_name = string(argv[1]);
			op1 = string(argv[2]);
			op2 = string(argv[3]);
			op3 = string(argv[4]);
			op4 = string(argv[5]);
			total_threads = 4;
			break;
		default:
			printf("USAGE: ./ssfs <disk file name> <op1> <op2> <op3>\n");
			return 0;		
	}

	// Read block size and number of blocks from superblock
	ifstream disk(disk_name, ios::binary | ios::in);
	disk.seekg(0);
	disk.read(reinterpret_cast<char*>(&num_blocks), sizeof(num_blocks));
	disk.read(reinterpret_cast<char*>(&block_size), sizeof(block_size));
	printf("Number of blocks: %d\nBlock size: %d\n", num_blocks, block_size);
	
	// Read the inode map and store the locations
	disk.seekg(block_size);
	int inode_map_blocks = (256 * 36) / block_size;
	if ((256 * 36) % block_size > 0) inode_map_blocks++;

	inode_map = new Inode_Map;
	int other = 0;

	for (int i = 0; i < 256; i++)
	{
		char *temp_file_name = new char[32];
		disk.read(temp_file_name, 32);
		if (string(temp_file_name) != "NULL_FILE")
		{
			inode_map->file_names.push_back(string(temp_file_name));
		}
		else other++;
	}
	for (int i = 0; i < 256; i++)
	{
		int temp_int;
		disk.read(reinterpret_cast<char*>(&temp_int), sizeof(temp_int));
		if (temp_int != -1)
		{
			inode_map->inode_locations.push_back(temp_int);
		}
	}
	cout << "Map size: " << inode_map->file_names.size()  << " and we skipped: " << other << endl;

	// Read in the free block list
	int start_location = (1 + inode_map_blocks) * block_size;
	disk.seekg(start_location);

	free_block_list = new int[num_blocks];

	for (int i = 0; i < num_blocks; i++)
	{
		int temp_int;
		disk.read(reinterpret_cast<char*>(&temp_int), sizeof(temp_int));
		free_block_list[i] = temp_int;
	}

	disk.close();

	// Create threads for the operations
	vector<thread> threads;
	string ops_files[4] = {op1, op2, op3, op4};

	threads.push_back(thread(execute_commands, disk_name));
	for (int i = 0; i < total_threads; i++)
	{
		threads.push_back(thread(read_thread_ops, ops_files[i]));
	}
	for (int i = 0; i < total_threads + 1; i++)
	{
		threads[i].join();
	}
}
