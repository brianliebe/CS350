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

mutex command_mutex, data_mutex, freeblock_mutex, map_mutex, inode_mutex, job_mutex; // make sure we have locks surrounding all thread-accessible data
condition_variable cond;
int threads_finished = 0;
int total_threads = 0;
int job_id_count;

int num_blocks = 0;
int block_size = 0;
bool force_close = false;
string disk_name = "DISK";

vector<char*> *responses;

void read_from_file(string, int, int, int);
void delete_file(string);

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

int getJobId()
{
	unique_lock<mutex> lck(job_mutex);
	int temp = job_id_count++;
	lck.unlock();
	return temp;
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

void addCommandToQueue(vector<Command*> bc)
{
	unique_lock<mutex> lck(command_mutex);
	for (int i = 0; i < bc.size(); i++)
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
			printf("ERROR: File name already exists: %s\n", filename.c_str());
		}

		else{
		
			Inode *createInode = new Inode(filename, 0, block_size);
			inodes.push_back(createInode);
			inode_map->file_names.push_back(filename);
			inode_map->inode_locations.push_back(getFreeBlockNumber());
		}
		
	}
	else{
		printf("ERROR: Not enough space, ignoring.\n");
	}
		
}

void import_file(string ssfs_file, string unix_file)
{
	bool file_already_exists = false;
	int index = 0;
	for (unsigned int i = 0; i < inodes.size(); i++)
	{
		if (inodes[i]->file_name == ssfs_file) 
		{ 
			file_already_exists = true; 
			index = i;
			break;
		}
	}

	if (!file_already_exists)
	{
		create_file(ssfs_file);
	}
	else 
	{
		delete_file(ssfs_file);
		create_file(ssfs_file);
		for (unsigned int i = 0; i < inodes.size(); i++)
		{
			if (inodes[i]->file_name == ssfs_file)
			{
				index = i;
				break;
			}
		}
	}

	ifstream unix(unix_file, std::fstream::ate | std::ifstream::binary);
	int unix_size = unix.tellg();

	inodes[index]->file_size = unix_size;

	unix.seekg(0);
	while (unix_size)
	{
		char *data = new char[block_size];
		if (unix_size <= block_size)
		{
			unix.read(data, unix_size);
			unix_size = 0;
		}
		else
		{
			unix_size -= block_size;
			unix.read(data, block_size);
		}
		int block = getFreeBlockNumber();
		Command *comm = new Command("WRITE", block, data);

		bool found_in_direct = false;
		for (int i = 0; i < 12; i++) 
		{
			if (inodes[index]->direct_block_pointers[i] == -1)
			{
				inodes[index]->direct_block_pointers[i] = block;
				found_in_direct = true;
				break;
			}
		}
		if (!found_in_direct)
		{
			for (int i = 0; i < block_size / 4; i++)
			{
				if (inodes[index]->indirect_block_pointers[i] == -1)
				{
					inodes[index]->indirect_block_pointers[i] = block;
					break;
				}
			}
		}
		addCommandToQueue(comm);
	}
	unix.close();
}

void cat_file(string filename, int thread_id)
{	
	int temp_size = 0;
	for (int i = 0; i < inodes.size(); i++) 
	{
		if (inodes[i]->file_name == filename) 
		{
			temp_size = inodes[i]->file_size;
			break;
		}
	}
	read_from_file(filename, 0, temp_size, thread_id);
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






void write_to_file(string filename, char letter, int start_byte, int num_bytes, int thread_id)
{
	vector<int> job_ids;
	int block_id = 0;
	Inode *inode;
	vector<Command *> commands;
	for(int i = 0; i < inode_map->file_names.size(); i++)
	{
		if(inode_map->file_names[i] == filename)
		{
							
			block_id = inode_map->inode_locations[i];
			break;
		}
	}
	if(block_id != 0)
	{
		for(int i = 0; i < inodes.size(); i++)
		{
			if(inodes[i]->file_name == filename)
			{
				inode = inodes[i];
				break;
			}
		}
		if(start_byte + num_bytes > inode->file_size)
		{
			//update the file size to the new file size
			inode->file_size = inode->file_size + num_bytes;
		}
		
		
		//Direct block
		
		//Updating the direct block pointers && indirect block pointers
		int num_of_blocks = (num_bytes / block_size) + 1;
		if(num_of_blocks < 0)
		{
			num_of_blocks == 1;
		}
		int num_of_blocks_left = num_of_blocks;
		for(int i = 0; i < num_of_blocks; i++)
		{		
			int free_block_number = getFreeBlockNumber();
			if(free_block_number != -1)
			{
				for(int j = 0; j < 12; j++)
				{
					//Check if direct block pointer list has space
					if(inode->direct_block_pointers[j] == -1)
					{
						inode->direct_block_pointers[j] = free_block_number;
						num_of_blocks_left--;
					}
					if(j == 12 && num_of_blocks_left > 0)
					{
						for(int k = 0; k < block_size / 4; k++)
						{
							if(inode->indirect_block_pointers[k] == -1)
							{
								inode->indirect_block_pointers[k] = free_block_number;
							}
						}	

					}
				}

			}
			else
			{
				perror("Not enough free blocks!");
			}
				
		}
		

		char *data = new char[num_bytes];
		for(int u = 0; u < num_bytes; u++){
			data[u] = letter;
		}
		int start_block = start_byte / block_size;
		int end_block = ((start_byte + num_bytes) / block_size);
		cout << start_block << endl;
		cout << end_block << endl;
		
		
		if(end_block <= 12){
			for(int i = start_block; i <= end_block; i++)
			{
					
					int id = getJobId();
					job_ids.push_back(id);
					
					Command *comm = new Command("WRITE", id, inode->direct_block_pointers[i], data, thread_id);
				
					commands.push_back(comm);
					
	
			}

		}
		
		
	}

	addCommandToQueue(commands);
}






void read_from_file(string filename, int start_byte, int num_bytes, int thread_id)
{
	int block_id = 0;
	Inode *inode;
	vector<Command*> commands;
	vector<int> job_ids;

	for (int i = 0; i < inode_map->file_names.size(); i++)
	{
		if (inode_map->file_names[i] == filename)
		{
			block_id = inode_map->inode_locations[i];
			break;
		}
	}

	if (block_id != 0)
	{
		for (int i = 0; i < inodes.size(); i++) 
		{
			if (inodes[i]->file_name == filename) 
			{
				inode = inodes[i];
				break;
			}
		}
		if (start_byte + num_bytes > inode->file_size) 
		{
			printf("ERROR: range too large\n");
			return;
		}
		int start_block = (start_byte / block_size);
		int end_block = (start_byte + num_bytes) / block_size;

		if (end_block < 12) 
		{
			for (int i = start_block; i <= end_block; i++)
			{
				int id = getJobId();
				job_ids.push_back(id);
				Command *comm = new Command("READ", id, inode->direct_block_pointers[i], NULL, thread_id);
				commands.push_back(comm);
			}
		}
		else 
		{
			if (start_block < 12)
			{
				for (int i = start_block; i < 12; i++)
				{
					int id = getJobId();
					job_ids.push_back(id);
					Command *comm = new Command("READ", id, inode->direct_block_pointers[i], NULL, thread_id);
					commands.push_back(comm);
				}
				for (int i = 0; i <= end_block - 12; i++)
				{
					if (inode->indirect_block_pointers[i] != -1)
					{
						int id = getJobId();
						job_ids.push_back(id);
						Command *comm = new Command("READ", id, inode->indirect_block_pointers[i], NULL, thread_id);
						commands.push_back(comm);
					}
				}
			}
			else
			{
				for (int i = start_block - 12; i <= end_block - 12; i++)
				{
					if (inode->indirect_block_pointers[i] != -1)
					{
						int id = getJobId();
						job_ids.push_back(id);
						Command *comm = new Command("READ", id, inode->indirect_block_pointers[i], NULL, thread_id);
						commands.push_back(comm);
					}
				}
			}
		}
	}
	addCommandToQueue(commands);

	while (responses[thread_id].size() != job_ids.size());

	printf("OUTPUT:\n");
	if (responses[thread_id].size() == 1)
	{
		int new_start_byte = start_byte % block_size;
		printf("%.*s", num_bytes, responses[thread_id][0]);
	}
	else if (responses[thread_id].size() == 2)
	{
		int new_start_byte = start_byte % block_size;
		int new_num_bytes = block_size - new_start_byte;
		printf("%.*s", new_num_bytes, responses[thread_id][0]);
		new_start_byte = 0;
		new_num_bytes = num_bytes - new_num_bytes;
		printf("%.*s", new_num_bytes, responses[thread_id][1]);
	}
	else 
	{
		int remaining = num_bytes;
		int new_start_byte = start_byte % block_size;
		int new_num_bytes = block_size - new_start_byte;
		remaining -= new_num_bytes;
		printf("%.*s", new_num_bytes, responses[thread_id][0]);
		for (int i = 1; i < responses[thread_id].size(); i++)
		{
			printf("%.*s", block_size, responses[thread_id][i]);
			remaining -= block_size;
		}
		new_start_byte = 0;
		new_num_bytes = remaining;
		printf("%.*s", new_num_bytes, responses[thread_id][1]);
	}
	printf("\n");
}

void list_files()
{	
	for(int i = 0; i < inodes.size(); i++)
	{
		printf("File name: %s, Size: %d\n", inodes[i]->file_name.c_str(), inodes[i]->file_size);
	}
	if (inodes.size() == 0) printf("No files.\n");
}

void shutdown_ssfs()
{
	force_close = true;

	this_thread::sleep_for(chrono::seconds(2));
	fstream disk(disk_name.c_str(), ios::binary | ios::out | ios::in);

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
			char name[32];
			strcpy(name, inodes[i]->file_name.c_str());
			disk.seekp(block_number * block_size);

			disk.write(name, 32);
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
			// cout << "START: READ for job " << command->job_id << endl;
			ifstream disk(disk_name.c_str(), ios::binary | ios::in);
			char *buffer = new char [block_size];
			disk.seekg(command->block_id * block_size);
			disk.read(buffer, block_size);
			disk.close();
			
			responses[command->thread_id].push_back(buffer);

			// cout << "STOP: READ for job " << command->job_id << endl;
		}
		if (command->command == "WRITE")
		{
			// cout << "START: WRITE for job " << command->job_id << endl;
			fstream disk(disk_name.c_str(), ios::binary | ios::out | ios::in);
			disk.seekp(command->block_id * block_size, ios::beg);
			disk.write(command->data, block_size);
			disk.close();
			// cout << "STOP: WRITE for job " << command->job_id << endl;
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
void read_thread_ops(Thread_Arg *arg)
{
	string filename = arg->filename;
	int thread_id = arg->thread_id;
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
			cat_file(b, thread_id);
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
			write_to_file(b, c, d, e, thread_id);
		}
		else if (command == "READ") 
		{
			istringstream iss(line);
			string a, b;
			int c, d;
			if (!(iss >> a >> b >> c >> d)) { break; }
			read_from_file(b, c, d, thread_id);
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
	// printf("Number of blocks: %d\nBlock size: %d\n", num_blocks, block_size);
	
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
	// cout << "Map size: " << inode_map->file_names.size()  << " and we skipped: " << other << endl;

	// read the inodes
	for (unsigned int i = 0; i < inode_map->file_names.size(); i++)
	{
		int location = inode_map->inode_locations[i];
		string name = inode_map->file_names[i];

		Inode *inode = new Inode();
		inode->file_name = name;

		disk.seekg(location * block_size);
		char *t_file = new char[32];
		int t_size, t_indirect;
		int *directs = new int[12];
		
		disk.read(t_file, 32);
		
		disk.read(reinterpret_cast<char*>(&t_size), sizeof(int));

		for (int j = 0; j < 12; j++)
		{
			int temp;
			disk.read(reinterpret_cast<char*>(&temp), sizeof(int));
			directs[j] = temp;
		}
		disk.read(reinterpret_cast<char*>(&t_indirect), sizeof(int));
		inode->file_size = t_size;
		inode->indirect_block = t_indirect;

		inode->direct_block_pointers = directs;
		inodes.push_back(inode);
	}

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
	responses = new vector<char*>[total_threads + 1];
	for (int i = 0; i < total_threads; i++)
	{
		threads.push_back(thread(read_thread_ops, new Thread_Arg(ops_files[i], i)));
	}
	for (int i = 0; i < total_threads + 1; i++)
	{
		threads[i].join();
	}
}
