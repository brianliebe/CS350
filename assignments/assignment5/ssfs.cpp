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

#include "ssfs_file.h"

using namespace std;

/* GLOBAL VARIABLES */

vector<Command*> commands;
Inode_Map *inode_map;
int *free_block_list;

mutex command_mutex, data_mutex, freeblock_mutex, map_mutex; // make sure we have locks surrounding all thread-accessible data
condition_variable cond;
int threads_finished = 0;
int total_threads = 0;

int num_blocks = 0;
int block_size = 0;
bool force_close = false;

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

void addCommandToQueue(Command *bc) 
{
	unique_lock<mutex> lck(command_mutex);
	commands.push_back(bc);
	lck.unlock();
}

//Need to access the inode map so that we can see if the file name exists in the file map
//However, we cannot access it because it's not being declared in the scope of the create_file function
//Need to find a way to make the create_file have access to the inode map so I created the check_existence which takes 
//in the inode map and can just pass on the filename
//func read_thread_ops doesn't have the inode map so it cannot be passed onto the check_existence if we replace create_file with 
//check_existence(b) [line 168]

/*
You're right, so I made the Inode_Map a global variable (inode_map). I already created the struct for it, so I re-wrote your code
so that it used the existing struct (just for simplicity), but your code was fine. I think we should just return a true/false
rather than calling one function that will just call another. Call check_existence from create_file and return an error depending on
the boolean.
*/

void create_file(string filename)
{
	/*
	ofstream new_file;
	new_file.open(filename);
	new_file.close();
	*/

	/* peudocode:
	if (check_existence) -> continue
	else -> return an error

	inode = new inode() etc. // fill with the correct info for the file
	Command *bc = new Command;
	<save inode as a char*> -> put in bc
	addCommandToQueue(bc);
	*/
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
	/*
	vector<int>::iterator itr;
	if (find(inode_map->file_names.begin(), inode_map->file_names.end(), filename) != inode_map-file_names.end())
		perror("File name already exists!");
	}
	else{
		inode_map->file_names.push_back(name);
		create_file(name);
	} 
	*/
}

void import_file(string ssfs_file, string unix_file){
	/*ifstream unixFile;
	ofstream ssfsFile;
	unixFile.open(unix_file);
	ssfsFile.open(ssfs_file);
	
	char ch;
	while(!unixFile.eof()){
		unixFile.get(ch);
		ssfsFile<<ch;
	}
	unixFile.close();
	ssfsFile.close();	*/

	
}
void cat_file(string filename)
{

}
void delete_file(string filename)
{

}
void write_to_file(string filename, char letter, int start_byte, int num_bytes)
{

}
void read_from_file(string filename, int start_byte, int num_bytes)
{

}
void list_files()
{

}
void shutdown_ssfs()
{
	force_close = true; // make sure this is set
	
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
	should "figure out" what blocks need to be written to basic on what it's being asked to do.
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
	string op1, op2, op3, op4, disk_name;
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
